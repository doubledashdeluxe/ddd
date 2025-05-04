#!/usr/bin/env python3


import argparse
import glob
import os
import subprocess


parser = argparse.ArgumentParser()
parser.add_argument('--ci', action='store_true')
args = parser.parse_args()

files = set()
for ext in ['c', 'h', 'cc', 'hh']:
    files |= set(glob.glob(os.path.join('**', f'*.{ext}'), recursive=True))
    files -= set(glob.glob(os.path.join('vendor', '**', f'*.{ext}'), recursive=True))
    files -= set(glob.glob(os.path.join('build', '**', f'*.{ext}'), recursive=True))

process_args = ['clang-format']
if args.ci:
    process_args += ['--dry-run', '-Werror']
else:
    process_args += ['-i']
process_args += files
try:
    proc = subprocess.run(process_args)
except KeyboardInterrupt:
    returncode = 130 # 128 + SIGINT
else:
    returncode = proc.returncode
if returncode != 0:
    raise SystemExit(returncode)
