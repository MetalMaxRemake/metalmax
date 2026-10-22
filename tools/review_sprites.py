#!/usr/bin/env python3
"""Visual review of actual native sprite output against the original resources.

Only development PNG/HTML/report files are written. Runtime resources remain
module-local bin2c headers and maps are never read through a converted format.
Uses the Python standard library; no Pillow or external image tool required.
"""
from pathlib import Path
import hashlib
import json
import re
import shutil
import struct
import subprocess
import zlib
from import_reverse import PROJECT, CPP, nums
from sprite_reference import oam

ROOT = PROJECT.parent
HOST = ROOT / 'metalmax-analysis/host-tests'
OUT = PROJECT / 'docs/sprite_previews'
AUDIT = ROOT / 'metalmax-analysis/sprite-review'
BANKS = [0, 100, 102, 106, 107, 116, 148, 150, 184, 186, 188]
PALETTE = nums(CPP / 'graphic/palette_data.cpp', 'palette_rgb')
NORMAL = (HOST / 'normal_sprites.bin').read_bytes()
MODELS = (HOST / 'sprite_models.bin').read_bytes()
PLAYER = (HOST / 'player_sprites.bin').read_bytes()
assert len(NORMAL) == len(BANKS)*63*8*256
assert len(MODELS) == len(BANKS)*123*96*96
assert len(PLAYER) == 8*256


def png(path, width, height, pixels, scale=1):
    def chunk(kind, data):
        return struct.pack('>I',len(data))+kind+data+struct.pack('>I',zlib.crc32(kind+data)&0xffffffff)
    width2, height2 = width*scale, height*scale
    rows = bytearray()
    for y in range(height):
        row = bytearray([0])
        for value in pixels[y*width:(y+1)*width]:
            color = 0x374150 if value == 35 else PALETTE[value] & 0xffffff
            row.extend(bytes((color >> 16, color >> 8 & 255, color & 255))*scale)
        rows.extend(row*scale)
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes(b'\x89PNG\r\n\x1a\n'+chunk(b'IHDR',struct.pack('>IIBBBBB',width2,height2,8,2,0,0,0))+chunk(b'IDAT',zlib.compress(rows))+chunk(b'IEND',b''))


def paste(canvas, width, pixels, sprite_width, sprite_height, x, y):
    for yy in range(sprite_height):
        at = (y+yy)*width+x
        canvas[at:at+sprite_width] = pixels[yy*sprite_width:(yy+1)*sprite_width]


def mirror(frame):
    return b''.join(frame[y*16:(y+1)*16][::-1] for y in range(16))


def sheet(frames):
    canvas = bytearray([35])*(64*32)
    for direction in range(4):
        for step in range(2): paste(canvas,64,frames[(direction*2+step)*256:(direction*2+step+1)*256],16,16,direction*16,step*16)
    return canvas


def pose(bank, model, direction, step):
    at = (BANKS.index(bank)*63*8+(model-1)*8+direction*2+step)*256
    return NORMAL[at:at+256]


def original(name, symbol):
    source = subprocess.check_output(['git','show','HEAD:app/src/main/cpp/sprite/'+name],cwd=PROJECT,text=True)
    body = re.search(r'\b'+symbol+r'\s*(?:\[[^\]]*\]\s*)*=\s*\{(.*?)\};',source,re.S)[1]
    return bytes(int(v,0) for v in re.findall(r'-?0x[\da-fA-F]+|-?\d+',body))


def main():
    AUDIT.mkdir(parents=True, exist_ok=True)
    OUT.mkdir(parents=True, exist_ok=True)
    archive = AUDIT / 'previous_previews'
    # Keep the inaccurate previews as evidence before replacing their paths.
    if not archive.exists():
        archive.mkdir()
        for path in OUT.glob('model_*.png'): shutil.copy2(path,archive/path.name)
    legacy = original('sprite_player.cpp','sprite_player')
    assert len(legacy) == 1024
    down, left0, left1, up = [legacy[i*256:(i+1)*256] for i in range(4)]
    old = up+mirror(up)+down+mirror(down)+left0+left1+mirror(left0)+mirror(left1)
    old_sheet, fixed_sheet = sheet(old), sheet(PLAYER)
    png(OUT/'original_player.png',64,32,old_sheet,6)
    png(OUT/'player_09.png',64,32,fixed_sheet,6)
    comparison = bytearray([35])*(64*104)
    paste(comparison,64,old_sheet,64,32,0,0)
    before = AUDIT / 'before_sprites.bin'
    if before.exists():
        before_bytes = before.read_bytes()
        at = (BANKS.index(148)*63*8+35*8)*256
        before_sheet = sheet(before_bytes[at:at+2048])
        png(OUT/'previous_player.png',64,32,before_sheet,6)
        paste(comparison,64,before_sheet,64,32,0,36)
    paste(comparison,64,fixed_sheet,64,32,0,72)
    png(OUT/'player_comparison.png',64,104,comparison,6)
    # All63 models, both frames and all11 source-bank configurations.
    for bank in BANKS:
        canvas = bytearray([35])*(256*512)
        for model in range(1,64):
            frames = b''.join(pose(bank,model,d,s) for d in range(4) for s in range(2))
            pixels = sheet(frames)
            paste(canvas,256,pixels,64,32,model%4*64,model//4*32)
            if bank == 100: png(OUT/f'model_{model:02x}.png',64,32,pixels,4)
        png(OUT/f'bank_{bank:02x}.png',256,512,canvas,2)
    for kind, count, offset, label in [(1,69,0,'special'),(2,54,69,'system')]:
        canvas = bytearray([35])*(864*((count+8)//9)*96)
        for model in range(count):
            at = (BANKS.index(100)*123+offset+model)*9216
            pixels = MODELS[at:at+9216]
            paste(canvas,864,pixels,96,96,model%9*96,model//9*96)
        png(OUT/f'{label}_models.png',864,((count+8)//9)*96,canvas)
    # Render every exported NPC with its own map bank and model type. An
    # atlas of arbitrary model/bank combinations is not a scene reference.
    resources=ROOT/'metalmax-reverse/resources'
    npcs=json.loads((resources/'sprites/npc-index.json').read_text())
    props=json.loads((resources/'editor_state/mapPropertiesEditor.json').read_text())['mapProperties']
    bindings=[]
    for npc in npcs:
        raw=npc['raw'];model=raw['type']>>2;direction=raw['type']&3;kind=raw['talk1']&3
        path=f'npc_{npc["map"]:03d}_{npc["sprite"]:02d}.png'
        if model==0:
            bindings.append({'map':npc['map'],'npc':npc['sprite'],'model':0,'visible':False})
            continue
        context=props.get(str(npc['map']))
        if context is None:
            records=[oam(model,d,s) for d in range(4) for s in range(2)] if kind==0 else [oam(model,kind=kind)]
            if any(tile>=128 for record in records for tile in record[1::4]):
                bindings.append({'map':npc['map'],'npc':npc['sprite'],'model':model,'type':kind,'visible':True,'status':'no_map_chr_context'})
                continue
            # Every accessed tile is in fixed banks04/05; no map-bank guess.
            bank=0;context_status='fixed_chr_only'
        else:
            bank=context['spriteIndex'];context_status='map_properties'
        assert bank in BANKS,(npc['map'],bank)
        if kind==0:
            pixels=pose(bank,model,direction,0);width=height=16
        else:
            assert kind in (1,2)
            at=(BANKS.index(bank)*123+(0 if kind==1 else 69)+model-1)*9216
            source=MODELS[at:at+9216]
            nonempty=[(i%96,i//96) for i,v in enumerate(source) if v!=35]
            x1=min((x for x,y in nonempty),default=32);y1=min((y for x,y in nonempty),default=32)
            x2=max((x for x,y in nonempty),default=47)+1;y2=max((y for x,y in nonempty),default=47)+1
            width=x2-x1;height=y2-y1
            pixels=b''.join(source[y*96+x1:y*96+x2] for y in range(y1,y2))
        png(OUT/path,width,height,pixels,4)
        bindings.append({'map':npc['map'],'npc':npc['sprite'],'bank':bank,'model':model,'direction':direction,'type':kind,'visible':True,'image':path,'context_source':context_status})
    # Use existing NPC pictures for visual comparison too, recording shape
    # correspondence without assuming their array index is a ROM model ID.
    old_npcs = original('sprite_bmp.cpp','sprites')
    canvas = bytearray([35])*(256*144)
    for i in range(len(old_npcs)//256): paste(canvas,256,old_npcs[i*256:(i+1)*256],16,16,i%16*16,i//16*16)
    png(OUT/'original_npcs.png',256,144,canvas,3)
    def silhouette(pixels): return int.from_bytes(bytes(v!=35 for v in pixels),'little')
    variants = [(silhouette(pose(b,m,d,s)),b,m,d,s) for b in BANKS for m in range(1,64) for d in range(4) for s in range(2)]
    matches = []
    for i in range(len(old_npcs)//256):
        target = silhouette(old_npcs[i*256:(i+1)*256])
        error, bank, model, direction, step = min(((target^bits).bit_count(),b,m,d,s) for bits,b,m,d,s in variants)
        matches.append({'legacy_index':i,'silhouette_difference':error,'bank':bank,'model':model,'direction':direction,'step':step})
    player_diffs = []
    for d in range(4):
        for s in range(2):
            expected = old[(d*2+s)*256:(d*2+s+1)*256]
            actual = PLAYER[(d*2+s)*256:(d*2+s+1)*256]
            player_diffs.append({'direction':d,'step':s,'silhouette_difference':sum((a==35)!=(b==35) for a,b in zip(actual,expected)),
                                 'rgb_difference':sum((PALETTE[a]&0xffffff)!=(PALETTE[b]&0xffffff) for a,b in zip(actual,expected))})
    # Original hand-split side frames also differ in pixels; record that
    # difference rather than changing ROM poses to force a legacy match.
    for d in (0,1):
        for s in range(2): assert player_diffs[d*2+s]['silhouette_difference']==0
    report = {'native_normal_poses':len(NORMAL)//256,'native_model_cases':len(MODELS)//9216,'player_model':9,
              'player_fixed_banks':[4,5],'palette_rom_file_offset':'0x7d737','player_comparison':player_diffs,
              'legacy_npc_images':len(old_npcs)//256,'legacy_npc_exact_silhouettes':sum(m['silhouette_difference']==0 for m in matches),
              'legacy_npc_nearest_shapes':matches,'npc_bindings':bindings,'npc_records':len(bindings),
              'native_output_sha256':hashlib.sha256(NORMAL+MODELS+PLAYER).hexdigest()}
    (AUDIT/'review_report.json').write_text(json.dumps(report,ensure_ascii=False,indent=2))
    html = ['<!doctype html><meta charset="utf-8"><title>Sprite review</title>',
            '<style>body{font:16px sans-serif;background:#202833;color:white;margin:24px}img{image-rendering:pixelated;max-width:100%}figure{display:inline-block;vertical-align:top;margin:8px}a{color:#9cd4ff}</style>',
            '<h1>Role sprite review</h1><p>Directions: UP / DOWN / LEFT / RIGHT. Top row: frame0; bottom row: frame1.</p>']
    for label,name in [('Original player','original_player'),('Previous incorrect player','previous_player'),('Corrected native player09','player_09'),('Original static NPC images','original_npcs')]:
        if (OUT/f'{name}.png').exists(): html.append(f'<figure><figcaption>{label}</figcaption><img src="{name}.png"></figure>')
    html.append('<h2>All normal sprites, bank64</h2>')
    for model in range(1,64): html.append(f'<figure><figcaption>Model {model:02x}</figcaption><img src="model_{model:02x}.png"></figure>')
    for label,name in [('Special models (bank64)','special_models'),('System OAM models (bank64)','system_models')]:
        html.append(f'<h2>{label}</h2><img src="{name}.png">')
    html.append('<h2>NPC records in their map CHR context</h2><p>Hidden controller records with model0 have no picture.</p>')
    for binding in bindings:
        if binding.get('image'):
            html.append(f'<figure><figcaption>Map {binding["map"]:03d} NPC {binding["npc"]:02d}<br>bank {binding["bank"]:02x} / type {binding["type"]} / model {binding["model"]:02x}</figcaption><img src="{binding["image"]}"></figure>')
        elif binding.get('status'):
            html.append(f'<p>Group {binding["map"]:03d} NPC {binding["npc"]:02d}: no map CHR context; source retained without a guessed preview.</p>')
    html.append('<h2>All source configurations</h2>')
    for bank in BANKS: html.append(f'<p><a href="bank_{bank:02x}.png">CHR bank {bank:02x}: all directions and both frames</a></p>')
    (OUT/'index.html').write_text('\n'.join(html))
    print(f'SPRITE_VISUAL_REVIEW_OK poses={len(NORMAL)//256} models={len(MODELS)//9216} original_npcs={len(matches)} exact_shapes={report["legacy_npc_exact_silhouettes"]}')


if __name__ == '__main__': main()
