#!/usr/bin/env python3


from argparse import ArgumentParser
from dataclasses import dataclass
import mmap
import re
import shutil
import sys


@dataclass
class Patch:
    def __init__(self, offset, byte):
        self.offset = offset
        self.byte = byte


def generate_patch_list(dif_filepath):
    patch_list = []
    try:
        with open(dif_filepath, 'r', newline='\n') as dif_file:
            line_number = 0
            for line in dif_file:
                line_number += 1

                line = line.rstrip('\r\n')
                match = re.match(r'^([0-9A-Fa-f]{8}): ([0-9A-Fa-f]{2}) ([0-9A-Fa-f]{2})', line)
                if not match:
                    continue

                offset = int(match.group(1), 16)
                byte = bytes.fromhex(match.group(3))

                patch_list.append(Patch(offset, byte))
    except Exception:
        return None

    return patch_list


def apply_patch_list(patch_list, input_filepath, output_filepath):
    try:
        shutil.copy(input_filepath, output_filepath)
    except Exception:
        return False

    num_patches_applied = 0
    try:
        with open(output_filepath, 'r+b') as output_file:
            with mmap.mmap(output_file.fileno(), 0) as mmapped_output_file:
                for patch in patch_list:
                    if patch.offset >= len(mmapped_output_file):
                        continue

                    mmapped_output_file[patch.offset:patch.offset + 1] = patch.byte
                    num_patches_applied += 1
    except Exception:
        return False

    return num_patches_applied == len(patch_list)


parser = ArgumentParser()
parser.add_argument('dif_filepath')
parser.add_argument('input_filepath')
parser.add_argument('output_filepath')
args = parser.parse_args()

patch_list = generate_patch_list(args.dif_filepath)
if patch_list is None:
    sys.exit(f'Failed to read the patch(es) from the file \'{args.dif_filepath}\'!')

if not apply_patch_list(patch_list, args.input_filepath, args.output_filepath):
    sys.exit(f'Failed to apply the patch(es) to the file \'{args.output_filepath}\'!')
