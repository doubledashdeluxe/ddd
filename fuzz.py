#!/usr/bin/env python3


import concurrent.futures
import glob
import os
import subprocess
import sys


def run_fuzzer_binary(fuzzer_binary):
    return subprocess.run([fuzzer_binary, *sys.argv[1:]], capture_output=True)


pattern = '*.exe' if 'win' in sys.platform or 'msys' in sys.platform else '*'
paths = glob.glob(os.path.join('out', 'fuzzers', '**', pattern), recursive=True)
fuzzer_binaries = [path for path in paths if os.path.isfile(path)]
with concurrent.futures.ThreadPoolExecutor() as executor:
    procs = executor.map(run_fuzzer_binary, fuzzer_binaries)
for proc in procs:
    print(proc.args[0], file=sys.stderr)
    print(proc.stderr.decode(), end='', file=sys.stderr)
for proc in procs:
    if proc.returncode != 0:
        raise SystemExit(proc.returncode)
