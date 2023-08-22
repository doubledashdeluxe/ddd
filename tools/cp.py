#!/usr/bin/env python3


from argparse import ArgumentParser
import shutil


parser = ArgumentParser()
parser.add_argument('in_path')
parser.add_argument('out_path')
args = parser.parse_args()

shutil.copyfile(args.in_path, args.out_path)
