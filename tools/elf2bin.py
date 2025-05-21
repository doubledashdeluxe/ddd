#!/usr/bin/env python3


from argparse import ArgumentParser
from elftools.elf.constants import P_FLAGS
from elftools.elf.elffile import ELFFile
import zlib


def segment_is_text(segment):
    return segment['p_flags'] & P_FLAGS.PF_X == P_FLAGS.PF_X


parser = ArgumentParser()
parser.add_argument('kind', choices=['I', 'D'])
parser.add_argument('in_path')
parser.add_argument('out_path')
args = parser.parse_args()

with open(args.in_path, 'rb') as elf_file, open(args.out_path, 'wb') as bin_file:
    elf_file = ELFFile(elf_file)
    segments = [segment for segment in elf_file.iter_segments()]
    segments = [segment for segment in segments if segment_is_text(segment) == (args.kind == 'I')]
    segments = sorted(segments, key = lambda segment: segment['p_vaddr'])

    vaddr = segments[0]['p_vaddr']
    out_data = bytes()
    for segment in segments:
        if segment['p_vaddr'] < vaddr:
            raise SystemExit('Segments are overlapping!')
        else:
            out_data += bytes([0x00] * (segment['p_vaddr'] - vaddr))
        out_data += segment.data()
        out_data += bytes([0x00] * (segment['p_memsz'] - segment['p_filesz']))
        vaddr = segment['p_vaddr'] + segment['p_memsz']
    out_data = zlib.compress(out_data, level = zlib.Z_BEST_COMPRESSION, wbits = -15)
    bin_file.write(out_data)
