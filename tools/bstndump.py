#!/usr/bin/env python3


from argparse import ArgumentParser
import struct


def unpack_u32(data, offset, **kwargs):
    return struct.unpack_from('>I', data, offset)[0]


parser = ArgumentParser()
parser.add_argument('in_path')
args = parser.parse_args()

with open(args.in_path, 'rb') as bstn_file:
    data = bstn_file.read()
    tree_offset = unpack_u32(data, 0x0c)
    tree = data[tree_offset:]
    category_count = unpack_u32(tree, 0x0)
    for category_index in range(category_count):
        category_offset = unpack_u32(tree, 0x4 + category_index * 0x4)
        category = data[category_offset:]
        subcategory_count = unpack_u32(category, 0x0)
        category_name_offset = unpack_u32(category, 0x4)
        category_name = data[category_name_offset:].split(b'\0', 1)[0].decode('ascii')
        for subcategory_index in range(subcategory_count):
            subcategory_offset = unpack_u32(category, 0x8 + subcategory_index * 0x4)
            subcategory = data[subcategory_offset:]
            sound_count = unpack_u32(subcategory, 0x0)
            subcategory_name_offset = unpack_u32(subcategory, 0x4)
            subcategory_name = data[subcategory_name_offset:].split(b'\0', 1)[0].decode('ascii')
            for sound_index in range(sound_count):
                sound_name_offset = unpack_u32(subcategory, 0x8 + sound_index * 4)
                sound_name = data[sound_name_offset:].split(b'\0', 1)[0].decode('ascii')
                sound_id = category_index << 24 | subcategory_index << 16 | sound_index
                print(hex(sound_id), sound_name)
