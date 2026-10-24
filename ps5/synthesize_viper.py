"""Original Viper-inspired twin-cannon sound design; no sampled TV/game audio.

48 kHz stereo, short energy sweep + cannon body + electrical/mechanical attack.
One clip contains the twin-barrel micro-stagger; one voice is posted per volley.
Pure standard-library DSP keeps builds reproducible on Windows and Ubuntu WSL.
"""
from pathlib import Path
from array import array
import argparse, hashlib, json, math, random, sys, wave

RATE=48000
GAIN=.42
FILES=('fire.f32','fire_alt1.f32','fire_alt2.f32')

def barrel(variant):
    rng=random.Random(0xB57100+variant)
    samples=[];phase=0.;thump_phase=0.;low_noise=0.;previous_noise=0.
    detune=(1.,.978,1.023)[variant]
    for i in range(round(RATE*.205)):
        t=i/RATE
        noise=rng.uniform(-1,1)
        low_noise+=.24*(noise-low_noise)
        high_noise=noise-previous_noise;previous_noise=noise
        phase+=math.tau*(260+1550*math.exp(-t/.022))*detune/RATE
        thump_phase+=math.tau*(100+110*math.exp(-t/.013))/RATE
        # Fast, rich down-sweep rather than a pure sine-wave 'pew'.
        energy=(math.sin(phase+1.5*math.exp(-t/.025)*math.sin(phase*.51))
                +.18*math.sin(phase*2.01))*math.exp(-t/.034)*.58
        body=math.sin(thump_phase)*math.exp(-t/.026)*.3
        crack=high_noise*math.exp(-t/.004)*.095+low_noise*math.exp(-t/.017)*.24
        metallic=math.sin(math.tau*2850*detune*t)*math.exp(-t/.008)*.075
        attack=1-math.exp(-t/.00055)
        release=min(1.,max(0.,(.205-t)/.014))
        release=release*release*(3-2*release)
        samples.append((energy+body+crack+metallic)*attack*release)
    return samples

def volley(variant):
    first=barrel(variant);second=barrel((variant+1)%3)
    stagger=round(RATE*.007)
    frames=len(first)+stagger
    left=[0.]*frames;right=[0.]*frames
    for i,sample in enumerate(first):
        left[i]+=sample*.57;right[i]+=sample*.43
    for i,sample in enumerate(second):
        left[i+stagger]+=sample*.43;right[i+stagger]+=sample*.57
    # A tiny filtered reflection adds weight without a long reverberant loop.
    delay=round(RATE*.016)
    for channel in (left,right):
        original=channel[:]
        for i in range(delay,frames):channel[i]+=(original[i-delay]+original[max(0,i-delay-1)])*.045
        # DC-block, then fade both boundaries to avoid discontinuities.
        old_x=old_y=0.
        for i,x in enumerate(channel):
            y=x-old_x+.995*old_y;old_x=x;old_y=y
            fade=min(1.,i/48,(frames-1-i)/576)
            channel[i]=y*max(0.,fade)
    peak=max(abs(x) for c in (left,right) for x in c)
    scale=.82/peak
    return [value*scale for pair in zip(left,right) for value in pair]

def encode(samples):
    data=array('f',samples)
    if sys.byteorder!='little':data.byteswap()
    return data.tobytes()

def write_wav(path,samples):
    pcm=array('h',(round(max(-1,min(1,x))*32767) for x in samples))
    if sys.byteorder!='little':pcm.byteswap()
    with wave.open(str(path),'wb') as wav:
        wav.setnchannels(2);wav.setsampwidth(2);wav.setframerate(RATE);wav.writeframes(pcm.tobytes())

def firing_preview(clips):
    out=[0.]*(RATE*6*2)
    # One isolated volley, normal fire, a short gap, then rapid-fire upgrade.
    times=[.15]+[.9+i*.16 for i in range(10)]+[3.2+i*.095 for i in range(23)]
    for i,t in enumerate(times):
        start=round(t*RATE)*2
        for j,x in enumerate(clips[i%3]):out[start+j]+=x*GAIN
    return out

def generate(directory):
    directory=Path(directory);directory.mkdir(parents=True,exist_ok=True)
    clips=[volley(i) for i in range(3)]
    hashes={}
    for name,samples in zip(FILES,clips):
        data=encode(samples);(directory/name).write_bytes(data);hashes[name]=hashlib.sha256(data).hexdigest()
    write_wav(directory/'viper-cannon-single.wav',clips[0])
    write_wav(directory/'viper-cannons-preview.wav',firing_preview(clips))
    meta={'design':'Original Viper-inspired energy/cannon hybrid; no sampled show/game material',
          'sample_rate':RATE,'channels':2,'frames':len(clips[0])//2,'duration_seconds':len(clips[0])/2/RATE,
          'format':'little-endian interleaved float32 stereo','mixer_gain':GAIN,'sha256':hashes}
    (directory/'sound-design.json').write_text(json.dumps(meta,indent=2)+'\n')
    return meta

if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('output',type=Path)
    print(json.dumps(generate(parser.parse_args().output),indent=2))
