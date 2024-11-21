#!/usr/bin/env python3


from argparse import ArgumentParser
import itertools
import math
from PIL import Image


parser = ArgumentParser()
parser.add_argument('in_path')
parser.add_argument('out_path')
args = parser.parse_args()

with Image.open(args.in_path) as in_image:
    w, h = in_image.size
    in_data = in_image.getdata()
    out_data = [(0, 0)] * w * h
    for x, y in itertools.product(range(w), range(h)):
        p = in_data[y * w + x]
        if p == (0, 0) or p == (255, 255):
            out_data[y * w + x] = p
            continue
        best = None
        for other_x, other_y in itertools.product(range(w), range(h)):
            p = in_data[other_y * w + other_x]
            l, a = p
            if a != 255:
                continue
            if p == (0, 255):
                continue
            sq_dist = (other_x - x) ** 2 + (other_y - y) ** 2
            dist = math.sqrt(sq_dist)
            dist += (255 - l) / 255
            if best is None:
                best = (dist, p)
            else:
                best_dist, _ = best
                if dist < best_dist:
                    best = (dist, p)
        dist, p = best
        l, a = p
        if dist < 2:
            out_data[y * w + x] = (255 - int((dist - 0.25) * 255), 255)
        else:
            out_data[y * w + x] = (0, int((3.75 - dist) * 255))
    out_image = Image.new('LA', in_image.size)
    out_image.putdata(out_data)
    out_image.save(args.out_path)
