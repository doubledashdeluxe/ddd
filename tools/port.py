#!/usr/bin/env python3


from argparse import ArgumentParser
from dataclasses import dataclass
import sys
from typing import Dict, List


@dataclass
class Section:
    name: str
    start: int
    end: int

    def __contains__(self, address):
        return self.start <= address < self.end

@dataclass
class Binary:
    sections: List[Section]

    def __contains__(self, address):
        return any(address in section for section in self.sections)

@dataclass
class Chunk:
    src_start: int
    src_end: int
    dst_start: int

    def __contains__(self, address):
        return self.src_start <= address < self.src_end

    def swap(self):
        return Chunk(self.dst_start, self.dst_start + self.src_end - self.src_start, self.src_start)

    def port(self, address):
        return address - self.src_start + self.dst_start


BINARIES = {
    'P': Binary(
        [
            Section('init'      , 0x80003100, 0x80005600),
            Section('extab'     , 0x80005600, 0x80005660),
            Section('extabindex', 0x80005660, 0x800056c0),
            Section('text'      , 0x800056c0, 0x802d84a0),
            Section('ctors'     , 0x802d84a0, 0x802d8a80),
            Section('dtors'     , 0x802d8a80, 0x802d8aa0),
            Section('BINARY'    , 0x802d8aa0, 0x8032cbe0),
            Section('rodata'    , 0x8032cbe0, 0x80346c20),
            Section('data'      , 0x80346c20, 0x8036dde0),
            Section('bss'       , 0x8036dde0, 0x803d3240),
            Section('sdata'     , 0x803d3240, 0x803d4980),
            Section('sbss'      , 0x803d4980, 0x803d63e0),
            Section('sdata2'    , 0x803d63e0, 0x803dc0c0),
            Section('sbss2'     , 0x803dc0c0, 0x803dc134),
        ],
    ),
    'E': Binary(
        [
            Section('init'      , 0x80003100, 0x80005600),
            Section('extab'     , 0x80005600, 0x80005660),
            Section('extabindex', 0x80005660, 0x800056c0),
            Section('text'      , 0x800056c0, 0x802d8520),
            Section('ctors'     , 0x802d8520, 0x802d8b00),
            Section('dtors'     , 0x802d8b00, 0x802d8b20),
            Section('BINARY'    , 0x802d8b20, 0x80322da0),
            Section('rodata'    , 0x80322da0, 0x8033cde0),
            Section('data'      , 0x8033cde0, 0x80363fa0),
            Section('bss'       , 0x80363fa0, 0x803c9420),
            Section('sdata'     , 0x803c9420, 0x803cab40),
            Section('sbss'      , 0x803cab40, 0x803cc5a0),
            Section('sdata2'    , 0x803cc5a0, 0x803d2280),
            Section('sbss2'     , 0x803d2280, 0x803d22f4),
        ],
    ),
    'J': Binary(
        [
            Section('init'      , 0x80003100, 0x80005600),
            Section('extab'     , 0x80005600, 0x80005660),
            Section('extabindex', 0x80005660, 0x800056c0),
            Section('text'      , 0x800056c0, 0x802d8540),
            Section('ctors'     , 0x802d8540, 0x802d8b20),
            Section('dtors'     , 0x802d8b20, 0x802d8b40),
            Section('BINARY'    , 0x802d8b40, 0x8033d3c0),
            Section('rodata'    , 0x8033d3c0, 0x80357400),
            Section('data'      , 0x80357400, 0x8037e5c0),
            Section('bss'       , 0x8037e5c0, 0x80353a40),
            Section('sdata'     , 0x803e3a40, 0x803e5160),
            Section('sbss'      , 0x803e5160, 0x803e6bc0),
            Section('sdata2'    , 0x803e6bc0, 0x803ec8a0),
            Section('sbss2'     , 0x803ec8a0, 0x803ec914),
        ],
    ),
}

CHUNKS = {
    'P': {
        'E': [
            Chunk(0x80003100, 0x800b07e8, 0x80003100),
            Chunk(0x800b07e8, 0x800eac24, 0x800b0824),
            Chunk(0x800ead2c, 0x8011ab60, 0x800eacfc),
            Chunk(0x8011ab60, 0x8013d29c, 0x8011ab3c),
            Chunk(0x8013d2a0, 0x8013d9a4, 0x8013d27c),
            Chunk(0x8013da58, 0x801520b8, 0x8013da28),
            Chunk(0x80152374, 0x801528fc, 0x801533d0),
            Chunk(0x80152d64, 0x801607a4, 0x80153dd8),
            Chunk(0x80160d88, 0x80189790, 0x80161ee4),
            Chunk(0x80189814, 0x801c9b20, 0x8018a984),
            Chunk(0x801ca048, 0x801ca538, 0x8015283c),
            Chunk(0x801ca714, 0x801cab9c, 0x80152f48),
            Chunk(0x801cab9c, 0x801cd85c, 0x801cac88),
            Chunk(0x801cda70, 0x801cf9b8, 0x801cdb30),
            Chunk(0x801cfa38, 0x801cfa60, 0x801cfb00),
            Chunk(0x801d02f8, 0x801d038c, 0x801d03fc),
            Chunk(0x801d0ad8, 0x801d2498, 0x801d0b4c),
            Chunk(0x801d2518, 0x801d5f04, 0x801d2560),
            Chunk(0x801d5f0c, 0x801d641c, 0x801d5f60),
            Chunk(0x801d6478, 0x801d8a34, 0x801d64c4),
            Chunk(0x801d8bf0, 0x801d93b0, 0x801d8c10),
            Chunk(0x801d93c4, 0x801dcf50, 0x801d93e0),
            Chunk(0x801dcf7c, 0x801dece8, 0x801dcfbc),
            Chunk(0x801deef0, 0x801e583c, 0x801def18),
            Chunk(0x801e583c, 0x8021683c, 0x801e586c),
            Chunk(0x8021684c, 0x80219c1c, 0x80216870),
            Chunk(0x80219c6c, 0x8022c398, 0x80219c88),
            Chunk(0x8022c534, 0x8023258c, 0x8022c550),
            Chunk(0x80232598, 0x8024e040, 0x802325c8),
            Chunk(0x8024e084, 0x80263270, 0x8024e0b4),
            Chunk(0x802633a4, 0x8027b790, 0x80263404),
            Chunk(0x8027b8d0, 0x802963d4, 0x8027b8f0),
            Chunk(0x802963f4, 0x802ae9c4, 0x80296418),
            Chunk(0x802ae9e8, 0x802d4854, 0x802aea24),
            Chunk(0x802d4854, 0x802d6100, 0x802d48d4),
            Chunk(0x802d6188, 0x802d8494, 0x802d6214),
            Chunk(0x80346c20, 0x80346c38, 0x8033cde0),
            Chunk(0x80347af8, 0x80347b08, 0x8033dcb8),
            Chunk(0x80347e70, 0x80347ec0, 0x8033e030),
            Chunk(0x8035c638, 0x8035c688, 0x803527f8),
            Chunk(0x80371e08, 0x80371e68, 0x80367fc8),
            Chunk(0x803ae998, 0x803ae99c, 0x803a4b78),
            Chunk(0x803bad54, 0x803bb1a0, 0x803b0f34),
            Chunk(0x803bb27c, 0x803bb508, 0x803b145c),
            Chunk(0x803bd5c8, 0x803be22c, 0x803b37a8),
            Chunk(0x803c5fe0, 0x803c65f4, 0x803bc1c0),
            Chunk(0x803d3248, 0x803d3250, 0x803cab48),
            Chunk(0x803d3b08, 0x803d3b10, 0x803c9ce0),
            Chunk(0x803d4980, 0x803d4988, 0x803cab40),
            Chunk(0x803d4aa0, 0x803d4aac, 0x803cac68),
            Chunk(0x803d4b90, 0x803d4b94, 0x803cad58),
            Chunk(0x803d4d50, 0x803d4d64, 0x803caf18),
            Chunk(0x803d4e0c, 0x803d4e18, 0x803cafd4),
            Chunk(0x803d51c4, 0x803d51d8, 0x803cb38c),
            Chunk(0x803d5498, 0x803d549c, 0x803cb658),
            Chunk(0x803d5578, 0x803d5584, 0x803cb738),
            Chunk(0x803d55e0, 0x803d5604, 0x803cb7a0),
            Chunk(0x803d5630, 0x803d5634, 0x803cb7f0),
            Chunk(0x803d5b68, 0x803d5c64, 0x803cbd28),
            Chunk(0x803d5cd0, 0x803d5cdc, 0x803cbe90),
        ],
        'J': [
            Chunk(0x80003100, 0x800b07e8, 0x80003100),
            Chunk(0x800b07e8, 0x800eac24, 0x800b0824),
            Chunk(0x800ead2c, 0x8011ab60, 0x800eacfc),
            Chunk(0x8011ab60, 0x8013d29c, 0x8011ab3c),
            Chunk(0x8013d2a0, 0x8013d9a4, 0x8013d27c),
            Chunk(0x8013da58, 0x801520b8, 0x8013da28),
            Chunk(0x80152374, 0x801528fc, 0x801533d0),
            Chunk(0x80152d64, 0x801607a4, 0x80153dd8),
            Chunk(0x80160d88, 0x80189790, 0x80161ee4),
            Chunk(0x80189814, 0x801c9b20, 0x8018a984),
            Chunk(0x801ca048, 0x801ca538, 0x8015283c),
            Chunk(0x801ca714, 0x801cab9c, 0x80152f48),
            Chunk(0x801cab9c, 0x801cd85c, 0x801cac88),
            Chunk(0x801cda70, 0x801cf9b8, 0x801cdb30),
            Chunk(0x801cfa38, 0x801cfa60, 0x801cfb00),
            Chunk(0x801d02f8, 0x801d038c, 0x801d03fc),
            Chunk(0x801d0ad8, 0x801d2498, 0x801d0b4c),
            Chunk(0x801d2518, 0x801d5f04, 0x801d2560),
            Chunk(0x801d5f0c, 0x801d641c, 0x801d5f60),
            Chunk(0x801d6478, 0x801d70a0, 0x801d64c4),
            Chunk(0x801d70a4, 0x801d7ca8, 0x801d70f0),
            Chunk(0x801d84a8, 0x801d8a34, 0x801d851c),
            Chunk(0x801d8bf0, 0x801d93b0, 0x801d8c38),
            Chunk(0x801d93c4, 0x801dcf50, 0x801d9408),
            Chunk(0x801dcf7c, 0x801dece8, 0x801dcfe4),
            Chunk(0x801deef0, 0x801e583c, 0x801def40),
            Chunk(0x801e583c, 0x8021683c, 0x801e5894),
            Chunk(0x8021684c, 0x80219c1c, 0x80216898),
            Chunk(0x80219c6c, 0x8022c398, 0x80219cb0),
            Chunk(0x8022c534, 0x8023258c, 0x8022c578),
            Chunk(0x80232598, 0x8024e040, 0x802325f0),
            Chunk(0x8024e084, 0x80263270, 0x8024e0dc),
            Chunk(0x802633a4, 0x8027b790, 0x8026342c),
            Chunk(0x8027b8d0, 0x802963d4, 0x8027b918),
            Chunk(0x802963f4, 0x802ae9c4, 0x80296440),
            Chunk(0x802ae9e8, 0x802d4854, 0x802aea4c),
            Chunk(0x802d4854, 0x802d6100, 0x802d48fc),
            Chunk(0x802d6188, 0x802d848c, 0x802d623c),
            Chunk(0x80346c20, 0x80346c38, 0x80357400),
            Chunk(0x80347af8, 0x80347b08, 0x803582d8),
            Chunk(0x80347e70, 0x80347ec0, 0x80358650),
            Chunk(0x8035c638, 0x8035c688, 0x8036ce18),
            Chunk(0x80371e08, 0x80371e68, 0x803825e8),
            Chunk(0x803ae998, 0x803ae99c, 0x803bf198),
            Chunk(0x803bad54, 0x803bb1a0, 0x803cb554),
            Chunk(0x803bb27c, 0x803bb508, 0x803cba7c),
            Chunk(0x803bd5c8, 0x803be22c, 0x803cddc8),
            Chunk(0x803c5fe0, 0x803c65f4, 0x803d67e0),
            Chunk(0x803d3248, 0x803d3250, 0x803e5168),
            Chunk(0x803d3b08, 0x803d3b10, 0x803e42f8),
            Chunk(0x803d4980, 0x803d4984, 0x803e3a44),
            Chunk(0x803d4aa0, 0x803d4aac, 0x803e5290),
            Chunk(0x803d4b90, 0x803d4b94, 0x803e5380),
            Chunk(0x803d4d50, 0x803d4d64, 0x803e5540),
            Chunk(0x803d4e0c, 0x803d4e18, 0x803e55fc),
            Chunk(0x803d51c4, 0x803d51d8, 0x803e59c4),
            Chunk(0x803d5498, 0x803d549c, 0x803e5c78),
            Chunk(0x803d5578, 0x803d5584, 0x803e5d58),
            Chunk(0x803d55e0, 0x803d5604, 0x803e5dc0),
            Chunk(0x803d5630, 0x803d5634, 0x803e5e10),
            Chunk(0x803d5b68, 0x803d5c64, 0x803e6348),
            Chunk(0x803d5cd0, 0x803d5cdc, 0x803e64b0),
        ],
    },
    'E': {
        'J': [
            Chunk(0x80003100, 0x801d70ec, 0x80003100),
            Chunk(0x801d70f0, 0x801d7cf4, 0x801d70f0),
            Chunk(0x801d84f4, 0x802d8518, 0x801d851c),
        ],
    },
}


def write_symbol(out_file, name, address):
    out_file.write(f'{address:x} {name}\n');

def port(in_region, out_region, in_address):
    if in_region == out_region:
        return in_address
    if in_region in CHUNKS and out_region in CHUNKS[in_region]:
        chunks = CHUNKS[in_region][out_region]
        for chunk in chunks:
            if in_address in chunk:
                return chunk.port(in_address)
        return None
    else:
        chunks = CHUNKS[out_region][in_region]
        for chunk in chunks:
            if in_address in chunk.swap():
                return chunk.swap().port(in_address)
        return None


parser = ArgumentParser()
parser.add_argument('in_region')
parser.add_argument('out_region')
subparsers = parser.add_subparsers(required = True)
address_parser = subparsers.add_parser('address')
address_parser.add_argument('in_address')
file = subparsers.add_parser('file')
file.add_argument('in_path')
file.add_argument('out_path')
args = parser.parse_args()

for region in [args.in_region, args.out_region]:
    if region not in BINARIES:
        sys.exit(
            f'The specified region \'{region}\' is invalid! '
            f'Valid regions include: {", ".join(BINARIES)}!'
        )

if 'in_address' in args:
    in_address = int(args.in_address, 16)
    out_address = port(args.in_region, args.out_region, in_address)
    if out_address is None:
        sys.exit(
            f'Couldn\'t port address {args.in_address} from {args.in_region} to {args.out_region}!'
        )
    print(f'{out_address:x}')
else:
    with open(args.out_path, 'w') as out_file:
        write_symbol(out_file, 'dol_start', BINARIES[args.out_region].sections[0].start)
        write_symbol(out_file, 'dol_end', BINARIES[args.out_region].sections[-1].end)
        out_file.write('\n')

        for section in BINARIES[args.out_region].sections:
            write_symbol(out_file, f'dol_{section.name}_start', section.start)
            write_symbol(out_file, f'dol_{section.name}_end', section.end)
        out_file.write('\n')

        with open(args.in_path, 'r') as symbols:
            for symbol in symbols.readlines():
                if symbol.isspace():
                    out_file.write('\n')
                    continue
                in_address, name = symbol.split()
                in_address = int(in_address, 16)
                address = port(args.in_region, args.out_region, in_address)
                if address is None:
                    sys.exit(
                        f'Couldn\'t port symbol {name} from {args.in_region} to {args.out_region}!'
                    )
                write_symbol(out_file, name, address)
