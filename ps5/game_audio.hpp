#pragma once
#include "audio/music.hpp"
#include <fstream>
#include <iterator>
namespace battlestar {
struct GameAudio {
 hui::audio::Mixer *mixer=nullptr;
 hui::audio::MusicTrack tracks[2];hui::audio::StreamRing rings[2];
 std::array<std::vector<float>,3> shots;std::array<hui::audio::Clip,3> clips;
 unsigned nextShot=0;bool started=false;int mode=-1;
 static std::string read(const std::string &path){std::ifstream f(path,std::ios::binary);return {std::istreambuf_iterator<char>(f),std::istreambuf_iterator<char>()};}
 void init(hui::audio::Mixer &m,const std::string &path){mixer=&m;
  for(int i=0;i<2;i++){auto error=tracks[i].open(read(path+(i?"combat.ogg":"menu.ogg")));if(!error.empty())throw std::runtime_error(error);m.attach_stream(i,&rings[i]);}
  const char* names[]={"fire.f32","fire_alt1.f32","fire_alt2.f32"};
  for(size_t i=0;i<shots.size();i++){
   auto bytes=read(path+names[i]);if(bytes.empty()||bytes.size()%8)throw std::runtime_error("Invalid firing audio");
   shots[i].resize(bytes.size()/4);std::memcpy(shots[i].data(),bytes.data(),bytes.size());clips[i]={shots[i].data(),shots[i].size()/2};
  }
  m.set_bus_gain(hui::audio::Bus::music,.23f);pump(false,false);
  started=audio_start([](int16_t *out,int frames,void *user){static_cast<hui::audio::Mixer*>(user)->render(out,frames);},&m);
 }
 void pump(bool combat,bool duck){if(!mixer)return;
  float buffer[4096];for(int i=0;i<2;i++)while(rings[i].available()<33600){int n=tracks[i].decode(buffer,2048);if(n<=0)break;rings[i].write(buffer,n);}
  int next=(combat?1:0)+(duck?2:0);if(next!=mode){mode=next;mixer->set_stream_gain(0,combat?0.f:(duck?.3f:1.f),.35f);mixer->set_stream_gain(1,combat?(duck?.3f:1.f):0.f,.35f);}
 }
 // A clip already contains both barrels; spread upgrades must not multiply voices.
 void fire(float x){if(started){const auto &clip=clips[nextShot++%clips.size()];mixer->play_clip(&clip,{.gain=.42f,.pitch=1.f,.pan=std::clamp((x/960-1)*.45f,-.45f,.45f)});}}
 void impact(bool explosion,float x){if(started)mixer->play_tone({hui::audio::Wave::soft_square,explosion?120.f:260.f,40.f,explosion?.3f:.13f,.004f,.1f},{.gain=.22f,.pitch=1.f,.pan=x/960-1});}
 void enemy_fire(float x){if(started)mixer->play_tone({hui::audio::Wave::triangle,520,180,.12f,.004f,.08f},{.gain=.065f,.pitch=1.f,.pan=std::clamp(x/960-1,-1.f,1.f)});}
 void reward(bool victory=false){if(started)mixer->play_tone({hui::audio::Wave::sine,victory?440.f:660.f,victory?880.f:1100.f,victory?.65f:.2f,.01f,.12f},{.gain=.2f});}
 void shutdown(){if(started){audio_stop();started=false;}}
 ~GameAudio(){shutdown();}
};
}
