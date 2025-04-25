#!/usr/bin/env python3


import argparse
import glob
import io
import os
import subprocess
import sys
import tempfile

from vendor.ninja_syntax import Writer


def get_flags(tool, platform, target, format_code_dirs, args):
    flags = []
    if tool == 'c' or tool == 'cc':
        flags += [
            '-Cpp_exceptions', 'off',
            '-enum', 'int',
            '-fp', 'hard',
            '-gccdep',
            '-gccinc',
            '-I.',
            '-Ilibc',
            '-Ivendor',
            '-ipa', 'file',
            '-opt size,level=4,schedule,peephole',
            '-pragma', '"cpp1x on"',
            '-pragma', '"no_register_save_helpers on"',
            '-proc', '604e',
            '-rostr',
            '-RTTI', 'off',
            '-sdata', '0',
            '-sdata2', '0',
            '-w', 'most',
            '-w', 'noextracomma',
        ]
        if target == 'payload':
            flags += [
                '-Ipayload',
            ]
        if args.ci:
            flags += [
                '-w', 'error',
            ]
        if target == 'bootstrap':
            if args.dolphin_force_gamecube:
                flags += [
                    '-d', 'DOLPHIN_FORCE_GAMECUBE',
                ]
    if tool == 'c':
        flags += [
            '-lang', 'c99',
        ]
    if tool == 'cc':
        flags += [
            '-d', 'override=',
        ]
        for code_dir in format_code_dirs:
            flags += [
                f'-I{code_dir}'
            ]
        if target == 'formats':
            flags += [
                '-w', 'nounusedarg',
            ]
    if tool == 'ld':
        flags += [
            '--entry=Start',
            '-n',
        ]
        if args.ci:
            flags += [
                '--fatal-warnings',
            ]
    if tool == 'mc' or tool == 'mcc' or tool == 'mld':
        flags += [
            '-fsanitize=address,undefined',
        ]
        if args.ci:
            flags += [
                '-Werror',
            ]
    if tool == 'mc' or tool == 'mcc':
        flags += [
            '-fdata-sections',
            '-ffunction-sections',
            '-fno-sanitize-recover=all',
            '-isystem', '.',
            '-isystem', 'vendor',
            '-O2',
            '-Wall',
            '-Werror=vla',
            '-Wextra',
        ]
        if platform == 'cube':
            if target == 'bootstrap':
                if args.dolphin_force_gamecube:
                    flags += [
                        '-D', 'DOLPHIN_FORCE_GAMECUBE',
                    ]
        if target == 'payload':
            flags += [
                '-iquote', 'payload',
                '-isystem', 'payload',
            ]
    if tool == 'mc':
        flags += [
            '-std=c2x',
            '-Werror=implicit-function-declaration',
            '-Werror=incompatible-pointer-types',
        ]
    if tool == 'mcc':
        flags += [
            '-fcheck-new',
            '-std=c++20',
            '-Wno-unused-private-field',
            '-Wsuggest-override',
        ]
        for code_dir in format_code_dirs:
            flags += ['-isystem', code_dir]
        if target == 'formats':
            flags += [
                '-Wno-unused-parameter',
            ]
    if tool == 'mld':
        flags += [
            '-fuse-ld=lld',
        ]
        if 'win' in sys.platform or 'msys' in sys.platform:
            flags += [
                '-Wl,/opt:ref',
            ]
        else:
            flags += [
                '-Wl,--gc-sections',
            ]
    return flags


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
parser.add_argument('--dolphin-force-gamecube', action='store_true')
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
n.variable('dir2arc', os.path.join('tools', 'dir2arc.py'))
n.variable('elf2bin', os.path.join('tools', 'elf2bin.py'))
n.variable('elf2dol', os.path.join('tools', 'elf2dol.py'))
n.variable('file_patcher', os.path.join('tools', 'file_patcher.py'))
if 'win' in sys.platform or 'msys' in sys.platform:
    n.variable('mwcc', os.path.join('tools', 'cw', 'modified_mwcceppc'))
else:
    n.variable('mwcc', os.path.join('tools', 'mwcc.py'))
n.variable('patch', os.path.join('tools', 'patch.py'))
n.variable('port', os.path.join('tools', 'port.py'))
n.variable('script', os.path.join('tools', 'script.py'))
n.newline()

n.rule(
    'bin2c',
    command = f'{sys.executable} $bin2c $name $in $out',
    description = 'BIN2C $out',
)
n.newline()

if 'win' in sys.platform or 'msys' in sys.platform:
    c_command = '$mwcc -MDfile $out.d $flags -c $in -o $out'
else:
    c_command = f'{sys.executable} $mwcc -MDfile $out.d $flags -c $in -o $out'
n.rule(
    'c',
    command = c_command,
    depfile = '$out.d',
    deps = 'gcc',
    description = 'C $out',
)
n.newline()

if 'win' in sys.platform or 'msys' in sys.platform:
    cc_command = '$mwcc -MDfile $out.d $flags -c $in -o $out'
else:
    cc_command = f'{sys.executable} $mwcc -MDfile $out.d $flags -c $in -o $out'
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
    'dir2arc',
    command = f'{sys.executable} $dir2arc --compress $in_dir $out',
    description = 'DIR2ARC $out',
)
n.newline()

n.rule(
    'elf2bin',
    command = f'{sys.executable} $elf2bin $kind $in $out',
    description = 'ELF2BIN $out',
)
n.newline()

n.rule(
    'elf2dol',
    command = f'{sys.executable} $elf2dol $in $out',
    description = 'ELF2DOL $out',
)
n.newline()

n.rule(
    'format',
    command = 'cargo -q --color always run -r --bin ddd-formats -- --$format --$ext --output $out',
    description = 'FORMAT $out',
)
n.newline()

if 'win' in sys.platform or 'msys' in sys.platform:
    ld_command = 'ld.lld.exe $flags $in -o $out'
else:
    ld_command = 'ld.lld $flags $in -o $out'
n.rule(
    'ld',
    command = ld_command,
    description = 'LD $out',
)
n.newline()

n.rule(
    'mc',
    command = 'clang -MD -MT $out -MF $out.d $flags -c $in -o $out',
    depfile = '$out.d',
    deps = 'gcc',
    description = 'MC $out',
)
n.newline()

n.rule(
    'mcc',
    command = 'clang++ -MD -MT $out -MF $out.d $flags -c $in -o $out',
    depfile = '$out.d',
    deps = 'gcc',
    description = 'MCC $out',
)
n.newline()

mld_command = 'clang++ $flags $in -o $out'
n.rule(
    'mld',
    command = mld_command,
    description = 'MLD $out',
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

asset_arc_files = []
for in_dir in sorted(glob.glob(os.path.join('assets', '*'))):
    in_files = sorted(glob.glob(os.path.join(in_dir, '**'), recursive=True))
    arc_file = os.path.join('$builddir', in_dir + '.arc')
    asset_arc_files += [arc_file]
    n.build(
        arc_file,
        'dir2arc',
        in_files,
        variables = {
            'in_dir': in_dir,
        },
        implicit = '$dir2arc',
    )
    n.newline()

asset_c_files = []
for arc_file in asset_arc_files:
    c_file = os.path.splitext(arc_file)[0] + '.c'
    asset_c_files += [c_file]
    n.build(
        c_file,
        'bin2c',
        arc_file,
        variables = {
            'name': os.path.splitext(os.path.basename(arc_file))[0].lower() + 'Archive',
        },
        implicit = '$bin2c',
    )
    n.newline()

asset_o_files = []
for c_file in asset_c_files:
    o_file = os.path.splitext(c_file)[0] + '.o'
    asset_o_files += [o_file]
    n.build(
        o_file,
        'c',
        c_file,
        variables = {
            'flags': get_flags('c', 'cube', 'channel', [], args),
        },
    )
    n.newline()

format_kc_names = ['client-state', 'server-state']
format_implicit = sorted(glob.glob(os.path.join('formats', '**'), recursive=True))
format_code_dirs = []
format_hh_files = []
format_cc_files = []
for format_kc_name in format_kc_names:
    format_sc_name = format_kc_name.replace('-', '_')
    code_dir = os.path.join('$builddir', 'formats', format_sc_name)
    format_code_dirs += [code_dir]
    format_pc_name = format_kc_name.replace('-', ' ').title().replace(' ', '')
    hh_file = os.path.join(code_dir, 'formats', f'{format_pc_name}.hh')
    format_hh_files += [hh_file]
    cc_file = os.path.join(code_dir, 'formats', f'{format_pc_name}.cc')
    format_cc_files += [cc_file]
    for code_file in [hh_file, cc_file]:
        _, ext = os.path.splitext(code_file)
        n.build(
            code_file,
            'format',
            implicit = format_implicit,
            variables = {
                'format': format_kc_name,
                'ext': ext[1:],
            }
        )
        n.newline()

code_in_files = {
    'formats': format_cc_files,
    'vendor': None,
    'libc': None,
    'portable': None,
    'cube': None,
    'freestanding': None,
    'bootstrap': None,
    'channel': None,
    'payload': None,
}
for target in code_in_files:
    in_files = code_in_files[target]
    if in_files is None:
        code_in_files[target] = [
            *sorted(glob.glob(os.path.join(target, '**', '*.c'), recursive=True)),
            *sorted(glob.glob(os.path.join(target, '**', '*.cc'), recursive=True)),
        ]
code_out_files = {target: [] for target in code_in_files}
for target in code_in_files:
    for in_file in code_in_files[target]:
        _, ext = os.path.splitext(in_file)
        if in_file.startswith('$builddir'):
            out_file = in_file + '.o'
        else:
            out_file = os.path.join('$builddir', in_file + '.o')
        code_out_files[target] += [out_file]
        tool = ext[1:]
        n.build(
            out_file,
            tool,
            in_file,
            variables = {
                'flags': get_flags(tool, 'cube', target, format_code_dirs, args),
            },
            order_only = format_hh_files,
            implicit=os.path.join('tools', 'cw', 'modified_mwcceppc.exe'),
        )
        n.newline()

n.build(
    os.path.join('$builddir', 'payload', 'payload.o'),
    'ld',
    [
        *code_out_files['vendor'],
        *code_out_files['libc'],
        *code_out_files['portable'],
        *code_out_files['cube'],
        *code_out_files['payload'],
    ],
    variables = {
        'flags': ' '.join([
            *get_flags('ld', 'cube', 'payload', format_code_dirs, args),
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
        'flags': get_flags('cc', 'cube', 'payload', format_code_dirs, args),
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
            os.path.join('cube', 'Symbols.txt'),
            os.path.join('$builddir', 'payload', f'Symbols{region}.txt'),
            os.path.join('payload', 'GM4.ld.template'),
        ],
        implicit = '$script',
    )
    n.newline()

for region in ['P', 'E', 'J']:
    image_base = {
        'P': '0x802d84a0',
        'E': '0x802d8520',
        'J': '0x802d8540',
    }[region]
    n.build(
        os.path.join('$builddir', 'payload', f'payload{region}.elf'),
        'ld',
        [
            *code_out_files['formats'],
            *code_out_files['vendor'],
            *code_out_files['libc'],
            *code_out_files['portable'],
            *code_out_files['cube'],
            *code_out_files['payload'],
            os.path.join('$builddir', 'payload', f'Patches.cc.o'),
        ],
        variables = {
            'flags': ' '.join([
                *get_flags('ld', 'cube', 'payload', format_code_dirs, args),
                '--gc-sections',
                f'--image-base={image_base}',
                '-T', os.path.join('$builddir', 'payload', f'GM4{region}.ld'),
            ]),
        },
        implicit = os.path.join('$builddir', 'payload', f'GM4{region}.ld'),
    )
    n.newline()

for region in ['P', 'E', 'J']:
    for kind in ['I', 'D']:
        n.build(
            os.path.join('$builddir', 'payload', f'payload{region}{kind}.bin'),
            'elf2bin',
            os.path.join('$builddir', 'payload', f'payload{region}.elf'),
            variables = {
                'kind': kind,
            },
            implicit = '$elf2bin',
        )
        n.newline()

for region in ['P', 'E', 'J']:
    for kind in ['I', 'D']:
        n.build(
            os.path.join('$builddir', 'payload', f'payload{region}{kind}.c'),
            'bin2c',
            os.path.join('$builddir', 'payload', f'payload{region}{kind}.bin'),
            variables = {
                'name': f'payload{region}{kind}',
            },
            implicit = '$bin2c',
        )
        n.newline()

for region in ['P', 'E', 'J']:
    for kind in ['I', 'D']:
        out_file = os.path.join('$builddir', 'payload', f'payload{region}{kind}.o')
        code_out_files['channel'] += [out_file]
        n.build(
            out_file,
            'c',
            os.path.join('$builddir', 'payload', f'payload{region}{kind}.c'),
            variables = {
                'flags': get_flags('c', 'cube', 'channel', format_code_dirs, args),
            },
        )
        n.newline()

n.build(
    os.path.join('$builddir', 'channel', 'GM4.ld'),
    'script',
    [
        os.path.join('cube', 'Symbols.txt'),
        os.path.join('channel', 'Symbols.txt'),
        os.path.join('channel', 'GM4.ld.template'),
    ],
    implicit = '$script',
)
n.newline()

n.build(
    os.path.join('$builddir', 'channel', 'channel.elf'),
    'ld',
    [
        *code_out_files['vendor'],
        *code_out_files['libc'],
        *code_out_files['portable'],
        *code_out_files['cube'],
        *code_out_files['freestanding'],
        *code_out_files['channel'],
        *asset_o_files,
    ],
    variables = {
        'flags' : ' '.join([
            *get_flags('ld', 'cube', 'channel', format_code_dirs, args),
            '--gc-sections',
            '-T', os.path.join('$builddir', 'channel', 'GM4.ld'),
        ]),
    },
    implicit = os.path.join('$builddir', 'channel', 'GM4.ld'),
)
n.newline()

n.build(
    os.path.join('$builddir', 'channel', 'channel.dol'),
    'elf2dol',
    os.path.join('$builddir', 'channel', 'channel.elf'),
    implicit = '$elf2dol',
)
n.newline()

n.build(
    os.path.join('$builddir', 'channel', 'channel.c'),
    'bin2c',
    os.path.join('$builddir', 'channel', 'channel.dol'),
    variables = {
        'name': 'channel',
    },
    implicit = '$bin2c',
)
n.newline()

n.build(
    os.path.join('$builddir', 'channel', 'channel.o'),
    'c',
    os.path.join('$builddir', 'channel', 'channel.c'),
    variables = {
        'flags': get_flags('c', 'cube', 'bootstrap', format_code_dirs, args),
    },
)
n.newline()

n.build(
    os.path.join('$builddir', 'bootstrap', 'GM4.ld'),
    'script',
    [
        os.path.join('cube', 'Symbols.txt'),
        os.path.join('bootstrap', 'Symbols.txt'),
        os.path.join('bootstrap', 'GM4.ld.template'),
    ],
    implicit = '$script',
)
n.newline()

n.build(
    os.path.join('$builddir', 'bootstrap', 'bootstrap.elf'),
    'ld',
    [
        *code_out_files['vendor'],
        *code_out_files['libc'],
        *code_out_files['portable'],
        *code_out_files['cube'],
        *code_out_files['freestanding'],
        *code_out_files['bootstrap'],
        os.path.join('$builddir', 'channel', 'channel.o'),
    ],
    variables = {
        'flags' : ' '.join([
            *get_flags('ld', 'cube', 'bootstrap', format_code_dirs, args),
            '--gc-sections',
            '-T', os.path.join('$builddir', 'bootstrap', 'GM4.ld'),
        ]),
    },
    implicit = os.path.join('$builddir', 'bootstrap', 'GM4.ld'),
)
n.newline()

n.build(
    os.path.join('$outdir', 'boot.dol'),
    'elf2dol',
    os.path.join('$builddir', 'bootstrap', 'bootstrap.elf'),
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
    'ddd',
    'phony',
    [
        os.path.join('$outdir', 'boot.dol'),
        os.path.join('$outdir', 'meta.xml'),
    ],
)
n.newline()

native_code_in_files = {
    'vendor': [
        *sorted(glob.glob(os.path.join('vendor', '**', '*.c'), recursive=True)),
        *sorted(glob.glob(os.path.join('vendor', '**', '*.cc'), recursive=True)),
    ],
    'portable': [
        *sorted(glob.glob(os.path.join('portable', '**', '*.c'), recursive=True)),
        *sorted(glob.glob(os.path.join('portable', '**', '*.cc'), recursive=True)),
    ],
    'native': [
        *sorted(glob.glob(os.path.join('native', '**', '*.c'), recursive=True)),
        *sorted(glob.glob(os.path.join('native', '**', '*.cc'), recursive=True)),
    ],
    'tests': [
        *sorted(glob.glob(os.path.join('tests', '**', '*.cc'), recursive=True)),
    ],
}
native_code_out_files = {target: [] for target in native_code_in_files}
for target in native_code_in_files:
    for in_file in native_code_in_files[target]:
        _, ext = os.path.splitext(in_file)
        out_file = os.path.join('$builddir', 'native', in_file + '.o')
        native_code_out_files[target] += [out_file]
        tool = f'm{ext[1:]}'
        n.build(
            out_file,
            tool,
            in_file,
            variables = {
                'flags': get_flags(tool, 'native', target, format_code_dirs, args),
            },
            order_only = format_hh_files,
        )
        n.newline()

test_binary = os.path.join('$outdir', 'tests')
if 'win' in sys.platform or 'msys' in sys.platform:
    test_binary += '.exe'
n.build(
    test_binary,
    'mld',
    [
        *native_code_out_files['vendor'],
        *native_code_out_files['portable'],
        *native_code_out_files['native'],
        *native_code_out_files['tests'],
    ],
    variables = {
        'flags': get_flags('mld', 'native', target, format_code_dirs, args),
    },
)
n.newline()

n.build(
    'tests',
    'phony',
    test_binary,
)
n.newline()

check_code_in_files = code_in_files
check_code_out_files = {target: [] for target in check_code_in_files}
for target in check_code_in_files:
    for in_file in check_code_in_files[target]:
        _, ext = os.path.splitext(in_file)
        out_file = os.path.join('$builddir', 'checks', in_file + '.o')
        check_code_out_files[target] += [out_file]
        tool = f'm{ext[1:]}'
        n.build(
            out_file,
            tool,
            in_file,
            variables = {
                'flags': get_flags(tool, 'cube', target, format_code_dirs, args),
            },
            order_only = format_hh_files,
        )
        n.newline()

check_libraries = []
for target in ['bootstrap', 'channel', 'payload']:
    check_library = os.path.join('$builddir', 'checks', target, f'{target}.o')
    check_libraries += [check_library]
    n.build(
        check_library,
        'mld',
        [
            *check_code_out_files['vendor'],
            *check_code_out_files['portable'],
            *check_code_out_files['cube'],
            *(check_code_out_files['freestanding'] if target != 'payload' else []),
            *check_code_out_files[target],
        ],
        variables = {
            'flags': [
                *get_flags('mld', 'cube', target, format_code_dirs, args),
                '-r',
            ],
        },
    )

n.build(
    'checks',
    'phony',
    check_libraries,
)

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
