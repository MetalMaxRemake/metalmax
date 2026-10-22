#!/usr/bin/env python3
"""Execute the ROM's sprite routines and rasterize their OAM for host checks.

This deliberately does not use TileSetHelper or the C++ pose formulas.
The small bounded CPU only runs A57A, 835C, 9031 and 8000 with fixed PRG
windows. It is an offline sprite oracle, not a general NES emulator.
"""
from pathlib import Path
import json
import struct
from import_reverse import PROJECT, CPP, nums

ROOT = PROJECT.parent
OUT = ROOT / 'metalmax-analysis/sprite-reference'
BANKS = [0, 100, 102, 106, 107, 116, 148, 150, 184, 186, 188]
ROM = (ROOT / 'mm.nes').read_bytes()
assert ROM[:4] == b'NES\x1a' and len(ROM) == 0xc0010
PRG = ROM[16:0x80010]
CHR = ROM[0x80010:]
PALETTE = nums(CPP / 'graphic/palette_data.cpp', 'palette_rgb')
SYSTEM = json.loads((ROOT / 'metalmax-reverse/resources/graphics/system_palette.json').read_text())['colors']
COLORS = []
for nes in ROM[0x7d737:0x7d747]:
    rgb = SYSTEM[nes]['argb'] & 0xffffff
    COLORS.append(next(i for i, value in enumerate(PALETTE) if i != 35 and value & 0xffffff == rgb))

# Subset of NMOS 6502 instructions used by the four source routines.
OPS = {}
for name, modes in {
    'lda': {'imm':0xa9, 'zp':0xa5, 'zpx':0xb5, 'abs':0xad, 'abx':0xbd, 'aby':0xb9, 'iny':0xb1},
    'ldx': {'imm':0xa2, 'zp':0xa6, 'abs':0xae},
    'ldy': {'imm':0xa0, 'zp':0xa4},
    'sta': {'zp':0x85, 'abs':0x8d, 'abx':0x9d, 'aby':0x99},
    'stx': {'zp':0x86}, 'sty': {'zp':0x84},
    'adc': {'imm':0x69, 'zp':0x65, 'abx':0x7d},
    'sbc': {'imm':0xe9},
    'and': {'imm':0x29},
    'ora': {'zp':0x05, 'aby':0x19},
    'eor': {'imm':0x49, 'zp':0x45, 'iny':0x51},
    'asl': {'acc':0x0a, 'zp':0x06},
    'lsr': {'acc':0x4a, 'zp':0x46},
    'rol': {'acc':0x2a}, 'ror': {'acc':0x6a},
    'dec': {'zp':0xc6}, 'cmp': {'imm':0xc9}, 'cpx': {'imm':0xe0},
}.items():
    for mode, op in modes.items():
        OPS[op] = (name, mode)
for name, op in [('clc',0x18),('sec',0x38),('tax',0xaa),('tay',0xa8),('txa',0x8a),('tya',0x98),
                 ('inx',0xe8),('iny',0xc8),('dex',0xca),('dey',0x88),('pha',0x48),('pla',0x68),('rts',0x60)]:
    OPS[op] = (name, 'imp')
for name, op in [('bpl',0x10),('bmi',0x30),('bne',0xd0),('beq',0xf0),('bcc',0x90),('bcs',0xb0)]:
    OPS[op] = (name, 'rel')
OPS[0x4c] = ('jmp', 'abs')


class Cpu:
    def __init__(self):
        self.mem = bytearray(65536)
        self.mem[0x8000:0xa000] = PRG[0x26000:0x28000]
        self.mem[0xa000:0xc000] = PRG[0x34000:0x36000]
        self.a = self.x = self.y = self.c = self.n = self.z = 0
        self.stack = []

    def nz(self, value):
        value &= 255
        self.n, self.z = value >> 7, int(value == 0)
        return value

    def run(self, pc):
        for _ in range(4096):
            origin = pc
            op = self.mem[pc]
            if op not in OPS:
                raise AssertionError(f'unsupported opcode {op:02x} at {pc:04x}')
            name, mode = OPS[op]
            pc += 1
            address = None
            if mode in ('imm', 'zp', 'zpx', 'iny', 'rel'):
                address = self.mem[pc]
                pc += 1
                if mode == 'zpx': address = (address + self.x) & 255
                if mode == 'iny': address = (self.mem[address] | self.mem[(address+1)&255] << 8) + self.y
            elif mode in ('abs', 'abx', 'aby'):
                address = self.mem[pc] | self.mem[pc+1] << 8
                pc += 2
                if mode == 'abx': address += self.x
                if mode == 'aby': address += self.y
            if address is not None: address &= 65535
            value = self.a if mode == 'acc' else address if mode in ('imm', 'rel') else self.mem[address] if address is not None else 0
            if name == 'rts':
                assert not self.stack
                return
            if name == 'jmp': pc = address
            elif mode == 'rel':
                condition = {'bpl':not self.n, 'bmi':self.n, 'bne':not self.z, 'beq':self.z, 'bcc':not self.c, 'bcs':self.c}[name]
                if condition: pc = (pc + (value if value < 128 else value - 256)) & 65535
            elif name in ('lda', 'ldx', 'ldy'): setattr(self, name[-1], self.nz(value))
            elif name in ('sta', 'stx', 'sty'):
                assert address < 0x8000, (origin, address)
                self.mem[address] = getattr(self, name[-1])
            elif name in ('adc', 'sbc'):
                total = self.a + (value if name == 'adc' else value ^ 255) + self.c
                self.c = int(total > 255)
                self.a = self.nz(total)
            elif name in ('and', 'ora', 'eor'):
                self.a = self.nz(self.a & value if name == 'and' else self.a | value if name == 'ora' else self.a ^ value)
            elif name in ('asl', 'lsr', 'rol', 'ror'):
                carry = self.c
                self.c = (value >> 7) if name in ('asl', 'rol') else (value & 1)
                result = self.nz((value << 1) | (carry if name == 'rol' else 0) if name in ('asl', 'rol') else (value >> 1) | (carry << 7 if name == 'ror' else 0))
                if mode == 'acc': self.a = result
                else: self.mem[address] = result
            elif name == 'dec': self.mem[address] = self.nz(value - 1)
            elif name in ('cmp', 'cpx'):
                register = self.a if name == 'cmp' else self.x
                self.c = int(register >= value)
                self.nz(register - value)
            elif name in ('clc', 'sec'): self.c = int(name == 'sec')
            elif name in ('tax', 'tay', 'txa', 'tya'): setattr(self, name[-1], self.nz(getattr(self, name[1])))
            elif name in ('inx', 'iny', 'dex', 'dey'): setattr(self, name[-1], self.nz(getattr(self, name[-1]) + (1 if name.startswith('in') else -1)))
            elif name == 'pha': self.stack.append(self.a)
            elif name == 'pla': self.a = self.nz(self.stack.pop())
            else: raise AssertionError((origin, name))
        raise AssertionError('sprite routine exceeded instruction budget')


def oam(model, direction=0, step=0, kind=0):
    cpu = Cpu()
    if kind == 0:
        cpu.a = step << 4  # Four ASL instructions shift the walk bit into C.
        cpu.y = model
        cpu.mem[0] = direction
        cpu.run(0xa57a)
        cpu.x = 0
        cpu.y = cpu.mem[0x150]
        # The C++ API takes the normal sprite's top-left origin. In the ROM
        # its Y anchor is two pixels lower (SBC4, offset1, OAM Y+1).
        cpu.mem[0x110] = 64
        cpu.mem[0x130] = 66
        cpu.run(0x835c)
    else:
        cpu.mem[0] = 64
        cpu.mem[1] = 66
        cpu.y = model
        cpu.run(0x9031 if kind == 1 else 0x8000)
    return cpu.mem[0x700:0x700+cpu.mem[0xad]]


def raster(records, bank, size=16, origin=64):
    result = bytearray([35]) * (size * size)
    banks = [4, 5, bank, bank+1]
    # NES OAM gives the earliest sprite priority on overlapping opaque pixels.
    for i in reversed(range(0, len(records), 4)):
        y, tile, flags, x = records[i:i+4]
        y += 1
        data = CHR[banks[tile//64]*1024 + tile%64*16:][:16]
        for yy in range(8):
            sy = 7-yy if flags & 128 else yy
            for xx in range(8):
                sx = 7-xx if flags & 64 else xx
                color = (data[sy] >> (7-sx) & 1) | ((data[sy+8] >> (7-sx) & 1) << 1)
                dx, dy = x + xx - origin, y + yy - origin
                if color and 0 <= dx < size and 0 <= dy < size:
                    result[dy*size+dx] = COLORS[(flags & 3)*4+color]
    return result


def main():
    OUT.mkdir(parents=True, exist_ok=True)
    normal, offsets, models = bytearray(), bytearray(), bytearray()
    for bank in BANKS:
        for model in range(1, 64):
            for direction in range(4):
                for step in range(2):
                    records = oam(model, direction, step)
                    dy = min(records[::4]) + 1 - 64
                    offsets.append(dy)
                    shifted = bytearray(records)
                    for i in range(0, len(shifted), 4): shifted[i] -= dy
                    normal.extend(raster(shifted, bank))
        for kind, count in [(1, 69), (2, 54)]:
            for model in range(1, count+1):
                models.extend(raster(oam(model, kind=kind), bank, 96, 32))
    (OUT / 'normal.bin').write_bytes(normal)
    (OUT / 'normal_y_offsets.bin').write_bytes(offsets)
    (OUT / 'models.bin').write_bytes(models)
    (OUT / 'reference_metadata.json').write_text(json.dumps({'banks':BANKS,'normal_poses':len(offsets),'model_cases':len(models)//9216,'palette_indices':COLORS,'source':'actual ROM 6502 sprite routines; OAM rasterization'}, indent=2))
    print(f'SPRITE_ROM_REFERENCE_OK poses={len(offsets)} model_cases={len(models)//9216}')


if __name__ == '__main__': main()
