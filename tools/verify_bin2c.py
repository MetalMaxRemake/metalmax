#!/usr/bin/env python3
"""Check module-local headers and restored maps against actual C++ output."""
from pathlib import Path
import hashlib,json,re,struct,subprocess,zipfile
from import_reverse import nums,PROJECT,CPP
BUILD=PROJECT/'tools/build';HOST=PROJECT.parent/'metalmax-analysis/host-tests'
manifest=json.loads((BUILD/'manifest.json').read_text())
checked=0
for name,meta in manifest['resources'].items():
    assert not name.startswith('maps/'),name
    data=(BUILD/meta['binary']).read_bytes();header=(CPP/name).read_text()
    body=re.search(r'=\s*\{(.*?)\};',header,re.S)[1]
    reconstructed=bytes(int(v,0) for v in re.findall(r'0x[0-9a-fA-F]+|\d+',body))
    assert reconstructed==data and hashlib.sha256(data).hexdigest()==meta['sha256'],name
    checked+=1
# maps/ data files and original headers really returned to their original paths.
# map.cpp alone keeps bounded viewport code, reading raw short_map_data/bitmaps.
maps_checked=0
frozen_maps={'tile_bmp.cpp','tile_bmp.h','map_data.cpp','map_data.h'}
for path in (CPP/'maps').rglob('*'):
    if not path.is_file():continue
    rel=path.relative_to(CPP/'maps')
    if not (str(rel).startswith('map_data/') or path.name in frozen_maps):continue
    original=subprocess.run(['git','show','HEAD:'+str(path.relative_to(PROJECT))],cwd=PROJECT,capture_output=True)
    if original.returncode==0:
        assert path.read_bytes()==original.stdout,str(path)
        maps_checked+=1
for name in ['water_block_id','wave','water']:
    current=nums(CPP/'maps/active_tile_bmp.cpp',name)
    original=subprocess.run(['git','show','HEAD:app/src/main/cpp/maps/active_tile_bmp.cpp'],cwd=PROJECT,capture_output=True,check=True).stdout.decode()
    body=re.search(r'\b'+name+r'\s*(?:\[[^\]]*\]\s*)*=\s*\{(.*?)\};',re.sub(r'//[^\n]*|/\*.*?\*/','',original,flags=re.S),re.S)[1]
    expected=[int(v,0) for v in re.findall(r'-?0x[\da-fA-F]+|-?\d+',body)]
    assert current==expected,'active_tile_bmp:'+name
    maps_checked+=1
for name,digest in manifest['maps_unchanged'].items():
    assert hashlib.sha256((CPP/name).read_bytes()).hexdigest()==digest,name
# CPU resource output contains all existing map tiles followed by monster/title/
# water images. Only non-map pictures use a packed per-image header.
data=(HOST/'decoded-images.bin').read_bytes();position=0;actual={}
for image_id in range(2041):
    w,h=struct.unpack_from('<HH',data,position);position+=4
    pixels=data[position:position+w*h];position+=w*h
    actual[image_id]=(w,h,hashlib.sha256(pixels).hexdigest())
assert position==len(data)
tiles=nums(CPP/'maps/tile_bmp.cpp','bitmaps')
for i in range(1904):
    assert actual[i]==(16,16,hashlib.sha256(bytes(tiles[i*256:(i+1)*256])).hexdigest()),i
for image in json.loads((BUILD/'pixel-hashes.json').read_text()):
    assert actual[image['id']]==(image['width'],image['height'],image['sha256']),image['name']
water=nums(CPP/'maps/active_tile_bmp.cpp','water')
for i in range(3):assert actual[2038+i]==(16,16,hashlib.sha256(bytes(water[i*256:(i+1)*256])).hexdigest())
ref=PROJECT.parent/'metalmax-reverse/resources'
tileset=json.loads((ref/'editor_state/tileSetEditor.json').read_text());props=json.loads((ref/'editor_state/mapPropertiesEditor.json').read_text())['mapProperties']
data=(HOST/'decoded-maps.bin').read_bytes();position=0;cells=0
for mid in range(240):
    original=nums(CPP/f'maps/map_data/map_{mid}.cpp',f'map_{mid}')
    raw=json.loads((ref/('maps/world.json' if mid==0 else f'maps/{mid:03d}.json')).read_text())['tiles']
    p=props[str(mid)];attrs=tileset['worldAttributes'] if mid==0 else [tileset['attributes'][p['combinationA']],tileset['attributes'][p['combinationB']]]
    logical=[attrs[t//64]['attributes'][t%64] for row in raw for t in row]
    for tile,attribute in zip(original,logical):
        a,b=struct.unpack_from('<HB',data,position);position+=3
        assert (a,b)==(tile,attribute),(mid,cells);cells+=1
assert position==len(data)
sprite_ref=PROJECT.parent/'metalmax-analysis/sprite-reference'
for actual_name,reference_name in [('normal_sprites.bin','normal.bin'),('sprite_models.bin','models.bin')]:
    assert (HOST/actual_name).read_bytes()==(sprite_ref/reference_name).read_bytes(),actual_name
player=(HOST/'player_sprites.bin').read_bytes()
normal=(HOST/'normal_sprites.bin').read_bytes()
assert player==normal[8*8*256:9*8*256], 'Character must use ROM on-foot model09'
original_palette_text=subprocess.run(['git','show','HEAD:app/src/main/cpp/graphic/palette_data.cpp'],cwd=PROJECT,capture_output=True,check=True).stdout.decode()
original_palette= [int(v,0) for v in re.findall(r'-?0x[0-9a-fA-F]+|-?\d+',re.search(r'\bpalette_rgb\s*(?:\[[^\]]*\]\s*)*=\s*\{(.*?)\};',original_palette_text,re.S)[1])]
assert nums(CPP/'graphic/palette_data.cpp','palette_rgb')==original_palette
assert not any(p.is_file() for p in (PROJECT/'app/src/main/assets').rglob('*'))
assert not (CPP/'data/generated').exists()
report={'module_headers_roundtrip':checked,'maps_original_files_exact':maps_checked,
        'generator_maps_no_writes':len(manifest['maps_unchanged']),
        'legacy_images_exact':2041,'map_and_attribute_cells_exact':cells,
        'module_resource_bytes':manifest['packed_bytes'],
        'sprite_rom_poses_exact':len(normal)//256,
        'sprite_rom_models_exact':len((HOST/'sprite_models.bin').read_bytes())//9216,
        'player_model':9,
        'runtime':(HOST/'run.log').read_text().strip()}
apk=PROJECT.parent/'metalmax-analysis/metalmax-debug.apk'
if apk.exists():
    archive=zipfile.ZipFile(apk);bad=[n for n in archive.namelist() if n.startswith('assets/') or n.endswith('.json')]
    assert not bad,bad[:5];report['apk_no_assets_or_json']=True
    report['apk_bytes']=apk.stat().st_size;report['apk_sha256']=hashlib.sha256(apk.read_bytes()).hexdigest()
(BUILD/'validation.json').write_text(json.dumps(report,ensure_ascii=False,indent=2));print(json.dumps(report,ensure_ascii=False,indent=2))
