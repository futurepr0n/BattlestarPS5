// Native PS5 arcade shooter. Original artwork and movement concept: PSPGameCPP.
// Rendering uses the PS5 Vulkan Template and PS5_VKHomebrewUI.
#include "kit.hpp"

class VulkanExample : public ps5ui::KitExample {
    battlestar::Combat game;
    battlestar::GameAudio audio;
    battlestar::ScoreStore scores;
    bool savedRun=false;
    battlestar::Flight &flight=game.ship;
    uint32_t raider=0,boss=0,round=0,enemyRound=0,stars=0;
    hui::gfx::DrawList scene;
    std::array<uint32_t,battlestar::bank_frames> ships{};
    uint32_t logo = 0;
    bool flying = false;
    float exitHold = 0;

    void loadArt() {
        auto load=[&](const char* name){int w,h,n;auto* p=stbi_load((getAssetPath()+"game/"+name).c_str(),&w,&h,&n,4);if(!p)vks::tools::exitFatal(name,-1);auto t=kit.renderer.create_texture(w,h,p);stbi_image_free(p);if(!t)vks::tools::exitFatal(name,-1);return t;};
        stars=load("starfield_one.png");raider=load("raidersm.png");boss=load("battlestar.png");round=load("chain.png");enemyRound=load("enemychain.PNG");
        int w=0,h=0,n=0;
        auto *pixels = stbi_load((getAssetPath()+"game/vipersm.png").c_str(), &w,&h,&n,4);
        if (!pixels) vks::tools::exitFatal("Could not load original Viper sprite",-1);
        // Rotate copies at display-pixel resolution; keep the source PNG untouched.
        constexpr int side=256;
        for(int frame=0;frame<battlestar::bank_frames;frame++) {
            const float angle=(frame-battlestar::max_bank_degrees)*3.14159265f/180.f;
            const float co=std::cos(angle),si=std::sin(angle);
            std::vector<uint8_t> rgba(side*side*4,0);
            for(int y=0;y<side;y++) for(int x=0;x<side;x++) {
                const float dx=(x+.5f-side/2)/4.f,dy=(y+.5f-side/2)/4.f;
                int sx=(int)std::floor(co*dx+si*dy+w*.5f);
                int sy=(int)std::floor(-si*dx+co*dy+h*.5f);
                if(sx>=0 && sy>=0 && sx<w && sy<h)
                    std::memcpy(&rgba[(y*side+x)*4],&pixels[(sy*w+sx)*4],4);
            }
            ships[frame]=kit.renderer.create_texture(side,side,rgba.data());
            if (!ships[frame]) vks::tools::exitFatal("Could not upload ship texture",-1);
        }
        stbi_image_free(pixels);
        pixels=stbi_load((getAssetPath()+"game/bsg_title.png").c_str(),&w,&h,&n,4);
        if (!pixels) vks::tools::exitFatal("Could not load original title artwork",-1);
        logo=kit.renderer.create_texture(w,h,pixels);
        stbi_image_free(pixels);
        if (!logo) vks::tools::exitFatal("Could not upload title artwork",-1);
    }
    void text(const std::string &s,float x,float y,float size,uint32_t color=0xd3dce9) {
        hui::ui::text(scene,kit.fonts.regular,s,x,y,size,hui::gfx::Color::rgb(color));
    }
    void centered(const std::string &s,float y,float size,uint32_t color=0xd3dce9) {
        text(s,(1920.f-kit.fonts.regular.font->measure(s,size))*.5f,y,size,color);
    }
    void drawEnemy(const battlestar::Enemy &e) {
        using hui::gfx::Color;using battlestar::Role;
        float x=game.lerp(e.px,e.x),y=game.lerp(e.py,e.y);
        float w=e.boss?396.f:e.role==Role::bomber?116.f:e.role==Role::scout?76.f:84.f;
        float h=e.boss?136.f:e.role==Role::bomber?56.f:e.role==Role::scout?36.f:40.f;
        uint32_t color=e.boss?0xffffff:e.role==Role::scout?0x85d9ff:e.role==Role::bomber?0xffce80:e.role==Role::escort?0x9cffa9:0xffb4a8;
        scene.image(e.boss?boss:raider,{x-w/2,y-h/2,w,h},hui::gfx::kFullUv,Color::rgb(color));
        if(e.flash>0)scene.glow({x-w/2,y-h/2,w,h},8,14,Color::rgb(0xffffff,e.flash*4));
        if(game.boss_shielded(e))scene.bordered_rect({x-w/2-10,y-h/2-10,w+20,h+20},20,Color::rgb(0x76bfff,.05f),3,Color::rgb(0x76bfff,.7f));
        if(e.muzzle>0)scene.glow({x-e.half_width()-12,y-7,24,14},7,12,Color::rgb(0xff8170,e.muzzle*6));
        if(e.age>=e.delay && e.fire<=.55f && e.x>flight.x+220 && e.x<1850){
            scene.line(x-e.half_width(),y,e.aimX,e.aimY,2,Color::rgb(0xff8069,.28f));
            scene.ring(e.aimX,e.aimY,17,2,Color::rgb(0xffb37b,.65f));
            if(e.boss&&e.phase>=2)scene.line(x-180,y,e.aimX,e.aimY,6,Color::rgb(0xffa65e,.42f));
        }
    }
    void drawSupplies() {
        using hui::gfx::Color;
        for(const auto &p:game.pickups)if(p.active){
            float x=game.lerp(p.px,p.x),y=p.y+std::sin(p.age*4)*6;
            uint32_t color=p.type==battlestar::Supply::repair?0x70e8a0:p.type==battlestar::Supply::shield?0x70caff:p.type==battlestar::Supply::rapid?0xffce80:0xc5a0ff;
            scene.glow({x-19,y-19,38,38},12,14,Color::rgb(color,.25f));
            scene.rotated_rect({x-18,y-18,36,36},5,.785398f,Color::rgb(color,.9f));
            const char* label=p.type==battlestar::Supply::repair?"H":p.type==battlestar::Supply::shield?"S":p.type==battlestar::Supply::rapid?"R":"W";
            text(label,x-8,y+7,22,0x081321);
        }
    }
    void drawBursts() {
        using hui::gfx::Color;
        for(size_t i=0;i<game.bursts.size();i++){const auto &b=game.bursts[i];if(!b.active)continue;
            float t=1-b.life/b.duration,radius=6+t*(b.small?24:90);
            scene.glow({b.x-radius/2,b.y-radius/2,radius,radius},radius/2,18,Color::rgb(b.small?0xffe9b5:0xffa65c,(1-t)*.85f));
            scene.ring(b.x,b.y,radius,2,Color::rgb(0xffd699,(1-t)*.6f));
            for(int k=0;k<(b.small?3:8);k++){float angle=k*2.39996f+i,dx=std::cos(angle),dy=std::sin(angle),distance=t*(b.small?35:130);
                scene.line(b.x+dx*distance,b.y+dy*distance,b.x+dx*(distance+8),b.y+dy*(distance+8),b.small?2:3,Color::rgb(0xffdf98,1-t));}
        }
    }
    void results() {
        using hui::gfx::Color;
        scene.rounded_rect({0,0,1920,1080},0,Color::rgb(0x02060c,.88f));
        centered(game.won?"MISSION COMPLETE":"VIPER LOST",335,52,game.won?0x70e8a0:0xf47366);
        char line[160];std::snprintf(line,sizeof(line),"SCORE %06d     BEST %06d",game.score,scores.best);centered(line,415,34);
        std::snprintf(line,sizeof(line),"SECTOR %d / 3    WAVE %d / 12    TIME %02d:%02d",game.sector(),game.wave,(int)game.elapsed/60,(int)game.elapsed%60);centered(line,478,25,0x83bed4);
        std::snprintf(line,sizeof(line),"DESTROYED %d    ESCAPED %d    ACCURACY %d%%",game.kills,game.escaped,game.roundsFired?100*game.roundsHit/game.roundsFired:0);centered(line,532,25);
        std::snprintf(line,sizeof(line),"SUPPLIES COLLECTED %d",game.collected);centered(line,580,23,0x83bed4);
        centered("CROSS to fly again / CIRCLE for title",665,27);
        if(scores.failed)centered("Best score kept for this session; saving was unavailable",725,22,0xf0c07d);
    }
public:
    VulkanExample() {
        title="BattleStar - PS5 Combat Prototype"; name="battlestar";
        settings.overlay=false; ps5.ownOverlay=true;
        defaultClearColor={{.005f,.009f,.018f,1.f}};
    }
    void prepare() override {
        VulkanExampleBase::prepare();
        prepareKit({.sound=false,.music=false,.covers=false});
        loadArt(); audio.init(kit.mixer,getAssetPath()+"game/");
        scores.init("/data/battlestar-ps5/highscore.txt","/app0/battlestar-highscore.txt"); flying=ps5.frameBudget>0; prepared=true;
        say("BattleStar ready: %ux%u output, 1920x1080 game board",width,height);
    }
    void render() override {
        if(!prepared) return;
        prepareFrame();
        const auto &pad=ps5_pad();
        const float dt=std::clamp(frameTimer,0.f,.05f);
        const bool intercepted=(pad.held&PAD_INTERCEPTED)!=0;
        if(!intercepted && !ps5.frameBudget) {
            if(pad.pressed&PAD_CROSS) { if(!flying || game.over) { flying=true; game.reset(); savedRun=false; } }
            if(pad.pressed&PAD_CIRCLE) { flying=false; game.reset(); savedRun=false; }
            if((pad.pressed&PAD_OPTIONS) && flying) flight.paused=!flight.paused;
            if((pad.pressed&PAD_TRIANGLE) && game.over) { game.reset(); savedRun=false; }
            exitHold=(pad.held&PAD_OPTIONS)?exitHold+dt:0;
            if(exitHold>=1.25f) quit=true;
        }
        if(intercepted) exitHold=0;
        battlestar::Input input;
        if(!intercepted) {
            input.x=pad.left_x; input.y=pad.left_y;
            if(pad.held&(PAD_LEFT|PAD_RIGHT)) input.x=((pad.held&PAD_RIGHT)?1.f:0.f)-((pad.held&PAD_LEFT)?1.f:0.f);
            if(pad.held&(PAD_UP|PAD_DOWN)) input.y=((pad.held&PAD_DOWN)?1.f:0.f)-((pad.held&PAD_UP)?1.f:0.f);
            input.boost=pad.r2>.2f;
        }
        game.eventCount=0;
        if(flying && !intercepted) game.update(input,(pad.held&PAD_CROSS)||(pad.held&PAD_R1),frameTimer);
        audio.pump(flying,intercepted||flight.paused||game.over);
        // Bound feedback per rendered frame; simulation events never block audio.
        int impacts=0,explosions=0,enemyShots=0;
        for(int i=0;i<game.eventCount;i++){const auto &e=game.events[i];using battlestar::EventKind;
            switch(e.kind){
                case EventKind::shot:audio.fire(e.x);break;
                case EventKind::enemy_shot:if(enemyShots++<2)audio.enemy_fire(e.x);break;
                case EventKind::impact:if(impacts++<3)audio.impact(false,e.x);break;
                case EventKind::explosion:if(explosions++<3)audio.impact(true,e.x);break;
                case EventKind::hurt:audio.impact(true,e.x);break;
                case EventKind::pickup:audio.reward();break;
                case EventKind::victory:audio.reward(true);break;
                default:break;
            }
        }
        if(game.over&&!savedRun){scores.record(game.score);savedRun=true;}
        scene.clear();
        using hui::gfx::Color;
        scene.image(stars,{0,0,1920,1080},hui::gfx::kFullUv,Color::rgb(0xa9c9e8,.15f));
        // Deterministic stars fill the expanded board, independent of resolution.
        for(int i=0;i<180;i++) {
            float depth=.25f+(i%5)*.18f;
            float x=std::fmod((float)((i*811+137)%1920)-flight.time*(18+depth*35)+192000.f,1920.f);
            float y=(float)((i*373+71)%1080);
            scene.circle(x,y,i%7==0?1.6f:.8f,Color::rgb(0xa9c9e8,.25f+depth*.5f));
        }
        if(!flying) {
            scene.image(logo,{480,150,960,544},hui::gfx::kFullUv,Color::rgb(0xffffff));
            centered("ARCADE ASSAULT",720,30,0x83bed4);
            centered("Press CROSS to launch",800,28);
            char best[80];std::snprintf(best,sizeof(best),"BEST SCORE %06d",scores.best);centered(best,866,25,0x70e8a0);
            centered("3 SECTORS / 12 WAVES / SHIELDED CAPITAL SHIPS",920,23,0x8293ab);
            centered("Collect H: hull   S: shield   R: rapid fire   W: spread fire",980,23,0x8293ab);
        } else {
            float shake=std::min(1.f,game.shake/.28f)*5;
            scene.push_transform(1,0,0,std::sin(game.elapsed*73)*shake,std::cos(game.elapsed*91)*shake);
            for(const auto &e:game.enemies)if(e.active)drawEnemy(e);
            drawSupplies();
            for(const auto &b:game.bullets)if(b.active){float x=game.lerp(b.px,b.x),y=game.lerp(b.py,b.y);
                if(!b.hostile){float dx=b.vx/1400.f,dy=b.vy/1400.f;scene.line(x-dx*12,y-dy*12,x+dx*4,y+dy*4,3,Color::rgb(0xffdf98,.85f));}
                if(b.heavy)scene.glow({x-10,y-10,20,20},10,12,Color::rgb(0xff8d61,.55f));
                float w=b.heavy?24.f:16.f,h=b.heavy?18.f:10.f;
                scene.image(b.hostile?enemyRound:round,{x-w/2,y-h/2,w,h},hui::gfx::kFullUv,Color::rgb(b.heavy?0xffac70:0xffffff));}
            drawBursts();
            float visualBank=game.lerp(game.oldBank,flight.bank);
            int bank=std::clamp((int)std::lround(visualBank+battlestar::max_bank_degrees),0,battlestar::bank_frames-1);
            float sx=game.lerp(game.oldX,flight.x),sy=game.lerp(game.oldY,flight.y);
            auto pose=battlestar::ship_pose(sx,sy,visualBank);
            if(input.boost && !flight.paused && !game.over) {
                auto exhaust=pose.point(-54);scene.glow({exhaust.x-15,exhaust.y-6,30,12},6,18,Color::rgb(0x65ceff,.7f));
            }
            if(game.invincible<=0 || ((int)(game.elapsed*12)%2==0)) scene.image(ships[bank],{sx-64,sy-64,128,128},hui::gfx::kFullUv,Color::rgb(0xffffff));
            if(game.shield>0)scene.ring(sx,sy,58,3,Color::rgb(0x70caff,.65f));
            if(game.muzzle>0)for(float side:{-3.f,3.f}){auto at=pose.point(48,side);scene.glow({at.x-7,at.y-7,14,14},7,13,Color::rgb(0xffe6a0,game.muzzle*12));}
            scene.pop_transform();
            if(game.hurtFlash>0)scene.rounded_rect({0,110,1920,880},0,Color::rgb(0xe74e4e,game.hurtFlash*.45f));
            scene.rounded_rect({0,0,1920,110},0,Color::rgb(0x08111f,.92f));
            text("BATTLESTAR",48,52,30);
            char status[180];std::snprintf(status,sizeof(status),"SCORE %06d    SECTOR %d / 3    WAVE %02d / 12",game.score,game.sector(),game.wave);text(status,630,52,26,0x83bed4);
            if(game.shield>0)std::snprintf(status,sizeof(status),"HULL %d%%   SHD %d / %ds",game.health,game.shield,(int)std::ceil(game.shieldTime));
            else std::snprintf(status,sizeof(status),"HULL %d%%   SHIELD 0",game.health);
            text(status,48,86,20,0x9ed6c5);
            scene.rounded_rect({330,75,230,8},4,Color::rgb(0x263347));scene.rounded_rect({330,75,230*game.health/100.f,8},4,Color::rgb(game.health>40?0x65cdb3:0xf47366));
            if(game.weaponTime>0)std::snprintf(status,sizeof(status),"%s CANNONS  %ds",game.weapon_name(),(int)std::ceil(game.weaponTime));
            else std::snprintf(status,sizeof(status),"TWIN CANNONS");
            text(status,630,86,20,0xffdc98);
            std::snprintf(status,sizeof(status),"BEST %06d",scores.best);text(status,1510,52,25,0x8293ab);
            for(const auto &e:game.enemies)if(e.active&&e.boss){
                std::snprintf(status,sizeof(status),"CAPITAL SHIP / PHASE %d%s",e.phase,game.boss_shielded(e)?" / DESTROY ESCORTS":"");
                text(status,990,86,18,game.boss_shielded(e)?0x70caff:0xf0c07d);
                scene.rounded_rect({1000,116,600,6},3,Color::rgb(0x263347));scene.rounded_rect({1000,116,600.f*e.hp/e.maxHp,6},3,Color::rgb(0xf47366));
            }
            if(game.banner>0&&!game.over){char label[120];std::snprintf(label,sizeof(label),"SECTOR %d - %s",game.sector(),game.wave_name());centered(label,185,30,0x83bed4);}
            if(game.wave==0&&!game.over)centered("GET READY",245,32,0x83bed4);
            if(game.cleared&&!game.over)centered("WAVE CLEAR / REPAIR +10",245,25,0x70e8a0);
            if(game.noticeTime>0&&!game.over)centered(battlestar::Combat::supply_name(game.lastSupply),935,26,0x70e8a0);
            text("STICK / D-PAD  Move     CROSS / R1  Fire     R2  Boost",48,1028,23,0x8293ab);
            text("OPTIONS  Pause     CIRCLE  Title",1400,1028,22,0x8293ab);
            if(game.over)results();
            if(flight.paused && !game.over) {
                scene.rounded_rect({0,0,1920,1080},0,Color::rgb(0x02060c,.65f));
                centered("PAUSED",515,48);
                centered("OPTIONS to resume",575,25,0x8293ab);
            }
        }
        if(exitHold>.25f) centered("Hold OPTIONS to exit",88,25,0xf0c07d);
        kit.renderer.begin(); kit.renderer.draw(scene);
        buildKitCommandBuffer(); submitFrame();
    }
    ~VulkanExample() override {
        audio.shutdown();
        if(device) { vkDeviceWaitIdle(device); for(auto t:{raider,boss,round,enemyRound,stars})if(t)kit.renderer.destroy_texture(t); for(auto t:ships) if(t) kit.renderer.destroy_texture(t); if(logo) kit.renderer.destroy_texture(logo); }
    }
};
VULKAN_EXAMPLE_MAIN()
