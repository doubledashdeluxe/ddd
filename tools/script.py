#!/usr/bin/env python3


from argparse import ArgumentParser
from string import Template


parser = ArgumentParser()
parser.add_argument('in_common_symbols_path')
parser.add_argument('in_target_symbols_path')
parser.add_argument('in_template_path')
parser.add_argument('out_script_path')
args = parser.parse_args()

symbols = []
for in_symbols_path in [args.in_target_symbols_path, args.in_common_symbols_path]:
    with open(in_symbols_path, 'r') as in_symbols:
        symbols += [[]]
        for in_symbol in in_symbols.readlines():
            if in_symbol.isspace():
                symbols[-1] += ['']
                continue
            address, name = in_symbol.split()
            address = int(address, 16)
            symbols[-1] += [f'    {name} = {address:#x};']
symbols = ['\n'.join(part) for part in symbols if part != []]
symbols = '\n\n'.join(symbols)

with open(args.in_template_path, 'r') as in_template:
    template = Template(in_template.read())

with open(args.out_script_path, 'w') as out_script:
    out_script.write(template.substitute(symbols = symbols))
