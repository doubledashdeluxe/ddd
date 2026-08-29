#!/usr/bin/env python3


import argparse
import glob
import io
import itertools
import json
import os
import subprocess
import sys
import tempfile

from vendor.ninja_syntax import Writer

def is_windows():
    return ('win' in sys.platform and sys.platform != 'darwin') or 'msys' in sys.platform

def get_llvm_tool(tool, args):
    llvm_dir = args.llvm_dir
    if llvm_dir is None:
        return tool
    return os.path.join(llvm_dir, 'bin', tool)

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
        if platform == 'cube':
            flags += [
                '-d', 'CUBE',
            ]
        if target == 'bootstrap':
            if args.dolphin_force_gamecube:
                flags += [
                    '-d', 'DOLPHIN_FORCE_GAMECUBE',
                ]
        if target == 'payload':
            flags += [
                '-Ipayload',
            ]
        if args.ci:
            flags += [
                '-w', 'error',
            ]
    if tool == 'c':
        flags += [
            '-lang', 'c99',
        ]
    if tool == 'cc':
        flags += [
            '-d', 'EXPLICIT=',
            '-d', 'override=',
        ]
        for code_dir in format_code_dirs:
            flags += [
                f'-I{code_dir}'
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
        if platform == 'cube':
            flags += [
                '-nostdlib',
                '--target=ppc32-none-eabi',
            ]
        if args.ci:
            flags += [
                '-Werror',
            ]
    if tool == 'mc' or tool == 'mcc':
        flags += [
            '-D', 'EXPLICIT=explicit',
            '-fdata-sections',
            '-ffunction-sections',
            '-fno-sanitize-recover=all',
            '-g',
            '-isystem', '.',
            '-isystem', 'vendor',
            '-O2',
            '-Wall',
            '-Werror=vla',
            '-Wextra',
            '-Wimplicit-fallthrough',
        ]
        if platform == 'cube':
            flags += [
                '-D', 'CUBE',
                '-isystem', 'libc',
            ]
            if target == 'bootstrap':
                if args.dolphin_force_gamecube:
                    flags += [
                        '-D', 'DOLPHIN_FORCE_GAMECUBE',
                    ]
        if platform == 'native':
            flags += [
                '-fsanitize=address,fuzzer,undefined',
            ]
            if is_windows():
                flags += [
                    '-D', '_CRT_SECURE_NO_WARNINGS',
                    '-D', '_DISABLE_STL_ANNOTATION',
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
            '-Wno-overloaded-virtual',
            '-Wno-unused-private-field',
            '-Wold-style-cast',
            '-Wsuggest-override',
        ]
        for code_dir in format_code_dirs:
            flags += ['-isystem', code_dir]
        if platform == 'cube':
            flags += [
                '-fno-exceptions',
            ]
    if tool == 'mld':
        flags += [
            '-fuse-ld=lld',
        ]
        if platform == 'cube':
            flags += [
                '-Wl,--gc-sections',
            ]
        if platform == 'native':
            flags += [
                '-fsanitize=address,undefined',
            ]
        if platform == 'native':
            if is_windows():
                flags += [
                    '-Wl,/opt:nolldtailmerge',
                    '-Wl,/opt:ref',
                ]
            elif sys.platform == 'darwin':
                flags += [
                    '-Wl,-dead_strip',
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
parser.add_argument('--clean', action='store_true')
parser.add_argument('--ci', action='store_true')
parser.add_argument('--llvm-dir')
parser.add_argument('--dolphin-force-gamecube', action='store_true')
args = parser.parse_args(our_argv)

out_buf = io.StringIO()
n = Writer(out_buf)

n.variable('ninja_required_version', '1.3')
n.newline()

n.variable('builddir', 'build')
n.newline()

n.variable('bin2c', os.path.join('tools', 'bin2c.py'))
n.variable('cp', os.path.join('tools', 'cp.py'))
n.variable('dir2arc', os.path.join('tools', 'dir2arc.py'))
n.variable('ddd-formats', os.path.join('target', 'release', 'ddd-formats'))
n.variable('file_patcher', os.path.join('tools', 'file_patcher.py'))
n.variable('meta', os.path.join('tools', 'meta.py'))
if is_windows():
    n.variable('mwcc', os.path.join('tools', 'cw', 'modified_mwcceppc'))
else:
    n.variable('mwcc', os.path.join('target', 'release', 'mwcc'))
n.variable('obj2bin', os.path.join('target', 'release', 'obj2bin'))
n.variable('obj2dol', os.path.join('target', 'release', 'obj2dol'))
n.variable('patch', os.path.join('target', 'release', 'patch'))
n.variable('port', os.path.join('tools', 'port.py'))
n.variable('script', os.path.join('tools', 'script.py'))
n.variable('split', os.path.join('target', 'release', 'split'))
n.variable('zip', os.path.join('target', 'release', 'zip'))
n.newline()

n.rule(
    'bin2c',
    command = f'{sys.executable} $bin2c $name $in $out',
    description = 'BIN2C $out',
)
n.newline()

n.rule(
    'c',
    command = '$mwcc -MDfile $out.d $flags -c $in -o $out',
    depfile = '$out.d',
    deps = 'gcc',
    description = 'C $out',
)
n.newline()

n.rule(
    'cargo',
    command = 'cargo -q --color always build -r --bin $target',
    description = 'CARGO $out',
    restat = True,
)
n.newline()

n.rule(
    'cc',
    command = '$mwcc -MDfile $out.d $flags -c $in -o $out',
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
    'format',
    command = '$ddd-formats --$format --hh --output $out',
    description = 'FORMAT $out',
)
n.newline()

ld = get_llvm_tool('ld.lld', args)
if is_windows():
    ld += '.exe'
n.rule(
    'ld',
    command = f'{ld} $flags $in -o $out',
    description = 'LD $out',
)
n.newline()

mc = get_llvm_tool('clang', args)
n.rule(
    'mc',
    command = f'{mc} -MD -MT $out -MF $out.d $flags -c $in -o $out',
    depfile = '$out.d',
    deps = 'gcc',
    description = 'MC $out',
)
n.newline()

mcc = get_llvm_tool('clang++', args)
n.rule(
    'mcc',
    command = f'{mcc} -MD -MT $out -MF $out.d $flags -c $in -o $out',
    depfile = '$out.d',
    deps = 'gcc',
    description = 'MCC $out',
)
n.newline()

n.rule(
    'meta',
    command = f'{sys.executable} $meta $in $out',
    description = 'META $out',
)
n.newline()

mld = get_llvm_tool('clang++', args)
n.rule(
    'mld',
    command = f'{mld} $flags $in -o $out',
    description = 'MLD $out',
)
n.newline()

n.rule(
    'obj2bin',
    command = '$obj2bin -$kind $in $out',
    description = 'OBJ2BIN $out',
)
n.newline()

n.rule(
    'obj2dol',
    command = '$obj2dol $in $out',
    description = 'OBJ2DOL $out',
)
n.newline()

n.rule(
    'patch',
    command = '$patch $in $out',
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

n.rule(
    'split',
    command = '$split $in $out',
    description = 'SPLIT $out',
)
n.newline()

n.rule(
    'zip',
    command = '$zip -- $in_dir $out $key',
    description = 'ZIP $out',
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

cargo_dirs = {
    'formats': ['ddd-formats'],
    'tools': [
        'mwcc',
        'obj2bin',
        'obj2dol',
        'patch',
        'split',
        'zip',
    ],
}
for cargo_dir in cargo_dirs:
    for target in cargo_dirs[cargo_dir]:
        n.build(
            os.path.join('target', 'release', target),
            'cargo',
            [
                *sorted(glob.glob(os.path.join(cargo_dir, '**'), recursive=True)),
                os.path.join('data', 'version.txt'),
            ],
            variables = {
                'target': target,
            },
        )

compile_commands = []

repo_dir = os.path.dirname(os.path.abspath(__file__))

asset_arc_files = []
for in_dir in sorted(glob.glob(os.path.join('data', 'archives', '*'))):
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

format_kc_names = ['online', 'version']
format_code_dirs = []
format_hh_files = []
for format_kc_name in format_kc_names:
    format_sc_name = format_kc_name.replace('-', '_')
    code_dir = os.path.join('$builddir', 'formats', format_sc_name)
    format_code_dirs += [code_dir]
    format_pc_name = format_kc_name.replace('-', ' ').title().replace(' ', '')
    hh_file = os.path.join(code_dir, 'formats', f'{format_pc_name}.hh')
    format_hh_files += [hh_file]
    _, ext = os.path.splitext(hh_file)
    n.build(
        hh_file,
        'format',
        implicit = '$ddd-formats',
        variables = {
            'format': format_kc_name,
        },
    )
    n.newline()

code_in_files = {
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
for c_file in asset_c_files:
    code_in_files['channel'] += [c_file]
for region in ['P', 'E', 'J']:
    for kind in ['I', 'D']:
        in_file = os.path.join('$builddir', 'payload', f'payload{region}{kind}.c')
        code_in_files['channel'] += [in_file]
code_in_files['bootstrap'] += [os.path.join('$builddir', 'channel', 'channel.c')]
code_out_files = {target: [] for target in code_in_files}
for target in code_in_files:
    for in_file in code_in_files[target]:
        _, ext = os.path.splitext(in_file)
        if in_file.startswith('$builddir'):
            tmp_file = in_file + '.o'
        else:
            tmp_file = os.path.join('$builddir', in_file + '.o')
        tool = ext[1:]
        n.build(
            tmp_file,
            tool,
            in_file,
            variables = {
                'flags': get_flags(tool, 'cube', target, format_code_dirs, args),
            },
            order_only = format_hh_files,
            implicit = [
                os.path.join('tools', 'cw', 'modified_mwcceppc.exe'),
                '$mwcc',
            ],
        )
        n.newline()
        base, _ = os.path.splitext(tmp_file)
        out_file = base + '.split.o'
        code_out_files[target] += [out_file]
        n.build(
            out_file,
            'split',
            tmp_file,
            implicit = '$split',
        )

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
            'obj2bin',
            os.path.join('$builddir', 'payload', f'payload{region}.elf'),
            variables = {
                'kind': kind.lower(),
            },
            implicit = '$obj2bin',
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
    'obj2dol',
    os.path.join('$builddir', 'channel', 'channel.elf'),
    implicit = '$obj2dol',
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
    os.path.join('$builddir', 'ddd', 'boot.dol'),
    'obj2dol',
    os.path.join('$builddir', 'bootstrap', 'bootstrap.elf'),
    implicit = '$obj2dol',
)
n.newline()

n.build(
    os.path.join('$builddir', 'ddd', 'meta.xml'),
    'meta',
    [
        os.path.join('data', 'version.txt'),
        os.path.join('data', 'meta.xml.template'),
    ],
    implicit = '$meta',
)
n.newline()

for in_file in ['version.txt', 'changelog.txt']:
    n.build(
        os.path.join('$builddir', 'ddd', in_file),
        'cp',
        os.path.join('data', in_file),
    )
    n.newline()

license_out_files = []
for in_file in sorted(glob.glob(os.path.join('vendor', '*', 'LICEN[CS]E*'))):
    out_file = os.path.join('$builddir', 'ddd', 'licenses', os.path.relpath(in_file, 'vendor'))
    license_out_files += [out_file]
    n.build(
        out_file,
        'cp',
        in_file,
        implicit = '$cp',
    )
for in_file in ['LICENSE.md', 'NOTICE.md']:
    out_file = os.path.join('$builddir', 'ddd', 'licenses', 'ddd', in_file)
    license_out_files += [out_file]
    n.build(
        out_file,
        'cp',
        in_file,
        implicit = '$cp',
    )
n.newline()

ddd_out_files = [
    os.path.join('$builddir', 'ddd', 'boot.dol'),
    os.path.join('$builddir', 'ddd', 'meta.xml'),
    os.path.join('$builddir', 'ddd', 'version.txt'),
    os.path.join('$builddir', 'ddd', 'changelog.txt'),
    *license_out_files,
]
n.build(
    os.path.join('$builddir', 'update.zip'),
    'zip',
    ddd_out_files,
    variables = {
        'in_dir': os.path.join('$builddir', 'ddd'),
        'key': os.path.join('data', 'keys', 'update-ed25519-dev.bin'),
    },
    implicit = '$zip',
)
n.newline()

n.build(
    'ddd',
    'phony',
    os.path.join('$builddir', 'update.zip'),
)
n.newline()

native_code_in_files = {
    'vendor': None,
    'portable': None,
    'native': None,
    'tests': None,
    'fuzzers': None,
}
for target in native_code_in_files:
    in_files = native_code_in_files[target]
    if in_files is None:
        native_code_in_files[target] = [
            *sorted(glob.glob(os.path.join(target, '**', '*.c'), recursive=True)),
            *sorted(glob.glob(os.path.join(target, '**', '*.cc'), recursive=True)),
        ]
native_code_out_files = {target: [] for target in native_code_in_files}
for target in native_code_in_files:
    for in_file in native_code_in_files[target]:
        _, ext = os.path.splitext(in_file)
        if in_file.startswith('$builddir'):
            out_file = in_file.replace('$builddir', os.path.join('$builddir', 'native')) + '.o'
        else:
            out_file = os.path.join('$builddir', 'native', in_file + '.o')
        native_code_out_files[target] += [out_file]
        tool = f'm{ext[1:]}'
        flags = get_flags(tool, 'native', target, format_code_dirs, args)
        n.build(
            out_file,
            tool,
            in_file,
            variables = {
                'flags': flags,
            },
            order_only = format_hh_files,
        )
        n.newline()
        arguments = [
            {'c': 'clang', 'cc': 'clang++'}[ext[1:]],
            '-MD',
            '-MT',
            out_file,
            '-MF',
            f'{out_file}.d',
            *flags,
            '-c',
            in_file,
            '-o',
            out_file,
        ]
        compile_command = {
            'directory': repo_dir,
            'file': in_file,
            'arguments': arguments,
            'output': out_file,
        }
        compile_commands += [compile_command]

test_binary = os.path.join('$builddir', 'tests')
if is_windows():
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

fuzzer_binaries = []
for out_file in native_code_out_files['fuzzers']:
    base, _ = os.path.splitext(out_file)
    base, _ = os.path.splitext(base)
    target = out_file.split(os.path.sep)[3]
    fuzzer_binary = os.path.join('$builddir', os.path.join(*base.split(os.path.sep)[2:]))
    if is_windows():
        fuzzer_binary += '.exe'
    fuzzer_binaries += [fuzzer_binary]
    n.build(
        fuzzer_binary,
        'mld',
        [
            *native_code_out_files['vendor'],
            *native_code_out_files['portable'],
            *native_code_out_files['native'],
            out_file,
        ],
        variables = {
            'flags': [
                *get_flags('mld', 'native', target, format_code_dirs, args),
                '-fsanitize=fuzzer',
            ],
        },
    )
    n.newline()

n.build(
    'fuzzers',
    'phony',
    fuzzer_binaries,
)
n.newline()

check_code_in_files = {
    'vendor': None,
    'libc': None,
    'portable': None,
    'cube': None,
    'freestanding': None,
    'bootstrap': None,
    'channel': None,
    'payload': None,
}
for target in check_code_in_files:
    in_files = check_code_in_files[target]
    if in_files is None:
        check_code_in_files[target] = [
            *sorted(glob.glob(os.path.join(target, '**', '*.c'), recursive=True)),
            *sorted(glob.glob(os.path.join(target, '**', '*.cc'), recursive=True)),
        ]
for c_file in asset_c_files:
    check_code_in_files['channel'] += [c_file]
check_code_out_files = {target: [] for target in check_code_in_files}
for target in check_code_in_files:
    for in_file in check_code_in_files[target]:
        _, ext = os.path.splitext(in_file)
        if in_file.startswith('$builddir'):
            out_file = in_file.replace('$builddir', os.path.join('$builddir', 'checks')) + '.o'
        else:
            out_file = os.path.join('$builddir', 'checks', in_file + '.o')
        check_code_out_files[target] += [out_file]
        tool = f'm{ext[1:]}'
        flags = get_flags(tool, 'cube', target, format_code_dirs, args)
        n.build(
            out_file,
            tool,
            in_file,
            variables = {
                'flags': flags,
            },
            order_only = format_hh_files,
        )
        n.newline()
        arguments = [
            {'c': 'clang', 'cc': 'clang++'}[ext[1:]],
            '-MD',
            '-MT',
            out_file,
            '-MF',
            f'{out_file}.d',
            *flags,
            '-c',
            in_file,
            '-o',
            out_file,
        ]
        compile_command = {
            'directory': repo_dir,
            'file': in_file,
            'arguments': arguments,
            'output': out_file,
        }
        compile_commands += [compile_command]

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
n.newline()

n.build(
    'all',
    'phony',
    [
        'ddd',
        'tests',
        'fuzzers',
        'checks',
    ]
)
n.newline()

default = ['ddd', 'tests', 'fuzzers']
if not is_windows():
    default += ['checks']
n.default(default)

if args.dry:
    with open('build.ninja', 'w') as out_file:
        out_file.write(out_buf.getvalue())

    raise SystemExit

out_file = tempfile.NamedTemporaryFile('w+', delete=False)
out_file.write(out_buf.getvalue())
out_file.close()
n.close()

if args.clean:
    if not ('-t', 'clean') in itertools.pairwise(ninja_argv):
        ninja_argv.extend(['-t', 'clean'])

try:
    proc = subprocess.run(('ninja', '-f', out_file.name, *ninja_argv))
except KeyboardInterrupt:
    returncode = 130 # 128 + SIGINT
else:
    returncode = proc.returncode

with open(os.path.join('build', 'compile_commands.json'), 'w') as compile_commands_file:
    compile_commands = json.dumps(compile_commands, indent=4).replace('$builddir', 'build')
    compile_commands_file.write(compile_commands)

os.remove(out_file.name)
if returncode != 0:
    raise SystemExit(returncode)
