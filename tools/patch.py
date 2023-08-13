#!/usr/bin/env python3


from argparse import ArgumentParser
import copy
from elftools.elf.elffile import ELFFile
import sys


parser = ArgumentParser()
parser.add_argument('in_elf_path')
parser.add_argument('in_symbols_path')
parser.add_argument('out_symbols_path')
parser.add_argument('out_patches_path')
args = parser.parse_args()

replaced_symbols = []
replacement_symbols = []
regular_symbols = []
with open(args.in_elf_path, 'rb') as elf_file:
    elf = ELFFile(elf_file)

    replacements_section_index = None
    for index, section in enumerate(elf.iter_sections()):
        if section.name == 'replacements':
            replacements_section_index = index

    symtab = elf.get_section_by_name('.symtab')

    for symbol in symtab.iter_symbols():
        symbol_type = symbol['st_info']['type']
        if symbol['st_shndx'] == 'SHN_UNDEF':
            if 'thunk_replaced_' not in symbol.name:
                continue
            replaced_symbols += [symbol.name]
        elif symbol['st_shndx'] == replacements_section_index:
            if symbol_type != 'STT_FUNC':
                continue

            replacement_symbols += [symbol.name]
        elif symbol_type == 'STT_FUNC' or symbol_type == 'STT_OBJECT':
            regular_symbols += [symbol.name]

thunk_symbols = {}
for name in replaced_symbols:
    replacement_name = name.replace('thunk_replaced_', '', 1)
    if replacement_name not in replacement_symbols:
        sys.exit(f'REPLACED was used without REPLACE for symbol {name}!')
    thunk_symbols[replacement_name] = name

backup = copy.deepcopy(replacement_symbols)
out_symbols = ''
with open(args.in_symbols_path, 'r') as in_symbols_file:
    for symbol in in_symbols_file.readlines():
        if symbol.isspace():
            out_symbols += '\n'
            continue

        address, name = symbol.split()
        address = int(address, 16)

        if name in regular_symbols:
            sys.exit(f'Multiple definitions for symbol {name}!')

        if name in replacement_symbols:
            replacement_symbols.remove(name)
            name = 'replaced_' + name
        out_symbols += f'0x{address:08x} {name}\n'
for name in replacement_symbols:
    sys.exit(f'Attempted to REPLACE {name}, but it doesn\'t exist in the symbol file!')
replacement_symbols = backup

out_patches = '#include "payload/Patcher.hh"\n'
out_patches += '\n'
out_patches += '#pragma section RWX "thunks" "thunks"\n'
out_patches += '\n'
out_patches += f'const size_t Patcher::PatchCount = {len(replacement_symbols)};\n'
out_patches += '\n'
for name in replacement_symbols:
    out_patches += f'extern "C" int replaced_{name};\n'
    out_patches += f'extern "C" int {name};\n'
    if name in thunk_symbols:
        out_patches += 'extern "C" {\n'
        out_patches += f'__declspec(section "thunks") u32 {thunk_symbols[name]}[2];\n'
        out_patches += '}\n'
    out_patches += '\n'
out_patches += 'const Patcher::Patch Patcher::Patches[] = {\n'
for name in replacement_symbols:
    out_patches += '    {\n'
    out_patches += f'        &replaced_{name},\n'
    out_patches += f'        &{name},\n'
    if name in thunk_symbols:
        out_patches += f'        {thunk_symbols[name]},\n'
    else:
        out_patches += '        nullptr,\n'
    out_patches += '    },\n'
out_patches += '};\n'

with open(args.out_symbols_path, 'w') as out_symbols_file:
    out_symbols_file.write(out_symbols)

with open(args.out_patches_path, 'w') as out_patches_file:
    out_patches_file.write(out_patches)
