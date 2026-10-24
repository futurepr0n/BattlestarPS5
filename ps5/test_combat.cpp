#include "combat.hpp"
#include <cassert>
#include <iostream>
using namespace battlestar;
int main(){
 // Both muzzle positions rotate with the ship; the volley midpoint is at its nose.
 for(float angle:{-24.f,0.f,24.f}){
  Combat aim;aim.ship.bank=angle;assert(aim.fire_player());
  const auto &left=aim.bullets[0],&right=aim.bullets[1];
  assert(left.active&&right.active&&!left.hostile&&!right.hostile&&aim.shots==1);
  assert(std::abs(std::hypot(left.vx,left.vy)-1400)<.01f);
  assert(std::abs(std::hypot(left.x-right.x,left.y-right.y)-6)<.001f);
  float dx=(left.x+right.x)*.5f-aim.ship.x,dy=(left.y+right.y)*.5f-aim.ship.y;
  assert(std::abs(std::hypot(dx,dy)-48)<.001f);
  assert(std::abs(dx*left.vy-dy*left.vx)<.1f); // Nose offset and velocity are parallel.
  assert(left.vx>1278&&left.vx<=1400);
  if(angle<0)assert(left.vy< -569&&dy< -19);
  else if(angle>0)assert(left.vy>569&&dy>19);
  else {assert(left.vy==0);assert(std::abs(left.x-368)<.001f);}
  // Fired rounds keep their trajectory after the ship changes direction.
  float vy=left.vy;aim.ship.bank=-angle;aim.gap=999;aim.update({},false,1.f/60);assert(left.vy==vy);
 }
 Combat bank30,bank144;bank30.gap=bank144.gap=999;
 for(int i=0;i<30;i++)bank30.update({0,-1,false},true,1.f/30);
 for(int i=0;i<144;i++)bank144.update({0,-1,false},true,1.f/144);
 assert(bank30.shots==bank144.shots&&std::abs(bank30.ship.bank-bank144.ship.bank)<.001f);
 for(size_t i=0;i<bank30.bullets.size();i++){
  const auto &a=bank30.bullets[i],&b=bank144.bullets[i];assert(a.active==b.active);
  if(a.active){assert(std::abs(a.x-b.x)<.01f&&std::abs(a.y-b.y)<.01f);assert(std::abs(a.vy-b.vy)<.01f);}
 }
 Combat full;for(size_t i=0;i<full.bullets.size()-1;i++)full.bullets[i].active=true;
 assert(!full.fire_player()&&!full.bullets.back().active&&full.shots==0);
 assert(Combat::sweep(-100,0,200,0,5,5));assert(!Combat::sweep(-100,20,200,0,5,5));
 Combat a,b;for(int i=0;i<300;i++)a.update({},true,1.f/30);for(int i=0;i<1440;i++)b.update({},true,1.f/144);
 assert(a.wave==b.wave&&a.score==b.score&&a.health==b.health&&a.shots==b.shots);
 assert(std::abs(a.elapsed-b.elapsed)<.01f);
 Combat c;c.gap=999;c.bullet(320,540,0,0,true);c.update({},false,1.f/60);assert(c.health==80);c.damage();assert(c.health==80);
 c.ship.paused=true;float time=c.elapsed;c.update({},true,1);assert(c.elapsed==time);
 c.reset();c.gap=999;auto &e=c.enemies[0];e.active=true;e.x=e.px=600;e.y=e.py=540;e.home=540;e.hp=1;e.delay=999;
 c.bullet(500,540,24000,0,false);c.update({},false,1.f/120);assert(!e.active&&c.score==100);
 c.reset();c.gap=999;c.bullet(320,540,0,0,false);c.update({},false,1.f/60);assert(c.health==100);
 c.reset();for(int i=0;i<4;i++){for(auto &enemy:c.enemies)enemy.active=false;c.spawn();}assert(c.enemies[0].boss);
 // Long unattended run exercises pool reuse, transitions and bounds under sanitizers.
 c.reset();for(int i=0;i<120*180;i++){if(c.over)c.reset();c.update({0,std::sin(i*.01f),i%100<20},true,1.f/120);for(auto &enemy:c.enemies)assert(std::isfinite(enemy.x)&&std::isfinite(enemy.y));}
 std::cout<<"Combat: banked muzzle/aim, trajectory, timing, swept/team collisions, damage grace, pause, boss and pool endurance passed\n";
}
