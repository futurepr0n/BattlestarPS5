#pragma once
#include "game_state.hpp"
#include <array>
#include <limits>
namespace battlestar {
struct Body { float x=0,y=0,px=0,py=0; bool active=false; };
enum class Role { scout, fighter, bomber, escort, capital };
enum class Formation { line, wedge, pincer };
enum class Weapon { twin, rapid, spread };
enum class Supply { repair, shield, rapid, spread };
enum class EventKind { shot, enemy_shot, impact, explosion, hurt, pickup, wave, victory };
struct Event { EventKind kind; float x,y; };
struct Enemy:Body {
 float age=0,delay=0,fire=1.8f,home=540,anchor=1400,attackY=540,aimX=320,aimY=540,flash=0,muzzle=0;
 int hp=3,maxHp=3,phase=1,slot=0; bool boss=false,diving=false; Role role=Role::fighter;
 float half_width()const{return boss?180.f:role==Role::bomber?58.f:43.f;}
 float half_height()const{return boss?58.f:role==Role::bomber?28.f:22.f;}
};
struct Bullet:Body { float vx=0,vy=0,radius=4; int damage=1; bool hostile=false,heavy=false; };
struct Burst:Body { float life=0,duration=.5f; bool small=false; };
struct Pickup:Body { Supply type=Supply::repair; float life=12,age=0; };
struct WaveDef { const char* name; int scouts,fighters,bombers,escorts; Formation formation; bool boss=false; };
inline constexpr std::array<WaveDef,12> campaign{{
 {"SCOUT SCREEN",5,0,0,0,Formation::line},
 {"RAIDER WEDGE",2,4,0,0,Formation::wedge},
 {"BOMBER INTERCEPT",2,3,2,0,Formation::line},
 {"OUTPOST GUARDIAN",0,0,0,2,Formation::line,true},
 {"CROSSING PATROL",5,3,0,0,Formation::pincer},
 {"HEAVY ESCORT",0,4,3,0,Formation::wedge},
 {"PINCH POINT",4,4,2,0,Formation::pincer},
 {"CARRIER GROUP",0,0,0,3,Formation::line,true},
 {"DEEP STRIKE",6,4,0,0,Formation::wedge},
 {"BOMBER CORRIDOR",2,4,4,0,Formation::line},
 {"LAST DEFENCE",4,5,3,0,Formation::pincer},
 {"COMMAND SHIP",0,0,0,4,Formation::line,true}
}};
struct Combat {
 Flight ship;
 float oldX=320,oldY=540,oldBank=0,accumulator=0,alpha=0,cooldown=0,invincible=0,gap=2,elapsed=0;
 float muzzle=0,hurtFlash=0,shake=0,weaponTime=0,shieldTime=0,noticeTime=0,banner=0;
 int health=100,shield=0,score=0,wave=0,shots=0,hits=0,kills=0,roundsFired=0,roundsHit=0,collected=0,escaped=0;
 bool over=false,won=false,cleared=false; Weapon weapon=Weapon::twin; Supply lastSupply=Supply::repair;
 std::array<Enemy,24> enemies{}; std::array<Bullet,256> bullets{}; std::array<Burst,64> bursts{};
 std::array<Pickup,12> pickups{}; std::array<Event,96> events{}; int eventCount=0;
 void reset(){*this=Combat{};}
 int sector()const{return std::max(1,(wave+3)/4);}
 const char* wave_name()const{return wave>0&&wave<=12?campaign[wave-1].name:"LAUNCH SEQUENCE";}
 const char* weapon_name()const{return weapon==Weapon::rapid?"RAPID":weapon==Weapon::spread?"SPREAD":"TWIN";}
 static const char* supply_name(Supply type){switch(type){case Supply::repair:return "HULL REPAIRED";case Supply::shield:return "SHIELD ONLINE";case Supply::rapid:return "RAPID CANNONS";default:return "SPREAD CANNONS";}}
 void event(EventKind kind,float x,float y){if(eventCount<(int)events.size())events[eventCount++]={kind,x,y};}
 void bullet(float x,float y,float vx,float vy,bool hostile,bool heavy=false){
  if(hostile){int count=0;for(const auto &b:bullets)if(b.active&&b.hostile)count++;if(count>=160)return;}
  for(auto &b:bullets)if(!b.active){b={};b.active=true;b.x=b.px=x;b.y=b.py=y;b.vx=vx;b.vy=vy;b.hostile=hostile;b.heavy=heavy;b.radius=heavy?10.f:4.f;b.damage=hostile?(heavy?30:20):1;break;}
 }
 bool fire_player(){
  const int pairs=weapon==Weapon::spread?3:1;int free=0;for(const auto &b:bullets)if(!b.active)free++;
  if(free<pairs*2)return false;
  const auto pose=ship_pose(ship.x,ship.y,ship.bank);
  for(int p=0;p<pairs;p++){
   const float spread=pairs==1?0.f:(p-1)*9.f;
   const auto direction=ship_pose(0,0,ship.bank+spread);
   for(float side:{-3.f,3.f}){auto at=pose.point(48.f,side);bullet(at.x,at.y,direction.forward_x*1400,direction.forward_y*1400,false);}
  }
  shots++;roundsFired+=pairs*2;muzzle=.065f;event(EventKind::shot,ship.x,ship.y);return true;
 }
 void burst(float x,float y,bool small=false){for(auto &b:bursts)if(!b.active){b={};b.active=true;b.x=x;b.y=y;b.small=small;b.duration=small?.16f:.6f;b.life=b.duration;break;}}
 // Return first relative-motion contact; the smallest fraction is the actual first target.
 static float contact(float x,float y,float dx,float dy,float hx,float hy){
  float lo=0,hi=1;
  for(int axis=0;axis<2;axis++){float p=axis?y:x,d=axis?dy:dx,h=axis?hy:hx;
   if(std::abs(d)<1e-6f){if(std::abs(p)>h)return std::numeric_limits<float>::infinity();}
   else{float a=(-h-p)/d,b=(h-p)/d;if(a>b)std::swap(a,b);lo=std::max(lo,a);hi=std::min(hi,b);if(lo>hi)return std::numeric_limits<float>::infinity();}}
  return lo;
 }
 static bool sweep(float x,float y,float dx,float dy,float hx,float hy){return std::isfinite(contact(x,y,dx,dy,hx,hy));}
 void damage(int amount=20){
  if(invincible>0||over)return;
  const int absorb=std::min(shield,amount);shield-=absorb;amount-=absorb;health=std::max(0,health-amount);
  invincible=1.2f;hits++;hurtFlash=.22f;shake=.28f;event(EventKind::hurt,ship.x,ship.y);burst(ship.x,ship.y,true);
  if(!health)over=true;
 }
 void drop(float x,float y,Supply type){for(auto &p:pickups)if(!p.active){p={};p.active=true;p.type=type;p.x=p.px=std::clamp(x,300.f,1750.f);p.y=p.py=std::clamp(y,170.f,910.f);break;}}
 void collect(Supply type){
  if(type==Supply::repair)health=std::min(100,health+25);
  else if(type==Supply::shield){shield=std::min(60,shield+40);shieldTime=15;}
  else {weapon=type==Supply::rapid?Weapon::rapid:Weapon::spread;weaponTime=14;}
  collected++;lastSupply=type;noticeTime=2;score+=50;event(EventKind::pickup,ship.x,ship.y);
 }
 int escorts_alive()const{int count=0;for(const auto &e:enemies)if(e.active&&e.role==Role::escort)count++;return count;}
 bool boss_shielded(const Enemy &e)const{return e.boss&&escorts_alive()>0;}
 void spawn(){
  if(wave>=12||over)return;
  wave++;cleared=false;banner=2.5f;gap=2.5f;
  for(auto &b:bullets)if(b.hostile)b.active=false;
  const auto &def=campaign[wave-1];int next=0;
  auto add=[&](Role role){auto &e=enemies[next];e={};e.active=true;e.role=role;e.boss=role==Role::capital;e.slot=next;
   e.hp=e.maxHp=e.boss?54+sector()*18:role==Role::bomber?7:role==Role::scout?2:role==Role::escort?4:3;
   e.home=def.formation==Formation::wedge?540.f+((next%2)?-1:1)*(60+(next/2)*75):def.formation==Formation::pincer?((next%2)?840.f:240.f):210.f+(next%6)*128;
   e.home=std::clamp(e.home,180.f,900.f);e.anchor=1450-(next/6)*145.f;e.x=e.px=2040+(next/6)*130.f;e.y=e.py=e.boss?540:e.home;
   e.delay=e.boss?0:next*.22f;e.fire=1.8f+(next%4)*.3f;e.attackY=ship.y;next++;};
  if(def.boss)add(Role::capital);
  for(int i=0;i<def.scouts;i++)add(Role::scout);
  for(int i=0;i<def.fighters;i++)add(Role::fighter);
  for(int i=0;i<def.bombers;i++)add(Role::bomber);
  for(int i=0;i<def.escorts;i++)add(Role::escort);
  event(EventKind::wave,960,540);
 }
 void enemy_fire(Enemy &e){
  float dx=e.aimX-(e.x-e.half_width()),dy=e.aimY-e.y,angle=std::atan2(dy,dx);
  int count=e.boss?(e.phase==1?3:5):e.role==Role::bomber?3:1;
  float speed=e.boss?350+e.phase*25.f:e.role==Role::bomber?260.f:380.f;
  for(int i=0;i<count;i++){float a=angle+(i-(count-1)*.5f)*.14f;bullet(e.x-e.half_width(),e.y,std::cos(a)*speed,std::sin(a)*speed,true,e.role==Role::bomber);}
  if(e.boss&&e.phase>=2)bullet(e.x-180,e.y,std::cos(angle)*650,std::sin(angle)*650,true,true);
  e.fire=e.boss?(e.phase==3?1.15f:1.65f):e.role==Role::bomber?2.8f:2.4f+(e.slot%3)*.25f;
  e.muzzle=.1f;event(EventKind::enemy_shot,e.x,e.y);
 }
 void move_enemy(Enemy &e,float dt){
  e.px=e.x;e.py=e.y;e.age+=dt;e.flash=std::max(0.f,e.flash-dt);e.muzzle=std::max(0.f,e.muzzle-dt);
  if(e.age<e.delay)return;
  const float t=e.age-e.delay;
  if(e.boss){e.phase=e.hp*3>e.maxHp*2?1:e.hp*3>e.maxHp?2:3;e.x+=(1530-e.x)*(1-std::exp(-dt*1.5f));e.y=540+std::sin(t*(e.phase==3?.85f:.55f))*265;}
  else if(e.role==Role::escort){
   const Enemy* leader=nullptr;for(const auto &other:enemies)if(other.active&&other.boss){leader=&other;break;}
   if(leader){e.x+=(leader->x-240+std::cos(t+e.slot*1.7f)*115-e.x)*(1-std::exp(-dt*2.5f));e.y=std::clamp(leader->y+std::sin(t+e.slot*1.7f)*210,170.f,920.f);}
   else {e.x-=400*dt;e.y+=std::sin(t*3)*120*dt;}
  } else if(e.role==Role::scout){e.x-=(520+sector()*30)*dt;e.y=e.home+std::sin(t*2.5f+e.slot)*65;}
  else if(e.role==Role::bomber){if(t<11)e.x+=(e.anchor-e.x)*(1-std::exp(-dt*1.3f));else e.x-=210*dt;e.y=e.home+std::sin(t*.9f+e.slot)*42;}
  else if(t<3.5f){e.x+=(e.anchor-e.x)*(1-std::exp(-dt*1.8f));e.y=e.home+std::sin(t*2+e.slot)*30;}
  else {if(!e.diving){e.diving=true;e.attackY=ship.y;}e.x-=(340+sector()*20)*dt;e.y+=std::clamp((e.attackY-e.y)*1.6f,-260.f,260.f)*dt;}
  const bool canFire=e.x>ship.x+220&&e.x<1850;
  if(canFire){float previous=e.fire;e.fire-=dt;if(previous>.55f&&e.fire<=.55f){e.aimX=ship.x;e.aimY=ship.y;}if(e.fire<=0)enemy_fire(e);}
  else e.fire=std::max(e.fire,.7f);
  if(sweep(e.px-oldX,e.py-oldY,(e.x-e.px)-(ship.x-oldX),(e.y-e.py)-(ship.y-oldY),e.half_width()+32,e.half_height()+15))damage(e.boss?40:20);
  if(e.x< -240){e.active=false;escaped++;}
 }
 void destroy(Enemy &e){
  e.active=false;kills++;int reward=e.boss?1500:e.role==Role::bomber?200:e.role==Role::escort?150:100;
  score+=reward;burst(e.x,e.y);event(EventKind::explosion,e.x,e.y);shake=std::max(shake,e.boss?.3f:.06f);
  if(e.boss){for(int i=-2;i<=2;i++)burst(e.x+i*45,e.y+std::sin(i*2.f)*35);}
  else if(e.role==Role::bomber||kills%4==0){Supply type=static_cast<Supply>((kills/4+wave)%4);if(health<=40)type=Supply::repair;drop(e.x,e.y,type);}
 }
 void step(Input input,bool fire,float dt){
  if(over)return;
  elapsed+=dt;oldX=ship.x;oldY=ship.y;oldBank=ship.bank;ship.update(input,dt);
  auto decay=[&](float &v){v=std::max(0.f,v-dt);};decay(invincible);decay(muzzle);decay(hurtFlash);decay(shake);decay(banner);decay(noticeTime);
  decay(weaponTime);if(weaponTime==0)weapon=Weapon::twin;
  decay(shieldTime);if(shieldTime==0)shield=0;
  cooldown-=dt;if(fire&&cooldown<=0&&fire_player())cooldown=weapon==Weapon::rapid?.095f:.16f;
  for(auto &e:enemies)if(e.active)move_enemy(e,dt);
  for(auto &b:bullets){if(!b.active)continue;b.px=b.x;b.py=b.y;b.x+=b.vx*dt;b.y+=b.vy*dt;
   if(b.hostile){if(sweep(b.px-oldX,b.py-oldY,(b.x-b.px)-(ship.x-oldX),(b.y-b.py)-(ship.y-oldY),32+b.radius,14+b.radius)){b.active=false;damage(b.damage);}}
   else {Enemy* target=nullptr;float nearest=std::numeric_limits<float>::infinity();
    for(auto &e:enemies)if(e.active){float contactTime=contact(b.px-e.px,b.py-e.py,(b.x-b.px)-(e.x-e.px),(b.y-b.py)-(e.y-e.py),e.half_width()+b.radius,e.half_height()+b.radius);
     if(contactTime<nearest){target=&e;nearest=contactTime;}}
    if(target){b.active=false;roundsHit++;target->flash=.11f;burst(b.px+(b.x-b.px)*nearest,b.py+(b.y-b.py)*nearest,true);event(EventKind::impact,target->x,target->y);
     if(!boss_shielded(*target)&&--target->hp<=0)destroy(*target);}}
   if(b.x< -40||b.x>1980||b.y<100||b.y>990)b.active=false;
  }
  for(auto &b:bursts)if(b.active){b.life-=dt;if(b.life<=0)b.active=false;}
  for(auto &p:pickups)if(p.active){p.px=p.x;p.py=p.y;p.age+=dt;p.life-=dt;p.x-=105*dt;
   if(!over&&sweep(p.px-oldX,p.py-oldY,(p.x-p.px)-(ship.x-oldX),(p.y-p.py)-(ship.y-oldY),66,44)){collect(p.type);p.active=false;}
   if(p.life<=0||p.x< -40)p.active=false;
  }
  bool any=false;for(const auto &e:enemies)any|=e.active;
  if(!any&&!over){
   if(wave>0&&!cleared){cleared=true;gap=3;score+=250;health=std::min(100,health+10);
    if(wave==12){score+=2500;won=over=true;event(EventKind::victory,ship.x,ship.y);}
    else drop(1100,540,static_cast<Supply>((wave-1)%4));
   }
   if(!over){gap-=dt;if(gap<=0)spawn();}
  }
 }
 void update(Input input,bool fire,float dt){
  eventCount=0;
  if(ship.paused||over){accumulator=0;return;}
  if(!std::isfinite(dt)||dt<=0)return;
  accumulator+=std::min(dt,.25f);constexpr float tick=1.f/120;
  while(accumulator+1e-7f>=tick){step(input,fire,tick);accumulator-=tick;}
  alpha=std::clamp(accumulator/tick,0.f,1.f);
 }
 float lerp(float a,float b)const{return a+(b-a)*alpha;}
};
}
