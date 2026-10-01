#!/usr/bin/env python3
"""Generates std/unicode/tables.ax from the Unicode Character Database.

    python3 tools/gen_unicode.py [UCD directory]   # default /usr/share/unicode

The UCD comes with the Debian/Ubuntu package unicode-data (or from
https://www.unicode.org/Public/UCD/latest/ucd/). The output is deterministic.
"""
import os
import sys

U = (sys.argv[1] if len(sys.argv) > 1 else '/usr/share/unicode').rstrip('/') + '/'
OUT = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'std', 'unicode', 'tables.ax')
N = 0x110000


def data_lines(name):
    for line in open(U + name, encoding='utf8'):
        yield line.rstrip('\n')


def cp_range(field):
    field = field.strip()
    if '..' in field:
        a, b = field.split('..')
        return int(a, 16), int(b, 16)
    return int(field, 16), int(field, 16)


def prop_file(name, want=None, default=None):
    """A property file as a list of N values: @missing defaults, then data.
    With want, a boolean property: whether the value (2nd field) is want."""
    vals = [default] * N if want is None else [False] * N
    missing, explicit = [], []
    for line in data_lines(name):
        if line.startswith('# @missing:'):
            parts = [p.strip() for p in line[len('# @missing:'):].split(';')]
            if want is None and len(parts) == 2:
                missing.append((cp_range(parts[0]), parts[1]))
            continue
        line = line.split('#')[0].strip()
        if not line:
            continue
        parts = [p.strip() for p in line.split(';')]
        explicit.append((cp_range(parts[0]), parts[1:]))
    for (a, b), v in missing:
        for c in range(a, b + 1):
            vals[c] = v
    for (a, b), rest in explicit:
        if want is None:
            v = rest[0]
        else:
            if rest[0] != want:
                continue
            v = True
        for c in range(a, b + 1):
            vals[c] = v
    return vals


def prop3(name, prop, value):
    """Binary property from a file of 'range; Prop; Value' lines (InCB)."""
    out = [None] * N
    for line in data_lines(name):
        line = line.split('#')[0].strip()
        if not line:
            continue
        parts = [p.strip() for p in line.split(';')]
        if len(parts) >= 3 and parts[1] == prop:
            a, b = cp_range(parts[0])
            for c in range(a, b + 1):
                out[c] = parts[2]
    return out


# --- UnicodeData ------------------------------------------------------------

GC_NAMES = ['Cn', 'Lu', 'Ll', 'Lt', 'Lm', 'Lo', 'Mn', 'Mc', 'Me', 'Nd', 'Nl', 'No', 'Pc', 'Pd', 'Ps', 'Pe', 'Pi', 'Pf',
            'Po', 'Sm', 'Sc', 'Sk', 'So', 'Zs', 'Zl', 'Zp', 'Cc', 'Cf', 'Cs', 'Co']
gc = ['Cn'] * N
ccc = [0] * N
decomp = {}
upper, lower, title = {}, {}, {}
rows = [l.split(';') for l in data_lines('UnicodeData.txt')]
i = 0
while i < len(rows):
    f = rows[i]
    c = int(f[0], 16)
    if f[1].endswith('First>'):
        e = int(rows[i + 1][0], 16)
        for x in range(c, e + 1):
            gc[x] = f[2]
            ccc[x] = int(f[3])
        i += 2
        continue
    gc[c] = f[2]
    ccc[c] = int(f[3])
    if f[5]:
        parts = f[5].split()
        compat = parts[0].startswith('<')
        if compat:
            parts = parts[1:]
        decomp[c] = (compat, [int(p, 16) for p in parts])
    if f[12]:
        upper[c] = int(f[12], 16)
    if f[13]:
        lower[c] = int(f[13], 16)
    if f[14]:
        title[c] = int(f[14], 16)
    i += 1

GB_NAMES = ['Other', 'CR', 'LF', 'Control', 'Extend', 'ZWJ', 'Regional_Indicator', 'Prepend', 'SpacingMark', 'L', 'V',
            'T', 'LV', 'LVT']
WB_NAMES = ['Other', 'CR', 'LF', 'Newline', 'Extend', 'ZWJ', 'Regional_Indicator', 'Format', 'Katakana', 'Hebrew_Letter',
            'ALetter', 'Single_Quote', 'Double_Quote', 'MidNumLet', 'MidLetter', 'MidNum', 'Numeric', 'ExtendNumLet',
            'WSegSpace']
SB_NAMES = ['Other', 'CR', 'LF', 'Extend', 'Sep', 'Format', 'Sp', 'Lower', 'Upper', 'OLetter', 'Numeric', 'ATerm',
            'SContinue', 'STerm', 'Close']
LB_NAMES = ['XX', 'BK', 'CR', 'LF', 'CM', 'NL', 'SG', 'WJ', 'ZW', 'GL', 'SP', 'ZWJ', 'B2', 'BA', 'BB', 'HY', 'CB', 'CL',
            'CP', 'EX', 'IN', 'NS', 'OP', 'QU', 'IS', 'NU', 'PO', 'PR', 'SY', 'AI', 'AK', 'AL', 'AP', 'AS', 'CJ', 'EB',
            'EM', 'H2', 'H3', 'HL', 'ID', 'JL', 'JV', 'JT', 'RI', 'SA', 'VF', 'VI']
EA_NAMES = ['N', 'A', 'H', 'W', 'F', 'Na']
BIDI_NAMES = ['L', 'R', 'AL', 'EN', 'ES', 'ET', 'AN', 'CS', 'NSM', 'BN', 'B', 'S', 'WS', 'ON', 'LRE', 'LRO', 'RLE', 'RLO',
              'PDF', 'LRI', 'RLI', 'FSI', 'PDI']
BIDI_LONG = {'Left_To_Right': 'L', 'Right_To_Left': 'R', 'Arabic_Letter': 'AL', 'European_Number': 'EN',
             'European_Separator': 'ES', 'European_Terminator': 'ET', 'Arabic_Number': 'AN', 'Common_Separator': 'CS',
             'Nonspacing_Mark': 'NSM', 'Boundary_Neutral': 'BN', 'Paragraph_Separator': 'B', 'Segment_Separator': 'S',
             'White_Space': 'WS', 'Other_Neutral': 'ON', 'Left_To_Right_Embedding': 'LRE',
             'Left_To_Right_Override': 'LRO', 'Right_To_Left_Embedding': 'RLE', 'Right_To_Left_Override': 'RLO',
             'Pop_Directional_Format': 'PDF', 'Left_To_Right_Isolate': 'LRI', 'Right_To_Left_Isolate': 'RLI',
             'First_Strong_Isolate': 'FSI', 'Pop_Directional_Isolate': 'PDI'}

gb = prop_file('auxiliary/GraphemeBreakProperty.txt', default='Other')
wb = prop_file('auxiliary/WordBreakProperty.txt', default='Other')
sb = prop_file('auxiliary/SentenceBreakProperty.txt', default='Other')
lb = prop_file('LineBreak.txt', default='XX')
ea = prop_file('EastAsianWidth.txt', default='N')
bidi = [BIDI_LONG.get(v, v) for v in prop_file('extracted/DerivedBidiClass.txt', default='L')]
ext_pict = prop_file('emoji/emoji-data.txt', want='Extended_Pictographic')
emoji_pres = prop_file('emoji/emoji-data.txt', want='Emoji_Presentation')
white = prop_file('PropList.txt', want='White_Space')
mirrored = prop_file('extracted/DerivedBinaryProperties.txt', want='Bidi_Mirrored')
alpha = prop_file('DerivedCoreProperties.txt', want='Alphabetic')
lowerp = prop_file('DerivedCoreProperties.txt', want='Lowercase')
upperp = prop_file('DerivedCoreProperties.txt', want='Uppercase')
ignorable = prop_file('DerivedCoreProperties.txt', want='Default_Ignorable_Code_Point')
id_start = prop_file('DerivedCoreProperties.txt', want='ID_Start')
id_cont = prop_file('DerivedCoreProperties.txt', want='ID_Continue')
cased = prop_file('DerivedCoreProperties.txt', want='Cased')
case_ign = prop_file('DerivedCoreProperties.txt', want='Case_Ignorable')
incb = prop3('DerivedCoreProperties.txt', 'InCB', None)
INCB = {None: 0, 'Linker': 1, 'Consonant': 2, 'Extend': 3}

for name, vals, names in [('gb', gb, GB_NAMES), ('wb', wb, WB_NAMES), ('sb', sb, SB_NAMES), ('lb', lb, LB_NAMES),
                          ('ea', ea, EA_NAMES), ('bidi', bidi, BIDI_NAMES), ('gc', gc, GC_NAMES)]:
    bad = set(vals) - set(names)
    assert not bad, (name, bad)

# The fields of a record (bit offset, width)
FIELDS = [('GC', 5), ('GB', 4), ('WB', 5), ('SB', 4), ('LB', 6), ('EA', 3), ('BIDI', 5), ('CCC', 8), ('EXT_PICT', 1),
          ('WHITE', 1), ('ALPHA', 1), ('LOWER', 1), ('UPPER', 1), ('IGNORABLE', 1), ('INCB', 2), ('EMOJI_PRES', 1),
          ('MIRRORED', 1), ('ID_START', 1), ('ID_CONTINUE', 1), ('CASED', 1), ('CASE_IGNORABLE', 1)]
offsets = {}
at = 0
for name, width in FIELDS:
    offsets[name] = (at, width)
    at += width
assert at <= 64


def record(c):
    vals = {'GC': GC_NAMES.index(gc[c]), 'GB': GB_NAMES.index(gb[c]), 'WB': WB_NAMES.index(wb[c]),
            'SB': SB_NAMES.index(sb[c]), 'LB': LB_NAMES.index(lb[c]), 'EA': EA_NAMES.index(ea[c]),
            'BIDI': BIDI_NAMES.index(bidi[c]), 'CCC': ccc[c], 'EXT_PICT': int(ext_pict[c]), 'WHITE': int(white[c]),
            'ALPHA': int(alpha[c]), 'LOWER': int(lowerp[c]), 'UPPER': int(upperp[c]), 'IGNORABLE': int(ignorable[c]),
            'INCB': INCB[incb[c]], 'EMOJI_PRES': int(emoji_pres[c]), 'MIRRORED': int(mirrored[c]),
            'ID_START': int(id_start[c]), 'ID_CONTINUE': int(id_cont[c]), 'CASED': int(cased[c]),
            'CASE_IGNORABLE': int(case_ign[c])}
    r = 0
    for name, width in FIELDS:
        o, w = offsets[name]
        assert vals[name] < (1 << w), (name, vals[name])
        r |= vals[name] << o
    return r


recs = {}
rec_of = [0] * N
for c in range(N):
    r = record(c)
    if r not in recs:
        recs[r] = len(recs)
    rec_of[c] = recs[r]
run_start, run_rec = [], []
prev = None
for c in range(N):
    if rec_of[c] != prev:
        run_start.append(c)
        run_rec.append(rec_of[c])
        prev = rec_of[c]
LOW = 0x800

# --- case ---------------------------------------------------------------------

special_lower, special_upper, special_title = {}, {}, {}
for line in data_lines('SpecialCasing.txt'):
    line = line.split('#')[0].strip()
    if not line:
        continue
    parts = [p.strip() for p in line.split(';')]
    if len(parts) > 4 and parts[4]:
        continue  # conditional (language or context): handled in code, or not at all
    c = int(parts[0], 16)
    for table, field in [(special_lower, parts[1]), (special_title, parts[2]), (special_upper, parts[3])]:
        seq = [int(x, 16) for x in field.split()]
        if len(seq) != 1 or seq[0] != c:
            table[c] = seq
fold_full, fold_simple = {}, {}
for line in data_lines('CaseFolding.txt'):
    line = line.split('#')[0].strip()
    if not line:
        continue
    parts = [p.strip() for p in line.split(';')]
    c, status, seq = int(parts[0], 16), parts[1], [int(x, 16) for x in parts[2].split()]
    if status in ('C', 'F'):
        fold_full[c] = seq
    if status in ('C', 'S'):
        fold_simple[c] = seq[0]

# --- normalization ------------------------------------------------------------

fce = prop_file('DerivedNormalizationProps.txt', want='Full_Composition_Exclusion')
compose = {}
for c, (compat, seq) in decomp.items():
    if not compat and len(seq) == 2 and not fce[c]:
        compose[(seq[0], seq[1])] = c

# --- bidi extras -------------------------------------------------------------

mirror = {}
for line in data_lines('BidiMirroring.txt'):
    line = line.split('#')[0].strip()
    if line:
        a, b = [p.strip() for p in line.split(';')]
        mirror[int(a, 16)] = int(b, 16)
brackets = {}
for line in data_lines('BidiBrackets.txt'):
    line = line.split('#')[0].strip()
    if line:
        a, b, t = [p.strip() for p in line.split(';')]
        brackets[int(a, 16)] = (int(b, 16), t)

# --- output -------------------------------------------------------------------


def arr(name, ty, vals, per=16, doc=None):
    out = []
    if doc:
        out.append('/// ' + doc)
    out.append('pub let %s: [%d]%s = [' % (name, len(vals), ty))
    for k in range(0, len(vals), per):
        out.append('    ' + ', '.join(str(v) for v in vals[k:k + per]) + ',')
    out.append('];')
    return '\n'.join(out)


def seq_table(prefix, table, doc):
    keys = sorted(table)
    pool, offs = [], []
    for k in keys:
        seq = table[k]
        offs.append((len(pool) << 8) | len(seq))
        pool.extend(seq)
    return '\n\n'.join([
        arr(prefix + '_KEYS', 'u32', keys, doc=doc + ': the code points (sorted)'),
        arr(prefix + '_SEQ', 'u32', offs, doc='(offset into ' + prefix + '_POOL) << 8 | length'),
        arr(prefix + '_POOL', 'u32', pool)])


def pair_table(prefix, table, doc):
    keys = sorted(table)
    return '\n\n'.join([
        arr(prefix + '_FROM', 'u32', keys, doc=doc + ' (sorted)'),
        arr(prefix + '_TO', 'u32', [table[k] for k in keys])])


# the version, from DerivedAge's header
ver = None
for line in data_lines('DerivedAge.txt'):
    if line.startswith('# DerivedAge-'):
        ver = line[len('# DerivedAge-'):].split('.txt')[0]
        break
parts = ['''// =============================================================================
// std/unicode/tables.ax - Unicode %s character data, generated by
// tools/gen_unicode.py from the Unicode Character Database. Do not edit.
// =============================================================================
''' % ver]
parts.append('pub let VERSION: []u8 = "%s";' % ver)
parts.append('\n'.join('pub const F_%s: u32 = %d;' % (n, offsets[n][0]) for n, _ in FIELDS))
parts.append('\n'.join('pub const W_%s: u32 = %d;' % (n, offsets[n][1]) for n, _ in FIELDS))
rec_list = sorted(recs, key=lambda r: recs[r])
parts.append(arr('RECORDS', 'u64', rec_list, per=8, doc='The distinct property records (fields at F_*, widths W_*).'))
parts.append(arr('LOW', 'u16', rec_of[:LOW], per=32, doc='The record of each code point below U+%04X.' % LOW))
parts.append('pub const LOW_LIMIT: u32 = %d;' % LOW)
parts.append(arr('RUN_START', 'u32', run_start, doc='Runs of code points with one record: where each starts.'))
parts.append(arr('RUN_RECORD', 'u16', run_rec, per=24))
parts.append(pair_table('UPPER', upper, 'Simple uppercase mappings'))
parts.append(pair_table('LOWER', lower, 'Simple lowercase mappings'))
parts.append(pair_table('TITLE', {k: v for k, v in title.items() if upper.get(k) != v}, 'Simple titlecase mappings that differ from the uppercase ones'))
parts.append(pair_table('FOLD', fold_simple, 'Simple case folding'))
parts.append(seq_table('SPECIAL_UPPER', special_upper, 'Full uppercase mappings of more than one code point'))
parts.append(seq_table('SPECIAL_LOWER', special_lower, 'Full lowercase mappings of more than one code point'))
parts.append(seq_table('SPECIAL_TITLE', special_title, 'Full titlecase mappings of more than one code point'))
parts.append(seq_table('FULL_FOLD', {k: v for k, v in fold_full.items() if len(v) > 1}, 'Full case foldings of more than one code point'))
canon = {c: s for c, (compat, s) in decomp.items() if not compat}
compat_ = {c: s for c, (compat, s) in decomp.items() if compat}
parts.append(seq_table('CANON', canon, 'Canonical decompositions (one level)'))
parts.append(seq_table('COMPAT', compat_, 'Compatibility decompositions (one level)'))
ckeys = sorted(compose)
parts.append(arr('COMPOSE_KEY', 'u64', [(a << 21) | b for a, b in ckeys], per=8, doc='Canonical compositions: first << 21 | second (sorted)'))
parts.append(arr('COMPOSE_TO', 'u32', [compose[k] for k in ckeys]))
parts.append(pair_table('MIRROR', mirror, 'Bidi mirroring glyphs'))
bkeys = sorted(brackets)
parts.append(arr('BRACKET_FROM', 'u32', bkeys, doc='Bidi paired brackets (sorted)'))
parts.append(arr('BRACKET_PAIR', 'u32', [brackets[k][0] for k in bkeys]))
parts.append(arr('BRACKET_OPEN', 'u8', [1 if brackets[k][1] == 'o' else 0 for k in bkeys], per=32))
for label, names in [('GC', GC_NAMES), ('GB', GB_NAMES), ('WB', WB_NAMES), ('SB', SB_NAMES), ('LB', LB_NAMES),
                     ('EA', EA_NAMES), ('BIDI', BIDI_NAMES)]:
    parts.append('\n'.join('pub const %s_%s: u32 = %d;' % (label, n.upper(), k) for k, n in enumerate(names)))
    parts.append('pub let %s_NAMES: [%d][]u8 = [%s];' % (label, len(names), ', '.join('"%s"' % n for n in names)))
os.makedirs(os.path.dirname(OUT), exist_ok=True)
with open(OUT, 'w') as f:
    f.write('\n\n'.join(parts) + '\n')
# in axiom fmt's canonical layout (the repository is kept formatted)
root = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..')
for exe in ('selfhost/build/axiom', 'selfhost/build/good'):
    if os.path.exists(os.path.join(root, exe)):
        import subprocess
        subprocess.run([os.path.join(root, exe), 'fmt', OUT], check=True)
        break
else:
    print('note: build selfhost/build/axiom and run `axiom fmt` on the output')
print('records %d, runs %d, compositions %d -> %s' % (len(recs), len(run_start), len(compose), OUT))
