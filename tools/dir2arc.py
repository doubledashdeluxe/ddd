#!/usr/bin/env python3


from argparse import ArgumentParser
import os
import struct


def pack_u8(val, **kwargs):
    return struct.pack('>B', val)

def pack_u16(val, **kwargs):
    return struct.pack('>H', val)

def pack_u32(val, **kwargs):
    return struct.pack('>I', val)

def pack_hash16(name):
    h = 0
    for c in name.encode('ascii'):
        h *= 3
        h += c
    return pack_u16(h & 0xffff)

def pack_file(path, name, node_index, files, nodes, names):
    with open(path, 'rb') as file:
        file = file.read()
    nodes[node_index * 0x14:(node_index + 1) * 0x14] = b''.join([
        pack_u16(node_index),
        pack_hash16(name),
        pack_u8(0x11),
        pack_u32(len(names))[1:],
        pack_u32(len(files)),
        pack_u32(len(file)),
        pack_u32(0x0),
    ])
    names += name.encode('ascii') + b'\0'
    file = file.ljust((len(file) + 0x1f) & ~0x1f, b'\0')
    files += file

def pack_dir(path, name, node_index, files, dirs, nodes, names):
    node_names = sorted(os.listdir(path))
    first_node_index = len(nodes) // 0x14
    if node_index is not None:
        nodes[node_index * 0x14:(node_index + 1) * 0x14] = b''.join([
            pack_u16(0xffff),
            pack_hash16(name),
            pack_u8(0x2),
            pack_u32(len(names))[1:],
            pack_u32(len(dirs) // 0x10),
            pack_u32(0x10),
            pack_u32(0x0),
        ])
    dirs += b''.join([
        ('root' if node_index is None else name).ljust(4)[:4].upper().encode('ascii'),
        pack_u32(len(names)),
        pack_hash16(name),
        pack_u16(len(node_names)),
        pack_u32(first_node_index),
    ])
    names += name.encode('ascii') + b'\0'
    for _ in node_names:
        nodes += bytearray(0x14)
    for node_index, node_name in enumerate(node_names, start = first_node_index):
        node_path = os.path.join(path, node_name)
        pack_node(node_path, node_name, node_index, files, dirs, nodes, names)

def pack_node(path, name, node_index, files, dirs, nodes, names):
    if os.path.isdir(path):
        pack_dir(path, name, node_index, files, dirs, nodes, names)
    else:
        pack_file(path, name, node_index, files, nodes, names)

def pack_tree(path, tree, files):
    dirs = bytearray()
    nodes = bytearray()
    names = bytearray()
    name = os.path.basename(path).removesuffix('.arc.d')
    pack_dir(path, name, None, files, dirs, nodes, names)
    dir_count = len(dirs) // 0x10
    node_count = len(nodes) // 0x14
    dirs[:] = dirs.ljust((len(dirs) + 0x1f) & ~0x1f, b'\0')
    nodes[:] = nodes.ljust((len(nodes) + 0x1f) & ~0x1f, b'\0')
    names[:] = names.ljust((len(names) + 0x1f) & ~0x1f, b'\0')
    tree[:] = b''.join([
        pack_u32(dir_count),
        pack_u32(0x20),
        pack_u32(node_count),
        pack_u32(0x20 + len(dirs)),
        pack_u32(len(names)),
        pack_u32(0x20 + len(dirs) + len(nodes)),
        pack_u16(len(nodes) // 0x14),
        pack_u8(0x1),
        pack_u8(0x0),
        pack_u32(0x0),
        dirs,
        nodes,
        names,
    ])

def pack_archive(path):
    tree = bytearray()
    files = bytearray()
    pack_tree(path, tree, files)
    return b''.join([
        b'RARC',
        pack_u32(0x20 + len(tree) + len(files)),
        pack_u32(0x20),
        pack_u32(len(tree)),
        pack_u32(len(files)),
        pack_u32(len(files)),
        pack_u32(0x0),
        pack_u32(0x0),
        tree,
        files,
    ])

parser = ArgumentParser()
parser.add_argument('in_path')
parser.add_argument('out_path', nargs = '?')
args = parser.parse_args()

if args.out_path is not None:
    out_path = args.out_path
else:
    if args.in_path.endswith('.arc.d'):
        out_path = args.in_path.removesuffix('.arc.d')
    else:
        out_path = args.in_path + '.arc'
with open(out_path, 'wb') as file:
    file.write(pack_archive(args.in_path))
