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
common_ldflags = [
    '--entry=Start',
    '-n',
]
target_cflags = {
    'formats': [
        '-w', 'nounusedarg',
    ],
    'vendor': [],
    'libc': [],
    'common': [],
    'freestanding': [],
    'bootstrap': [],
    'channel': [],
    'payload': [
        '-Ipayload',
    ],
}
target_ccflags = {
    'formats': [
        '-w', 'nounusedarg',
    ],
    'vendor': [],
    'libc': [],
    'common': [],
    'freestanding': [],
    'bootstrap': [],
    'channel': [],
    'payload': [
        '-Ipayload',
    ],
}
common_ncflags = [
    '-fno-sanitize-recover=all',
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
    '-fcheck-new',
    '-fno-sanitize-recover=all',
    '-fsanitize=undefined',
    '-isystem', '.',
    '-isystem', 'vendor',
    '-O2',
    '-std=c++20',
    '-Wall',
    '-Werror=vla',
    '-Wextra',
    '-Wsuggest-override',
]
common_nldflags = [
    '-fsanitize=undefined',
]
target_ncflags = {
    'formats': [
        '-Wno-unused-parameter',
    ],
    'vendor': [],
    'libc': [],
    'common': [],
    'freestanding': [],
    'bootstrap': [],
    'channel': [],
    'payload': [
        '-iquote', 'payload',
        '-isystem', 'payload',
    ],
    'tests': [
        '-iquote', 'tests',
        '-isystem', 'tests',
    ],
    'helpers': [],
}
target_nccflags = {
    'formats': [
        '-Wno-unused-parameter',
    ],
    'vendor': [],
    'libc': [],
    'common': [],
    'freestanding': [],
    'bootstrap': [],
    'channel': [],
    'payload': [
        '-iquote', 'payload',
        '-isystem', 'payload',
    ],
    'tests': [
        '-iquote', 'tests',
        '-isystem', 'tests',
    ],
    'helpers': [],
}
if 'win' in sys.platform or 'msys' in sys.platform:
    common_ncflags += [
        '-flto=auto',
        '-fsanitize-undefined-trap-on-error',
    ]
    common_nccflags += [
        '-flto=auto',
        '-fno-sanitize=vptr',
        '-fsanitize-undefined-trap-on-error',
    ]
    common_nldflags += [
        '-flto=auto',
        '-fno-sanitize=vptr',
        '-fsanitize-undefined-trap-on-error',
    ]
else:
    common_ncflags += [
        '-fdata-sections',
        '-ffunction-sections',
        '-fsanitize=address',
    ]
    common_nccflags += [
        '-fdata-sections',
        '-ffunction-sections',
        '-fsanitize=address',
        '-Wno-unused-private-field',
    ]
    common_nldflags += [
        '-fsanitize=address',
        '-fuse-ld=lld',
        '-Wl,--gc-sections',
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
if args.dolphin_force_gamecube:
    target_cflags['bootstrap'] += [
        '-d', 'DOLPHIN_FORCE_GAMECUBE',
    ]
    target_ccflags['bootstrap'] += [
        '-d', 'DOLPHIN_FORCE_GAMECUBE',
    ]
    target_ncflags['bootstrap'] += [
        '-D', 'DOLPHIN_FORCE_GAMECUBE',
    ]
    target_nccflags['bootstrap'] += [
        '-D', 'DOLPHIN_FORCE_GAMECUBE',
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
    ld_command = 'ld.lld.exe $ldflags $in -o $out'
else:
    ld_command = 'ld.lld $ldflags $in -o $out'
n.rule(
    'ld',
    command = ld_command,
    description = 'LD $out',
)
n.newline()

if 'win' in sys.platform or 'msys' in sys.platform:
    nc_command = 'gcc.exe -MD -MT $out -MF $out.d $cflags -c $in -o $out'
else:
    nc_command = 'clang -MD -MT $out -MF $out.d $cflags -c $in -o $out'
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
    ncc_command = 'clang++ -MD -MT $out -MF $out.d $ccflags -c $in -o $out'
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
    nld_command = 'clang++ $ldflags $in -o $out'
n.rule(
    'nld',
    command = nld_command,
    description = 'NLD $out',
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
            'cflags': ' '.join([
                *common_ccflags,
                *target_ccflags['channel'],
            ]),
        },
    )
    n.newline()

format_kc_names = ['client-state', 'server-state']
format_implicit = sorted(glob.glob(os.path.join('formats', '**'), recursive=True))
format_hh_files = []
format_cc_files = []
for format_kc_name in format_kc_names:
    format_sc_name = format_kc_name.replace('-', '_')
    code_dir = os.path.join('$builddir', 'formats', format_sc_name)
    common_ccflags += [f'-I{code_dir}']
    common_nccflags += ['-isystem', code_dir]
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
    'freestanding': [
        *sorted(glob.glob(os.path.join('freestanding', '**', '*.c'), recursive=True)),
        *sorted(glob.glob(os.path.join('freestanding', '**', '*.cc'), recursive=True)),
    ],
    'bootstrap': [
        *sorted(glob.glob(os.path.join('bootstrap', '**', '*.c'), recursive=True)),
        *sorted(glob.glob(os.path.join('bootstrap', '**', '*.cc'), recursive=True)),
    ],
    'channel': [
        *sorted(glob.glob(os.path.join('channel', '**', '*.c'), recursive=True)),
        *sorted(glob.glob(os.path.join('channel', '**', '*.cc'), recursive=True)),
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
        if in_file.startswith('$builddir'):
            out_file = in_file + '.o'
        else:
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
                'cflags': ' '.join([
                    *common_ccflags,
                    *target_ccflags['channel'],
                ]),
            },
        )
        n.newline()

n.build(
    os.path.join('$builddir', 'channel', 'GM4.ld'),
    'script',
    [
        os.path.join('common', 'Symbols.txt'),
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
        *code_out_files['common'],
        *code_out_files['freestanding'],
        *code_out_files['channel'],
        *asset_o_files,
    ],
    variables = {
        'ldflags' : ' '.join([
            *common_ldflags,
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
        'cflags': ' '.join([
            *common_ccflags,
            *target_ccflags['bootstrap'],
        ]),
    },
)
n.newline()

n.build(
    os.path.join('$builddir', 'bootstrap', 'GM4.ld'),
    'script',
    [
        os.path.join('common', 'Symbols.txt'),
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
        *code_out_files['common'],
        *code_out_files['freestanding'],
        *code_out_files['bootstrap'],
        os.path.join('$builddir', 'channel', 'channel.o'),
    ],
    variables = {
        'ldflags' : ' '.join([
            *common_ldflags,
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
    **code_in_files,
    'tests': [
        *sorted(glob.glob(os.path.join('tests', 'libc', '**', '*.cc'), recursive=True)),
        *sorted(glob.glob(os.path.join('tests', 'common', '**', '*.cc'), recursive=True)),
        *sorted(glob.glob(os.path.join('tests', 'freestanding', '**', '*.cc'), recursive=True)),
        *sorted(glob.glob(os.path.join('tests', 'bootstrap', '**', '*.cc'), recursive=True)),
        *sorted(glob.glob(os.path.join('tests', 'channel', '**', '*.cc'), recursive=True)),
        *sorted(glob.glob(os.path.join('tests', 'payload', '**', '*.cc'), recursive=True)),
    ],
    'helpers': [
        *sorted(glob.glob(os.path.join('tests', 'helpers', '**', '*.cc'), recursive=True)),
    ],
}
native_code_out_files = {target: [] for target in native_code_in_files}
for target in native_code_in_files:
    for in_file in native_code_in_files[target]:
        _, ext = os.path.splitext(in_file)
        out_file = os.path.join('$builddir', 'native', in_file + '.o')
        native_code_out_files[target] += [out_file]
        n.build(
            out_file,
            f'n{ext[1:]}',
            in_file,
            variables = {
                'cflags': ' '.join([
                    *common_ncflags,
                    *target_ncflags[target],
                    *(target_ncflags[in_file.split(os.path.sep)[1]] if target == 'tests' else []),
                ]),
                'ccflags': ' '.join([
                    *common_nccflags,
                    *target_nccflags[target],
                    *(target_nccflags[in_file.split(os.path.sep)[1]] if target == 'tests' else []),
                ]),
            },
            order_only = format_hh_files,
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
            *native_code_out_files['vendor'],
            *native_code_out_files['common'],
            *(native_code_out_files['freestanding'] if target != 'payload' else []),
            *(native_code_out_files['bootstrap'] if target == 'bootstrap' else []),
            *(native_code_out_files['channel'] if target == 'channel' else []),
            *(native_code_out_files['payload'] if target == 'payload' else []),
            *native_code_out_files['helpers'],
            out_file,
        ],
        variables = {
            'ldflags': ' '.join(common_nldflags),
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
