#!/usr/bin/env python3
"""Generates std/graphics/svg/testdata: small SVG documents (written here)
and, for each, NAME.probes: pixels of skia's rendering of it (skia-python's
SVGDOM, at the document's size, transparent background), taken where the
picture is smooth (no edge within two pixels), for std/graphics/svg's tests.

The documents keep to what skia renders as SVG 1.1 says: it skips <style>,
symbol, switch, masks, nested viewport clipping, images, em units and
visible children of hidden groups, which std/graphics/svg's own tests (on
the hand-written documents in testdata) cover.

NAME.probes lines: x y r g b a (unpremultiplied 8-bit sRGB).
"""
import os

import numpy as np
import skia

root = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', '..', 'std', 'graphics', 'svg', 'testdata')

DOCS = {
    'alpha': '''\
<svg xmlns="http://www.w3.org/2000/svg" width="200" height="120">
  <rect width="200" height="120" fill="#fafad2"/>
  <g opacity="0.6"><rect x="10" y="10" width="60" height="60" fill="crimson"/><rect x="40" y="40" width="60" height="60" fill="navy"/></g>
  <rect x="110" y="10" width="60" height="60" fill="crimson" fill-opacity="0.5"/>
  <rect x="140" y="40" width="50" height="60" fill="seagreen" stroke="black" stroke-width="10" stroke-opacity="0.3" opacity="0.8"/>
  <linearGradient id="g"><stop offset="0" stop-color="purple" stop-opacity="0"/><stop offset="1" stop-color="orange" stop-opacity="0.9"/></linearGradient>
  <rect x="10" y="100" width="180" height="15" fill="url(#g)"/>
</svg>
''',
    'clip': '''\
<svg xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink" width="200" height="200">
  <defs>
    <clipPath id="c1"><rect x="20" y="20" width="60" height="60"/><circle cx="80" cy="80" r="30"/></clipPath>
    <clipPath id="c2" clipPathUnits="objectBoundingBox"><circle cx="0.5" cy="0.5" r="0.5"/></clipPath>
    <clipPath id="c3"><path d="M120 120 h60 v60 h-60z M135 135 v30 h30 v-30z" clip-rule="evenodd"/></clipPath>
  </defs>
  <rect width="200" height="200" fill="silver"/>
  <rect x="0" y="0" width="120" height="120" fill="navy" clip-path="url(#c1)"/>
  <rect x="120" y="10" width="70" height="90" fill="maroon" clip-path="url(#c2)"/>
  <g clip-path="url(#c3)"><rect x="100" y="100" width="100" height="100" fill="darkgreen"/></g>
</svg>
''',
    'fit': '''\
<svg xmlns="http://www.w3.org/2000/svg" width="200" height="100" viewBox="0 0 50 50" preserveAspectRatio="xMaxYMid meet">
  <rect width="50" height="50" fill="khaki"/>
  <circle cx="25" cy="25" r="20" fill="darkred"/>
</svg>
''',
    'fit2': '''\
<svg xmlns="http://www.w3.org/2000/svg" width="200" height="100" viewBox="0 0 50 50" preserveAspectRatio="xMinYMin slice">
  <rect width="50" height="50" fill="khaki"/>
  <circle cx="25" cy="25" r="20" fill="darkred"/>
</svg>
''',
    'fit3': '''\
<svg xmlns="http://www.w3.org/2000/svg" width="200" height="100" viewBox="0 0 50 50" preserveAspectRatio="none">
  <rect width="50" height="50" fill="khaki"/>
  <circle cx="25" cy="25" r="20" fill="darkred"/>
</svg>
''',
    'focal': '''\
<svg xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink" width="240" height="160">
  <defs>
    <radialGradient id="f1" fx="0.25" fy="0.3"><stop offset="0" stop-color="white"/><stop offset="0.6" stop-color="gold"/><stop offset="1" stop-color="darkred"/></radialGradient>
    <radialGradient id="f2" gradientUnits="userSpaceOnUse" cx="180" cy="50" r="40" fx="200" fy="70" spreadMethod="reflect"><stop offset="0" stop-color="navy"/><stop offset="1" stop-color="skyblue"/></radialGradient>
    <radialGradient id="f3" xlink:href="#f1" fx="0.9" fy="0.5"/>
    <radialGradient id="f4" gradientUnits="userSpaceOnUse" cx="180" cy="120" r="30" fy="140" spreadMethod="repeat"><stop offset="0" stop-color="seagreen"/><stop offset="1" stop-color="ivory"/></radialGradient>
  </defs>
  <rect x="10" y="10" width="100" height="70" fill="url(#f1)"/>
  <rect x="130" y="5" width="100" height="90" fill="url(#f2)"/>
  <circle cx="60" cy="120" r="35" fill="url(#f3)"/>
  <rect x="130" y="95" width="100" height="60" fill="url(#f4)"/>
</svg>
''',
    'gradients': '''\
<svg xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink" width="200" height="200">
  <defs>
    <linearGradient id="a" x1="0" y1="0" x2="1" y2="0">
      <stop offset="0" stop-color="red"/>
      <stop offset="0.5" stop-color="yellow"/>
      <stop offset="1" stop-color="blue"/>
    </linearGradient>
    <linearGradient id="b" xlink:href="#a" gradientUnits="userSpaceOnUse" x1="0" y1="100" x2="0" y2="140" spreadMethod="reflect"/>
    <radialGradient id="c" cx="0.5" cy="0.5" r="0.5">
      <stop offset="0%" stop-color="white"/>
      <stop offset="100%" stop-color="#004080"/>
    </radialGradient>
    <linearGradient id="d" gradientTransform="rotate(90 0.5 0.5)">
      <stop offset="0" stop-color="black"/>
      <stop offset="1" stop-color="lime"/>
    </linearGradient>
  </defs>
  <rect x="10" y="10" width="180" height="40" fill="url(#a)"/>
  <rect x="10" y="60" width="80" height="120" fill="url(#b)"/>
  <circle cx="145" cy="105" r="40" fill="url(#c)"/>
  <rect x="110" y="150" width="80" height="40" fill="url(#d)"/>
</svg>
''',
    'groups': '''\
<svg xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink" width="200" height="200" viewBox="0 0 400 400">
  <defs>
    <g id="star"><polygon points="0,-40 12,-12 40,-12 18,6 26,36 0,18 -26,36 -18,6 -40,-12 -12,-12" /></g>
    <path id="tri" d="M0 0 L40 0 L20 30 z"/>
  </defs>
  <g fill="maroon" transform="translate(100 100)">
    <use xlink:href="#star"/>
    <use xlink:href="#star" x="150" fill="olive" transform="scale(1.2)"/>
  </g>
  <g transform="rotate(30 300 100) skewX(10)"><rect x="260" y="60" width="80" height="80" fill="slateblue"/></g>
  <use xlink:href="#tri" x="50" y="250" fill="darkmagenta" stroke="black" stroke-width="4"/>
  <a href="#x"><circle cx="150" cy="330" r="30" fill="chocolate"/></a>
</svg>
''',
    'paths': '''\
<svg xmlns="http://www.w3.org/2000/svg" width="200" height="200" viewBox="0 0 100 100">
  <path d="M10 10 h30 v30 h-30 z" fill="crimson"/>
  <path d="M50,10 C 60,0 90,20 90,40 S 70,50 60,40 Q 50,30 55,20 T 50,10" fill="steelblue"/>
  <path d="M10 60 A 20 10 30 1 1 40 80 L 10 80 Z" fill="seagreen"/>
  <path d="m60 60 a15 15 0 0 0 30 0 a15 15 0 0 0 -30 0z m8 0 a7 7 0 0 1 14 0 a7 7 0 0 1 -14 0z" fill="darkorange" fill-rule="evenodd"/>
  <path d="M5 95 L95 95" stroke="black" stroke-width="2" stroke-dasharray="6 3"/>
</svg>
''',
    'shapes': '''\
<svg xmlns="http://www.w3.org/2000/svg" width="200" height="150">
  <rect x="10" y="10" width="60" height="40" fill="#e03020"/>
  <rect x="80" y="10" width="60" height="40" rx="10" ry="15" fill="navy" stroke="gold" stroke-width="4"/>
  <circle cx="170" cy="30" r="20" fill="rgb(0,128,0)"/>
  <ellipse cx="40" cy="90" rx="30" ry="15" fill="purple"/>
  <line x1="80" y1="70" x2="190" y2="110" stroke="black" stroke-width="6" stroke-linecap="round"/>
  <polyline points="10,140 40,115 70,140 100,115" fill="none" stroke="teal" stroke-width="5" stroke-linejoin="round"/>
  <polygon points="120,140 150,110 180,140" fill="orange" stroke="brown" stroke-width="3" stroke-linejoin="bevel"/>
</svg>
''',
    'strokes': '''\
<svg xmlns="http://www.w3.org/2000/svg" width="240" height="200">
  <g fill="none" stroke="darkblue" stroke-width="10">
    <path d="M20 40 L60 10 L100 40" stroke-linejoin="miter"/>
    <path d="M120 40 L160 10 L200 40" stroke-linejoin="round"/>
    <path d="M20 90 L60 60 L100 90" stroke-linejoin="bevel"/>
    <path d="M120 90 L160 80 L200 90" stroke-miterlimit="1.5"/>
    <path d="M20 120 H100" stroke-linecap="butt"/>
    <path d="M130 120 H200" stroke-linecap="square"/>
    <path d="M20 150 H220" stroke-width="6" stroke-dasharray="20,10,5" stroke-dashoffset="7"/>
    <rect x="20" y="170" width="200" height="20" stroke="firebrick" stroke-width="3" stroke-dasharray="12 6"/>
  </g>
</svg>
''',
    'transforms': '''\
<svg xmlns="http://www.w3.org/2000/svg" width="200" height="200" viewBox="-100 -100 200 200">
  <g transform="matrix(0.8 0.2 -0.2 0.8 -40 -40)"><rect x="-30" y="-30" width="60" height="60" fill="indigo"/></g>
  <g transform="translate(40,-40) rotate(-20) scale(1.5 0.7)"><circle r="25" fill="darkgreen"/></g>
  <g transform="translate(-40 40)"><g transform="skewY(20)"><rect x="-25" y="-25" width="50" height="50" fill="sienna"/></g></g>
  <rect x="20" y="20" width="50" height="50" transform="rotate(45, 45, 45)" fill="goldenrod"/>
</svg>
''',
}


def render(path):
    dom = skia.SVGDOM.MakeFromStream(skia.Stream.MakeFromFile(path))
    w, h = int(dom.containerSize().width()), int(dom.containerSize().height())
    surf = skia.Surface(w, h)
    c = surf.getCanvas()
    c.clear(skia.ColorTRANSPARENT)
    dom.setContainerSize(skia.Size(w, h))
    dom.render(c)
    return surf.makeImageSnapshot().toarray(colorType=skia.kRGBA_8888_ColorType, alphaType=skia.kUnpremul_AlphaType)


def probes(a):
    """Every 37th point (in raster order) of those with no colour change
    over 8 within two pixels."""
    h, w, _ = a.shape
    x = a.astype(int)
    smooth = np.ones((h, w), bool)
    for dy in range(-2, 3):
        for dx in range(-2, 3):
            smooth &= np.abs(np.roll(np.roll(x, dy, 0), dx, 1) - x).max(-1) <= 8
    smooth[:2, :] = smooth[-2:, :] = smooth[:, :2] = smooth[:, -2:] = False
    ys, xs = np.nonzero(smooth)
    return [(int(px), int(py), *map(int, a[py, px])) for py, px in list(zip(ys, xs))[::37]]


def main():
    os.makedirs(root, exist_ok=True)
    for name, text in sorted(DOCS.items()):
        path = os.path.join(root, name + '.svg')
        with open(path, 'w') as f:
            f.write(text)
        pts = probes(render(path))
        with open(os.path.join(root, name + '.probes'), 'w') as f:
            f.write('# Generated by tests/internal/tools/gen_svg_tests.py from skia %s. Do not edit.\n' % skia.__version__)
            for p in pts:
                f.write('%d %d %d %d %d %d\n' % p)


if __name__ == '__main__':
    main()
