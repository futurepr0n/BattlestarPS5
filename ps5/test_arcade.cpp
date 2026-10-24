#include "combat.hpp"
#include "score_store.hpp"
#include <cassert>
#include <iostream>
#include <cstdlib>
#include <unistd.h>
using namespace battlestar;
int main(){
 // Every authored encounter fits the pool and introduces the intended roles.
 Combat campaignTest;
 for(int wave=1;wave<=12;wave++){
  for(auto &e:campaignTest.enemies)e.active=false;
  campaignTest.spawn();const auto &def=campaign[wave-1];
  std::array<int,5> counts{};for(const auto &e:campaignTest.enemies)if(e.active)counts[(int)e.role]++;
  assert(counts[0]==def.scouts&&counts[1]==def.fighters&&counts[2]==def.bombers&&counts[3]==def.escorts&&counts[4]==(def.boss?1:0));
  assert(campaignTest.wave==wave&&campaignTest.sector()==(wave+3)/4);
 }
 for(auto &e:campaignTest.enemies)e.active=false;
 campaignTest.update({},false,1.f/120);
 assert(campaignTest.won&&campaignTest.over);
 int finalScore=campaignTest.score;campaignTest.update({},true,10);assert(campaignTest.score==finalScore&&campaignTest.eventCount==0);
 Combat clear;clear.spawn();for(auto &e:clear.enemies)e.active=false;clear.health=60;
 clear.update({},false,1.f/120);assert(clear.cleared&&clear.health==70&&clear.score==250);
 for(int i=0;i<60;i++)clear.update({},false,1.f/120);
 assert(clear.health==70&&clear.score==250);
 for(int i=0;i<360;i++)clear.update({},false,1.f/120);
 assert(clear.wave==2);
 // Scouts cross faster; fighters retain an attack target instead of tracking forever.
 Combat movement;movement.gap=999;auto &scout=movement.enemies[0];scout.active=true;scout.role=Role::scout;scout.x=scout.px=1600;scout.home=scout.y=scout.py=300;
 auto &bomber=movement.enemies[1];bomber=scout;bomber.role=Role::bomber;bomber.y=bomber.py=bomber.home=800;bomber.anchor=1600;
 for(int i=0;i<120;i++)movement.update({},false,1.f/120);
 assert(scout.x<1100&&bomber.x>1500);
 auto &fighter=movement.enemies[2];fighter=scout;fighter.role=Role::fighter;fighter.age=3.6f;fighter.x=1500;fighter.y=300;fighter.diving=false;
 movement.ship.y=650;movement.update({},false,1.f/120);assert(fighter.attackY==650);
 movement.ship.y=200;movement.update({},false,1.f/120);assert(fighter.attackY==650);
 // Escort shields block boss damage; removing escorts exposes the hull.
 Combat boss;boss.wave=3;boss.spawn();auto &capital=boss.enemies[0];capital.x=capital.px=1530;capital.y=capital.py=540;capital.delay=999;
 int hp=capital.hp;boss.bullet(1350,540,24000,0,false);boss.update({},false,1.f/120);assert(capital.hp==hp);
 for(auto &e:boss.enemies)if(e.role==Role::escort)e.active=false;
 boss.bullet(1350,540,24000,0,false);boss.update({},false,1.f/120);assert(capital.hp==hp-1);
 capital.delay=0;capital.hp=capital.maxHp/2;capital.fire=.56f;boss.update({},false,1.f/60);assert(capital.phase==2);
 float aim=capital.aimY;boss.ship.y=200;
 for(int i=0;i<80;i++)boss.update({},false,1.f/120);
 bool heavy=false;for(auto &b:boss.bullets)if(b.active&&b.hostile&&b.heavy){heavy=true;assert(std::abs((capital.aimX-b.x)*b.vy-(aim-b.y)*b.vx)<.5f);}assert(heavy);
 capital.hp=1;boss.update({},false,1.f/120);assert(capital.phase==3);
 Combat finish;finish.wave=11;finish.spawn();for(auto &e:finish.enemies)if(!e.boss)e.active=false;
 auto &lastBoss=finish.enemies[0];lastBoss.x=lastBoss.px=1530;lastBoss.y=lastBoss.py=540;lastBoss.hp=1;lastBoss.delay=999;
 finish.bullet(1350,540,24000,0,false);finish.update({},false,1.f/120);
 assert(finish.won&&finish.over&&finish.kills==1&&finish.score==4250);
 // Expanded boss hull contacts a swept round before a nearer fighter's centre.
 Combat order;order.gap=999;auto &wide=order.enemies[0];wide.active=wide.boss=true;wide.x=wide.px=700;wide.y=wide.py=540;wide.delay=999;wide.hp=2;
 auto &narrow=order.enemies[1];narrow.active=true;narrow.x=narrow.px=600;narrow.y=narrow.py=540;narrow.delay=999;narrow.hp=1;
 order.bullet(400,540,60000,0,false);order.update({},false,1.f/120);assert(wide.hp==1&&narrow.hp==1);
 // Supplies collect once, shields absorb damage, and temporary weapons expire.
 Combat supply;supply.gap=999;supply.health=40;supply.drop(supply.ship.x,supply.ship.y,Supply::repair);supply.update({},false,1.f/120);
 assert(supply.health==65&&supply.collected==1&&supply.score==50);
 supply.collect(Supply::shield);supply.damage();assert(supply.health==65&&supply.shield==20);
 supply.invincible=0;supply.damage(30);assert(supply.health==55&&supply.shield==0);
 supply.collect(Supply::spread);assert(supply.fire_player());int rounds=0;for(const auto &b:supply.bullets)if(b.active){rounds++;assert(std::abs(std::hypot(b.vx,b.vy)-1400)<.01f);}assert(rounds==6);
 supply.weaponTime=.001f;supply.update({},false,1.f/120);assert(supply.weapon==Weapon::twin);
 Combat rapid,twin;rapid.gap=twin.gap=999;rapid.collect(Supply::rapid);
 for(int i=0;i<120;i++){rapid.update({},true,1.f/120);twin.update({},true,1.f/120);}assert(rapid.shots>twin.shots);
 supply.ship.paused=true;float paused=supply.elapsed;supply.update({},true,1);assert(supply.elapsed==paused&&supply.eventCount==0);
 // Hostile fire leaves capacity for the player's largest volley.
 Combat pool;for(int i=0;i<250;i++)pool.bullet(1700,500,-300,0,true);
 int hostile=0;for(auto &b:pool.bullets)hostile+=b.active&&b.hostile;assert(hostile==160);
 pool.weapon=Weapon::spread;assert(pool.fire_player());
 // Persistence: reload, monotonic best, corrupt data, atomic temp cleanup, fallback.
 char directory[]="/tmp/battlestar-score-XXXXXX";assert(mkdtemp(directory));std::string path=std::string(directory)+"/best.txt";
 ScoreStore store;store.init(path,"");assert(store.best==0&&store.record(1234));
 ScoreStore reload;reload.init(path,"");assert(reload.best==1234&&reload.record(12)&&ScoreStore::read(path)==1234);
 {auto *f=std::fopen(path.c_str(),"wb");assert(f);std::fputs("BATTLESTAR1 999999999999999999999\n",f);std::fclose(f);}assert(ScoreStore::read(path)==0);
 store.init("/proc/battlestar-not-writable/best.txt",path);assert(store.record(2345)&&ScoreStore::read(path)==2345);
 assert(access((path+".tmp").c_str(),F_OK)!=0);
 ScoreStore denied;denied.init("/proc/battlestar-not-writable/best.txt","");assert(!denied.record(987)&&denied.best==987&&denied.failed);
 std::remove(path.c_str());rmdir(directory);
 std::cout<<"Arcade: authored campaign, roles, boss shields/phases/warnings, supplies, upgrades, pool reserve, results and score persistence passed\n";
}
