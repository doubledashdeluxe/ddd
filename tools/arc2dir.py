#!/usr/bin/env python3


from argparse import ArgumentParser
import os
import struct


def unpack_u8(data, offset, **kwargs):
    return struct.unpack_from('>B', data, offset)[0]

def unpack_u16(data, offset, **kwargs):
    return struct.unpack_from('>H', data, offset)[0]

def unpack_u32(data, offset, **kwargs):
    return struct.unpack_from('>I', data, offset)[0]

def unpack_file(data, path):
    with open(path, 'wb') as file:
        file.write(data)

def unpack_dir(data, files, dirs, nodes, names, path):
    os.mkdir(path)
    node_count = unpack_u16(data, 0x0a)
    first_node_index = unpack_u32(data, 0x0c)
    for node_index in range(first_node_index, first_node_index + node_count):
        unpack_node(nodes[node_index * 0x14:], files, dirs, nodes, names, path)

def unpack_node(data, files, dirs, nodes, names, path):
    is_dir = unpack_u8(data, 0x04) & 0x2 == 0x2
    name_offset = unpack_u32(data, 0x04) & 0xffffff
    name = names[name_offset:].split(b'\0', 1)[0].decode('ascii')
    if name == '.' or name == '..' or '/' in name or '\\' in name:
        return
    path = os.path.join(path, name)
    if is_dir:
        dir_index = unpack_u32(data, 0x08)
        unpack_dir(dirs[dir_index * 0x10:], files, dirs, nodes, names, path)
    else:
        file_offset = unpack_u32(data, 0x08)
        file_size = unpack_u32(data, 0x0c)
        unpack_file(files[file_offset:file_offset + file_size], path)

def unpack_tree(data, files, path):
    dir_count = unpack_u32(data, 0x00)
    dirs_offset = unpack_u32(data, 0x04)
    node_count = unpack_u32(data, 0x08)
    nodes_offset = unpack_u32(data, 0x0c)
    names_offset = unpack_u32(data, 0x14)
    dirs = data[dirs_offset:]
    nodes = data[nodes_offset:]
    names = data[names_offset:]
    unpack_dir(dirs, files, dirs, nodes, names, path)

def unpack_archive(data, path):
    tree_offset = unpack_u32(data, 0x08)
    files_offset = unpack_u32(data, 0x0c)
    files = data[tree_offset + files_offset:]
    unpack_tree(data[tree_offset:], files, path)


parser = ArgumentParser()
parser.add_argument('in_path')
parser.add_argument('out_path', nargs = '?')
args = parser.parse_args()

if args.out_path is not None:
    out_path = args.out_path
else:
    out_path = args.in_path + '.d'
with open(args.in_path, 'rb') as arc_file:
    unpack_archive(arc_file.read(), out_path)
