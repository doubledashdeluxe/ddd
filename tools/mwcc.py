#!/usr/bin/env python3


import os
import pathlib
import subprocess
import sys
import tempfile


os.environ['WINEDEBUG'] = '-all'

argv = sys.argv[1:]
for i in range(len(sys.argv) - 1):
    if argv[i] in ['-Mfile', '-MMfile', '-MDfile', '-MMDfile']:
        makefile_path = pathlib.Path(argv[i + 1])
        break

with tempfile.TemporaryFile() as stdout:
    with tempfile.TemporaryFile() as stderr:
        returncode = subprocess.run(
            ['wine', os.path.join('tools', 'cw', 'modified_mwcceppc.exe'), *argv],
            stdout = stdout,
            stderr = stderr,
        ).returncode
        stdout.seek(0)
        sys.stdout.buffer.write(stdout.read())
        sys.stdout.buffer.flush()
        stderr.seek(0)
        sys.stdout.buffer.write(stderr.read())
        sys.stdout.buffer.flush()
        if returncode != 0:
            sys.exit(returncode)

if makefile_path is not None:
    makefile_in = makefile_path.read_text()
    makefile_out = ''
    while makefile_in != '':
        if makefile_in.startswith('Z:'):
            path = makefile_in.split(None, 1)[0]
            makefile_out += subprocess.check_output(
                ['winepath', '-u', path],
                encoding = 'utf-8',
            ).rstrip('\n')
            makefile_in = makefile_in.split(path, 1)[1]
        else:
            makefile_out += makefile_in[0]
            makefile_in = makefile_in[1:]
    makefile_path.write_text(makefile_out)
