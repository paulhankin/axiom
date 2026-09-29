#!/usr/bin/env python3
"""Generates std/image/testdata: images in GIF, TIFF, WebP and AVIF made
with Pillow (and its libraries: libtiff, libwebp, libavif), each with the
pixels Pillow decodes from it as PNG files to compare with.

    tools/gen_image_tests.py [gif|tiff|webp|avif ...]
"""
import os
import random
import sys

from PIL import Image, ImageDraw, ImageSequence

root = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'std', 'image', 'testdata')
rng = random.Random(1987)


def picture(w, h, mode='RGB', seed=0):
    """Shapes and gradients, so that codecs have something to do."""
    r = random.Random(seed)
    im = Image.new('RGB', (w, h))
    px = im.load()
    for y in range(h):
        for x in range(w):
            px[x, y] = ((x * 255) // max(w - 1, 1), (y * 255) // max(h - 1, 1), ((x + y) * 7) & 255)
    d = ImageDraw.Draw(im)
    for _ in range(6):
        x0, y0 = r.randrange(w), r.randrange(h)
        d.ellipse([x0, y0, x0 + r.randrange(4, w // 2 + 5), y0 + r.randrange(4, h // 2 + 5)],
                  fill=(r.randrange(256), r.randrange(256), r.randrange(256)))
    if mode == 'RGBA':
        im = im.convert('RGBA')
        a = Image.new('L', (w, h))
        ap = a.load()
        for y in range(h):
            for x in range(w):
                ap[x, y] = 0 if (x // 8 + y // 8) % 3 == 0 else (x * 4 + 60) & 255
        im.putalpha(a)
    elif mode != 'RGB':
        im = im.convert(mode)
    return im


def expect(path, im):
    im.save(path, 'PNG')


def gif():
    d = os.path.join(root, 'gif')
    os.makedirs(d, exist_ok=True)
    # a still image, in 256 colours; interlaced; 4 colours (a 2-bit code)
    p = picture(61, 45).quantize(256)
    p.save(os.path.join(d, 'still.gif'))
    p.save(os.path.join(d, 'interlaced.gif'), interlace=True)
    q = picture(33, 17).quantize(4)
    q.save(os.path.join(d, 'four.gif'))
    # transparency
    t = picture(40, 30).quantize(64)
    t.info['transparency'] = 5
    t.save(os.path.join(d, 'transparent.gif'), transparency=5)
    # an animation: frames smaller than the canvas, each disposal method
    frames = []
    for k in range(5):
        f = Image.new('RGBA', (48, 40), (0, 0, 0, 0))
        dr = ImageDraw.Draw(f)
        dr.rectangle([4 * k, 3 * k, 4 * k + 20, 3 * k + 15], fill=(50 * k, 255 - 40 * k, 100, 255))
        frames.append(f)
    frames[0].save(os.path.join(d, 'anim.gif'), save_all=True, append_images=frames[1:], duration=[100, 50, 70, 30, 200],
                   loop=0, disposal=[0, 1, 2, 3, 1], transparency=0)
    for name in os.listdir(d):
        if not name.endswith('.gif'):
            continue
        im = Image.open(os.path.join(d, name))
        for k, fr in enumerate(ImageSequence.Iterator(im)):
            expect(os.path.join(d, '%s.%d.png' % (name[:-4], k)), fr.convert('RGBA'))
        print(name, im.n_frames if hasattr(im, 'n_frames') else 1)


for what in sys.argv[1:] or ['gif']:
    globals()[what]()
