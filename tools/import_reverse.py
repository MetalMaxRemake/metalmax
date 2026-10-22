#!/usr/bin/env python3
"""Generate separate bin2c headers in their owning modules. Never writes maps.

No global resource blob, no application assets, no JSON at runtime. Maps keep
their existing .cpp/.h arrays byte-for-byte and remain the rendering source.
"""
from pathlib import Path
from collections import Counter, defaultdict
import argparse, hashlib, json, re, struct
from bin2c import write
from script_compile import OP_NAMES, compile_script
from normalize_native_style import rewrite_source, to_snake

PROJECT=Path(__file__).resolve().parents[1]
CPP=PROJECT/'app/src/main/cpp'
LEGACY=PROJECT/'tools/inputs/legacy'
BUILD=PROJECT/'tools/build'
def load(p):return json.loads(p.read_text())
def nums(path,name):
    s=re.sub(r'//[^\n]*|/\*.*?\*/','',path.read_text(),flags=re.S)
    m=re.search(r'\b'+name+r'\s*(?:\[[^\]]*\]\s*)*=\s*\{(.*?)\};',s,re.S)
    return [int(v,0) for v in re.findall(r'-?0x[\da-fA-F]+|-?\d+',m[1])]
def u16(v):return int(v).to_bytes(2,'little')
def u24(v):return int(v).to_bytes(3,'little')
def packbits(values,bits):
    result=bytearray();acc=0;n=0
    for v in values:
        assert 0<=v<1<<bits
        acc|=v<<n;n+=bits
        while n>=8:result.append(acc&255);acc>>=8;n-=8
    if n:result.append(acc)
    return bytes(result)
def gridblock(values):
    colors=sorted(set(values));bits=0 if len(colors)==1 else 1 if len(colors)<=2 else 2 if len(colors)<=4 else 4 if len(colors)<=16 else 6
    local=bytes([bits,len(colors)-1])+bytes(colors)+packbits([colors.index(v) for v in values],bits)
    raw=bytes([255])+bytes(values)
    return local if len(local)<len(raw) else raw
def pixel_image(values,w,h):
    assert len(values)==w*h
    lookup={};blocks=[];refs=[]
    for by in range(0,h,8):
        for bx in range(0,w,8):
            block=bytes(values[(by+y)*w+bx+x] if bx+x<w and by+y<h else 35 for y in range(8) for x in range(8))
            if block not in lookup:
                colors=sorted(set(block));bits=0 if len(colors)==1 else 1 if len(colors)<=2 else 2 if len(colors)<=4 else 4 if len(colors)<=16 else 8
                lookup[block]=len(blocks);blocks.append(bytes([bits,len(colors)-1])+bytes(colors)+packbits([colors.index(v) for v in block],bits))
            refs.append(lookup[block])
    offsets=[];body=bytearray()
    for block in blocks:offsets.append(u24(len(body)));body.extend(block)
    return u16(w)+u16(h)+u16(len(blocks))+b''.join(u16(v) for v in refs)+b''.join(offsets)+body

def main():
    parser=argparse.ArgumentParser();parser.add_argument('--export',type=Path,default=PROJECT.parent/'metalmax-reverse')
    args=parser.parse_args();res=args.export/'resources';BUILD.mkdir(parents=True,exist_ok=True)
    manifest={'resources':{},'inputs':{},'maps_unchanged':{},'runtime_assets':False,'script_support':{}}
    def read(name):
        p=res/name;manifest['inputs'][name]=hashlib.sha256(p.read_bytes()).hexdigest();return load(p)
    before_maps={str(p.relative_to(CPP)):hashlib.sha256(p.read_bytes()).hexdigest() for p in (CPP/'maps').rglob('*') if p.is_file()}
    def emit(path,data,symbol):
        path=Path(path)
        if path.parts[0]=='maps':raise ValueError('maps is a read-only resource input')
        if not re.fullmatch(r'[a-z0-9]+(?:_[a-z0-9]+)*\.[a-z0-9]+',path.name):raise ValueError('resource filename must be lower_snake_case: '+str(path))
        symbol=to_snake(symbol)
        write(data,CPP/path,symbol)
        intermediate=BUILD/'bin'/path.with_suffix('.bin');intermediate.parent.mkdir(parents=True,exist_ok=True);intermediate.write_bytes(data)
        manifest['resources'][str(path)]={'symbol':symbol,'bytes':len(data),'binary':str(intermediate.relative_to(BUILD)),'sha256':hashlib.sha256(data).hexdigest()}
        return symbol
    def textfile(path,content):
        if Path(path).parts[0]=='maps':raise ValueError('maps is a read-only resource input')
        if not re.fullmatch(r'[a-z0-9]+(?:_[a-z0-9]+)*\.[a-z0-9]+',Path(path).name):raise ValueError('generated filename must be lower_snake_case: '+str(path))
        content=rewrite_source(content)
        p=CPP/path;p.parent.mkdir(parents=True,exist_ok=True)
        if not p.exists() or p.read_text()!=content:p.write_text(content)
    def cpp(module,headers,body,external_headers=()):
        include=''.join('#include "'+str(Path(h).relative_to(Path(module).parent))+'"\n' for h in headers)
        external=''.join('#include "'+str(Path(h).relative_to(Path(module).parent))+'"\n' for h in external_headers)
        textfile(module,'// Generated module-local resource dispatch.\n#include "../data/game_data.h"\n#include "../data/resource_decode.h"\n#include <cstring>\n#include <algorithm>\n'+external+'namespace {\n'+include+'}\n'+body)
    # Sprite CHR banks stay separate, as do each special/system model.
    tiles=read('editor_state/tileSetEditor.json');props=read('editor_state/mapPropertiesEditor.json')['mapProperties']
    palette=nums(CPP/'graphic/palette_data.cpp','palette_rgb');pal=read('palettes/data.json');system=[c['argb']&0xffffff for c in read('graphics/system_palette.json')['colors']]
    headers=[];bankcases=[]
    banks=sorted(set([4,5,0,1,148,149]+[b for p in props.values() for b in [p['spriteIndex'],p['spriteIndex']+1]]))
    for bank in banks:
        pool=[];dedup={};indices=[]
        for tile in tiles['tiles'][bank]['tiles']:
            raw=bytes(tile['tileData'])
            if raw not in dedup:dedup[raw]=len(pool);pool.append(raw)
            indices.append(dedup[raw])
        path=f'sprite/sprite_data/chr_{bank:02x}.h';symbol=emit(path,bytes(indices)+b''.join(pool),f'k_chr_{bank:02x}');headers.append(path)
        bankcases.append(f'case {bank}: return {symbol}+64+{symbol}[id]*16;')
    # The editor guesses 217 bytes for these tables. The CPU indexes the PRG
    # addresses with an eight-bit Y register, including image50's index217.
    # Preserve the complete addressable window instead of inventing a fallback.
    rom_path=args.export.parent/'mm.nes';rom=rom_path.read_bytes()
    assert rom[:4]==b'NES\x1a' and len(rom)==0xc0010
    manifest['inputs']['../mm.nes']=hashlib.sha256(rom).hexdigest()
    sprite_windows={'x847B':0x2647b,'x8552':0x26552,'x8629':0x26629}
    tables={};pose=bytearray()
    for key in ['xA597','xA59B','xA59E','xA5DD','x83F2','x83FA','x847B','x8552','x8629']:
        data=bytes(tiles[key])
        if key in sprite_windows:
            start=16+sprite_windows[key];raw=rom[start:start+256]
            assert raw[:len(data)]==data,key
            data=raw
        tables[key]=len(pose);pose.extend(data)
    path='sprite/sprite_data/pose_table.h';emit(path,pose,'k_sprite_pose');headers.append(path)
    colors=[]
    def color(rgb):
        for i,v in enumerate(palette):
            if i!=35 and (v&0xffffff)==rgb:return i
        raise ValueError('Required sprite color missing from unchanged palette: '+hex(rgb))
    for row in pal['spritePalette']:colors.extend(color(system[v]) for v in row['paletteRow'])
    path='sprite/sprite_data/sprite_palette.h';emit(path,bytes(colors),'k_sprite_palette');headers.append(path)
    models=read('sprites/models.json');specialcases=[];systemcases=[]
    for i,m in sorted(models['spriteModels'].items(),key=lambda x:int(x[0])):
        path=f'sprite/sprite_data/special_{int(i):03d}.h';sym=emit(path,bytes([m['head'],m['attribute']])+u16(len(m['model']))+bytes(m['model']),f'k_special_{int(i)}');headers.append(path);specialcases.append(f'case {int(i)+1}: return {sym};')
    for i,rows in sorted(models['systemSpriteModels'].items(),key=lambda x:int(x[0])):
        path=f'sprite/sprite_data/system_{int(i):03d}.h';sym=emit(path,bytes([len(rows)])+b''.join(bytes(r[k] for k in ['x','y','tile','attribute']) for r in rows),f'k_system_{int(i)}');headers.append(path);systemcases.append(f'case {int(i)+1}: return {sym};')
    assert bytes(v for row in pal['spritePalette'] for v in row['paletteRow'])==rom[0x7d737:0x7d747]
    sprite_body='''namespace {
const uint8_t *chr_tile(int bank,int id) {
    static const uint8_t empty[16]={}; if(id<0 || id>=64)return empty;
    switch(bank) { __BANKS__ default:return empty; }
}
const uint8_t *sprite_tile(int bank,unsigned id) {
 const int banks[4]={4,5,bank,bank+1};
 return id<256 ? chr_tile(banks[id/64],id%64) : chr_tile(-1,0);
}
const uint8_t *special(int id) { switch(id) { __SPECIAL__ default:return nullptr; } }
const uint8_t *system_model(int id) { switch(id) { __SYSTEM__ default:return nullptr; } }
unsigned pose_index(int model,int direction,int step) {
 return uint8_t(k_sprite_pose[__xA59E__+model]+k_sprite_pose[__xA597__+direction]+(step&1));
}
uint8_t pixel(const uint8_t *p,int x,int y) {return ((p[y]>>(7-x))&1)|(((p[y+8]>>(7-x))&1)<<1);}
void put(uint8_t *s,int x,int y,uint8_t c) {if(x>=0&&x<455&&y>=0&&y<256&&c!=35)s[y*455+x]=c;}
void tile_draw(int bank,int id,int x,int y,int flags,int pal,uint8_t *s) {
 const uint8_t *t=sprite_tile(bank,id);
 for(int yy=0;yy<8;yy++)for(int xx=0;xx<8;xx++) {auto c=pixel(t,flags&64?7-xx:xx,flags&128?7-yy:yy);if(c)put(s,x+xx,y+yy,k_sprite_palette[pal*4+c]);}
}
}
namespace metalmax::game_data {
void apply_palette() {} // Sprite colors refer to the unchanged global palette.
void decode_sprite(int bank,int model,int direction,int step,uint8_t result[256]) {
 memset(result,35,256);if(model<=0||model>=64||direction<0||direction>=4)return;
 unsigned index=pose_index(model,direction,step);
 uint8_t flags=k_sprite_pose[__x847B__+index],attr=k_sprite_pose[__xA5DD__+model]|k_sprite_pose[__xA59B__+direction];
 uint8_t top=k_sprite_pose[__x8552__+index],bottom=k_sprite_pose[__x8629__+index];
 uint8_t ids[4]={top,uint8_t(top+k_sprite_pose[__x83F2__+(flags&7)]),bottom,uint8_t(bottom+k_sprite_pose[__x83FA__+(flags&7)])};
 for(int y=0;y<16;y++)for(int x=0;x<16;x++) {
  int sx=attr&64?15-x:x,sy=attr&128?15-y:y;unsigned q=sy/8*2+sx/8;int px=sx%8;if(flags&(1<<(q+3)))px=7-px;
  uint8_t c=pixel(sprite_tile(bank,ids[q]),px,sy%8);unsigned pal=sy>=8?attr&3:(attr>>2)&3;if(c)result[y*16+x]=k_sprite_palette[pal*4+c];
 }
}
void draw_sprite(int bank,int model,int direction,int step,int type,int x,int y,uint8_t *screen) {
 if(model<=0)return;
 if(type==0) {
  if(model>=64||direction<0||direction>=4)return;
  uint8_t f[256];decode_sprite(bank,model,direction,step,f);
  y+=k_sprite_pose[__x847B__+pose_index(model,direction,step)]>>7;
  for(int yy=0;yy<16;yy++)for(int xx=0;xx<16;xx++)put(screen,x+xx,y+yy,f[yy*16+xx]);
 } else if(type==1) {
  const auto *d=special(model);if(!d)return;
  unsigned width=((d[1]>>2)&7)+1,n=resource_decode::u16(d+2);
  // ROM9031 subtracts the head offsets. The LSR/EOR/ADC sequence also
  // subtracts one: OAM Y+1 places its origin seven pixels below normal.
  x-=d[0]&0x1c;y+=7-((d[0]&0xe0)>>3);
  static const uint8_t flipPatterns[4]={0x00,0x55,0xaa,0x33};
  for(unsigned i=0;i<n;i++) {
   if(!d[4+i])continue; // A zero tile omits the OAM entry entirely.
   unsigned col=i%width;uint8_t pattern=uint8_t(flipPatterns[d[1]&3]<<col);
   tile_draw(bank,d[4+i],x+col*8,y+i/width*8,pattern&0x20?64:0,d[0]&3,screen);
  }
 } else if(type==2) {
  const auto *d=system_model(model);if(!d)return;
  // The ROM stores signed offsets around the actor anchor. Earliest OAM
  // entries take priority, so draw them last in this indexed framebuffer.
  for(int i=d[0]-1;i>=0;i--) {
   const auto *p=d+1+i*4;
   tile_draw(bank,p[2],x+int8_t(p[0]),y+3+int8_t(p[1]),p[3],p[3]&3,screen);
  }
 }
}
}
'''.replace('__BANKS__',''.join(bankcases)).replace('__SPECIAL__',''.join(specialcases)).replace('__SYSTEM__',''.join(systemcases))
    for key,off in tables.items():sprite_body=sprite_body.replace('__'+key+'__',str(off))
    cpp('sprite/sprite_resources.cpp',headers,sprite_body)
    # NPC and additional movement attributes each belong to their map group,
    # under sprite/, leaving every existing maps/ byte entirely untouched.
    npcs=read('sprites/npc-index.json');headers=[];ranges=[];indexcases=[];attrcases=[];bankcases=[];index=0
    for mid in range(250):
        entries=sorted([n for n in npcs if n['map']==mid],key=lambda n:n['sprite']);ranges.append(f'case {mid}:return {{{index},{len(entries)}}};')
        if entries:
            path=f'sprite/npc_data/npc_map_{mid:03d}.h';sym=emit(path,b''.join(bytes(n['raw'][k] for k in ['type','x','y','talk1','talk2','action']) for n in entries),f'kNpcMap{mid}');headers.append(path)
            indexcases.append(f'if(i>={index}&&i<{index+len(entries)}){{memcpy(&n,{sym}+(i-{index})*6,6);return n;}}');index+=len(entries)
        if mid>=240:continue
        meta=props[str(mid)];bankcases.append(f'case {mid}:return {meta["spriteIndex"]};')
        raw=read('maps/world.json' if mid==0 else f'maps/{mid:03d}.json')['tiles'];h,w=len(raw),len(raw[0]);at=tiles['worldAttributes'] if mid==0 else [tiles['attributes'][meta['combinationA']],tiles['attributes'][meta['combinationB']]]
        values=[at[t//64]['attributes'][t%64] for row in raw for t in row];blocks=[];dedup={};refs=[]
        for by in range(0,h,8):
            for bx in range(0,w,8):
                block=tuple(values[(by+y)*w+bx+x] if bx+x<w and by+y<h else 128 for y in range(8) for x in range(8))
                if block not in dedup:dedup[block]=len(blocks);blocks.append(gridblock(block))
                refs.append(dedup[block])
        offsets=[];body=bytearray()
        for b in blocks:offsets.append(u16(len(body)));body.extend(b)
        data=u16(len(blocks))+b''.join(u16(i) for i in refs)+b''.join(offsets)+body
        path=f'sprite/npc_data/map_{mid:03d}_attributes.h';sym=emit(path,data,f'kMapAttrs{mid}');headers.append(path);attrcases.append(f'case {mid}:p={sym};break;')
    cpp('sprite/npc_resources.cpp',headers,'''#include "../maps/map_data/map_data.h"
namespace metalmax::game_data {
range_t map_npcs(unsigned map){switch(map){__RANGES__ default:return {};}}
npc_data_t npc(unsigned i){npc_data_t n={};__INDICES__ return n;}
uint8_t map_sprite_bank(unsigned map){switch(map){__BANKS__ default:return 0;}}
uint8_t map_attribute(unsigned map,int x,int y){
 if(map>=240||x<0||y<0||x>=map_size[map*2+1]||y>=map_size[map*2])return 128;
 const uint8_t *p=nullptr;switch(map){__ATTRS__ default:return 128;}
 unsigned cols=(map_size[map*2+1]+7)/8,rows=(map_size[map*2]+7)/8,nrefs=cols*rows;
 unsigned block=resource_decode::u16(p+2+(y/8*cols+x/8)*2),n=resource_decode::u16(p);
 const auto *b=p+2+nrefs*2+n*2+resource_decode::u16(p+2+nrefs*2+block*2);
 return resource_decode::attribute(b,y%8*8+x%8);
}
uint16_t map_tile(unsigned map,int x,int y){if(map>=240)return 0;if(x<0||y<0||x>=map_size[map*2+1]||y>=map_size[map*2])return map_fill[map];return short_map_data[map][y*map_size[map*2+1]+x];}
}
'''.replace('__RANGES__',''.join(ranges)).replace('__INDICES__',''.join(indexcases)).replace('__BANKS__',''.join(bankcases)).replace('__ATTRS__',''.join(attrcases)))
    # Supplemental entrance_t/border_t records are per-map event headers in logic.
    headers=[];ranges=[];indices=[];borders=[];index=0
    for row in read('maps/entrances.json'):
        mid=row['id'];entries=row['entries'];targets=row['border_targets'];typ={'LAST':0,'FIXED':1,'DIRECTION':2}[row['border_type']]
        data=bytes([typ,len(entries)])+b''.join(bytes([t['map'],t['x'],t['y']]) for t in (targets+[{'map':0,'x':0,'y':0}]*4)[:4])+b''.join(bytes([e['key']['x'],e['key']['y'],e['value']['x'],e['value']['y'],e['value']['map']]) for e in entries)
        path=f'logic/map_events/map_{mid:03d}_events.h';sym=emit(path,data,f'kMapEvent{mid}');headers.append(path)
        ranges.append(f'case {mid}:return {{{index},{len(entries)}}};');borders.append(f'case {mid}:b.type={sym}[0];memcpy(b.targets,{sym}+2,12);return b;')
        if entries:indices.append(f'if(i>={index}&&i<{index+len(entries)}){{memcpy(&e,{sym}+14+(i-{index})*5,5);return e;}}')
        index+=len(entries)
    cpp('logic/map_event_resources.cpp',headers,'''namespace metalmax::game_data {
range_t map_entrances(unsigned map){switch(map){__RANGES__ default:return {};}}
entrance_t entrance_at(unsigned i){entrance_t e={};__INDICES__ return e;}
border_t map_border(unsigned map){border_t b={};switch(map){__BORDERS__ default:return b;}}
}
'''.replace('__RANGES__',''.join(ranges)).replace('__INDICES__',''.join(indices)).replace('__BORDERS__',''.join(borders)))
    # Each script has its own header. A compact range_t dispatcher identifies it
    # without a data pointer array or one merged instruction pool.
    headers=[];scriptcases=[];nodecases=[];unsupported=Counter();offset=0
    for kind,count in [('dynamic',169),('static',108)]:
        manifest['script_support'][kind]={'complete_table_translation':[],'has_unsupported_barrier':[]}
        for sid in range(count):
            nodes=compile_script(read(f'scripts/{kind}/{sid:03d}.ast.json'),OP_NAMES,unsupported)
            path=f'logic/scripts/{kind}_{sid:03d}.h';sym=emit(path,b''.join(struct.pack('<BhBBBh',n[0],n[1],n[2]&255,n[3]&255,n[4]&255,n[5]) for n in nodes),f'k{kind.title()}Script{sid}');headers.append(path)
            gameid=sid if kind=='dynamic' else 169+sid;scriptcases.append(f'case {gameid}:return {{{offset},{len(nodes)}}};')
            nodecases.append(f'if(i>={offset}&&i<{offset+len(nodes)}){{p={sym}+(i-{offset})*8;}}')
            offset+=len(nodes);key='has_unsupported_barrier' if any(n[0]==OP_NAMES.index('Unsupported') for n in nodes) else 'complete_table_translation';manifest['script_support'][kind][key].append(sid)
    cpp('logic/script_resources.cpp',headers,'''namespace metalmax::game_data {
script_data_t script(bool dynamic,unsigned id){if((dynamic&&id>=169)||(!dynamic&&(!id||id>108)))return {};switch(dynamic?id:168+id){__SCRIPTS__ default:return {};}}
script_node_t script_node_at(unsigned i){const uint8_t *p=nullptr;__NODES__ if(!p)return {0,0,0,0,-1,k_stop};return {int16_t(resource_decode::u16(p+1)),p[3],p[4],p[5],int16_t(resource_decode::u16(p+6)),p[0]};}
}
'''.replace('__SCRIPTS__',''.join(scriptcases)).replace('__NODES__','else '.join(nodecases)))
    # Text pages and glyph source banks are independently generated headers.
    headers=[];textcases=[]
    for page in range(22):
        rows=read(f'text/page_{page:02X}.json');pool=bytearray(b'\0');offsets={'':0};refs=[]
        for row in rows:
            text=re.sub(r'\[[0-9A-Fa-f]+\]','',row['text'].replace('[9F]','').replace('[FE]','\f').replace('\t','\n'))
            if text not in offsets:offsets[text]=len(pool);pool.extend(text.encode()+b'\0')
            refs.append(u16(offsets[text]))
        assert len(pool)<65536;data=u16(len(rows))+b''.join(refs)+pool
        path=f'charset/text_data/page_{page:02x}.h';sym=emit(path,data,f'k_textPage{page}');headers.append(path)
        textcases.append(f'case {page}:p={sym};break;')
    cpp('charset/text_resources.cpp',headers,'''namespace metalmax::game_data {
const char *text(int page,int row){const uint8_t *p=nullptr;switch(page){__PAGES__ default:return "";}unsigned count=resource_decode::u16(p);if(row<0||unsigned(row)>=count)return "";return reinterpret_cast<const char*>(p+2+count*2+resource_decode::u16(p+2+row*2));}
}
'''.replace('__PAGES__',''.join(textcases)))
    glyphs=read('fonts/glyphs/index.json');groups=defaultdict(list)
    for g in glyphs:groups[g['bank_selected']].append(g)
    headers=[];search=[]
    for bank,rows in sorted(groups.items()):
        rows.sort(key=lambda g:ord(g['character']));data=b''.join(u16(ord(g['character']))+(res/'fonts/glyphs'/('%02X%02X.bin'%tuple(g['code']))).read_bytes() for g in rows)
        path=f'charset/font_data/bank_{bank:02x}.h';sym=emit(path,data,f'k_font_bank_{bank}');headers.append(path);search.append(f'if(const uint8_t *p=find({sym},{len(rows)},unicode))return p;')
    font_external=[]
    for name,shape in [('asci','[96][8]'),('zh_cn','[4][12][2]')]:
        data=(PROJECT/'tools/inputs'/(name+'.bin')).read_bytes();path=f'charset/font_data/{name}.h';emit(path,data,'k'+name.title());font_external.append(path)
        textfile(path,'// Generated font data.\n#include <stdint.h>\nnamespace metalmax {\nextern const uint8_t '+name+shape+'={'+','.join(map(str,data))+'};\n} // namespace metalmax\n')
    cpp('charset/font_resources.cpp',headers,'''namespace {
const uint8_t *find(const uint8_t *p,unsigned n,uint32_t unicode){unsigned lo=0,hi=n;while(lo<hi){unsigned mid=(lo+hi)/2;if(resource_decode::u16(p+mid*20)<unicode)lo=mid+1;else hi=mid;}return lo<n&&resource_decode::u16(p+lo*20)==unicode?p+lo*20+2:nullptr;}
}
namespace metalmax::game_data {const uint8_t *glyph(uint32_t unicode){__SEARCH__ return nullptr;}}
'''.replace('__SEARCH__',''.join(search)),font_external)
    # Each monster image is stored separately in monster_NNN.h; stats/names
    # are its module-local data, never pooled with maps or title images.
    monsters=read('monsters/interpreted.json');headers=[];imagecases=[];namecases=[];stats=[];pixelhashes=[]
    for i,m in enumerate(monsters):
        src=LEGACY/f'monster/monster_data/monster_{i}.cpp';h,w=map(int,re.search(r'\bmonster_'+str(i)+r'\[(\d+)\]\[(\d+)\]',src.read_text()).groups());pixels=nums(src,'monster_'+str(i));path=f'monster/monster_data/monster_{i:03d}.h';sym=emit(path,pixel_image(pixels,w,h),f'k_monster_image_{i}');headers.append(path);imagecases.append(f'case {i}:return {sym};')
        path=f'monster/monster_data/monster_{i:03d}_name.h';ns=emit(path,m['name'].encode()+b'\0',f'k_monster_name_{i}');headers.append(path);namecases.append(f'case {i}:return reinterpret_cast<const char*>({ns});')
        v=m['interpreted'];stats.append(b''.join(u16(v[k]) for k in ['hp','attack','defense'])+bytes([m['attributes']['speed'],v['hit'],v['evasion']])+u16(v['exp'])+u16(v['gold']))
        pixelhashes.append({'id':1904+i,'name':f'monster_{i}','width':w,'height':h,'sha256':hashlib.sha256(bytes(pixels)).hexdigest()})
    path='monster/monster_data/monster_stats.h';emit(path,b''.join(stats),'k_monster_stats');headers.append(path)
    legacy=LEGACY/'monster/monster_data/monster_data.c'
    monster_external=[]
    for name,size in [('monster_group',930),('monster_distribution_world',256),('monster_distribution_4_samll_maps',112),('monster_properties',1834)]:
        path=f'monster/monster_data/{name}.h';raw=bytes(nums(legacy,name));emit(path,raw,'k'+name.title().replace('_',''));monster_external.append(path)
        textfile(path,'// Generated monster module table.\n#include <stdint.h>\nextern const uint8_t '+name+f'[{size}]={{'+','.join(map(str,raw))+'};\n')
    cpp('monster/monster_resources.cpp',headers,'''namespace {const char *monster_name(unsigned i){switch(i){__NAMES__ default:return "";}}}
extern const int monster_count=131;
namespace metalmax::game_data {
const uint8_t *monsterImage(unsigned id){switch(id){__IMAGES__ default:return nullptr;}}
monster_data_t monster(unsigned i){if(i>=131)i=0;const auto *p=k_monster_stats+i*13;return {monster_name(i),resource_decode::u16(p),resource_decode::u16(p+2),resource_decode::u16(p+4),resource_decode::u16(p+9),resource_decode::u16(p+11),p[6],p[7],p[8]};}
}
'''.replace('__NAMES__',''.join(namecases)).replace('__IMAGES__',''.join(imagecases)),monster_external)
    # Three title pictures retain one file each under the existing splash module.
    headers=[];cases=[]
    for i,name in enumerate(['splash','logo','logo_2']):
        src=LEGACY/f'splash/{name}.cpp';h,w=map(int,re.search(r'\b'+name+r'\[(\d+)\]\[(\d+)\]',src.read_text()).groups());pixels=nums(src,name);path=f'splash/{name}.h';sym=emit(path,pixel_image(pixels,w,h),'k'+name.title().replace('_',''));headers.append(path);cases.append(f'case {i}:return {sym};')
        pixelhashes.append({'id':2035+i,'name':name,'width':w,'height':h,'sha256':hashlib.sha256(bytes(pixels)).hexdigest()})
    cpp('splash/splash_resources.cpp',headers,'''namespace metalmax::game_data {const uint8_t *splashImage(unsigned id){switch(id){__CASES__ default:return nullptr;}}}
'''.replace('__CASES__',''.join(cases)))
    # Items are grouped by their existing equipment/tool categories.
    items=read('items/interpreted.json');categories=defaultdict(list);kinds={'PlayerArmor':1,'PlayerWeapon':2,'PlayerItem':3,'TankWeapon':4,'TankCUnit':5,'TankEngine':6,'TankChassis':7,'TankItem':8}
    for item in items:categories[item['attributes']['$type']].append(item)
    headers=[];cases=[]
    for kind,rows in sorted(categories.items()):
        records=[];pool=bytearray()
        for i,item in enumerate(rows):
            at=item['attributes'];flags=kinds[kind]|(16 if not item.get('priceless',False) else 0)|(32 if item.get('can_discard',False) else 0)
            records.append(bytes([flags,at.get('canEquipped',0)])+u16(item.get('attack_value',item.get('defense_value',0)))+u24(item.get('price_value',0))+u16(len(pool)));pool.extend(item['name'].encode()+b'\0');cases.append(f'case {item["id"]}:p=k{kind}Data+{i*9};names=k{kind}Data+{len(rows)*9};break;')
        filename=re.sub(r'(?<!^)(?=[A-Z])','_',kind).lower();path=f'item/item_data/{filename}.h';emit(path,b''.join(records)+pool,f'k{kind}Data');headers.append(path)
    cpp('item/item_resources.cpp',headers,'''namespace metalmax::game_data {item_data_t item(unsigned id){const uint8_t *p=nullptr,*names=nullptr;switch(id){__CASES__ default:return {"",0,0,0,0,false,false};}return {reinterpret_cast<const char*>(names+resource_decode::u16(p+7)),resource_decode::u24(p+4),resource_decode::u16(p+2),uint8_t(p[0]&15),p[1],bool(p[0]&16),bool(p[0]&32)};}}
'''.replace('__CASES__',''.join(cases)))
    # Stores retain separate lists per category/page instead of one stats blob.
    lists=read('lists/data.json')['typeLists'];types=['TANK_EQUIPMENT','TANK_ITEM','PLAYER_EQUIPMENT','PLAYER_ITEM','TANK_TAX','TANK_SPECIAL_SHELL','IHH','BAR_A','BAR_B','DECORATION','JUKEBOX','VENDING_MACHINE_ITEM','VENDING_MACHINE_SHELL','VENDING_MACHINE_SP','PHARMACY','ELEVATOR'];headers=[];shops=[];entrycases=[];index=0
    for t,typ in enumerate(types):
        for page,rows in enumerate(lists[typ]):
            path=f'store/store_data/{typ.lower()}_{page:02d}.h';sym=emit(path,b''.join(bytes([r['item'],r.get('count',1)]) for r in rows),f'kStore{t}_{page}');headers.append(path);shops.append(f'case {t*256+page}:return {{{index},{t},{page},{len(rows)}}};')
            if rows:entrycases.append(f'if(i>={index}&&i<{index+len(rows)})return {{{sym}[(i-{index})*2],{sym}[(i-{index})*2+1]}};')
            index+=len(rows)
    cpp('store/store_resources.cpp',headers,'''namespace metalmax::game_data {shop_data_t shop(int type,int page){if(type<0||type>=16||page<0||page>255)return {};switch(type*256+page){__SHOPS__ default:return {};}}shop_entry_t shop_entry_at(unsigned i){__ENTRIES__ return {};}}
'''.replace('__SHOPS__',''.join(shops)).replace('__ENTRIES__',''.join(entrycases)))
    # Player, experience, value, treasure and computer tables belong to their
    # individual existing modules, each with dedicated headers/translation units.
    initial=read('players/data.json');headers=[];playercases=[]
    for i in range(3):
        v=initial['initialAttributes'][f'PLAYER_{i}'];data=b''.join(u16(v[k]) for k in ['health','maxHealth','attack','defense'])+bytes([v['level'],v['speed']])+u24(v['experience'])+bytes([v['teamStatus']])+bytes(v['equipment'])+bytes(v['inventory'])+bytes([v['equipmentState']]);path=f'logic/status/player_data/player_{i}.h';sym=emit(path,data,f'kPlayer{i}');headers.append(path);playercases.append(f'case {i}:p={sym};break;')
    path='logic/status/player_data/initial_money.h';emit(path,u24(initial['money']),'k_initial_money');headers.append(path)
    exp=read('player_experience/data.json')['experiences'];path='logic/status/player_data/experience.h';emit(path,b''.join(u24(exp.get(str(i),0)) for i in range(100)),'k_experience');headers.append(path)
    # This generated file is two levels deep; includes are relative to it.
    body='''namespace metalmax::game_data {player_data_t initial_player(unsigned i){player_data_t r={};const uint8_t *p=nullptr;switch(i){__PLAYERS__ default:return r;}r.hp=resource_decode::u16(p);r.maxHp=resource_decode::u16(p+2);r.attack=resource_decode::u16(p+4);r.defense=resource_decode::u16(p+6);r.level=p[8];r.speed=p[9];r.experience=resource_decode::u24(p+10);r.team=p[13];memcpy(r.equipment,p+14,8);memcpy(r.inventory,p+22,8);r.equipmentState=p[30];return r;}uint32_t initial_money(){return resource_decode::u24(k_initial_money);}uint32_t experience(unsigned i){return i<100?resource_decode::u24(k_experience+i*3):0;}}
'''.replace('__PLAYERS__',''.join(playercases))
    cpp('logic/player_resources.cpp',headers,body)
    values=read('values/data.json')['VALUES'];path='item/item_data/value_table.h';emit(path,b''.join(u24(values.get(str(i),0)) for i in range(256)),'k_item_values')
    cpp('item/value_resources.cpp',[path],'''namespace metalmax::game_data {uint32_t value(unsigned i){return i<256?resource_decode::u24(k_item_values+i*3):0;}}
''')
    # Tank initial state is structured once into its own module. Runtime uses
    # the generated header, never the JSON source.
    tanks=read('tanks/data.json')
    order=['NO_1','NO_2','NO_3','NO_4','NO_5','NO_6','NO_7','NO_8']
    tank_records=bytearray()
    for name in order:
        t=tanks['tankInitAttributes'][name]
        tank_records.extend(bytes(t['equipment']))
        tank_records.extend(bytes([t['equipmentState'],t['shells']]))
        for key in ['defense','maxDefense','weight','sp']:
            tank_records.extend(u16(t[key]))
        tank_records.extend(bytes([t.get('map',255),t.get('x',0),t.get('y',0),t['slot']]))
    emit('tank/tank_data/tank_initial.h',bytes(tank_records),'k_tank_initial')
    emit('tank/tank_data/tank_meta.h',bytes([tanks['defenseUpStep'],tanks['shellsUpStep'],tanks['maxShells']])+bytes(tanks['weaponShellCapacities']),'k_tank_meta')
    treasures=read('treasures/data.json')['treasures'];headers=[];cases=[]
    for i,t in enumerate(treasures):
        path=f'logic/treasure_data/treasure_{i:03d}.h';sym=emit(path,bytes(t[k] for k in ['map','x','y','item']),f'k_treasure_{i}');headers.append(path);cases.append(f'case {i}:memcpy(&t,{sym},4);break;')
    cpp('logic/treasure_resources.cpp',headers,'''namespace metalmax::game_data {treasure_data_t treasure(unsigned i){treasure_data_t t={};switch(i){__CASES__ default:break;}return t;}}
'''.replace('__CASES__',''.join(cases)))
    path='computer/computer_data.h';raw=bytes(nums(LEGACY/'computer/computer_data.cpp','computers'));emit(path,raw,'k_computers');textfile(path,'#ifndef METALMAX_COMPUTER_DATA_H\n#define METALMAX_COMPUTER_DATA_H\n#include <stdint.h>\n#define computer_count 123\n#ifdef METALMAX_DEFINE_COMPUTER_DATA\nextern const uint8_t computers[492]={'+','.join(map(str,raw))+'};\n#else\nextern const uint8_t computers[492];\n#endif\n#endif\n');textfile('computer/computer_data.cpp','#define METALMAX_DEFINE_COMPUTER_DATA\n#include "computer_data.h"\n')
    teleports=read('teleports/data.json')['teleportLocations'];teleport_bytes=bytearray()
    for i in range(16):
        t=teleports[i] if i<len(teleports) else {}
        teleport_bytes.extend(bytes([t.get('map',255),t.get('x',0),t.get('y',0)]))
    emit('computer/computer_data/teleport_locations.h',bytes(teleport_bytes),'k_teleport_locations')
    # NSF stays under audio with its own header, not a central generated folder.
    emit('audio/mm_sound_data.h',(PROJECT/'tools/inputs/all_sound.bin').read_bytes(),'k_nsf')
    # Cold reverse-engineered source ranges are split by editor owner; one
    # header per physical region. They remain binary, never JSON/AST strings.
    rom=(args.export/'provenance/mm.original.nes').read_bytes();regions=read('regions-expanded.json');byowner=defaultdict(list)
    owners={'tankEditor':'tank','dogSystemEditor':'computer','elevatorEditor':'logic','eventTilesEditor':'logic','treasureEditor':'logic','spriteScriptEditor':'logic','spriteEditor':'sprite','monsterProbability':'monster','audioEditor':'audio','textEditor':'charset','itemEditor':'item'}
    for r in regions:
        if r.get('editor') in owners:
            start=r.get('file_start');n=r.get('actual_export_length',r.get('length',0))
            if start is not None and 0<n<100000:byowner[r['editor']].append((start,start+n))
    for a in read('audio/index.json'):
        byowner['audioEditor'].extend([(a['record_file_offset'],a['record_file_offset']+a['record_bytes']),(a['pointer_file_offset'],a['pointer_file_offset']+2)])
        byowner['audioEditor'].extend((c['file_start'],c['file_start']+c['bytes']) for c in a['channels'])
    for d in read('audio/dpcm/index.json'):byowner['audioEditor'].extend([(d['file_offset'],d['file_offset']+d['sample_length']),(d['descriptor_file'],d['descriptor_file']+3)])
    for r in read('items/source-fields.json'):byowner['itemEditor'].extend((o,o+1) for o in r['fields'].values())
    dispatch=[];externs=[]
    for owner,spans in sorted(byowner.items()):
        merged=[]
        for start,end in sorted(spans):
            if merged and start<=merged[-1][1]:merged[-1][1]=max(merged[-1][1],end)
            else:merged.append([start,end])
        module=owners[owner];headers=[];cases=[]
        for start,end in merged:
            file_owner=re.sub(r'(?<!^)(?=[A-Z])','_',owner).lower()
            path=f'{module}/source_data/{file_owner}_{start:06x}.h';sym=emit(path,rom[start:end],f'k_source_{owner}{start:06X}');headers.append(path);cases.append(f'if(file>={start} && file-{start}<={end-start} && length<={end-start}-(file-{start}))return {sym}+file-{start};')
        fun=f'{owner}Source';cpp(f'{module}/{file_owner}_source.cpp',headers,f'namespace metalmax::game_data {{const uint8_t *{fun}(uint32_t file,unsigned length){{'+''.join(cases)+'return nullptr;}}\n');externs.append(f'const uint8_t *{fun}(uint32_t,unsigned);');dispatch.append(f'if(const uint8_t *p={fun}(file,length))return p;')
    textfile('data/game_data.cpp','// Resource ownership stays in the original modules. No data is defined here.\n#include "game_data.h"\nnamespace metalmax::game_data {\n'+''.join(externs)+'\nconst uint8_t *source_bytes(uint32_t file,unsigned length){'+''.join(dispatch)+'return nullptr;}\nunsigned packed_bytes(){return '+str(sum(r['bytes'] for r in manifest['resources'].values()))+';}\n}\n')
    after_maps={str(p.relative_to(CPP)):hashlib.sha256(p.read_bytes()).hexdigest() for p in (CPP/'maps').rglob('*') if p.is_file()}
    assert before_maps==after_maps,'Generator changed maps unexpectedly'
    manifest['maps_unchanged']=before_maps;manifest['unsupported_scripts']=dict(unsupported);manifest['packed_bytes']=sum(r['bytes'] for r in manifest['resources'].values());manifest['sprite_palette']=colors
    (BUILD/'manifest.json').write_text(json.dumps(manifest,ensure_ascii=False,indent=2));(BUILD/'pixel-hashes.json').write_text(json.dumps(pixelhashes,indent=2))
    print('MODULE_HEADERS',len(manifest['resources']),'BYTES',manifest['packed_bytes'],'MAPS_UNCHANGED',len(before_maps))
if __name__=='__main__':main()
