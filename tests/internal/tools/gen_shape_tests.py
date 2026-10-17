#!/usr/bin/env python3
"""Generates std/graphics/shape/testdata/cases.txt: text shaped by HarfBuzz (through
uharfbuzz), which std/graphics/shape's tests must reproduce glyph for glyph.

    pip install uharfbuzz
    python3 tests/internal/tools/gen_shape_tests.py

Each line is: font path, text (UTF-8), features (comma separated), then the
glyphs as id:cluster:x_advance:y_advance:x_offset:y_offset, tab separated.
The fonts are the system ones the font tests use (Debian and Ubuntu package
them as fonts-dejavu-core, fonts-freefont-ttf, fonts-liberation and
fonts-unifont); the tests skip fonts that are absent. The output is
deterministic.
"""
import os
import random
import unicodedata

import uharfbuzz as hb

OUT = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', '..', 'std', 'graphics', 'shape', 'testdata', 'cases.txt')

DEJAVU = '/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf'
DEJAVU_SERIF = '/usr/share/fonts/truetype/dejavu/DejaVuSerif.ttf'
FREESERIF = '/usr/share/fonts/truetype/freefont/FreeSerif.ttf'
FREESANS = '/usr/share/fonts/truetype/freefont/FreeSans.ttf'
LIBERATION = '/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf'
UNIFONT = '/usr/share/fonts/opentype/unifont/unifont.otf'

SAMPLES = {
    'latin': [
        'The quick brown fox jumps over the lazy dog.', 'office affine flag waffle', 'AVATAR Wave To Yes',
        'naïve café résumé', 'ä́ ệ q̣̇', '1⁄2 3⁄4 x⁄y',
        'Ǆ ǅ ǆ ĳ', 'soft­hyphen zero​width', 'x y z w q k',
        '́abc', 'é̂̃̄̅', '(a[b]{c})',
    ],
    'arabic': [
        'السلام عليكم', 'العربية', 'لا إله إلا الله', 'بِسْمِ ٱللَّٰهِ ٱلرَّحْمَٰنِ ٱلرَّحِيمِ', 'كتاب (١٢٣) 456',
        'ـبـ ـلا', 'ب‍ب ب‌ب', 'فارسی: ژ پ چ گ', 'بَّ بَّ',
    ],
    'hebrew': ['שָׁלוֹם עוֹלָם', 'בְּרֵאשִׁית בָּרָא אֱלֹהִים', 'שּׁ שּׂ', 'אַֽ (א) [ב]'],
    'deva': ['नमस्ते दुनिया', 'क्षत्रिय', 'र्क र्कि', 'हिन्दी', 'श्री', 'क्‍ष क्‌ष', 'द्द्ध', 'अा रु'],
    'beng': ['বাংলা ভাষা', 'ক্ষ', 'র্ক', 'কো কৌ', 'ব্র্য'],
    'guru': ['ਪੰਜਾਬੀ ਭਾਸ਼ਾ', 'ਕ੍ਰ', 'ਸਿੱਖ'],
    'gujr': ['ગુજરાતી', 'ક્ષ', 'ર્ક', 'અાૅ'],
    'orya': ['ଓଡ଼ିଆ', 'କ୍ଷ', 'ର୍କ'],
    'taml': ['தமிழ்', 'க்ஷ', 'கொ கோ கௌ', 'ஸ்ரீ'],
    'telu': ['తెలుగు', 'క్ష', 'ర్క', 'కై'],
    'knda': ['ಕನ್ನಡ', 'ಕ್ಷ', 'ರ್ಕ', 'ಕೊ'],
    'mlym': ['മലയാളം', 'ക്ഷ', 'ര്ക', 'ന്റ', 'കൊ'],
}


def block(a, b):
    return [chr(c) for c in range(a, b + 1) if unicodedata.category(chr(c)) != 'Cn']


EXTRA = ['‌', '‍', ' ', '◌', ' ']
RANDOM = {
    'latin': block(0x20, 0x7e) + block(0xc0, 0x17f) + block(0x300, 0x36f) + ['­', '⁄', ' ', ' '],
    'arabic': block(0x600, 0x6ff) + ['‌', '‍', ' ', '(', ')', '1'],
    'hebrew': block(0x591, 0x5f4) + [' ', '(', ')'],
    'deva': block(0x900, 0x97f) + EXTRA,
    'beng': block(0x980, 0x9ff) + EXTRA,
    'guru': block(0xa00, 0xa7f) + EXTRA,
    'gujr': block(0xa80, 0xaff) + EXTRA,
    'orya': block(0xb00, 0xb7f) + EXTRA,
    'taml': block(0xb80, 0xbff) + EXTRA,
    'telu': block(0xc00, 0xc7f) + EXTRA,
    'knda': block(0xc80, 0xcff) + EXTRA,
    'mlym': block(0xd00, 0xd7f) + EXTRA,
}

INDIC = ['deva', 'beng', 'guru', 'gujr', 'orya', 'taml', 'telu', 'knda', 'mlym']
PLAN = [
    (DEJAVU, ['latin', 'arabic', 'hebrew']),
    (DEJAVU_SERIF, ['latin', 'hebrew']),  # no Hebrew GPOS: fallback mark positioning
    (LIBERATION, ['latin', 'hebrew']),
    (FREESERIF, ['arabic', 'hebrew'] + INDIC),
    (FREESANS, INDIC),
    (UNIFONT, ['arabic', 'latin']),       # CFF, no Arabic features: presentation-form fallback
]
FEATURES = [
    (DEJAVU, 'office affine', '-liga'), (DEJAVU, 'office affine', 'liga[0:3]=0'),
    (DEJAVU, 'AVATAR Wave', '-kern'), (DEJAVU, 'AVATAR Wave', 'kern[2:5]=0'),
    (FREESERIF, 'Small Caps 123', 'smcp,onum'), (FREESERIF, 'ffi fl st', 'dlig'),
    (FREESERIF, 'क्षत्रिय र्क', '-rphf,-half'), (DEJAVU, 'السلام', '-init,-medi'),
    (LIBERATION, '1/2 x', 'frac'),
]


def parse_features(fs):
    d = {}
    for f in [x for x in fs.split(',') if x]:
        val = 1
        if f[0] in '+-':
            val = 0 if f[0] == '-' else 1
            f = f[1:]
        rng = None
        if '=' in f:
            f, v = f.split('=')
            val = int(v)
        if '[' in f:
            f, r = f.split('[')
            a, b = r.rstrip(']').split(':')
            rng = (int(a) if a else 0, int(b) if b else 0xffffffff)
        if rng is None:
            d[f] = val
        else:
            d.setdefault(f, []).append((rng[0], rng[1], val))
    return d


def shape(font, text, feats):
    buf = hb.Buffer()
    buf.add_utf8(text.encode())
    buf.guess_segment_properties()
    buf.flags = hb.BufferFlags.BOT | hb.BufferFlags.EOT
    fd = parse_features(feats)
    hb.shape(font, buf, fd if fd else None)
    return ' '.join('%d:%d:%d:%d:%d:%d' % (i.codepoint, i.cluster, p.x_advance, p.y_advance, p.x_offset, p.y_offset)
                    for i, p in zip(buf.glyph_infos, buf.glyph_positions))


def main():
    rng = random.Random(62)
    lines = []
    fonts = {}

    def font(path):
        if path not in fonts:
            fonts[path] = hb.Font(hb.Face(hb.Blob.from_file_path(path)))
        return fonts[path]

    for path, scripts in PLAN:
        for sc in scripts:
            texts = list(SAMPLES[sc])
            for _ in range(24):
                texts.append(''.join(rng.choice(RANDOM[sc]) for _ in range(rng.randint(1, 10))))
            for t in texts:
                lines.append('%s\t%s\t\t%s' % (path, t, shape(font(path), t, '')))
    for path, t, f in FEATURES:
        lines.append('%s\t%s\t%s\t%s' % (path, t, f, shape(font(path), t, f)))
    for l in lines:
        assert '\n' not in l and l.count('\t') == 3, repr(l)
    os.makedirs(os.path.dirname(OUT), exist_ok=True)
    with open(OUT, 'w') as f:
        f.write('\n'.join(lines) + '\n')
    print('wrote %s: %d cases' % (OUT, len(lines)))


if __name__ == '__main__':
    main()
