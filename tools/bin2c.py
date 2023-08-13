#!/usr/bin/env python3


from argparse import ArgumentParser
import io


parser = ArgumentParser()
parser.add_argument('in_name')
parser.add_argument('in_path')
parser.add_argument('out_path')
args = parser.parse_args()

with open(args.in_path, 'rb') as bin_file, open(args.out_path, 'w') as c_file:
    c_file.write('#include <common/Types.h>\n')
    c_file.write('\n')
    bin_file.seek(0, io.SEEK_END)
    c_file.write(f'const size_t {args.in_name}_size = {bin_file.tell():#x};\n')
    bin_file.seek(0, io.SEEK_SET)
    c_file.write(f'const u8 {args.in_name}[] = {{\n')
    eof = False
    while not eof:
        c_file.write('   ')
        for _ in range(8):
            byte = bin_file.read(1)
            if byte == b'':
                eof = True
                break
            c_file.write(f' 0x{byte[0]:02x},')
        c_file.write('\n')
    c_file.write('};\n')
