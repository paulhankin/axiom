#!/usr/bin/env python3
"""Generates std/shape/tables.ph (the data std/shape needs) from the Unicode
Character Database.

    python3 tools/gen_shape_tables.py [UCD directory]   # default /usr/share/unicode

The tables follow HarfBuzz's generators (gen-arabic-table.py and
gen-indic-table.py): scripts (Scripts.txt), Arabic joining types
(ArabicShaping.txt), the Indic shaper's character categories and positions
(IndicSyllabicCategory.txt, IndicPositionalCategory.txt, Blocks.txt) and, from
HarfBuzz's Indic syllable grammar (hb-ot-shaper-indic-machine.rl), a DFA that
finds syllables. The output is deterministic.
"""
import os
import sys

U = (sys.argv[1] if len(sys.argv) > 1 else '/usr/share/unicode').rstrip('/') + '/'
OUT = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'std', 'shape', 'tables.ph')


def data(name):
    """(first, last, fields) for each data line of a UCD file."""
    for line in open(U + name, encoding='utf8'):
        line = line.split('#')[0].strip()
        if not line:
            continue
        f = [x.strip() for x in line.split(';')]
        r = f[0].split('..')
        a = int(r[0], 16)
        b = int(r[1], 16) if len(r) > 1 else a
        yield a, b, f[1:]


def tag(s):
    s = (s + '    ')[:4]
    return (ord(s[0]) << 24) | (ord(s[1]) << 16) | (ord(s[2]) << 8) | ord(s[3])


# -----------------------------------------------------------------------------
# Scripts: ranges of code points and their ISO 15924 codes
# -----------------------------------------------------------------------------

def scripts():
    alias = {}
    for line in open(U + 'PropertyValueAliases.txt', encoding='utf8'):
        f = [x.strip() for x in line.split('#')[0].split(';')]
        if len(f) >= 3 and f[0] == 'sc':
            alias[f[2]] = f[1]
            for extra in f[3:]:
                alias[extra] = f[1]
    ranges = []
    for a, b, f in data('Scripts.txt'):
        ranges.append((a, b, alias[f[0]]))
    ranges.sort()
    merged = []
    for a, b, s in ranges:
        if merged and merged[-1][2] == s and merged[-1][1] + 1 == a:
            merged[-1] = (merged[-1][0], b, s)
        else:
            merged.append((a, b, s))
    return merged


# -----------------------------------------------------------------------------
# Arabic joining types (gen-arabic-table.py)
# -----------------------------------------------------------------------------

JOINING = {'U': 0, 'L': 1, 'R': 2, 'D': 3, 'C': 3, 'ALAPH': 4, 'DALATH RISH': 5, 'T': 6}


def joining():
    out = {}
    for a, b, f in data('ArabicShaping.txt'):
        # fields: name; joining type; joining group
        v = JOINING[f[2]] if f[2] in ('ALAPH', 'DALATH RISH') else JOINING[f[1]]
        for u in range(a, b + 1):
            out[u] = v
    return sorted(out.items())


# -----------------------------------------------------------------------------
# Arabic presentation forms, for fonts without Arabic features (the shaping
# and ligature tables of gen-arabic-table.py, from UnicodeData.txt)
# -----------------------------------------------------------------------------

LIGATURES = (
    0xF2EE, 0xFC08, 0xFC0E, 0xFC12, 0xFC32, 0xFC3F, 0xFC40, 0xFC41, 0xFC42,
    0xFC43, 0xFC44, 0xFC4E, 0xFC5E, 0xFC60, 0xFC61, 0xFC62, 0xFC6A, 0xFC6D,
    0xFC6F, 0xFC70, 0xFC73, 0xFC75, 0xFC86, 0xFC8F, 0xFC91, 0xFC94, 0xFC9C,
    0xFC9D, 0xFC9E, 0xFC9F, 0xFCA1, 0xFCA2, 0xFCA3, 0xFCA4, 0xFCA8, 0xFCAA,
    0xFCAC, 0xFCB0, 0xFCC9, 0xFCCA, 0xFCCB, 0xFCCC, 0xFCCD, 0xFCCE, 0xFCCF,
    0xFCD0, 0xFCD1, 0xFCD2, 0xFCD3, 0xFCD5, 0xFCDA, 0xFCDB, 0xFCDC, 0xFCDD,
    0xFD30, 0xFD88, 0xFEF5, 0xFEF6, 0xFEF7, 0xFEF8, 0xFEF9, 0xFEFA, 0xFEFB,
    0xFEFC, 0xF201, 0xF211, 0xF2EE,
)


def arabic_shaping():
    shapes = {}
    ligatures = {}
    lines = open(U + 'UnicodeData.txt', encoding='utf8').read().split('\n')
    lines += [
        "F201;PUA ARABIC LIGATURE LELLAH ISOLATED FORM;Lo;0;AL;<isolated> 0644 0644 0647;;;;N;;;;;",
        "F211;PUA ARABIC LIGATURE LAM WITH MEEM WITH JEEM INITIAL FORM;Lo;0;AL;<initial> 0644 0645 062C;;;;N;;;;;",
        "F2EE;PUA ARABIC LIGATURE SHADDA WITH FATHATAN ISOLATED FORM;Lo;0;AL;<isolated> 0020 064B 0651;;;;N;;;;;",
    ]
    for line in lines:
        fields = [x.strip() for x in line.split(';')]
        if len(fields) < 6 or fields[5][0:1] != '<':
            continue
        items = fields[5].split(' ')
        shape, items = items[0][1:-1], tuple(int(x, 16) for x in items[1:])
        c = int(fields[0], 16)
        if shape not in ['initial', 'medial', 'isolated', 'final']:
            continue
        if len(items) != 1:
            if items[0] == 0x0020:
                items = items[:0:-1]
                shape = None
            if c not in LIGATURES:
                continue
            ligatures.setdefault(items, {})[shape] = c
        else:
            shapes.setdefault(items[0], {})[shape] = c
    ligas_2, ligas_3, ligas_mark_2 = {}, {}, {}
    for key in ligatures:
        for shape in ligatures[key]:
            c = ligatures[key][shape]
            if len(key) == 3:
                if shape == 'isolated':
                    liga = (shapes[key[0]]['initial'], shapes[key[1]]['medial'], shapes[key[2]]['final'])
                elif shape == 'final':
                    liga = (shapes[key[0]]['medial'], shapes[key[1]]['medial'], shapes[key[2]]['final'])
                else:
                    liga = (shapes[key[0]]['initial'], shapes[key[1]]['medial'], shapes[key[2]]['medial'])
                ligas_3.setdefault(liga[0], []).append((liga[1], liga[2], c))
            else:
                if shape is None:
                    ligas_mark_2.setdefault(key[0], []).append((key[1], c))
                    continue
                elif shape == 'isolated':
                    liga = (shapes[key[0]]['initial'], shapes[key[1]]['final'])
                elif shape == 'final':
                    liga = (shapes[key[0]]['medial'], shapes[key[1]]['final'])
                else:
                    liga = (shapes[key[0]]['initial'], shapes[key[1]]['medial'])
                ligas_2.setdefault(liga[0], []).append((liga[1], c))
    lo, hi = min(shapes), max(shapes)
    table = []
    for u in range(lo, hi + 1):
        table += [shapes.get(u, {}).get(sh, 0) for sh in ['initial', 'medial', 'final', 'isolated']]
    def rows(d, n):
        out = []
        for first in sorted(d):
            for liga in d[first]:
                out.append((first,) + liga)
        return out
    return lo, hi, table, rows(ligas_2, 2), rows(ligas_3, 3), rows(ligas_mark_2, 2)


# -----------------------------------------------------------------------------
# Indic categories and positions (gen-indic-table.py)
# -----------------------------------------------------------------------------

ALLOWED_SINGLES = [0x00A0, 0x25CC]
ALLOWED_BLOCKS = [
    'Basic Latin', 'Latin-1 Supplement', 'Devanagari', 'Bengali', 'Gurmukhi', 'Gujarati', 'Oriya', 'Tamil',
    'Telugu', 'Kannada', 'Malayalam', 'Myanmar', 'Khmer', 'Vedic Extensions', 'General Punctuation',
    'Superscripts and Subscripts', 'Devanagari Extended', 'Myanmar Extended-B', 'Myanmar Extended-A',
    'Myanmar Extended-C',
]

# The machine's category numbers (hb-ot-shaper-indic-machine.rl and the
# Khmer and Myanmar machines, which share the table).
CATEGORY = {
    'X': 0, 'C': 1, 'V': 2, 'N': 3, 'H': 4, 'ZWNJ': 5, 'ZWJ': 6, 'M': 7, 'SM': 8, 'A': 9, 'VD': 9,
    'PLACEHOLDER': 10, 'DOTTEDCIRCLE': 11, 'RS': 12, 'MPst': 13, 'Repha': 14, 'Ra': 15, 'CM': 16,
    'Symbol': 17, 'CS': 18, 'SMPst': 57,
    # Khmer
    'VAbv': 20, 'VBlw': 21, 'VPre': 22, 'VPst': 23, 'Robatic': 25, 'Xgroup': 26, 'Ygroup': 27,
    # Myanmar
    'IV': 2, 'As': 32, 'DB': 3, 'GB': 10, 'MH': 35, 'MR': 36, 'MW': 37, 'MY': 38, 'PT': 39, 'VS': 40,
    'ML': 41,
}
POSITION = {
    'START': 0, 'RA_TO_BECOME_REPH': 1, 'PRE_M': 2, 'PRE_C': 3, 'BASE_C': 4, 'AFTER_MAIN': 5, 'ABOVE_C': 6,
    'BEFORE_SUB': 7, 'BELOW_C': 8, 'AFTER_SUB': 9, 'BEFORE_POST': 10, 'POST_C': 11, 'AFTER_POST': 12,
    'SMVD': 13, 'END': 14,
}

CATEGORY_MAP = {
    'Other': 'X', 'Avagraha': 'Symbol', 'Bindu': 'SM', 'Brahmi_Joining_Number': 'PLACEHOLDER',
    'Cantillation_Mark': 'A', 'Consonant': 'C', 'Consonant_Dead': 'C', 'Consonant_Final': 'CM',
    'Consonant_Head_Letter': 'C', 'Consonant_Initial_Postfixed': 'C', 'Consonant_Killer': 'M',
    'Consonant_Medial': 'CM', 'Consonant_Placeholder': 'PLACEHOLDER', 'Consonant_Preceding_Repha': 'Repha',
    'Consonant_Prefixed': 'X', 'Consonant_Subjoined': 'CM', 'Consonant_Succeeding_Repha': 'CM',
    'Consonant_With_Stacker': 'CS', 'Gemination_Mark': 'SM', 'Invisible_Stacker': 'H', 'Joiner': 'ZWJ',
    'Modifying_Letter': 'X', 'Non_Joiner': 'ZWNJ', 'Nukta': 'N', 'Number': 'PLACEHOLDER',
    'Number_Joiner': 'PLACEHOLDER', 'Pure_Killer': 'M', 'Register_Shifter': 'RS', 'Syllable_Modifier': 'SM',
    'Tone_Letter': 'X', 'Tone_Mark': 'N', 'Virama': 'H', 'Visarga': 'SM', 'Vowel': 'V',
    'Vowel_Dependent': 'M', 'Vowel_Independent': 'V',
}
POSITION_MAP = {
    'Not_Applicable': 'END', 'Left': 'PRE_C', 'Top': 'ABOVE_C', 'Bottom': 'BELOW_C', 'Right': 'POST_C',
    'Bottom_And_Right': 'POST_C', 'Left_And_Right': 'POST_C', 'Top_And_Bottom': 'BELOW_C',
    'Top_And_Bottom_And_Left': 'BELOW_C', 'Top_And_Bottom_And_Right': 'POST_C', 'Top_And_Left': 'ABOVE_C',
    'Top_And_Left_And_Right': 'POST_C', 'Top_And_Right': 'POST_C', 'Overstruck': 'AFTER_MAIN',
    'Visual_order_left': 'PRE_M',
}
CATEGORY_OVERRIDES = {}
for u in range(0xFE00, 0xFE10):
    CATEGORY_OVERRIDES[u] = 'VS'
CATEGORY_OVERRIDES.update({
    0x2015: 'PLACEHOLDER', 0x2022: 'PLACEHOLDER', 0x25FB: 'PLACEHOLDER', 0x25FC: 'PLACEHOLDER',
    0x25FD: 'PLACEHOLDER', 0x25FE: 'PLACEHOLDER',
    0x0930: 'Ra', 0x09B0: 'Ra', 0x09F0: 'Ra', 0x0A30: 'Ra', 0x0AB0: 'Ra', 0x0B30: 'Ra', 0x0BB0: 'Ra',
    0x0C30: 'Ra', 0x0CB0: 'Ra', 0x0D30: 'Ra',
    0x0953: 'SM', 0x0954: 'SM', 0x0A40: 'MPst', 0x0A72: 'C', 0x0A73: 'C',
    0x1CE2: 'A', 0x1CE3: 'A', 0x1CE4: 'A', 0x1CE5: 'A', 0x1CE6: 'A', 0x1CE7: 'A', 0x1CE8: 'A', 0x1CED: 'A',
    0xA8F2: 'Symbol', 0xA8F3: 'Symbol', 0xA8F4: 'Symbol', 0xA8F5: 'Symbol', 0xA8F6: 'Symbol',
    0xA8F7: 'Symbol', 0x1CE9: 'Symbol', 0x1CEA: 'Symbol', 0x1CEB: 'Symbol', 0x1CEC: 'Symbol',
    0x1CEE: 'Symbol', 0x1CEF: 'Symbol', 0x1CF0: 'Symbol', 0x1CF1: 'Symbol',
    0x0A51: 'M', 0x11301: 'SM', 0x11302: 'SM', 0x11303: 'SM', 0x1133B: 'N', 0x1133C: 'N',
    0x0AFB: 'N', 0x0B55: 'N', 0x09FC: 'PLACEHOLDER', 0x0C80: 'PLACEHOLDER', 0x0D04: 'PLACEHOLDER',
    0x25CC: 'DOTTEDCIRCLE',
    0x179A: 'Ra', 0x17CC: 'Robatic', 0x17C9: 'Robatic', 0x17CA: 'Robatic',
    0x17C6: 'Xgroup', 0x17CB: 'Xgroup', 0x17CD: 'Xgroup', 0x17CE: 'Xgroup', 0x17CF: 'Xgroup',
    0x17D0: 'Xgroup', 0x17D1: 'Xgroup', 0x17C7: 'Ygroup', 0x17C8: 'Ygroup', 0x17DD: 'Ygroup',
    0x17D3: 'Ygroup', 0x17D9: 'PLACEHOLDER',
    0x104E: 'C', 0x1004: 'Ra', 0x101B: 'Ra', 0x105A: 'Ra', 0x1032: 'A', 0x1036: 'A', 0x103A: 'As',
    0x103E: 'MH', 0x1060: 'ML', 0x103C: 'MR', 0x103D: 'MW', 0x1082: 'MW', 0x103B: 'MY', 0x105E: 'MY',
    0x105F: 'MY', 0x1063: 'PT', 0x1064: 'PT', 0x1069: 'PT', 0x106A: 'PT', 0x106B: 'PT', 0x106C: 'PT',
    0x106D: 'PT', 0xAA7B: 'PT', 0x1038: 'SM', 0x1087: 'SM', 0x1088: 'SM', 0x1089: 'SM', 0x108A: 'SM',
    0x108B: 'SM', 0x108C: 'SM', 0x108D: 'SM', 0x108F: 'SM', 0x109A: 'SM', 0x109B: 'SM', 0x109C: 'SM',
    0x104A: 'PLACEHOLDER',
})
POSITION_OVERRIDES = {0x0A51: 'BELOW_C', 0x0B01: 'BEFORE_SUB'}


def matra_pos_right(u, block):
    return {
        'Devanagari': 'AFTER_SUB', 'Bengali': 'AFTER_POST', 'Gurmukhi': 'AFTER_POST', 'Gujarati': 'AFTER_POST',
        'Oriya': 'AFTER_POST', 'Tamil': 'AFTER_POST', 'Malayalam': 'AFTER_POST',
        'Telugu': 'BEFORE_SUB' if u <= 0x0C42 else 'AFTER_SUB',
        'Kannada': 'BEFORE_SUB' if u < 0x0CC3 or u > 0x0CD6 else 'AFTER_SUB',
    }.get(block, 'AFTER_SUB')


def matra_pos_top(u, block):
    return {
        'Devanagari': 'AFTER_SUB', 'Gurmukhi': 'AFTER_POST', 'Gujarati': 'AFTER_SUB', 'Oriya': 'AFTER_MAIN',
        'Tamil': 'AFTER_SUB', 'Telugu': 'BEFORE_SUB', 'Kannada': 'BEFORE_SUB',
    }.get(block, 'AFTER_SUB')


def matra_pos_bottom(u, block):
    return {
        'Devanagari': 'AFTER_SUB', 'Bengali': 'AFTER_SUB', 'Gurmukhi': 'AFTER_POST', 'Gujarati': 'AFTER_POST',
        'Oriya': 'AFTER_SUB', 'Tamil': 'AFTER_POST', 'Telugu': 'BEFORE_SUB', 'Kannada': 'BEFORE_SUB',
        'Malayalam': 'AFTER_POST',
    }.get(block, 'AFTER_SUB')


def matra_position(u, pos, block):
    if pos == 'PRE_C':
        return 'PRE_M'
    if pos == 'POST_C':
        return matra_pos_right(u, block)
    if pos == 'ABOVE_C':
        return matra_pos_top(u, block)
    if pos == 'BELOW_C':
        return matra_pos_bottom(u, block)
    raise ValueError(pos)


def indic():
    files = ['IndicSyllabicCategory.txt', 'IndicPositionalCategory.txt', 'Blocks.txt']
    ucd = [{} for _ in files]
    for i, name in enumerate(files):
        for a, b, f in data(name):
            for u in range(a, b + 1):
                ucd[i][u] = f[0]
    defaults = ('Other', 'Not_Applicable', 'No_Block')
    combined = {}
    for i, d in enumerate(ucd):
        for u, v in d.items():
            if i == 2 and u not in combined:
                continue
            if u not in combined:
                combined[u] = list(defaults)
            combined[u][i] = v
    combined = {k: v for k, v in combined.items() if k in ALLOWED_SINGLES or v[2] in ALLOWED_BLOCKS}
    out = {}
    for k, (cat, pos, block) in combined.items():
        cat = CATEGORY_MAP[cat]
        if cat == 'SM' and pos == 'Not_Applicable':
            cat = 'SMPst'
        out[k] = (cat, POSITION_MAP[pos], block)
    dflt = ('X', 'END', 'No_Block')
    for k, c in CATEGORY_OVERRIDES.items():
        _, pos, _ = out.get(k, dflt)
        out[k] = (c, pos, ucd[2].get(k, 'No_Block'))
    for k, (cat, pos, block) in out.items():
        if cat not in ('CM', 'SM', 'RS', 'H', 'M', 'MPst'):
            out[k] = (cat, 'END', block)
    for k, (cat, pos, block) in out.items():
        if cat in ('C', 'CS', 'Ra', 'CM', 'V', 'PLACEHOLDER', 'DOTTEDCIRCLE'):
            pos = 'BASE_C'
        elif cat in ('M', 'MPst'):
            if block.startswith('Khmer') or block.startswith('Myanmar'):
                cat = {'PRE_C': 'VPre', 'ABOVE_C': 'VAbv', 'BELOW_C': 'VBlw', 'POST_C': 'VPst'}[pos]
            else:
                pos = matra_position(k, pos, block)
        elif cat in ('SM', 'SMPst', 'VD', 'A', 'Symbol'):
            pos = 'SMVD'
        out[k] = (cat, pos, block)
    for k, p in POSITION_OVERRIDES.items():
        cat, _, _ = out.get(k, dflt)
        out[k] = (cat, p, ucd[2].get(k, 'No_Block'))
    return sorted((k, CATEGORY[c] | (POSITION[p] << 8)) for k, (c, p, _) in out.items()
                  if (c, p) != ('X', 'END'))


# -----------------------------------------------------------------------------
# The Indic syllable machine: a small regular-expression compiler (Thompson
# NFA, then subset construction) for the grammar of
# hb-ot-shaper-indic-machine.rl. Longest match wins; ties go to the pattern
# listed first, as in a Ragel scanner.
# -----------------------------------------------------------------------------

class Nfa:
    def __init__(self):
        self.eps = []    # state -> [state]
        self.moves = []  # state -> [(symbols, state)]

    def state(self):
        self.eps.append([])
        self.moves.append([])
        return len(self.eps) - 1


# machine symbols: the categories 0..18 themselves, SMPst 19, others 20
SMPST_SYM = 19
OTHER_SYM = 20
NSYM = 21


def sym(*names):
    return ('sym', frozenset(SMPST_SYM if n == 'SMPst' else CATEGORY[n] for n in names))


def cat(*xs):
    return ('cat', xs)


def alt(*xs):
    return ('alt', xs)


def star(x):
    return ('star', x)


def opt(x):
    return ('opt', x)


ANY = ('any',)


def build(nfa, e, alphabet):
    """(start, end) states of a fragment for e."""
    s, t = nfa.state(), nfa.state()
    k = e[0]
    if k in ('sym', 'any'):
        syms = e[1] if k == 'sym' else frozenset(alphabet)
        nfa.moves[s].append((syms, t))
    elif k == 'cat':
        cur = s
        for x in e[1]:
            a, b = build(nfa, x, alphabet)
            nfa.eps[cur].append(a)
            cur = b
        nfa.eps[cur].append(t)
    elif k == 'alt':
        for x in e[1]:
            a, b = build(nfa, x, alphabet)
            nfa.eps[s].append(a)
            nfa.eps[b].append(t)
    elif k == 'star':
        a, b = build(nfa, e[1], alphabet)
        nfa.eps[s] += [a, t]
        nfa.eps[b] += [a, t]
    elif k == 'opt':
        a, b = build(nfa, e[1], alphabet)
        nfa.eps[s] += [a, t]
        nfa.eps[b].append(t)
    return s, t


def indic_machine():
    c = sym('C', 'Ra')
    n = cat(opt(cat(opt(sym('ZWNJ')), sym('RS'))), opt(cat(sym('N'), opt(sym('N')))))
    z = sym('ZWJ', 'ZWNJ')
    reph = alt(cat(sym('Ra'), sym('H')), sym('Repha'))
    sm = sym('SM', 'SMPst')
    cn = cat(c, opt(sym('ZWJ')), opt(n))
    symbol = cat(sym('Symbol'), opt(sym('N')))
    matra_group = cat(star(z), alt(sym('M'), cat(opt(sm), sym('MPst'))), opt(sym('N')), opt(sym('H')))
    syllable_tail = cat(opt(cat(opt(z), sm, opt(sm), opt(sym('ZWNJ')))), star(sym('A', 'VD')))
    halant_group = cat(opt(z), sym('H'), opt(cat(sym('ZWJ'), opt(sym('N')))))
    final_halant_group = alt(halant_group, cat(sym('H'), sym('ZWNJ')))
    medial_group = opt(sym('CM'))
    halant_or_matra_group = alt(final_halant_group, star(matra_group))
    complex_syllable_tail = cat(star(cat(halant_group, cn)), medial_group, halant_or_matra_group, syllable_tail)
    consonant_syllable = cat(opt(sym('Repha', 'CS')), cn, complex_syllable_tail)
    vowel_syllable = cat(opt(reph), sym('V'), opt(n), alt(sym('ZWJ'), complex_syllable_tail))
    standalone_cluster = cat(alt(cat(opt(sym('Repha', 'CS')), sym('PLACEHOLDER')), cat(opt(reph), sym('DOTTEDCIRCLE'))),
                             opt(n), complex_syllable_tail)
    symbol_cluster = cat(symbol, syllable_tail)
    broken_cluster = cat(opt(reph), opt(n), complex_syllable_tail)
    # (pattern, syllable type): consonant 0, vowel 1, standalone 2, symbol 3,
    # broken 4, non-Indic 5
    patterns = [
        (consonant_syllable, 0), (vowel_syllable, 1), (standalone_cluster, 2), (symbol_cluster, 3),
        (sym('SMPst'), 5), (broken_cluster, 4), (ANY, 5),
    ]
    alphabet = list(range(NSYM))
    nfa = Nfa()
    start = nfa.state()
    accept = {}
    for i, (p, _) in enumerate(patterns):
        a, b = build(nfa, p, alphabet)
        nfa.eps[start].append(a)
        accept[b] = i

    def closure(states):
        seen = set(states)
        stack = list(states)
        while stack:
            s = stack.pop()
            for t in nfa.eps[s]:
                if t not in seen:
                    seen.add(t)
                    stack.append(t)
        return frozenset(seen)

    d0 = closure([start])
    dstates = {d0: 0}
    order = [d0]
    trans = []
    i = 0
    while i < len(order):
        cur = order[i]
        row = {}
        for a in alphabet:
            nxt = set()
            for s in cur:
                for syms, t in nfa.moves[s]:
                    if a in syms:
                        nxt.add(t)
            if nxt:
                cl = closure(nxt)
                if cl not in dstates:
                    dstates[cl] = len(order)
                    order.append(cl)
                row[a] = dstates[cl]
        trans.append(row)
        i += 1
    acc = []
    for st in order:
        pats = [accept[s] for s in st if s in accept]
        acc.append(patterns[min(pats)][1] if pats and st != d0 else 255)
    return alphabet, trans, acc


# -----------------------------------------------------------------------------
# Vowel constraints: independent vowels followed by dependent vowel signs that
# together look like another letter get a dotted circle between them
# (HarfBuzz's gen-vowel-constraints.py, from its ms-use/IndicShapingInvalidCluster.txt;
# the sequences for the scripts the Indic shaper handles). A sequence whose
# prefix is itself a sequence never matches; RA VIRAMA I (Devanagari) is the
# only three-character one left, and std/shape handles it in code.
# -----------------------------------------------------------------------------

VOWEL_CONSTRAINTS = """
0905 0946; 0905 093E; 0909 0941; 090F 0945; 090F 0946; 090F 0947; 0905 0949; 0906 0945; 0905 094A;
0906 0946; 0905 094B; 0906 0947; 0905 094C; 0906 0948; 0905 0945; 0905 093A; 0905 093B; 0906 093A;
0905 094F; 0905 0956; 0905 0957;
0985 09BE; 098B 09C3; 098C 09E2;
0A05 0A3E; 0A72 0A3F; 0A72 0A40; 0A73 0A41; 0A73 0A42; 0A72 0A47; 0A05 0A48; 0A73 0A4B; 0A05 0A4C;
0A85 0ABE; 0A85 0AC5; 0A85 0AC7; 0A85 0AC8; 0A85 0AC9; 0A85 0ACB; 0A85 0ACC; 0AC5 0ABE;
0B05 0B3E; 0B0F 0B57; 0B13 0B57;
0B85 0BC2;
0C12 0C55; 0C12 0C4C; 0C3F 0C55; 0C46 0C55; 0C4A 0C55;
0C89 0CBE; 0C92 0CCC; 0C8B 0CBE;
0D07 0D57; 0D09 0D57; 0D0E 0D46; 0D12 0D3E; 0D12 0D57;
"""


def vowel_constraints():
    pairs = []
    for item in VOWEL_CONSTRAINTS.replace('\n', ' ').split(';'):
        f = item.split()
        if f:
            pairs.append((int(f[0], 16), int(f[1], 16)))
    return sorted(pairs)


# -----------------------------------------------------------------------------
# Output
# -----------------------------------------------------------------------------

def arr(name, typ, vals, per=0):
    """A table, in phalange fmt's layout: on one line if it fits, else one
    value per line."""
    one = 'pub let %s: [%d]%s = [%s];' % (name, len(vals), typ, ', '.join(str(v) for v in vals))
    if len(one) <= 120:
        return one
    return '\n'.join(['pub let %s: [%d]%s = [' % (name, len(vals), typ)] + ['    %s,' % v for v in vals] + ['];'])


def main():
    out = ['// ' + '=' * 77,
           '// std/shape/tables.ph - character data for text shaping, generated by',
           '// tools/gen_shape_tables.py from the Unicode Character Database. Do not edit.',
           '// ' + '=' * 77, '']
    sc = scripts()
    out.append('/// Script ranges: SCRIPT_FIRST[i]..=SCRIPT_LAST[i] have ISO 15924 code SCRIPT_TAG[i].')
    out.append(arr('SCRIPT_FIRST', 'u32', [a for a, _, _ in sc]))
    out.append(arr('SCRIPT_LAST', 'u32', [b for _, b, _ in sc]))
    out.append(arr('SCRIPT_TAG', 'u32', ['0x%08x' % tag(s) for _, _, s in sc], 8))
    out.append('')
    jt = joining()
    out.append('/// Arabic joining types: U 0, L 1, R 2, D (and C) 3, Alaph 4, Dalath-Rish 5, T 6.')
    out.append(arr('JOINING_CP', 'u32', ['0x%x' % u for u, _ in jt], 10))
    out.append(arr('JOINING_TYPE', 'u8', [v for _, v in jt], 24))
    out.append('')
    lo, hi, shaping, l2, l3, lm = arabic_shaping()
    out.append('/// Arabic presentation forms: ARABIC_FORMS[4 * (u - 0x%04x) + k] is the' % lo)
    out.append('/// initial, medial, final or isolated (k = 0..3) form of u, for u up to')
    out.append('/// U+%04X (0: none).' % hi)
    out.append('pub const ARABIC_FORMS_FIRST: u32 = 0x%04x;' % lo)
    out.append('pub const ARABIC_FORMS_LAST: u32 = 0x%04x;' % hi)
    out.append(arr('ARABIC_FORMS', 'u16', ['0x%04x' % v for v in shaping], 8))
    out.append('/// Presentation-form ligatures: rows of (first, second, ligature), then')
    out.append('/// (first, second, third, ligature), and mark ligatures (first, second,')
    out.append('/// ligature), grouped by first character in table order.')
    out.append(arr('ARABIC_LIGA2', 'u16', ['0x%04x' % v for r in l2 for v in r], 12))
    out.append(arr('ARABIC_LIGA3', 'u16', ['0x%04x' % v for r in l3 for v in r], 12))
    out.append(arr('ARABIC_LIGA_MARK', 'u16', ['0x%04x' % v for r in lm for v in r], 12))
    out.append('')
    ic = indic()
    out.append('/// Indic categories (low byte) and positions (high byte); others are X and END.')
    out.append(arr('INDIC_CP', 'u32', ['0x%x' % u for u, _ in ic], 10))
    out.append(arr('INDIC_VALUE', 'u16', ['0x%04x' % v for _, v in ic], 12))
    out.append('')
    vc = vowel_constraints()
    out.append('/// Vowel constraints: a dotted circle goes between VOWEL_FIRST[i] and VOWEL_SECOND[i].')
    out.append(arr('VOWEL_FIRST', 'u32', ['0x%04x' % a for a, _ in vc], 10))
    out.append(arr('VOWEL_SECOND', 'u32', ['0x%04x' % b for _, b in vc], 10))
    out.append('')
    alphabet, trans, acc = indic_machine()
    nsym = NSYM
    flat = []
    for row in trans:
        for a in range(nsym):
            flat.append(row.get(a, 255))
    out.append('/// The Indic syllable machine: INDIC_DFA[state * 21 + symbol] is the next')
    out.append('/// state (255: none), where the symbol is the category for categories up')
    out.append('/// to 18, 19 for SMPst (57) and 20 for all others; INDIC_ACCEPT[state] is')
    out.append('/// the syllable type it accepts (255: none). State 0 starts.')
    out.append(arr('INDIC_DFA', 'u8', flat, 21))
    out.append(arr('INDIC_ACCEPT', 'u8', acc, 20))
    out.append('')
    os.makedirs(os.path.dirname(OUT), exist_ok=True)
    with open(OUT, 'w') as f:
        f.write('\n'.join(out))
    print('wrote %s: %d script ranges, %d joining, %d indic, %d DFA states' % (
        OUT, len(sc), len(jt), len(ic), len(trans)))


if __name__ == '__main__':
    main()
