"""Stage the port against the already validated local PS5 platform sources."""
from pathlib import Path
import json, shutil, subprocess, sys, hashlib

repo = Path(sys.argv[1]).resolve()
sys.path.insert(0,str(repo/'ps5'))
from synthesize_viper import generate as generate_weapon_audio, FILES as WEAPON_AUDIO_FILES
work = Path.home() / 'work'
template = work / 'ps5-rpcs3-app'
app = work / 'battlestar-ps5-app'
if not (template / '.deps/native/ps5-payload-sdk/bin/prospero-lld').exists():
    raise SystemExit('The prepared KB-RPCS3 PS5 toolchain is required.')
app.mkdir(exist_ok=True)
for source,target in [('README.md','GAME_README.md'),('REFERENCE-COMPARISON.md','REFERENCE-COMPARISON.md')]:
    shutil.copy2(repo/'ps5'/source,app/target)
for name in ['base', 'external', 'ps5', 'shaders']:
    shutil.copytree(template / name, app / name, dirs_exist_ok=True,
                    ignore=shutil.ignore_patterns('icon0.png'))
for name in ['LICENSE.md', 'LICENSING.md']:
    shutil.copy2(template / name, app / name)
deps = app / '.deps'
if not deps.exists(): deps.symlink_to(template / '.deps', target_is_directory=True)

cmake = app / 'ps5/CMakeLists.txt'
code = cmake.read_text()
for source in ['probe_rpcs3.cpp', 'ps5_dl.c', 'ps5_fastclock.c', 'ps5_tuning.c', 'import_check.c']:
    code = code.replace('\tsrc/' + source + '\n', '')
cmake.write_text(code)
prelude = app / 'ps5/src/prelude.h'
code = '\n'.join(line for line in prelude.read_text().splitlines() if '../../examples/rpcs3ps5/' not in line)
prelude.write_text(code + '\n#include "combat.hpp"\n#include "game_audio.hpp"\n#include "score_store.hpp"\n')
samples = app / 'ps5/src/samples.cpp'
samples.write_text(samples.read_text().replace('rpcs3ps5', 'battlestar').replace('KB-RPCS3 Alpha 0.1.0','Native PS5 flight prototype').replace('KB-RPCS3', 'BattleStar'))
for src,dst in [('main.cpp','ps5/src/main.cpp'),('combat.hpp','ps5/src/combat.hpp'),('game_audio.hpp','ps5/src/game_audio.hpp'),('score_store.hpp','ps5/src/score_store.hpp'),('game_state.hpp','ps5/src/game_state.hpp'),
                ('battlestar.cpp','examples/battlestar/battlestar.cpp'),('link-title.sh','ps5/tools/link-title.sh')]:
    target=app/dst; target.parent.mkdir(parents=True,exist_ok=True)
    target.write_bytes((repo/'ps5'/src).read_bytes().replace(b'\r\n',b'\n'))
for p in (app/'ps5').rglob('*.sh'):
    p.write_bytes(p.read_bytes().replace(b'\r\n',b'\n'))

param_path=app/'ps5/sce_sys/param.json'
param=json.loads(param_path.read_text())
param.update(titleId='PPSA99304', conceptId='99304', contentId='UP9000-PPSA99304_00-BATTLESTARPS5000',contentVersion='01.003.000')
param['localizedParameters']['en-US']['titleName']='BattleStar - Combat Prototype'
param_path.write_text(json.dumps(param,indent=2)+'\n')
icon = repo/'ps5/sce_sys/icon0.png'
if not icon.is_file():
    raise SystemExit('Missing launcher icon. Run ps5/Build-Icon.ps1 first.')
shutil.copy2(icon,app/'ps5/sce_sys/icon0.png')
manifest_path=app/'ps5/assets.json'
manifest=json.loads(manifest_path.read_text())
manifest['assets']=[a for a in manifest['assets'] if '/audio/' not in a['path']]
for name in ['vipersm.png','bsg_title.png']:
    target=app/'game_assets'/name; target.parent.mkdir(exist_ok=True)
    shutil.copy2(repo/name,target)
    manifest['assets'].append(dict(path='game/'+name, title='Original PSPGameCPP artwork',author='PSPGameCPP project',
        licence='See original project',url='https://github.com/futurepr0n/PSPGameCPP',
        used_by=['battlestar'], **{'from':'file'}, source='game_assets/'+name))
# Keep original reference files unchanged; transcode only audio for the native mixer.
assetdir=repo/'ps5/assets'
converted=app/'converted_audio';converted.mkdir(exist_ok=True)
recipes=[('theme_battlestar_galactica.mp3','menu.ogg',['-c:a','libvorbis','-q:a','4']),
         ('mrdeath_-_12th_moon_rising.xm','combat.ogg',['-c:a','libvorbis','-q:a','4'])]
for src,dst,args in recipes:
    output=converted/dst
    if not output.exists() or output.stat().st_mtime < (assetdir/src).stat().st_mtime:
        subprocess.run(['ffmpeg','-v','error','-y','-i',str(assetdir/src),'-ar','48000','-ac','2',*args,str(output)],check=True)
for source in list(assetdir.glob('*.png'))+list(assetdir.glob('*.PNG'))+[converted/dst for _,dst,_ in recipes]:
    target=app/'game_assets'/source.name;shutil.copy2(source,target)
    manifest['assets'].append(dict(path='game/'+source.name,title='BattleStar-OSLib reference asset',author='BattleStar-OSLib project / original audio authors',licence='See upstream project and THIRD_PARTY_NOTICES.txt',url='https://github.com/futurepr0n/BattleStar-OSLib',used_by=['battlestar'],**{'from':'file'},source='game_assets/'+source.name))
generated_audio=app/'generated_sfx'
weapon_audio=generate_weapon_audio(generated_audio)
preview=repo/'ps5/audio-preview';preview.mkdir(exist_ok=True)
for source in generated_audio.iterdir():shutil.copy2(source,preview/source.name)
for name in WEAPON_AUDIO_FILES:
    shutil.copy2(generated_audio/name,app/'game_assets'/name)
    manifest['assets'].append(dict(path='game/'+name,title='Original Viper-inspired twin cannon',author='Local BattleStar PS5 port',licence='Original procedural sound; see THIRD_PARTY_NOTICES.txt',url='https://github.com/futurepr0n/PSPGameCPP',used_by=['battlestar'],**{'from':'file'},source='game_assets/'+name))
manifest_path.write_text(json.dumps(manifest,indent=2)+'\n')
shutil.copy2(repo/'ps5/port_notices.py',app/'ps5/tools/port-notices.py')
provenance={'original_project':subprocess.check_output(['git','-C',str(repo),'rev-parse','HEAD']).decode().strip(),
            'port_revision':'viper-cannons-2026-10-23','weapon_audio':weapon_audio,
            'port_source_sha256':{p.relative_to(repo).as_posix():hashlib.sha256(p.read_bytes()).hexdigest()
                for p in sorted([*(repo/'ps5').rglob('*'),repo/'Build-PS5.ps1',repo/'.gitattributes']) if p.is_file() and (p.suffix in ('.cpp','.hpp','.py','.sh','.ps1','.md') or p==icon or p.name=='.gitattributes')},
            'platform_source':subprocess.check_output(['git','-C',str(template),'rev-parse','HEAD']).decode().strip(),
            'vulkan_source':subprocess.check_output(['git','-C',str(work/'PS5_Vulkan'),'rev-parse','HEAD']).decode().strip(),
            'reference_project':{'url':'https://github.com/futurepr0n/BattleStar-OSLib','commit':'d627d007e4d521b9b89cf38a13051c4bf9851ba9','asset_sha256':{p.name:hashlib.sha256(p.read_bytes()).hexdigest() for p in assetdir.iterdir()}},'title_id':'PPSA99304','logical_board':[1920,1080],'render_output':[3840,2160]}
(app/'build-provenance.json').write_text(json.dumps(provenance,indent=2)+'\n')
print(app)
