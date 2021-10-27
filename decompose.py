#!/usr/bin/env python3

from PIL import Image
import sys


def decompose(pix):
    r, g, b, *a = pix
    m = (r + g + b) / 3
    return (
        0 if r <= max(g, b) else int(255 * (r / m - 1)),
        0 if g <= max(b, r) else int(255 * (g / m - 1)),
        0 if b <= max(r, g) else int(255 * (b / m - 1)),
    )


infile, outfile = sys.argv[1:]
img = Image.open(infile)
new = Image.new("RGB", (img.width, img.height))

new.putdata(list(map(decompose, img.getdata())))
new.save(outfile)
