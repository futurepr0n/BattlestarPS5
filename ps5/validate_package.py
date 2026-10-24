"""Check the folder-title files ShadowMountPlus needs before deployment."""
from pathlib import Path
import argparse, datetime, hashlib, json, re, struct, zlib

GAME_ASSETS = ['vipersm.png','bsg_title.png','raidersm.png','battlestar.png','chain.png','enemychain.PNG','menu.ogg','combat.ogg','fire.f32','fire_alt1.f32','fire_alt2.f32']

def validate(root, title_id='PPSA99304', game_assets=GAME_ASSETS):
    root = Path(root)
    for name in ['eboot.bin', 'sce_sys/param.json', 'sce_sys/icon0.png', 'sce_module/libc.prx']:
        if not (root/name).is_file() or not (root/name).stat().st_size:
            raise ValueError(f'Missing required title file: {name}')
    param = json.loads((root/'sce_sys/param.json').read_text())
    if param['titleId'] != title_id or root.name != title_id:
        raise ValueError('Title ID must match the package folder and expected title')
    if not re.fullmatch(r'[A-Z]{2}\d{4}-'+re.escape(title_id)+r'_00-[A-Z0-9]{16}',param['contentId']):
        raise ValueError('Invalid content ID')
    languages=param['localizedParameters']
    if not languages[languages['defaultLanguage']]['titleName'].strip():
        raise ValueError('Missing default localized title name')
    with (root/'eboot.bin').open('rb') as f:
        if f.read(4) != b'\x4f\x15\x3d\x1d':
            raise ValueError('eboot.bin must be the packaged PS5 SELF, not an ELF renamed to .bin')
    png = (root/'sce_sys/icon0.png').read_bytes()
    if png[:8] != b'\x89PNG\r\n\x1a\n':
        raise ValueError('Launcher icon is not a PNG')
    offset = 8
    dimensions = None
    ended = False
    image_data = False
    while offset < len(png):
        if offset+12 > len(png):
            raise ValueError('Truncated launcher PNG chunk')
        size = struct.unpack_from('>I',png,offset)[0]
        if offset+size+12 > len(png):
            raise ValueError('Truncated launcher PNG data')
        kind = png[offset+4:offset+8]
        data = png[offset+8:offset+8+size]
        crc = struct.unpack_from('>I',png,offset+8+size)[0]
        if zlib.crc32(kind+data) & 0xffffffff != crc:
            raise ValueError('Launcher icon has a corrupt PNG chunk')
        if kind == b'IHDR':
            if offset != 8 or size != 13 or dimensions is not None:
                raise ValueError('Invalid launcher PNG header')
            w,h,depth,color,compression,filtering,interlace=struct.unpack('>IIBBBBB',data)
            dimensions=(w,h)
            if depth != 8 or color not in (2,6) or interlace != 0 or compression or filtering:
                raise ValueError('Launcher icon must be non-interlaced 8-bit RGB/RGBA')
        if kind == b'IDAT': image_data = True
        offset += size+12
        if kind == b'IEND':
            ended = size == 0
            break
    if not ended or not image_data or offset != len(png):
        raise ValueError('Incomplete launcher PNG or trailing bytes')
    if dimensions != (512,512):
        raise ValueError('Launcher icon must be 512 x 512')
    for name in game_assets:
        if not (root/'assets/game'/name).is_file() or not (root/'assets/game'/name).stat().st_size:
            raise ValueError(f'Missing gameplay asset: {name}')
    print('Folder-title validation passed: SELF signature, metadata, complete 512x512 icon, runtime, assets')

def write_report(root):
    root=Path(root)
    files={p.relative_to(root).as_posix():hashlib.sha256(p.read_bytes()).hexdigest()
           for p in sorted(root.rglob('*')) if p.is_file() and p.name not in ('validation.json','build-validation.json')}
    report={'status':'passed','console_tested':False,'verified_utc':datetime.datetime.now(datetime.timezone.utc).isoformat(),
            'title_id':root.name,'eboot_sha256':files['eboot.bin'],'checks':['folder/title/content ID agreement','packaged SELF magic (full integrity checked by native linker tool)','512x512 RGB/RGBA icon, PNG chunk CRCs and end marker','runtime library and required assets'],
            'file_sha256':files}
    (root/'validation.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')

if __name__ == '__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('root',type=Path)
    parser.add_argument('--title-id',default='PPSA99304')
    parser.add_argument('--generic',action='store_true',help='Skip BattleStar-specific gameplay asset checks')
    parser.add_argument('--write-report',action='store_true')
    args=parser.parse_args()
    validate(args.root,args.title_id,() if args.generic else GAME_ASSETS)
    if args.write_report: write_report(args.root)
