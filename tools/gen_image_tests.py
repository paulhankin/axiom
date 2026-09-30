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


def tiff():
    """TIFF files from tifffile (every layout and compression) and Pillow's
    libtiff (CCITT fax, JPEG). Each case has the pixels expected, as the raw
    samples of a std/image Image (little-endian), listed in cases.txt."""
    import numpy as np
    import tifffile
    d = os.path.join(root, 'tiff')
    os.makedirs(d, exist_ok=True)
    cases = []

    def arr(w, h, mode='RGB', seed=0):
        return np.asarray(picture(w, h, mode, seed))

    def case(name, want, depth, page=0, tol=0):
        want = np.asarray(want)
        if want.ndim == 2:
            want = want[:, :, None]
        h, w, c = want.shape
        if depth == 'u8':
            raw = want.astype('<u1')
        elif depth == 'u16':
            raw = want.astype('<u2')
        else:
            raw = want.astype('<f4')
        base = name.rsplit('.', 1)[0] + ('' if page == 0 else '.%d' % page)
        with open(os.path.join(d, base + '.raw'), 'wb') as f:
            f.write(raw.tobytes())
        cases.append('%s %s %d %d %d %s %d %d' % (name, base + '.raw', w, h, c, depth, page, tol))

    def write(name, data, **kw):
        tifffile.imwrite(os.path.join(d, name), data, **kw)

    rgb = arr(45, 37)
    write('rgb8.tif', rgb, photometric='rgb', rowsperstrip=7)
    case('rgb8.tif', rgb, 'u8')
    write('rgb8_lzw_be.tif', rgb, photometric='rgb', compression='lzw', predictor=2, byteorder='>',
          rowsperstrip=10)
    case('rgb8_lzw_be.tif', rgb, 'u8')
    rgba = arr(50, 41, 'RGBA', 1)
    write('rgba8_tiled.tif', rgba, photometric='rgb', compression='deflate', tile=(16, 16),
          extrasamples=[2])
    case('rgba8_tiled.tif', rgba, 'u8')
    # associated (premultiplied) alpha: comes back straight
    a = rgba[:, :, 3:].astype(np.int64)
    pre = rgba.astype(np.int64)
    pre[:, :, :3] = (pre[:, :, :3] * a + 127) // 255
    straight = pre.copy()
    nz = a[:, :, 0] > 0
    for c in range(3):
        ch = straight[:, :, c]
        ch[nz] = np.minimum(255, (pre[:, :, c][nz] * 255 + a[:, :, 0][nz] // 2) // a[:, :, 0][nz])
        ch[~nz] = 0
    write('rgba8_premul.tif', pre.astype(np.uint8), photometric='rgb', compression='packbits',
          extrasamples=[1])
    case('rgba8_premul.tif', straight, 'u8')
    gray = arr(39, 29, 'L', 2)
    write('gray8_packbits.tif', gray, photometric='minisblack', compression='packbits')
    case('gray8_packbits.tif', gray, 'u8')
    write('gray8_white.tif', 255 - gray, photometric='miniswhite')
    case('gray8_white.tif', gray, 'u8')
    ga = np.dstack([gray, (np.arange(39)[None, :] * 6 + np.arange(29)[:, None]).astype(np.uint8)])
    write('graya8.tif', ga, photometric='minisblack', extrasamples=[2], planarconfig='contig')
    case('graya8.tif', ga, 'u8')
    g16 = (gray.astype(np.uint16) * 257) ^ (np.arange(39, dtype=np.uint16)[None, :] * 3)
    write('gray16_lzw.tif', g16, photometric='minisblack', compression='lzw', predictor=2)
    case('gray16_lzw.tif', g16, 'u16')
    rgb16 = rgb.astype(np.uint16) * 257 + np.arange(45, dtype=np.uint16)[None, :, None]
    write('rgb16_planar_be.tif', np.moveaxis(rgb16, -1, 0), photometric='rgb', compression='deflate', planarconfig='separate',
          byteorder='>', rowsperstrip=8)
    case('rgb16_planar_be.tif', rgb16, 'u16')
    write('rgb16_planar_tiled.tif', np.moveaxis(rgb16, -1, 0), photometric='rgb', compression='lzw', planarconfig='separate',
          tile=(16, 32))
    case('rgb16_planar_tiled.tif', rgb16, 'u16')
    fl = (np.sin(np.arange(31 * 27).reshape(27, 31) / 7.0) * 3.5).astype(np.float32)
    write('gray32f_pred.tif', fl, photometric='minisblack', compression='deflate', predictor=3)
    case('gray32f_pred.tif', fl, 'f32')
    flrgb = (rgb.astype(np.float32) / 97.0 - 0.5)
    write('rgb32f_tiled.tif', flrgb, photometric='rgb', tile=(16, 16), byteorder='>')
    case('rgb32f_tiled.tif', flrgb, 'f32')
    write('gray16f.tif', fl.astype(np.float16), photometric='minisblack', compression='lzw')
    case('gray16f.tif', fl.astype(np.float16).astype(np.float32), 'f32')
    u32 = (gray.astype(np.uint32) << 24) | (np.arange(39, dtype=np.uint32)[None, :] * 999)
    write('gray32u.tif', u32, photometric='minisblack')
    case('gray32u.tif', u32 >> 16, 'u16')
    # 1, 2 and 4 bits
    bits = ((np.arange(53)[None, :] // 3 + np.arange(21)[:, None] // 2) % 2).astype(np.uint8)
    write('bilevel.tif', bits.astype(bool), photometric='minisblack')
    case('bilevel.tif', bits * 255, 'u8')
    write('bilevel_white.tif', (1 - bits).astype(bool), photometric='miniswhite', compression='packbits')
    case('bilevel_white.tif', bits * 255, 'u8')
    g4 = (gray >> 4).astype(np.uint8)
    write('gray4.tif', g4, photometric='minisblack', bitspersample=4)
    case('gray4.tif', g4 * 17, 'u8')
    g2 = (gray >> 6).astype(np.uint8)
    write('gray2.tif', g2, photometric='minisblack', bitspersample=2)
    case('gray2.tif', g2 * 85, 'u8')
    # palettes
    pal = picture(43, 35, 'RGB', 3).quantize(200)
    idx = np.asarray(pal)
    cmap = np.array(pal.getpalette()[:768] + [0] * (768 - len(pal.getpalette()[:768])),
                    dtype=np.uint16).reshape(256, 3)
    cm16 = (cmap * 257).T.copy()
    write('palette8.tif', idx, photometric='palette', colormap=cm16, compression='lzw')
    case('palette8.tif', cmap[idx].astype(np.uint8), 'u8')
    # CMYK: RGB = (255 - C) (255 - K) / 255, rounded
    cmyk = arr(33, 22, 'CMYK', 4)
    ci = cmyk.astype(np.int64)
    back = np.dstack([((255 - ci[:, :, c]) * (255 - ci[:, :, 3]) + 127) // 255 for c in range(3)])
    write('cmyk.tif', cmyk, photometric='separated', compression='lzw')
    case('cmyk.tif', back, 'u8')
    # BigTIFF; several pages
    write('bigtiff.tif', rgb, photometric='rgb', compression='lzw', bigtiff=True)
    case('bigtiff.tif', rgb, 'u8')
    pages = [arr(20 + 5 * k, 15 + 3 * k, 'RGB', 10 + k) for k in range(3)]
    with tifffile.TiffWriter(os.path.join(d, 'pages.tif')) as tw:
        for k, p in enumerate(pages):
            tw.write(p, photometric='rgb', compression=['none', 'lzw', 'deflate'][k])
    for k, p in enumerate(pages):
        case('pages.tif', p, 'u8', page=k)
    # made by libtiff (through Pillow): fax and JPEG
    fax = Image.fromarray((bits * 255).astype(np.uint8)).resize((301, 97)).convert('1', dither=Image.NONE)
    fbits = np.asarray(fax.convert('L'))
    for comp in ['group4', 'group3', 'tiff_ccitt']:
        fax.save(os.path.join(d, 'fax_%s.tif' % comp), compression=comp)
        case('fax_%s.tif' % comp, fbits, 'u8')
    text = Image.new('1', (211, 64), 1)
    ImageDraw.Draw(text).text((5, 5), 'Phalange TIFF\nGroup 4 fax', fill=0)
    text.save(os.path.join(d, 'fax_text.tif'), compression='group4')
    case('fax_text.tif', np.asarray(text.convert('L')), 'u8')
    photo = picture(64, 48, 'RGB', 5)
    photo.save(os.path.join(d, 'jpeg.tif'), compression='jpeg', quality=90)
    case('jpeg.tif', np.asarray(Image.open(os.path.join(d, 'jpeg.tif')).convert('RGB')), 'u8', tol=2)
    photo.convert('L').save(os.path.join(d, 'jpeg_gray.tif'), compression='jpeg')
    case('jpeg_gray.tif', np.asarray(Image.open(os.path.join(d, 'jpeg_gray.tif')).convert('L')), 'u8', tol=2)
    for comp in ['tiff_lzw', 'tiff_adobe_deflate', 'packbits']:
        photo.save(os.path.join(d, 'pil_%s.tif' % comp), compression=comp)
        case('pil_%s.tif' % comp, np.asarray(photo), 'u8')
    # check the expected pixels against Pillow where Pillow reads the file
    for line in cases:
        name, raw, w, h, c, depth, page, tol = line.split()
        if depth != 'u8':
            continue
        im = Image.open(os.path.join(d, name))
        im.seek(int(page))
        mode = {1: 'L', 2: 'LA', 3: 'RGB', 4: 'RGBA'}[int(c)]
        got = np.asarray(im.convert(mode)).astype(np.int64).reshape(int(h), int(w), int(c))
        want = np.frombuffer(open(os.path.join(d, raw), 'rb').read(), np.uint8).reshape(got.shape)
        diff = np.abs(got - want).max()
        if diff > max(int(tol), 1 if 'premul' in name or 'cmyk' in name else 0):
            print('Pillow disagrees:', name, page, diff)
    with open(os.path.join(d, 'cases.txt'), 'w') as f:
        f.write('\n'.join(cases) + '\n')
    print(len(cases), 'tiff cases')


def webp():
    """WebP files from libwebp (through Pillow): lossless with each effort
    (so every transform and the colour cache), palettes of 2 to 256 colours,
    lossy at several qualities, with and without alpha, and animations. The
    pixels expected are libwebp's decoding (fancy upsampling), as PNG."""
    d = os.path.join(root, 'webp')
    os.makedirs(d, exist_ok=True)
    names = []

    def save(name, im, **kw):
        im.save(os.path.join(d, name + '.webp'), 'WEBP', **kw)
        names.append(name)

    photo = picture(67, 45, 'RGB', 21)
    alpha = picture(53, 39, 'RGBA', 22)
    for m in [0, 3, 6]:
        save('lossless_m%d' % m, photo, lossless=True, method=m, quality=100)
    save('lossless_alpha', alpha, lossless=True, method=4, exact=True)
    save('lossless_q0', photo, lossless=True, method=0, quality=0)
    for n in [2, 4, 16, 200]:
        save('palette%d' % n, picture(41, 29, 'RGB', 23 + n).quantize(n).convert('RGB'), lossless=True, method=5)
    for q in [5, 50, 90, 100]:
        save('lossy_q%d' % q, photo, quality=q, method=4)
    save('lossy_big', picture(200, 150, 'RGB', 30), quality=75, method=6)
    save('lossy_alpha', alpha, quality=70, alpha_quality=100)
    save('lossy_alpha_q50', alpha, quality=70, alpha_quality=50, alpha_filtering=2)
    save('lossy_alpha_raw', alpha, quality=70, alpha_compression=0)
    save('lossy_tiny', picture(3, 5, 'RGB', 31), quality=80)
    frames = []
    for k in range(4):
        f = Image.new('RGBA', (48, 40), (0, 0, 0, 0))
        dr = ImageDraw.Draw(f)
        dr.rectangle([5 * k, 4 * k, 5 * k + 20, 4 * k + 15], fill=(60 * k, 255 - 50 * k, 90, 255))
        dr.ellipse([30 - 3 * k, 5, 45, 20 + k], fill=(200, 30 * k, 250, 128))
        frames.append(f)
    frames[0].save(os.path.join(d, 'anim_lossless.webp'), save_all=True, append_images=frames[1:],
                   duration=[100, 50, 70, 30], loop=0, lossless=True)
    names.append('anim_lossless')
    frames[0].save(os.path.join(d, 'anim_lossy.webp'), save_all=True, append_images=frames[1:],
                   duration=80, loop=3, lossless=False, quality=80, allow_mixed=True)
    names.append('anim_lossy')
    listing = []
    for name in names:
        im = Image.open(os.path.join(d, name + '.webp'))
        n = getattr(im, 'n_frames', 1)
        for k in range(n):
            im.seek(k)
            fr = im.convert('RGBA') if (im.mode == 'RGBA' or n > 1) else im.convert('RGB')
            expect(os.path.join(d, '%s.%d.png' % (name, k)), fr)
        listing.append('%s %d %d' % (name, n, im.info.get('loop', -1) if n > 1 else -1))
    with open(os.path.join(d, 'cases.txt'), 'w') as f:
        f.write('\n'.join(listing) + '\n')
    print(len(names), 'webp files')


for what in sys.argv[1:] or ['gif']:
    globals()[what]()
