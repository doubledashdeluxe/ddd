#!/usr/bin/env python3


import argparse
import glob
import io
import os
import subprocess
import sys
import tempfile

from vendor.ninja_syntax import Writer


our_argv = []
ninja_argv = []
found_seperator = False
for arg in sys.argv[1:]:
    if found_seperator:
        ninja_argv.append(arg)
    elif arg == '--':
        found_seperator = True
    else:
        our_argv.append(arg)

parser = argparse.ArgumentParser()
parser.add_argument('--dry', action='store_true')
parser.add_argument('--ci', action='store_true')
args = parser.parse_args(our_argv)

out_buf = io.StringIO()
n = Writer(out_buf)

n.variable('ninja_required_version', '1.3')
n.newline()

n.variable('builddir', 'build')
n.variable('outdir', 'out')
n.newline()

n.variable('bin2c', os.path.join('tools', 'bin2c.py'))
n.variable('cp', os.path.join('tools', 'cp.py'))
n.variable('elf2bin', os.path.join('tools', 'elf2bin.py'))
n.variable('elf2dol', os.path.join('tools', 'elf2dol.py'))
n.variable('file_patcher', os.path.join('tools', 'file_patcher.py'))
if 'win' in sys.platform or 'msys' in sys.platform:
    n.variable('mwcc', os.path.join('tools', 'cw', 'modified_mwcceppc.exe'))
else:
    n.variable('mwcc', os.path.join('tools', 'mwcc.py'))
n.variable('patch', os.path.join('tools', 'patch.py'))
n.variable('port', os.path.join('tools', 'port.py'))
n.variable('script', os.path.join('tools', 'script.py'))
n.newline()

common_cflags = [
    '-Cpp_exceptions', 'off',
    '-enum', 'int',
    '-fp', 'hard',
    '-gccdep',
    '-gccinc',
    '-I.',
    '-Ilibc',
    '-Ivendor',
    '-ipa', 'file',
    '-lang', 'c99',
    '-O4,p',
    '-pragma', '"cpp1x on"',
    '-pragma', '"no_register_save_helpers on"',
    '-rostr',
    '-RTTI', 'off',
    '-sdata', '0',
    '-sdata2', '0',
    '-w', 'most',
    '-w', 'noextracomma',
]
common_ccflags = [
    '-Cpp_exceptions', 'off',
    '-d', 'override=',
    '-enum', 'int',
    '-fp', 'hard',
    '-gccdep',
    '-gccinc',
    '-I.',
    '-Ilibc',
    '-Ivendor',
    '-ipa', 'file',
    '-O4,p',
    '-pragma', '"cpp1x on"',
    '-pragma', '"no_register_save_helpers on"',
    '-rostr',
    '-RTTI', 'off',
    '-sdata', '0',
    '-sdata2', '0',
    '-w', 'most',
    '-w', 'noextracomma',
]
common_ldflags = [
    '--entry=Start',
    '-n',
]
target_cflags = {
    'vendor': [],
    'libc': [],
    'common': [],
    'loader': [
        '-d', 'LOADER',
        '-Iloader',
    ],
    'payload': [
        '-d', 'PAYLOAD',
        '-Ipayload',
    ],
}
target_ccflags = {
    'vendor': [],
    'libc': [],
    'common': [],
    'loader': [
        '-d', 'LOADER',
        '-Iloader',
    ],
    'payload': [
        '-d', 'PAYLOAD',
        '-Ipayload',
    ],
}
common_ncflags = [
    '-fdata-sections',
    '-ffunction-sections',
    '-fsanitize=undefined',
    '-isystem', '.',
    '-isystem', 'vendor',
    '-O2',
    '-std=c2x',
    '-Wall',
    '-Werror=implicit-function-declaration',
    '-Werror=incompatible-pointer-types',
    '-Werror=vla',
    '-Wextra',
]
common_nccflags = [
    '-D', 'lest_FEATURE_AUTO_REGISTER',
    '-fdata-sections',
    '-ffunction-sections',
    '-fsanitize=undefined',
    '-fno-sanitize=vptr', # Order matters
    '-isystem', '.',
    '-isystem', 'vendor',
    '-O2',
    '-std=c++17',
    '-Wall',
    '-Werror=vla',
    '-Wextra',
    '-Wsuggest-override',
]
common_nldflags = [
    '-fsanitize=undefined',
    '-fno-sanitize=vptr',
]
target_ncflags = {
    'vendor': [],
    'libc': [],
    'common': [],
    'loader': [
        '-D', 'LOADER',
        '-iquote', 'loader',
        '-isystem', 'loader',
    ],
    'payload': [
        '-D', 'PAYLOAD',
        '-iquote', 'payload',
        '-isystem', 'payload',
    ],
}
target_nccflags = {
    'vendor': [],
    'libc': [],
    'common': [],
    'loader': [
        '-D', 'LOADER',
        '-iquote', 'loader',
        '-isystem', 'loader',
    ],
    'payload': [
        '-D', 'PAYLOAD',
        '-iquote', 'payload',
        '-isystem', 'payload',
    ],
}
if 'win' in sys.platform or 'msys' in sys.platform:
    common_ncflags += [
        '-fsanitize-undefined-trap-on-error',
    ]
    common_nccflags += [
        '-fsanitize-undefined-trap-on-error',
    ]
    common_nldflags += [
        '-fsanitize-undefined-trap-on-error',
    ]
else:
    common_ncflags += [
        '-fsanitize=address',
    ]
    common_nccflags += [
        '-fsanitize=address',
    ]
    common_nldflags += [
        '-fsanitize=address',
    ]
if args.ci:
    common_cflags += [
        '-w', 'error',
    ]
    common_ccflags += [
        '-w', 'error',
    ]
    common_ncflags += [
        '-Werror',
    ]
    common_nccflags += [
        '-Werror',
    ]

n.rule(
    'bin2c',
    command = f'{sys.executable} $bin2c $name $in $out',
    description = 'BIN2C $out',
)
n.newline()

if 'win' in sys.platform or 'msys' in sys.platform:
    c_command = '$mwcc -MDfile $out.d $cflags -c $in -o $out'
else:
    c_command = f'{sys.executable} $mwcc -MDfile $out.d $cflags -c $in -o $out'
n.rule(
    'c',
    command = c_command,
    depfile = '$out.d',
    deps = 'gcc',
    description = 'C $out',
)
n.newline()

if 'win' in sys.platform or 'msys' in sys.platform:
    cc_command = '$mwcc -MDfile $out.d $ccflags -c $in -o $out'
else:
    cc_command = f'{sys.executable} $mwcc -MDfile $out.d $ccflags -c $in -o $out'
n.rule(
    'cc',
    command = cc_command,
    depfile = '$out.d',
    deps = 'gcc',
    description = 'CC $out',
)
n.newline()

n.rule(
    'cp',
    command = f'{sys.executable} $cp $name $in $out',
    description = 'CP $out',
)
n.newline()

n.rule(
    'elf2bin',
    command = f'{sys.executable} $elf2bin $in $out',
    description = 'ELF2BIN $out',
)
n.newline()

n.rule(
    'elf2dol',
    command = f'{sys.executable} $elf2dol $in $out',
    description = 'ELF2DOL $out',
)
n.newline()

if 'win' in sys.platform or 'msys' in sys.platform:
    ld_command = 'ld.lld.exe $ldflags $in -o $out'
else:
    ld_command = 'ld.lld $ldflags $in -o $out'
n.rule(
    'ld',
    command = ld_command,
    description = 'LD $out',
)
n.newline()

n.rule(
    'nanopb',
    command = f'{sys.executable} $nanopb $in -I protobuf -L "#include <vendor/nanopb/%s>" -D build/protobuf -q',
    description = 'NANOPB $out',
)
n.newline()

if 'win' in sys.platform or 'msys' in sys.platform:
    nc_command = 'gcc.exe -MD -MT $out -MF $out.d $cflags -c $in -o $out'
else:
    nc_command = 'gcc -MD -MT $out -MF $out.d $cflags -c $in -o $out'
n.rule(
    'nc',
    command = nc_command,
    depfile = '$out.d',
    deps = 'gcc',
    description = 'NC $out',
)
n.newline()

if 'win' in sys.platform or 'msys' in sys.platform:
    ncc_command = 'g++.exe -MD -MT $out -MF $out.d $ccflags -c $in -o $out'
else:
    ncc_command = 'g++ -MD -MT $out -MF $out.d $ccflags -c $in -o $out'
n.rule(
    'ncc',
    command = ncc_command,
    depfile = '$out.d',
    deps = 'gcc',
    description = 'NCC $out',
)
n.newline()

if 'win' in sys.platform or 'msys' in sys.platform:
    nld_command = 'g++.exe $ldflags $in -o $out'
else:
    nld_command = 'g++ $ldflags $in -o $out'
n.rule(
    'nld',
    command = nld_command,
    description = 'NLD $out',
)
n.newline()

if 'win' in sys.platform or 'msys' in sys.platform:
    nobjcopy_command = 'objcopy.exe $objcopyflags $in $out'
else:
    nobjcopy_command = 'objcopy $objcopyflags $in $out'
n.rule(
    'nobjcopy',
    command = nobjcopy_command,
    description = 'NOBJCOPY $out',
)
n.newline()

n.rule(
    'patch',
    command = f'{sys.executable} $patch $in $out',
    description = 'PATCH $out'
)
n.newline()

n.rule(
    'patch_mwcceppc',
    command = f'{sys.executable} $file_patcher $in $out',
    description = 'PATCH_MWCCEPPC $out'
)

n.rule(
    'port',
    command = f'{sys.executable} $port P $region file $in $out',
    description = 'PORT $out'
)
n.newline()

n.rule(
    'script',
    command = f'{sys.executable} $script $in $out',
    description = 'SCRIPT $out',
)
n.newline()

n.build(
    os.path.join('tools', 'cw', 'modified_mwcceppc.exe'),
    'patch_mwcceppc',
    [
        os.path.join('tools', 'cw', 'patches.dif'),
        os.path.join('tools', 'cw', 'mwcceppc.exe'),
    ]
)
n.newline()

protobuf_proto_files = sorted(glob.glob('protobuf/*.proto'))
protobuf_h_files = []
protobuf_c_files = []
for proto_file in protobuf_proto_files:
    base, _ = os.path.splitext(proto_file)
    options_file = base + '.options'
    h_file = os.path.join('$builddir', base + '.pb.h')
    c_file = os.path.join('$builddir', base + '.pb.c')
    protobuf_h_files += [h_file]
    protobuf_c_files += [c_file]
    n.build(
        [
            h_file,
            c_file,
        ],
        'nanopb',
        proto_file,
        implicit = options_file,
    )
n.newline()

code_in_files = {
    'vendor': [
        *sorted(glob.glob(os.path.join('vendor', '**', '*.c'), recursive=True)),
        *sorted(glob.glob(os.path.join('vendor', '**', '*.cc'), recursive=True)),
    ],
    'libc': [
        *sorted(glob.glob(os.path.join('libc', '**', '*.c'), recursive=True)),
        *sorted(glob.glob(os.path.join('libc', '**', '*.cc'), recursive=True)),
    ],
    'common': [
        *sorted(glob.glob(os.path.join('common', '**', '*.c'), recursive=True)),
        *sorted(glob.glob(os.path.join('common', '**', '*.cc'), recursive=True)),
    ],
    'loader': [
        *sorted(glob.glob(os.path.join('loader', '**', '*.c'), recursive=True)),
        *sorted(glob.glob(os.path.join('loader', '**', '*.cc'), recursive=True)),
    ],
    'payload': [
        *sorted(glob.glob(os.path.join('payload', '**', '*.c'), recursive=True)),
        *sorted(glob.glob(os.path.join('payload', '**', '*.cc'), recursive=True)),
    ],
}
code_out_files = {target: [] for target in code_in_files}
for target in code_in_files:
    for in_file in code_in_files[target]:
        _, ext = os.path.splitext(in_file)
        out_file = os.path.join('$builddir', in_file + '.o')
        code_out_files[target] += [out_file]
        n.build(
            out_file,
            ext[1:],
            in_file,
            variables = {
                'cflags': ' '.join([
                    *common_cflags,
                    *target_cflags[target],
                ]),
                'ccflags': ' '.join([
                    *common_ccflags,
                    *target_ccflags[target],
                ]),
            },
            order_only = protobuf_h_files if target == 'payload' else [],
            implicit=os.path.join('tools', 'cw', 'modified_mwcceppc.exe'),
        )
        n.newline()

n.build(
    os.path.join('$builddir', 'payload', 'payload.o'),
    'ld',
    [
        *code_out_files['vendor'],
        *code_out_files['libc'],
        *code_out_files['common'],
        *code_out_files['payload'],
    ],
    variables = {
        'ldflags': ' '.join([
            *common_ldflags,
            '-r',
        ]),
    },
)
n.newline()

n.build(
    [
        os.path.join('$builddir', 'payload', 'Symbols.txt'),
        os.path.join('$builddir', 'payload', 'Patches.cc'),
    ],
    'patch',
    [
        os.path.join('$builddir', 'payload', f'payload.o'),
        os.path.join('payload', 'Symbols.txt'),
    ],
    implicit = '$patch',
)
n.newline()

n.build(
    os.path.join('$builddir', 'payload', f'Patches.cc.o'),
    'cc',
    os.path.join('$builddir', 'payload', f'Patches.cc'),
    variables = {
        'ccflags': ' '.join([
            *common_ccflags,
            *target_ccflags['payload'],
        ]),
    },
)
n.newline()

for region in ['P', 'E', 'J']:
    n.build(
        os.path.join('$builddir', 'payload', f'Symbols{region}.txt'),
        'port',
        os.path.join('$builddir', 'payload', 'Symbols.txt'),
        variables = {
            'region': region,
        },
        implicit = '$port',
    )
    n.newline()

for region in ['P', 'E', 'J']:
    n.build(
        os.path.join('$builddir', 'payload', f'GM4{region}.ld'),
        'script',
        [
            os.path.join('common', 'Symbols.txt'),
            os.path.join('$builddir', 'payload', f'Symbols{region}.txt'),
            os.path.join('payload', 'GM4.ld.template'),
        ],
        implicit = '$script',
    )
    n.newline()

for region in ['P', 'E', 'J']:
    image_base = {
        'P': '0x803ec140',
        'E': '0x803e2300',
        'J': '0x803fc920',
    }[region]
    n.build(
        os.path.join('$builddir', 'payload', f'payload{region}.elf'),
        'ld',
        [
            *code_out_files['vendor'],
            *code_out_files['libc'],
            *code_out_files['common'],
            *code_out_files['payload'],
            os.path.join('$builddir', 'payload', f'Patches.cc.o'),
        ],
        variables = {
            'ldflags': ' '.join([
                *common_ldflags,
                '--gc-sections',
                f'--image-base={image_base}',
                '-T', os.path.join('$builddir', 'payload', f'GM4{region}.ld'),
            ]),
        },
        implicit = os.path.join('$builddir', 'payload', f'GM4{region}.ld'),
    )
    n.newline()

for region in ['P', 'E', 'J']:
    n.build(
        os.path.join('$builddir', 'payload', f'payload{region}.bin'),
        'elf2bin',
        os.path.join('$builddir', 'payload', f'payload{region}.elf'),
        implicit = '$elf2bin',
    )
    n.newline()

for region in ['P', 'E', 'J']:
    n.build(
        os.path.join('$builddir', 'payload', f'payload{region}.c'),
        'bin2c',
        os.path.join('$builddir', 'payload', f'payload{region}.bin'),
        variables = {
            'name': f'payload{region}',
        },
        implicit = '$bin2c',
    )
    n.newline()

for region in ['P', 'E', 'J']:
    n.build(
        os.path.join('$builddir', 'payload', f'payload{region}.o'),
        'c',
        os.path.join('$builddir', 'payload', f'payload{region}.c'),
        variables = {
            'cflags': ' '.join([
                *common_ccflags,
                *target_ccflags['loader'],
            ]),
        },
    )
    n.newline()

n.build(
    os.path.join('$builddir', 'loader', 'GM4.ld'),
    'script',
    [
        os.path.join('common', 'Symbols.txt'),
        os.path.join('loader', 'Symbols.txt'),
        os.path.join('loader', 'GM4.ld.template'),
    ],
    implicit = '$script',
)
n.newline()

n.build(
    os.path.join('$builddir', 'loader', 'loader.elf'),
    'ld',
    [
        *code_out_files['vendor'],
        *code_out_files['libc'],
        *code_out_files['common'],
        *code_out_files['loader'],
        *[os.path.join('$builddir', 'payload', f'payload{region}.o') for region in ['P', 'E', 'J']],
    ],
    variables = {
        'ldflags' : ' '.join([
            *common_ldflags,
            '--gc-sections',
            '--image-base=0x80800000',
            '-T', os.path.join('$builddir', 'loader', 'GM4.ld'),
        ]),
    },
    implicit = os.path.join('$builddir', 'loader', 'GM4.ld'),
)
n.newline()

n.build(
    os.path.join('$outdir', 'boot.dol'),
    'elf2dol',
    os.path.join('$builddir', 'loader', 'loader.elf'),
    implicit = '$elf2dol',
)
n.newline()

n.build(
    os.path.join('$outdir', 'meta.xml'),
    'cp',
    os.path.join('meta.xml'),
    implicit = '$cp',
)
n.newline()

n.build(
    'app',
    'phony',
    [
        os.path.join('$outdir', 'boot.dol'),
        os.path.join('$outdir', 'meta.xml'),
    ],
)
n.newline()

native_code_in_files = {
    **code_in_files,
    'tests': [
        *sorted(glob.glob(os.path.join('tests', 'libc', '**', '*.cc'), recursive=True)),
        *sorted(glob.glob(os.path.join('tests', 'common', '**', '*.cc'), recursive=True)),
        *sorted(glob.glob(os.path.join('tests', 'loader', '**', '*.cc'), recursive=True)),
        *sorted(glob.glob(os.path.join('tests', 'payload', '**', '*.cc'), recursive=True)),
    ]
}
native_code_out_files = {target: [] for target in native_code_in_files}
for target in native_code_in_files:
    for in_file in native_code_in_files[target]:
        _, ext = os.path.splitext(in_file)
        out_file = os.path.join('$builddir', 'native', in_file + '.o')
        native_code_out_files[target] += [out_file]
        flags_target = in_file.split(os.path.sep)[1] if target == 'tests' else target
        n.build(
            out_file,
            f'n{ext[1:]}',
            in_file,
            variables = {
                'cflags': ' '.join([
                    *common_ncflags,
                    *target_ncflags[flags_target],
                ]),
                'ccflags': ' '.join([
                    *common_nccflags,
                    *target_nccflags[flags_target],
                ]),
            },
            order_only = protobuf_h_files if target == 'payload' else [],
        )
        n.newline()

native_weak_code_out_files = {target: [] for target in native_code_out_files}
for target in native_code_out_files:
    if target == 'tests':
        continue
    for out_file in native_code_out_files[target]:
        base, _ = os.path.splitext(out_file)
        weak_out_file = f'{base}W.o'
        native_weak_code_out_files[target] += [weak_out_file]
        n.build(
            weak_out_file,
            'nobjcopy',
            out_file,
            variables = {
                'objcopyflags': ' '.join([
                    '--weaken',
                ]),
            },
        )
    n.newline()

test_binaries = []
for out_file in native_code_out_files['tests']:
    base, _ = os.path.splitext(out_file)
    base, _ = os.path.splitext(base)
    target = out_file.split(os.path.sep)[3]
    test_binary = os.path.join('$outdir', os.path.join(*base.split(os.path.sep)[2:]))
    test_binaries += [test_binary]
    n.build(
        test_binary,
        'nld',
        [
            *native_weak_code_out_files['vendor'],
            *native_weak_code_out_files['common'],
            *(native_weak_code_out_files['loader'] if target == 'loader' else []),
            *(native_weak_code_out_files['payload'] if target == 'payload' else []),
            out_file,
        ],
        variables = {
            'ldflags': ' '.join([
                *common_nldflags,
                '-Wl,--gc-sections',
            ]),
        },
    )
    n.newline()

n.build(
    'tests',
    'phony',
    test_binaries,
)
n.newline()

if args.dry:
    with open('build.ninja', 'w') as out_file:
        out_file.write(out_buf.getvalue())

    raise SystemExit

out_file = tempfile.NamedTemporaryFile('w+', delete=False)
out_file.write(out_buf.getvalue())
out_file.close()
n.close()

try:
    proc = subprocess.run(('ninja', '-f', out_file.name, *ninja_argv))
except KeyboardInterrupt:
    returncode = 130 # 128 + SIGINT
else:
    returncode = proc.returncode

os.remove(out_file.name)
if returncode != 0:
    sys.exit(returncode)
