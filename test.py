#!/usr/bin/env python3


import build
import concurrent.futures
import subprocess
import sys


def run_test_binary(test_binary):
    test_binary = test_binary.replace('$outdir', 'out')
    return subprocess.run([test_binary], capture_output = True)


with concurrent.futures.ThreadPoolExecutor() as executor:
    procs = executor.map(run_test_binary, build.test_binaries)
print('', file = sys.stderr)
passed = 0
failed = 0
returncode = 0
for proc in procs:
    if proc.returncode == 0:
        passed += 1
    else:
        failed += 1
        print(proc.stderr.decode(), file = sys.stderr)
        if returncode == 0:
            returncode = proc.returncode
if failed == 0:
    print(f'All {passed} test suites passed.', file = sys.stderr)
else:
    print(f'{failed} out of {passed + failed} test suites failed.', file = sys.stderr)
sys.exit(returncode)
