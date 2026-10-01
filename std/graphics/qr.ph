// =============================================================================
// std/graphics/qr.ph - QR codes (ISO/IEC 18004:2015 model 2): encoding text or
// bytes into a symbol, and drawing it as an image, a draw.Path or SVG.
//
//     let mut code = qr.encode("https://example.org/", qr.Ecc.M)!;
//     defer code.deinit();
//     let dark = code.get(x, y);                           // a module
//     let mut img = qr.to_image(&code, 8, 4)!;            // 8 pixels a module, 4 modules of margin
//     defer img.deinit();
//     qr.write_svg(w, &code, 4)!;                          // to an io.Writer
//     code.add_to_path(&mut path, 72.0, 72.0, 2.0)!;       // for std/graphics/draw or pdf
//
// Versions 1 to 40 and the four error correction levels (Ecc: L, M, Q, H).
// Text is split into numeric, alphanumeric and byte segments, choosing
// where to switch modes so that the bit stream is as short as possible
// (dynamic programming over the characters, as in Project Nayuki's
// generator); the smallest version that holds it is used. Text with bytes
// outside ASCII is marked as UTF-8 with an ECI designator (assignment 26)
// unless Options.eci is false. Kanji mode is available for segments given
// explicitly in Shift JIS (Segment.kanji); automatic segmentation never
// uses it. The data is cut into Reed-Solomon blocks (GF(256), polynomial
// 0x11d), interleaved, placed, and masked with each of the eight masks,
// keeping the one with the lowest penalty (N1 to N4 of section 7.8.3, scored
// as segno does: before the format and version information are placed, and
// the N3 pattern 1011101 itself with four light modules, or the edge, on
// one side); format and version information carry BCH codes.
//
// Checked against matrices made by segno (std/graphics/qr/testdata, from
// tools/gen_qr_tests.py: 39 symbols, versions 1 to 40, every level, mode and
// mask), and against the worked "HELLO WORLD" 1-M and 1-Q examples'
// codewords and the standard's format and version information tables.
// =============================================================================

import "../collections/vec.ph" as vec;
import "../fmt.ph" as fmt;
import "../io.ph" as io;
import "../mem.ph" as mem;
import "../os/fs.ph" as fs;
import "../str.ph" as strs;
import "color.ph" as color;
import "draw.ph" as draw;
import "image.ph" as image;

error { OutOfMemory, DataTooLong, InvalidSegment, InvalidOption }

/// Error correction level: about 7%, 15%, 25% or 30% of the codewords can
/// be restored.
pub enum Ecc: u8 {
    L,
    M,
    Q,
    H,
}

/// How a segment's characters are coded.
pub enum Mode: u8 {
    NUMERIC,
    ALPHANUMERIC,
    BYTE,
    KANJI,
    ECI,
}

/// A piece of the data in one mode. data holds the characters: ASCII
/// digits (NUMERIC), characters of "0-9A-Z $%*+-./:" (ALPHANUMERIC), any
/// bytes (BYTE), Shift JIS double-byte characters (KANJI); an ECI segment
/// has no data, only its assignment number.
pub struct Segment {
    mode: Mode,
    data: []u8,
    eci: u32,

    /// Decimal digits.
    pub fn numeric(digits: []u8) Segment {
        return Segment.{ mode: Mode.NUMERIC, data: digits, eci: 0 };
    }

    /// Characters of "0-9A-Z $%*+-./:".
    pub fn alphanumeric(text: []u8) Segment {
        return Segment.{ mode: Mode.ALPHANUMERIC, data: text, eci: 0 };
    }

    /// Any bytes.
    pub fn bytes(data: []u8) Segment {
        return Segment.{ mode: Mode.BYTE, data: data, eci: 0 };
    }

    /// Kanji in Shift JIS: two bytes a character, 0x8140-0x9ffc or 0xe040-0xebbf.
    pub fn kanji(sjis: []u8) Segment {
        return Segment.{ mode: Mode.KANJI, data: sjis, eci: 0 };
    }

    /// An ECI designator (26 is UTF-8, 3 ISO-8859-1, 20 Shift JIS).
    pub fn eci_designator(assignment: u32) Segment {
        return Segment.{ mode: Mode.ECI, data: "", eci: assignment };
    }

    /// The number of characters the count indicator gives.
    fn char_count(self: Segment) usize {
        if self.mode == Mode.KANJI {
            return self.data.len / 2;
        }
        return self.data.len;
    }
}

/// More characters than any symbol holds (7089 digits at version 40-L).
const MAX_TEXT: usize = 7089;

/// Mask choice for Options.mask: the one with the lowest penalty.
pub const AUTO_MASK: u32 = 8;

/// How to encode.
pub struct Options {
    ecc: Ecc,
    min_version: u32,  // 1 to 40
    max_version: u32,
    mask: u32,        // 0 to 7, or AUTO_MASK
    eci: bool,        // mark non-ASCII text as UTF-8 with an ECI designator
    boost_ecc: bool,  // raise the level while the data still fits the version

    /// The defaults at a level: any version, the best mask, ECI for non-ASCII text.
    pub fn init(ecc: Ecc) Options {
        return Options.{ ecc: ecc, min_version: 1, max_version: 40, mask: AUTO_MASK, eci: true, boost_ecc: false };
    }
}

/// A QR code symbol: size x size modules (size = 4 version + 17).
pub struct Code {
    version: u32,
    size: usize,
    ecc: Ecc,
    mask: u32,
    modules: []mut u8,  // row by row, 1 dark, 0 light

    pub fn deinit(self: *mut Code) void {
        mem.free(self.modules);
    }

    /// Whether the module at column x, row y is dark (false outside the symbol).
    pub fn get(self: *Code, x: usize, y: usize) bool {
        if (x >= self.size) or (y >= self.size) {
            return false;
        }
        return self.modules[(y * self.size) + x] != 0;
    }

    /// Adds the dark modules as rectangles (a run of dark modules in a row
    /// is one rectangle) to a path: the symbol's top left at (x, y),
    /// module by module wide. Fill it with the nonzero rule.
    pub fn add_to_path(self: *Code, p: *mut draw.Path, x: f64, y: f64, module: f64) !void {
        let n = self.size;
        for r in @as(usize, 0)..n {
            let mut c: usize = 0;
            while c < n {
                if not self.get(c, r) {
                    c += 1;
                    continue;
                }
                let start = c;
                while (c < n) and self.get(c, r) {
                    c += 1;
                }
                p.rect(x + (@float_from_int(f64, start) * module), y + (@float_from_int(f64, r) * module),
                       @float_from_int(f64, c - start) * module, module)!;
            }
        }
    }
}

// -----------------------------------------------------------------------------
// Tables (ISO/IEC 18004:2015 table 9; the same as Project Nayuki's)
// -----------------------------------------------------------------------------

/// Error correction codewords per block, by level (L, M, Q, H) and version - 1.
let ECC_PER_BLOCK: [160]u8 = [
    7,
    10,
    15,
    20,
    26,
    18,
    20,
    24,
    30,
    18,
    20,
    24,
    26,
    30,
    22,
    24,
    28,
    30,
    28,
    28,
    28,
    28,
    30,
    30,
    26,
    28,
    30,
    30,
    30,
    30,
    30,
    30,
    30,
    30,
    30,
    30,
    30,
    30,
    30,
    30,
    10,
    16,
    26,
    18,
    24,
    16,
    18,
    22,
    22,
    26,
    30,
    22,
    22,
    24,
    24,
    28,
    28,
    26,
    26,
    26,
    26,
    28,
    28,
    28,
    28,
    28,
    28,
    28,
    28,
    28,
    28,
    28,
    28,
    28,
    28,
    28,
    28,
    28,
    28,
    28,
    13,
    22,
    18,
    26,
    18,
    24,
    18,
    22,
    20,
    24,
    28,
    26,
    24,
    20,
    30,
    24,
    28,
    28,
    26,
    30,
    28,
    30,
    30,
    30,
    30,
    28,
    30,
    30,
    30,
    30,
    30,
    30,
    30,
    30,
    30,
    30,
    30,
    30,
    30,
    30,
    17,
    28,
    22,
    16,
    22,
    28,
    26,
    26,
    24,
    28,
    24,
    28,
    22,
    24,
    24,
    30,
    28,
    28,
    26,
    28,
    30,
    24,
    30,
    30,
    30,
    30,
    30,
    30,
    30,
    30,
    30,
    30,
    30,
    30,
    30,
    30,
    30,
    30,
    30,
    30,
];

/// Error correction blocks, by level and version - 1.
let NUM_BLOCKS: [160]u8 = [
    1,
    1,
    1,
    1,
    1,
    2,
    2,
    2,
    2,
    4,
    4,
    4,
    4,
    4,
    6,
    6,
    6,
    6,
    7,
    8,
    8,
    9,
    9,
    10,
    12,
    12,
    12,
    13,
    14,
    15,
    16,
    17,
    18,
    19,
    19,
    20,
    21,
    22,
    24,
    25,
    1,
    1,
    1,
    2,
    2,
    4,
    4,
    4,
    5,
    5,
    5,
    8,
    9,
    9,
    10,
    10,
    11,
    13,
    14,
    16,
    17,
    17,
    18,
    20,
    21,
    23,
    25,
    26,
    28,
    29,
    31,
    33,
    35,
    37,
    38,
    40,
    43,
    45,
    47,
    49,
    1,
    1,
    2,
    2,
    4,
    4,
    6,
    6,
    8,
    8,
    8,
    10,
    12,
    16,
    12,
    17,
    16,
    18,
    21,
    20,
    23,
    23,
    25,
    27,
    29,
    34,
    34,
    35,
    38,
    40,
    43,
    45,
    48,
    51,
    53,
    56,
    59,
    62,
    65,
    68,
    1,
    1,
    2,
    4,
    4,
    4,
    5,
    6,
    8,
    8,
    11,
    11,
    16,
    16,
    18,
    16,
    19,
    21,
    25,
    25,
    25,
    34,
    30,
    32,
    35,
    37,
    40,
    42,
    45,
    48,
    51,
    54,
    57,
    60,
    63,
    66,
    70,
    74,
    77,
    81,
];

fn ecc_index(e: Ecc) usize {
    return @as(usize, @int_from_enum(e));
}

/// The level's two bits in the format information.
fn ecc_format_bits(e: Ecc) u32 {
    match e {
        .L => { return 1; }
        .M => { return 0; }
        .Q => { return 3; }
        .H => { return 2; }
    }
}

/// Modules that hold data and error correction (not function patterns or
/// format and version information), in bits.
fn raw_data_modules(version: u32) usize {
    let v = @as(usize, version);
    let mut r = (((16 * v) + 128) * v) + 64;
    if v >= 2 {
        let na = (v / 7) + 2;
        r -= (((25 * na) - 10) * na) - 55;
        if v >= 7 {
            r -= 36;
        }
    }
    return r;
}

/// Data codewords (after removing error correction) of a version and level.
fn data_codewords(version: u32, e: Ecc) usize {
    let i = (ecc_index(e) * 40) + @as(usize, version - 1);
    return (raw_data_modules(version) / 8) - (@as(usize, ECC_PER_BLOCK[i]) * @as(usize, NUM_BLOCKS[i]));
}

/// The bits of a mode's character count indicator in a version.
fn count_bits(m: Mode, version: u32) usize {
    let mut g: usize = 0;
    if version >= 27 {
        g = 2;
    } else if version >= 10 {
        g = 1;
    }
    match m {
        .NUMERIC      => { return 10 + (2 * g); }
        .ALPHANUMERIC => { return 9 + (2 * g); }
        .BYTE => {
            if g == 0 {
                return 8;
            }
            return 16;
        }
        .KANJI => { return 8 + (2 * g); }
        .ECI   => { return 0; }
    }
}

fn mode_bits(m: Mode) u32 {
    match m {
        .NUMERIC      => { return 1; }
        .ALPHANUMERIC => { return 2; }
        .BYTE         => { return 4; }
        .KANJI        => { return 8; }
        .ECI          => { return 7; }
    }
}

let ALNUM: []u8 = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ $%*+-./:";

/// The alphanumeric value of a character (45 if it has none).
fn alnum_value(c: u8) u32 {
    if (c >= '0') and (c <= '9') {
        return @as(u32, c - '0');
    }
    if (c >= 'A') and (c <= 'Z') {
        return @as(u32, c - 'A') + 10;
    }
    for i in @as(usize, 36)..45 {
        if ALNUM[i] == c {
            return @truncate(u32, i);
        }
    }
    return 45;
}

fn is_digit(c: u8) bool {
    return (c >= '0') and (c <= '9');
}

/// A Shift JIS kanji's 13-bit value, or 0x2000 if it is not one.
fn kanji_value(hi: u8, lo: u8) u32 {
    let v = (@as(u32, hi) << 8) | @as(u32, lo);
    let mut d: u32 = 0;
    if (v >= 0x8140) and (v <= 0x9ffc) {
        d = v - 0x8140;
    } else if (v >= 0xe040) and (v <= 0xebbf) {
        d = v - 0xc140;
    } else {
        return 0x2000;
    }
    if (lo < 0x40) or (lo == 0x7f) or (lo > 0xfc) {
        return 0x2000;
    }
    return ((d >> 8) * 0xc0) + (d & 0xff);
}

// -----------------------------------------------------------------------------
// Bit streams
// -----------------------------------------------------------------------------

struct Bits {
    bytes: vec.Vec.[u8],
    len: usize,  // in bits

    fn init() Bits {
        return Bits.{ bytes: vec.Vec.[u8].init(), len: 0 };
    }

    fn deinit(self: *mut Bits) void {
        self.bytes.deinit();
    }

    /// Appends the low n bits of v, most significant first.
    fn put(self: *mut Bits, v: u32, n: usize) !void {
        let mut i = n;
        while i > 0 {
            i -= 1;
            if (self.len % 8) == 0 {
                self.bytes.push(0)!;
            }
            if ((v >> @truncate(u32, i)) & 1) != 0 {
                let k = self.len / 8;
                self.bytes.set(k, self.bytes.get(k) | (@as(u8, 0x80) >> @truncate(u8, self.len % 8)));
            }
            self.len += 1;
        }
    }
}

/// The bits of a segment's data (without its header); checks the characters.
fn segment_data_bits(s: Segment) !usize {
    let n = s.data.len;
    match s.mode {
        .NUMERIC => {
            for c in s.data {
                if not is_digit(c) {
                    return error.InvalidSegment;
                }
            }
            let mut b = (n / 3) * 10;
            if (n % 3) == 1 {
                b += 4;
            } else if (n % 3) == 2 {
                b += 7;
            }
            return b;
        }
        .ALPHANUMERIC => {
            for c in s.data {
                if alnum_value(c) >= 45 {
                    return error.InvalidSegment;
                }
            }
            return ((n / 2) * 11) + ((n % 2) * 6);
        }
        .BYTE => { return n * 8; }
        .KANJI => {
            if (n % 2) != 0 {
                return error.InvalidSegment;
            }
            let mut i: usize = 0;
            while i < n {
                if kanji_value(s.data[i], s.data[i + 1]) >= 0x2000 {
                    return error.InvalidSegment;
                }
                i += 2;
            }
            return (n / 2) * 13;
        }
        .ECI => {
            if s.eci < 128 {
                return 8;
            }
            if s.eci < 16384 {
                return 16;
            }
            if s.eci < 1000000 {
                return 24;
            }
            return error.InvalidSegment;
        }
    }
}

/// The total bits of segments in a version (0x7fffffff... if a count does
/// not fit its indicator).
fn total_bits(segs: []Segment, version: u32) !usize {
    let mut t: usize = 0;
    for s in segs {
        let cb = count_bits(s.mode, version);
        if (cb > 0) and (s.char_count() >= (@as(usize, 1) << @truncate(u32, cb))) {
            return error.DataTooLong;
        }
        t += (4 + cb) + segment_data_bits(s)!;
    }
    return t;
}

fn append_segment(b: *mut Bits, s: Segment, version: u32) !void {
    b.put(mode_bits(s.mode), 4)!;
    if s.mode == Mode.ECI {
        if s.eci < 128 {
            b.put(s.eci, 8)!;
        } else if s.eci < 16384 {
            b.put(0x8000 | s.eci, 16)!;
        } else {
            b.put(0xc00000 | s.eci, 24)!;
        }
        return;
    }
    b.put(@truncate(u32, s.char_count()), count_bits(s.mode, version))!;
    let d = s.data;
    match s.mode {
        .NUMERIC => {
            let mut i: usize = 0;
            while i < d.len {
                let k = @min(3, d.len - i);
                let mut v: u32 = 0;
                for j in i..i + k {
                    v = (v * 10) + @as(u32, d[j] - '0');
                }
                b.put(v, (k * 3) + 1)!;
                i += k;
            }
        }
        .ALPHANUMERIC => {
            let mut i: usize = 0;
            while (i + 1) < d.len {
                b.put((alnum_value(d[i]) * 45) + alnum_value(d[i + 1]), 11)!;
                i += 2;
            }
            if i < d.len {
                b.put(alnum_value(d[i]), 6)!;
            }
        }
        .BYTE => {
            for c in d {
                b.put(@as(u32, c), 8)!;
            }
        }
        .KANJI => {
            let mut i: usize = 0;
            while (i + 1) < d.len {
                b.put(kanji_value(d[i], d[i + 1]), 13)!;
                i += 2;
            }
        }
        .ECI => {}
    }
}

// -----------------------------------------------------------------------------
// Segmentation: the cheapest sequence of modes for a text
// -----------------------------------------------------------------------------

const INF: usize = 0x3fffffff;  // above any cost of MAX_TEXT characters

fn ceil6(x: usize) usize {
    return ((x + 5) / 6) * 6;
}

/// Splits text into numeric, alphanumeric and byte segments whose bit
/// stream (in a version of the given range: 1-9, 10-26 or 27-40, which have
/// the same count indicator lengths) is as short as possible. The segments'
/// data points into text.
pub fn segment_text(text: []u8, version: u32, out: *mut vec.Vec.[Segment]) !void {
    out.clear();
    let n = text.len;
    if n == 0 {
        return;
    }
    if n > MAX_TEXT {
        return error.DataTooLong;
    }
    // costs in sixths of a bit; state m = the mode of the current character
    // (0 numeric, 1 alphanumeric, 2 byte)
    let modes: [3]Mode = [Mode.NUMERIC, Mode.ALPHANUMERIC, Mode.BYTE];
    let mut head: [3]usize = [0; 3];
    for m in @as(usize, 0)..3 {
        head[m] = (4 + count_bits(modes[m], version)) * 6;
    }
    // from[i * 3 + m]: the mode of character i - 1 on the best path that has
    // character i in mode m
    let from = mem.alloc.[u8](n * 3)!;
    defer mem.free(from);
    let mut cost: [3]usize = [INF; 3];
    for i in @as(usize, 0)..n {
        let c = text[i];
        let mut each: [3]usize = [INF, INF, 48];
        if is_digit(c) {
            each[0] = 20;
        }
        if alnum_value(c) < 45 {
            each[1] = 33;
        }
        let mut next: [3]usize = [INF; 3];
        for m in @as(usize, 0)..3 {
            if each[m] >= INF {
                continue;
            }
            if i == 0 {
                next[m] = head[m] + each[m];
                from[m] = @truncate(u8, m);
                continue;
            }
            // stay in m, or end a segment of mode k and start one of m
            let mut best = INF;
            let mut bk: usize = 0;
            if cost[m] < INF {
                best = cost[m];
                bk = m;
            }
            for k in @as(usize, 0)..3 {
                if (k != m) and (cost[k] < INF) {
                    let v = ceil6(cost[k]) + head[m];
                    if v < best {
                        best = v;
                        bk = k;
                    }
                }
            }
            next[m] = best + each[m];
            from[(i * 3) + m] = @truncate(u8, bk);
        }
        cost = next;
    }
    let mut m: usize = 2;
    for k in @as(usize, 0)..3 {
        if ceil6(cost[k]) < ceil6(cost[m]) {
            m = k;
        }
    }
    // walk back, collecting runs (from the end)
    let mut end = n;
    let mut i = n;
    while i > 0 {
        i -= 1;
        let prev = @as(usize, from[(i * 3) + m]);
        if (i == 0) or (prev != m) {
            out.push(Segment.{ mode: modes[m], data: text[i..end], eci: 0 })!;
            end = i;
        }
        m = prev;
    }
    vec.reverse(out.items());
}

// -----------------------------------------------------------------------------
// Encoding
// -----------------------------------------------------------------------------

/// Encodes text (UTF-8) at a level, in the smallest version that holds it.
pub fn encode(text: []u8, ecc: Ecc) !Code {
    return encode_with(text, Options.init(ecc));
}

/// Encodes text with options.
pub fn encode_with(text: []u8, opts: Options) !Code {
    if (opts.min_version < 1) or (opts.max_version > 40) or (opts.min_version > opts.max_version) {
        return error.InvalidOption;
    }
    let mut ascii = true;
    for c in text {
        if c >= 0x80 {
            ascii = false;
        }
    }
    if text.len > MAX_TEXT {
        return error.DataTooLong;
    }
    let use_eci = opts.eci and not ascii;
    let mut segs = vec.Vec.[Segment].init();
    defer segs.deinit();
    // the segmentation depends on the count indicator lengths: try each group
    let groups: [3][2]u32 = [[1, 9], [10, 26], [27, 40]];
    for g in groups {
        let lo = @max(g[0], opts.min_version);
        let hi = @min(g[1], opts.max_version);
        if lo > hi {
            continue;
        }
        segs.clear();
        if use_eci {
            segs.push(Segment.eci_designator(26))!;
        }
        let mut part = vec.Vec.[Segment].init();
        defer part.deinit();
        segment_text(text, lo, &mut part)!;
        segs.extend(part.items())!;
        let bits = total_bits(segs.items(), lo) catch continue;
        for v in lo..hi + 1 {
            if bits <= (data_codewords(v, opts.ecc) * 8) {
                let mut o = opts;
                o.min_version = v;
                o.max_version = v;
                return encode_segments(segs.items(), o);
            }
        }
    }
    return error.DataTooLong;
}

/// Encodes raw bytes (one byte segment, no ECI).
pub fn encode_binary(data: []u8, ecc: Ecc) !Code {
    let segs: [1]Segment = [Segment.bytes(data)];
    return encode_segments(segs[..], Options.init(ecc));
}

/// Encodes segments as given, in the smallest version in the options' range
/// that holds them.
pub fn encode_segments(segs: []Segment, opts: Options) !Code {
    if (opts.min_version < 1) or (opts.max_version > 40) or (opts.min_version > opts.max_version) or
       (opts.mask > AUTO_MASK) {
        return error.InvalidOption;
    }
    let mut version = opts.min_version;
    let mut used: usize = 0;
    loop {
        let fits = total_bits(segs, version) catch @max_value(usize);
        if fits <= (data_codewords(version, opts.ecc) * 8) {
            used = fits;
            break;
        }
        if version >= opts.max_version {
            if fits == @max_value(usize) {
                // (an invalid segment is not a length problem)
                for s in segs {
                    _ = segment_data_bits(s)!;
                }
            }
            return error.DataTooLong;
        }
        version += 1;
    }
    let mut ecc = opts.ecc;
    if opts.boost_ecc {
        let ups: [3]Ecc = [Ecc.M, Ecc.Q, Ecc.H];
        for e in ups {
            if (ecc_index(e) > ecc_index(ecc)) and (used <= (data_codewords(version, e) * 8)) {
                ecc = e;
            }
        }
    }
    let mut b = Bits.init();
    defer b.deinit();
    for s in segs {
        append_segment(&mut b, s, version)!;
    }
    // terminator, byte alignment, pad codewords
    let cap = data_codewords(version, ecc) * 8;
    b.put(0, @min(4, cap - b.len))!;
    b.put(0, (8 - (b.len % 8)) % 8)!;
    let mut pad: u32 = 0xec;
    while b.len < cap {
        b.put(pad, 8)!;
        pad = pad ^ (0xec ^ 0x11);
    }
    let cw = add_ecc(b.bytes.items(), version, ecc)!;
    defer mem.free(cw);
    return build(cw, version, ecc, opts.mask);
}

// -----------------------------------------------------------------------------
// Reed-Solomon
// -----------------------------------------------------------------------------

/// x * y in GF(2^8) modulo x^8 + x^4 + x^3 + x^2 + 1.
fn gf_mul(x: u8, y: u8) u8 {
    let mut z: u32 = 0;
    let mut i: u32 = 8;
    while i > 0 {
        i -= 1;
        z = (z << 1) ^ ((z >> 7) * 0x11d);
        z = z ^ (((@as(u32, y) >> i) & 1) * @as(u32, x));
    }
    return @truncate(u8, z);
}

/// The generator polynomial of degree n (its coefficients after the leading 1,
/// highest first): the product of (x - a^i) for i in 0..n.
fn rs_divisor(n: usize, out: []mut u8) void {
    @mem_set(out[..n], 0);
    out[n - 1] = 1;
    let mut root: u8 = 1;
    for _i in @as(usize, 0)..n {
        for j in @as(usize, 0)..n {
            out[j] = gf_mul(out[j], root);
            if (j + 1) < n {
                out[j] = out[j] ^ out[j + 1];
            }
        }
        root = gf_mul(root, 2);
    }
}

/// The remainder of data(x) x^n divided by the generator.
fn rs_remainder(data: []u8, div: []u8, out: []mut u8) void {
    let n = div.len;
    @mem_set(out[..n], 0);
    for d in data {
        let factor = d ^ out[0];
        for j in @as(usize, 1)..n {
            out[j - 1] = out[j];
        }
        out[n - 1] = 0;
        for j in @as(usize, 0)..n {
            out[j] = out[j] ^ gf_mul(div[j], factor);
        }
    }
}

/// The codewords to place: the data cut into blocks, each followed by its
/// error correction, interleaved.
fn add_ecc(data: []u8, version: u32, e: Ecc) ![]mut u8 {
    let ti = (ecc_index(e) * 40) + @as(usize, version - 1);
    let nblocks = @as(usize, NUM_BLOCKS[ti]);
    let ecl = @as(usize, ECC_PER_BLOCK[ti]);
    let raw = raw_data_modules(version) / 8;
    let nshort = nblocks - (raw % nblocks);
    let short_len = raw / nblocks;  // a short block, with its error correction
    let out = mem.alloc.[u8](raw)!;
    let mut div: [30]u8 = [0; 30];
    rs_divisor(ecl, div[..]);
    let mut rem: [30]u8 = [0; 30];
    let mut k: usize = 0;
    let short_data = short_len - ecl;
    for i in @as(usize, 0)..nblocks {
        let mut dl = short_data;
        if i >= nshort {
            dl += 1;
        }
        let blk = data[k..k + dl];
        k += dl;
        rs_remainder(blk, div[..ecl], rem[..]);
        // data codeword j of every block comes before j + 1; the long
        // blocks' extra codeword after all the others
        for j in @as(usize, 0)..dl {
            let mut pos = (j * nblocks) + i;
            if j == short_data {
                pos = (short_data * nblocks) + (i - nshort);
            }
            out[pos] = blk[j];
        }
        let dtotal = data.len;
        for j in @as(usize, 0)..ecl {
            out[dtotal + (j * nblocks) + i] = rem[j];
        }
    }
    return out;
}

// -----------------------------------------------------------------------------
// The matrix
// -----------------------------------------------------------------------------

struct Grid {
    n: usize,
    m: []mut u8,     // modules
    func: []mut u8,  // 1 where a function pattern (or format/version) is

    fn set(self: *mut Grid, x: usize, y: usize, dark: bool, function: bool) void {
        let mut v: u8 = 0;
        if dark {
            v = 1;
        }
        self.m[(y * self.n) + x] = v;
        if function {
            self.func[(y * self.n) + x] = 1;
        }
    }

    fn dark(self: *Grid, x: usize, y: usize) bool {
        return self.m[(y * self.n) + x] != 0;
    }
}

/// Positions (rows and columns) of the alignment patterns' centres.
fn alignment_positions(version: u32, out: []mut usize) usize {
    if version == 1 {
        return 0;
    }
    let v = @as(usize, version);
    let na = (v / 7) + 2;
    let step = ((((v * 8) + (na * 3)) + 5) / ((na * 4) - 4)) * 2;
    let size = (v * 4) + 17;
    out[0] = 6;
    for i in @as(usize, 1)..na {
        out[na - i] = (size - 7) - ((i - 1) * step);
    }
    return na;
}

fn draw_functions(g: *mut Grid, version: u32) void {
    let n = g.n;
    // timing patterns
    for i in @as(usize, 0)..n {
        g.set(6, i, (i % 2) == 0, true);
        g.set(i, 6, (i % 2) == 0, true);
    }
    // finders with their separators
    let corners: [3][2]usize = [[3, 3], [n - 4, 3], [3, n - 4]];
    for c in corners {
        for dy in @as(usize, 0)..9 {
            for dx in @as(usize, 0)..9 {
                let x = (c[0] + dx);
                let y = (c[1] + dy);
                if (x < 4) or (y < 4) or ((x - 4) >= n) or ((y - 4) >= n) {
                    continue;
                }
                let ax = @max(dx, 4) - @min(dx, 4);
                let ay = @max(dy, 4) - @min(dy, 4);
                let dist = @max(ax, ay);
                g.set(x - 4, y - 4, (dist != 2) and (dist != 4), true);
            }
        }
    }
    // alignment patterns (not over the finders)
    let mut pos: [7]usize = [0; 7];
    let na = alignment_positions(version, pos[..]);
    for i in @as(usize, 0)..na {
        for j in @as(usize, 0)..na {
            if ((i == 0) and (j == 0)) or ((i == 0) and (j == (na - 1))) or ((i == (na - 1)) and (j == 0)) {
                continue;
            }
            for dy in @as(usize, 0)..5 {
                for dx in @as(usize, 0)..5 {
                    let ax = @max(dx, 2) - @min(dx, 2);
                    let ay = @max(dy, 2) - @min(dy, 2);
                    g.set((pos[i] + dx) - 2, (pos[j] + dy) - 2, @max(ax, ay) != 1, true);
                }
            }
        }
    }
    // reserve the format information (drawn later) and the dark module
    draw_format(g, NO_INFO);
    draw_version(g, version, version_bits(version));
}

/// Draws the version information (versions 7 and up).
fn draw_version(g: *mut Grid, version: u32, bits: u32) void {
    if version < 7 {
        return;
    }
    let n = g.n;
    for i in @as(usize, 0)..18 {
        let bit = ((bits >> @truncate(u32, i)) & 1) != 0;
        let a = ((n - 11) + (i % 3));
        let b = i / 3;
        g.set(a, b, bit, true);
        g.set(b, a, bit, true);
    }
}

/// The 15 format information bits of a level and mask (BCH(15, 5), masked
/// with 0x5412).
pub fn format_bits(ecc: Ecc, mask: u32) u32 {
    let data = (ecc_format_bits(ecc) << 3) | mask;
    let mut rem = data;
    for _i in @as(usize, 0)..10 {
        rem = (rem << 1) ^ ((rem >> 9) * 0x537);
    }
    return ((data << 10) | (rem & 0x3ff)) ^ 0x5412;
}

/// The 18 version information bits (BCH(18, 6)) of versions 7 and up.
pub fn version_bits(version: u32) u32 {
    let mut rem = version;
    for _i in @as(usize, 0)..12 {
        rem = (rem << 1) ^ ((rem >> 11) * 0x1f25);
    }
    return (version << 12) | (rem & 0xfff);
}

/// Draws the format information and the dark module beside it (light too
/// when bits is NO_INFO, as while masks are scored).
fn draw_format(g: *mut Grid, bits: u32) void {
    let n = g.n;
    let mut b: [15]bool = [false; 15];
    for i in @as(usize, 0)..15 {
        b[i] = ((bits >> @truncate(u32, i)) & 1) != 0;
    }
    // around the top left finder
    for i in @as(usize, 0)..6 {
        g.set(8, i, b[i], true);
    }
    g.set(8, 7, b[6], true);
    g.set(8, 8, b[7], true);
    g.set(7, 8, b[8], true);
    for i in @as(usize, 9)..15 {
        g.set(14 - i, 8, b[i], true);
    }
    // the copy by the other two
    for i in @as(usize, 0)..8 {
        g.set((n - 1) - i, 8, b[i], true);
    }
    for i in @as(usize, 8)..15 {
        g.set(8, (n - 15) + i, b[i], true);
    }
    g.set(8, n - 8, bits != NO_INFO, true);  // always dark
}

const NO_INFO: u32 = 0x80000000;

/// Places the codewords in the two-column zigzag.
fn place_data(g: *mut Grid, cw: []u8) void {
    let n = g.n;
    let total = cw.len * 8;
    let mut i: usize = 0;
    let mut right = n - 1;
    loop {
        if right == 6 {
            right = 5;
        }
        let upward = ((right + 1) & 2) == 0;
        for vert in @as(usize, 0)..n {
            for j in @as(usize, 0)..2 {
                let x = right - j;
                let mut y = vert;
                if upward {
                    y = (n - 1) - vert;
                }
                if (g.func[(y * n) + x] == 0) and (i < total) {
                    let bit = ((cw[i / 8] >> @truncate(u8, 7 - (i % 8))) & 1) != 0;
                    g.set(x, y, bit, false);
                    i += 1;
                }
            }
        }
        if right < 2 {
            break;
        }
        right -= 2;
    }
}

fn mask_bit(mask: u32, x: usize, y: usize) bool {
    match mask {
        0    => { return ((x + y) % 2) == 0; }
        1    => { return (y % 2) == 0; }
        2    => { return (x % 3) == 0; }
        3    => { return ((x + y) % 3) == 0; }
        4    => { return (((x / 3) + (y / 2)) % 2) == 0; }
        5    => { return (((x * y) % 2) + ((x * y) % 3)) == 0; }
        6    => { return ((((x * y) % 2) + ((x * y) % 3)) % 2) == 0; }
        else => { return ((((x + y) % 2) + ((x * y) % 3)) % 2) == 0; }
    }
}

/// XORs a mask over the data modules (applying it twice undoes it).
fn apply_mask(g: *mut Grid, mask: u32) void {
    let n = g.n;
    for y in @as(usize, 0)..n {
        for x in @as(usize, 0)..n {
            if (g.func[(y * n) + x] == 0) and mask_bit(mask, x, y) {
                g.m[(y * n) + x] = g.m[(y * n) + x] ^ 1;
            }
        }
    }
}

/// Points for a line of modules (a row or a column): N1 runs and N3
/// finder-like patterns.
fn line_penalty(line: []u8) usize {
    let n = line.len;
    let mut p: usize = 0;
    // N1: runs of five or more: 3 + (length - 5)
    let mut run: usize = 1;
    for i in @as(usize, 1)..n + 1 {
        if (i < n) and (line[i] == line[i - 1]) {
            run += 1;
            continue;
        }
        if run >= 5 {
            p += run - 2;
        }
        run = 1;
    }
    // N3: dark light dark dark dark light dark with four light modules (or
    // the symbol's edge) before or after it
    let pat: [7]u8 = [1, 0, 1, 1, 1, 0, 1];
    if n < 7 {
        return p;
    }
    let mut i: usize = 0;
    while (i + 7) <= n {
        if not @mem_eq(line[i..i + 7], pat[..]) {
            i += 1;
            continue;
        }
        let mut before = true;
        for k in (@max(i, 4) - 4)..i {
            if line[k] != 0 {
                before = false;
            }
        }
        let mut after = true;
        for k in (i + 7)..@min(i + 11, n) {
            if line[k] != 0 {
                after = false;
            }
        }
        if (i == 0) or ((i + 7) == n) or before or after {
            p += 40;
            i += 7;
        } else {
            i += 4;
        }
    }
    return p;
}

fn penalty(g: *Grid, col: []mut u8) usize {
    let n = g.n;
    let mut p: usize = 0;
    let mut dark: usize = 0;
    for y in @as(usize, 0)..n {
        p += line_penalty(g.m[y * n..(y + 1) * n]);
        for x in @as(usize, 0)..n {
            col[x] = g.m[(x * n) + y];
            dark += @as(usize, g.m[(y * n) + x]);
            // N2: 2x2 blocks of one colour
            if (x > 0) and (y > 0) {
                let v = g.m[(y * n) + x];
                if (g.m[(y * n) + x - 1] == v) and (g.m[(((y - 1) * n) + x)] == v) and
                   (g.m[(((y - 1) * n) + x) - 1] == v) {
                    p += 3;
                }
            }
        }
        p += line_penalty(col[..n]);
    }
    // N4: 10 for each full 5% the dark proportion is away from 50%
    let total = n * n;
    let diff = @max(dark * 20, total * 10) - @min(dark * 20, total * 10);  // |dark/total - 1/2| * 20 total
    p += 10 * (diff / total);
    return p;
}

fn build(cw: []u8, version: u32, ecc: Ecc, mask: u32) !Code {
    let n = (@as(usize, version) * 4) + 17;
    let m = mem.alloc_zeroed.[u8](n * n)!;
    let func = mem.alloc_zeroed.[u8](n * n) catch |e| {
        mem.free(m);
        return e;
    };
    defer mem.free(func);
    let mut g = Grid.{ n: n, m: m, func: func };
    draw_functions(&mut g, version);
    place_data(&mut g, cw);
    let mut best = mask;
    if mask == AUTO_MASK {
        let col = mem.alloc.[u8](n) catch |e| {
            mem.free(m);
            return e;
        };
        defer mem.free(col);
        let mut lowest: usize = @max_value(usize);
        // masks are scored with the format and version information (and
        // the dark module) light, as segno does: ISO/IEC 18004:2015 7.8
        // places them after masking
        draw_version(&mut g, version, 0);
        for k in @as(u32, 0)..8 {
            apply_mask(&mut g, k);
            let p = penalty(&g, col);
            if p < lowest {
                lowest = p;
                best = k;
            }
            apply_mask(&mut g, k);
        }
    }
    apply_mask(&mut g, best);
    draw_format(&mut g, format_bits(ecc, best));
    draw_version(&mut g, version, version_bits(version));
    return Code.{ version: version, size: n, ecc: ecc, mask: best, modules: m };
}

// -----------------------------------------------------------------------------
// Output
// -----------------------------------------------------------------------------

/// The symbol as a gray image (one channel, 8 bits): scale pixels a module,
/// with quiet modules of light margin (the standard asks for 4), dark 0 and
/// light 255.
pub fn to_image(code: *Code, scale: usize, quiet: usize) !image.Image {
    return to_image_colors(code, scale, quiet, color.Color.rgb(0.0, 0.0, 0.0), color.Color.rgb(1.0, 1.0, 1.0));
}

/// The symbol as an RGBA image (8 bits) in two colours.
pub fn to_image_colors(code: *Code, scale: usize, quiet: usize, dark: color.Color, light: color.Color) !image.Image {
    if scale == 0 {
        return error.InvalidOption;
    }
    let side = (code.size + (2 * quiet)) * scale;
    let gray = (dark.r == dark.g) and (dark.g == dark.b) and (light.r == light.g) and (light.g == light.b) and
               (dark.a == 1.0) and (light.a == 1.0);
    let mut ch: usize = 4;
    if gray {
        ch = 1;
    }
    let mut img = image.Image.init(side, side, ch, image.Depth.U8)!;
    img.fill(light);
    // one pixel of each colour, copied
    img.set(0, 0, dark);
    let mut dpx: [4]u8 = [0; 4];
    @mem_copy(dpx[..ch], img.data[..ch]);
    img.set(0, 0, light);
    for y in @as(usize, 0)..code.size {
        for x in @as(usize, 0)..code.size {
            if not code.get(x, y) {
                continue;
            }
            for py in @as(usize, 0)..scale {
                let row = (((y + quiet) * scale) + py) * side;
                for px in @as(usize, 0)..scale {
                    let o = ((row + ((x + quiet) * scale)) + px) * ch;
                    @mem_copy(img.data[o..o + ch], dpx[..ch]);
                }
            }
        }
    }
    return img;
}

/// Writes the symbol as an SVG document: one unit a module, quiet modules of
/// margin, a white background and one black path (crisp edges).
pub fn write_svg(w: io.Writer, code: *Code, quiet: usize) !void {
    let side = code.size + (2 * quiet);
    let mut nb: [24]u8 = [0; 24];
    w.write_all("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<svg xmlns=\"http://www.w3.org/2000/svg\" ")!;
    w.write_all("version=\"1.1\" viewBox=\"0 0 ")!;
    w.write_all(io.fmt_u64(nb[..], @as(u64, side)))!;
    w.write_all(" ")!;
    w.write_all(io.fmt_u64(nb[..], @as(u64, side)))!;
    w.write_all("\" shape-rendering=\"crispEdges\">\n<rect width=\"100%\" height=\"100%\" fill=\"#fff\"/>\n")!;
    w.write_all("<path fill=\"#000\" d=\"")!;
    let mut first = true;
    for y in @as(usize, 0)..code.size {
        let mut x: usize = 0;
        while x < code.size {
            if not code.get(x, y) {
                x += 1;
                continue;
            }
            let start = x;
            while (x < code.size) and code.get(x, y) {
                x += 1;
            }
            if not first {
                w.write_all(" ")!;
            }
            first = false;
            w.write_all("M")!;
            w.write_all(io.fmt_u64(nb[..], @as(u64, start + quiet)))!;
            w.write_all(" ")!;
            w.write_all(io.fmt_u64(nb[..], @as(u64, y + quiet)))!;
            w.write_all("h")!;
            w.write_all(io.fmt_u64(nb[..], @as(u64, x - start)))!;
            w.write_all("v1h-")!;
            w.write_all(io.fmt_u64(nb[..], @as(u64, x - start)))!;
            w.write_all("z")!;
        }
    }
    w.write_all("\"/>\n</svg>\n")!;
}

/// write_svg into a new string (free it with mem.free).
pub fn svg(code: *Code, quiet: usize) ![]mut u8 {
    let mut s = strs.String.init();
    write_svg(s.writer(), code, quiet) catch |e| {
        s.deinit();
        return e;
    };
    return s.take();
}

/// The symbol as text: a line per row, "#" dark and "." light (for tests and
/// terminals).
pub fn to_text(code: *Code) ![]mut u8 {
    let n = code.size;
    let out = mem.alloc.[u8](n * (n + 1))!;
    for y in @as(usize, 0)..n {
        for x in @as(usize, 0)..n {
            let mut c: u8 = '.';
            if code.get(x, y) {
                c = '#';
            }
            out[(y * (n + 1)) + x] = c;
        }
        out[(y * (n + 1)) + n] = '\n';
    }
    return out;
}

// -----------------------------------------------------------------------------
// Tests
// -----------------------------------------------------------------------------

fn hex_value(c: u8) u8 {
    if c <= '9' {
        return c - '0';
    }
    return (c - 'a') + 10;
}

fn fails_with(r: !Code, want: error) bool {
    let mut c = r catch |e| return e == want;
    c.deinit();
    return false;
}

test "matrices made by segno" {
    let text = fs.read_file("std/graphics/qr/testdata/cases.txt")!;
    defer mem.free(text);
    let mut lines = strs.lines(text);
    let mut cases: usize = 0;
    loop {
        let line = lines.next() catch break;
        if not strs.starts_with(line, "case ") {
            continue;
        }
        let mut f = strs.fields(line);
        _ = f.next()!;
        let kind = f.next()!;
        let level = f.next()!;
        let mask_s = f.next()!;
        let eci_s = f.next()!;
        let hex = f.next() catch "";
        let data = mem.alloc.[u8](hex.len / 2)!;
        defer mem.free(data);
        for i in @as(usize, 0)..data.len {
            data[i] = (hex_value(hex[2 * i]) << 4) | hex_value(hex[(2 * i) + 1]);
        }
        let mut o = Options.init(Ecc.L);
        if level[0] == 'M' {
            o.ecc = Ecc.M;
        } else if level[0] == 'Q' {
            o.ecc = Ecc.Q;
        } else if level[0] == 'H' {
            o.ecc = Ecc.H;
        }
        if not strs.eq(mask_s, "auto") {
            o.mask = @as(u32, mask_s[0] - '0');
        }
        o.eci = eci_s[0] == '1';
        let mut code = Code.{ version: 0, size: 0, ecc: Ecc.L, mask: 0, modules: mem.alloc.[u8](0)! };
        if strs.eq(kind, "text") {
            mem.free(code.modules);
            code = encode_with(data, o)!;
        } else if strs.eq(kind, "binary") {
            let segs: [1]Segment = [Segment.bytes(data)];
            mem.free(code.modules);
            code = encode_segments(segs[..], o)!;
        } else {
            let segs: [1]Segment = [Segment.kanji(data)];
            mem.free(code.modules);
            code = encode_segments(segs[..], o)!;
        }
        defer code.deinit();
        let vl = lines.next()!;
        let mut vf = strs.fields(vl);
        _ = vf.next()!;
        let version = fmt.parse_int.[u32](vf.next()!)!;
        _ = vf.next()!;
        let mask = fmt.parse_int.[u32](vf.next()!)!;
        @expect_eq(code.version, version);
        @expect_eq(code.mask, mask);
        for y in @as(usize, 0)..code.size {
            let row = lines.next()!;
            @expect_eq(row.len, code.size);
            for x in @as(usize, 0)..code.size {
                @expect_eq(code.get(x, y), row[x] == '#');
            }
        }
        cases += 1;
    }
    @expect(cases >= 39);
}

test "reed-solomon codewords of HELLO WORLD" {
    // the worked examples (versions 1-M and 1-Q): data and error correction codewords
    let data_m: [16]u8 = [32, 91, 11, 120, 209, 114, 220, 77, 67, 64, 236, 17, 236, 17, 236, 17];
    let ecc_m: [10]u8 = [196, 35, 39, 119, 235, 215, 231, 226, 93, 23];
    let cw = add_ecc(data_m[..], 1, Ecc.M)!;
    defer mem.free(cw);
    @expect(@mem_eq(cw[..16], data_m[..]));
    @expect(@mem_eq(cw[16..], ecc_m[..]));
    let data_q: [13]u8 = [32, 91, 11, 120, 209, 114, 220, 77, 67, 64, 236, 17, 236];
    let ecc_q: [13]u8 = [168, 72, 22, 82, 217, 54, 156, 0, 46, 15, 180, 122, 16];
    let cq = add_ecc(data_q[..], 1, Ecc.Q)!;
    defer mem.free(cq);
    @expect(@mem_eq(cq[13..], ecc_q[..]));
    // the bit stream of HELLO WORLD in alphanumeric mode at 1-Q is those 13 codewords
    let mut b = Bits.init();
    defer b.deinit();
    append_segment(&mut b, Segment.alphanumeric("HELLO WORLD"), 1)!;
    @expect_eq(b.len, 74);
    let mut code = encode("HELLO WORLD", Ecc.Q)!;
    defer code.deinit();
    @expect_eq(code.version, 1);
    @expect_eq(code.size, 21);
}

test "format and version information" {
    // ISO/IEC 18004 table C.1 and table D.1
    @expect_eq(format_bits(Ecc.M, 0), 0x5412);
    @expect_eq(format_bits(Ecc.L, 0), 0x77c4);
    @expect_eq(format_bits(Ecc.L, 7), 0x6976);
    @expect_eq(format_bits(Ecc.Q, 0), 0x355f);
    @expect_eq(format_bits(Ecc.H, 0), 0x1689);
    @expect_eq(format_bits(Ecc.H, 7), 0x083b);
    @expect_eq(version_bits(7), 0x07c94);
    @expect_eq(version_bits(21), 0x15683);
    @expect_eq(version_bits(40), 0x28c69);
    // the alignment pattern centres of a few versions (table E.1)
    let mut pos: [7]usize = [0; 7];
    @expect_eq(alignment_positions(1, pos[..]), 0);
    @expect_eq(alignment_positions(2, pos[..]), 2);
    @expect_eq(pos[1], 18);
    @expect_eq(alignment_positions(32, pos[..]), 6);
    @expect_eq(pos[1], 34);
    @expect_eq(pos[5], 138);
    @expect_eq(alignment_positions(40, pos[..]), 7);
    @expect_eq(pos[1], 30);
    @expect_eq(pos[6], 170);
}

test "segmentation" {
    let mut segs = vec.Vec.[Segment].init();
    defer segs.deinit();
    segment_text("abc 1234567890123 XYZ", 1, &mut segs)!;
    @expect_eq(segs.len, 3);
    @expect(segs.get(0).mode == Mode.BYTE);
    @expect(strs.eq(segs.get(0).data, "abc "));
    @expect(segs.get(1).mode == Mode.NUMERIC);
    @expect(strs.eq(segs.get(1).data, "1234567890123"));
    @expect(segs.get(2).mode == Mode.ALPHANUMERIC);
    @expect_eq(total_bits(segs.items(), 1)!, 137);
    segment_text("HTTPS://EXAMPLE.ORG/A?X=1", 10, &mut segs)!;
    @expect_eq(segs.len, 2);
    @expect(segs.get(0).mode == Mode.ALPHANUMERIC);
    @expect(strs.eq(segs.get(1).data, "?X=1"));
    @expect_eq(total_bits(segs.items(), 10)!, 183);
    segment_text("0123456789", 27, &mut segs)!;
    @expect_eq(segs.len, 1);
    @expect(segs.get(0).mode == Mode.NUMERIC);
    segment_text("", 1, &mut segs)!;
    @expect_eq(segs.len, 0);
}

test "limits and errors" {
    // the largest symbols: 7089 digits, 2953 bytes at 40-L
    let digits = mem.alloc.[u8](7090)!;
    defer mem.free(digits);
    @mem_set(digits, '7');
    let mut c = encode(digits[..7089], Ecc.L)!;
    @expect_eq(c.version, 40);
    c.deinit();
    @expect(fails_with(encode(digits, Ecc.L), error.DataTooLong));
    let mut b = encode_binary(digits[..2953], Ecc.L)!;
    @expect_eq(b.version, 40);
    b.deinit();
    @expect(fails_with(encode_binary(digits[..2954], Ecc.L), error.DataTooLong));
    // bad segments and options
    let bad: [1]Segment = [Segment.numeric("12a")];
    @expect(fails_with(encode_segments(bad[..], Options.init(Ecc.L)), error.InvalidSegment));
    let badk: [1]Segment = [Segment.kanji("\x81")];
    @expect(fails_with(encode_segments(badk[..], Options.init(Ecc.L)), error.InvalidSegment));
    let mut o = Options.init(Ecc.L);
    o.min_version = 0;
    @expect(fails_with(encode_with("x", o), error.InvalidOption));
    // a version range that is too small
    o = Options.init(Ecc.H);
    o.max_version = 1;
    @expect(fails_with(encode_with("this text does not fit in version one", o), error.DataTooLong));
    // fixed version and boosted level
    o = Options.init(Ecc.L);
    o.min_version = 5;
    o.boost_ecc = true;
    let mut d = encode_with("BOOST", o)!;
    defer d.deinit();
    @expect_eq(d.version, 5);
    @expect(d.ecc == Ecc.H);
    // an empty text is a valid (version 1) symbol
    let mut e = encode("", Ecc.M)!;
    defer e.deinit();
    @expect_eq(e.size, 21);
}

test "rendering" {
    let mut code = encode("HELLO WORLD", Ecc.Q)!;
    defer code.deinit();
    let mut img = to_image(&code, 3, 4)!;
    defer img.deinit();
    @expect_eq(img.width, 87);
    @expect_eq(img.channels, 1);
    // the quiet zone is light, the top left finder's corner dark
    @expect_eq(img.data[0], 255);
    @expect_eq(img.data[(12 * 87) + 12], 0);
    @expect_eq(img.data[(14 * 87) + 14], 0);
    @expect_eq(img.data[(15 * 87) + 15], 255);  // the finder's light ring
    for y in @as(usize, 0)..code.size {
        for x in @as(usize, 0)..code.size {
            let px = img.data[((((y + 4) * 3) + 1) * 87) + (((x + 4) * 3) + 1)];
            @expect_eq(px == 0, code.get(x, y));
        }
    }
    let mut col = to_image_colors(&code, 1, 0, color.Color.rgb(0.0, 0.0, 1.0), color.Color.rgb(1.0, 1.0, 0.0))!;
    defer col.deinit();
    @expect_eq(col.channels, 4);
    @expect_eq(col.get(0, 0).to_rgba8().to_u32(), 0x0000ffff);
    @expect_eq(col.get(7, 0).to_rgba8().to_u32(), 0xffff00ff);
    // the path covers exactly the dark modules
    let mut p = draw.Path.init();
    defer p.deinit();
    code.add_to_path(&mut p, 2.0, 2.0, 2.0)!;
    let mut canvas_img = image.Image.init(46, 46, 1, image.Depth.U8)!;
    defer canvas_img.deinit();
    let mut cv = draw.Canvas.init(&mut canvas_img);
    defer cv.deinit();
    cv.clear(color.Color.rgb(1.0, 1.0, 1.0));
    cv.fill(&p, draw.Paint.solid(color.Color.rgb(0.0, 0.0, 0.0)), draw.FillRule.NONZERO)!;
    for y in @as(usize, 0)..code.size {
        for x in @as(usize, 0)..code.size {
            for k in @as(usize, 0)..4 {
                let v = canvas_img.data[((((y * 2) + 2) + (k / 2)) * 46) + (((x * 2) + 2) + (k % 2))];
                @expect_eq(v == 0, code.get(x, y));
                @expect((v == 0) or (v == 255));
            }
        }
    }
    // SVG: the viewBox, and a path of runs
    let s = svg(&code, 4)!;
    defer mem.free(s);
    @expect(strs.contains(s, "viewBox=\"0 0 29 29\""));
    @expect(strs.contains(s, "<path fill=\"#000\" d=\"M4 4h7v1h-7z"));
    @expect(strs.ends_with(s, "</svg>\n"));
    let t = to_text(&code)!;
    defer mem.free(t);
    @expect(strs.starts_with(t, "#######."));
}
