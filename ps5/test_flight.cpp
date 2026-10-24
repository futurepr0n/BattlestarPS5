#include "game_state.hpp"
#include <cassert>
#include <cstdio>
using namespace battlestar;
int main() {
    Flight a, b, c;
    for (int i=0;i<30;i++) a.update({1,0,false},1.f/30);
    for (int i=0;i<60;i++) b.update({1,0,false},1.f/60);
    for (int i=0;i<120;i++) c.update({1,0,false},1.f/120);
    assert(std::abs(a.x-b.x)<.02f && std::abs(c.x-b.x)<.02f);
    assert(std::abs(a.x-740.f)<.02f);
    Flight diagonal; diagonal.update({1,1,false},.05f);
    assert(std::abs(std::hypot(diagonal.x-320,diagonal.y-540)-21.f)<.01f);
    Flight boost; boost.update({1,0,true},.05f); assert(std::abs(boost.x-361.f)<.01f);
    boost.paused=true; boost.update({1,1,true},.05f); assert(boost.x==361.f);
    Flight edge;
    for(int i=0;i<10000;i++) edge.update({1,1,true},.05f);
    assert(edge.x==board_width-32-ship_width/2 && edge.y==board_height-100-ship_height/2);
    for(int i=0;i<10000;i++) edge.update({-1,-1,true},.05f);
    assert(edge.x==32+ship_width/2 && edge.y==112+ship_height/2);
    edge.reset(); assert(edge.x==320 && edge.y==540 && !edge.paused);
    puts("Flight checks passed: frame-rate independence, diagonal speed, boost, pause, boundaries, reset.");
}
