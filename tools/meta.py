#!/usr/bin/env python3


from argparse import ArgumentParser
from string import Template


parser = ArgumentParser()
parser.add_argument('in_version_path')
parser.add_argument('in_template_path')
parser.add_argument('out_meta_path')
args = parser.parse_args()


with open(args.in_version_path, 'r') as in_version:
    version = in_version.read().strip()

with open(args.in_template_path, 'r') as in_template:
    template = Template(in_template.read())

with open(args.out_meta_path, 'w') as out_meta:
    out_meta.write(template.substitute(version = version))
