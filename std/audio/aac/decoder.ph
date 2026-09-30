// =============================================================================
// std/audio/aac/decoder.ph - the AAC decoder (ISO/IEC 14496-3 subpart 4):
// raw data blocks of AAC Main, LC and LTP. Channel elements (single,
// pair, LFE), program configuration, section and scalefactor data, the
// spectral Huffman codebooks, pulses, noise substitution, M/S and
// intensity stereo, Main-profile prediction, long-term prediction, TNS,
// the sine and KBD windows and the inverse MDCT. Follows FFmpeg's float
// decoder (libavcodec/aacdec_template.c), with the same arithmetic.
// =============================================================================

import "../../math.ph" as math;
import "../../mem.ph" as mem;
import "tables.ph" as t;

error { InvalidAudio, Unsupported, OutOfMemory }

pub const AOT_MAIN: i32 = 1;
pub const AOT_LC: i32 = 2;
pub const AOT_LTP: i32 = 4;

pub let SAMPLE_RATES: [13]i32 = [96000, 88200, 64000, 48000, 44100, 32000, 24000, 22050, 16000, 12000, 11025, 8000,
                                 7350];

const ONLY_LONG: i32 = 0;
const LONG_START: i32 = 1;
const EIGHT_SHORT: i32 = 2;
const LONG_STOP: i32 = 3;
const ZERO_BT: i32 = 0;
const NOISE_BT: i32 = 13;
const INTENSITY_BT2: i32 = 14;
const INTENSITY_BT: i32 = 15;
const TYPE_SCE: i32 = 0;
const TYPE_CPE: i32 = 1;
const TYPE_CCE: i32 = 2;
const TYPE_LFE: i32 = 3;
const TYPE_DSE: i32 = 4;
const TYPE_PCE: i32 = 5;
const TYPE_FIL: i32 = 6;
const TYPE_END: i32 = 7;
const MAX_PREDICTORS: usize = 672;
const MAX_LTP_LONG_SFB: i32 = 40;
const MAX_CHANNELS: usize = 64;

fn us(v: i32) usize {
    return @as(usize, @bit_cast(u32, v));
}

fn si(v: usize) i32 {
    return @bit_cast(i32, @truncate(u32, v));
}

// -----------------------------------------------------------------------------
// Bits
// -----------------------------------------------------------------------------

pub struct Bits {
    buf: []u8,
    pos: usize,
    over: bool,

    pub fn init(buf: []u8) Bits {
        return Bits.{ buf: buf, pos: 0, over: false };
    }

    pub fn left(self: *Bits) i64 {
        return (@as(i64, @bit_cast(i64, @as(u64, self.buf.len))) * 8) - @bit_cast(i64, @as(u64, self.pos));
    }

    pub fn get1(self: *mut Bits) u32 {
        let byte = self.pos >> 3;
        if byte >= self.buf.len {
            self.over = true;
            self.pos += 1;
            return 0;
        }
        let b = (@as(u32, self.buf[byte]) >> @truncate(u32, 7 - (self.pos & 7))) & 1;
        self.pos += 1;
        return b;
    }

    pub fn get(self: *mut Bits, n: u32) u32 {
        let mut v: u32 = 0;
        let mut k = n;
        while k > 0 {
            let byte = self.pos >> 3;
            if byte >= self.buf.len {
                self.over = true;
                self.pos += @as(usize, k);
                return v << k;
            }
            let avail = 8 - @truncate(u32, self.pos & 7);
            let take = @min(avail, k);
            let bits = (@as(u32, self.buf[byte]) >> (avail - take)) & ((@as(u32, 1) << take) - 1);
            v = (v << take) | bits;
            self.pos += @as(usize, take);
            k -= take;
        }
        return v;
    }

    pub fn geti(self: *mut Bits, n: u32) i32 {
        return @bit_cast(i32, self.get(n));
    }

    pub fn skip(self: *mut Bits, n: usize) void {
        self.pos += n;
        if (self.pos >> 3) > self.buf.len {
            self.over = true;
        }
    }

    pub fn align(self: *mut Bits) void {
        self.pos = (self.pos + 7) & ~@as(usize, 7);
    }
}

// -----------------------------------------------------------------------------
// Audio specific configuration
// -----------------------------------------------------------------------------

pub struct Elem {
    typ: i32,
    id: i32,
}

pub struct Config {
    object_type: i32,
    sampling_index: i32,
    sample_rate: i32,
    chan_config: i32,
    sbr: bool,
    // from a program configuration element (chan_config 0)
    nelems: usize,
    elems: [64]Elem,
}

pub fn empty_config() Config {
    return Config.{ object_type: 0, sampling_index: 0, sample_rate: 0, chan_config: 0, sbr: false, nelems: 0,
                    elems: [Elem.{ typ: 0, id: 0 }; 64] };
}

fn get_aot(b: *mut Bits) i32 {
    let mut aot = b.geti(5);
    if aot == 31 {
        aot = 32 + b.geti(6);
    }
    return aot;
}

fn get_rate(b: *mut Bits, index: *mut i32) i32 {
    index.* = b.geti(4);
    if index.* == 15 {
        return b.geti(24);
    }
    if index.* > 12 {
        return 0;
    }
    return SAMPLE_RATES[us(index.*)];
}

fn rate_index(rate: i32) i32 {
    if rate >= 92017 {
        return 0;
    }
    if rate >= 75132 {
        return 1;
    }
    if rate >= 55426 {
        return 2;
    }
    if rate >= 46009 {
        return 3;
    }
    if rate >= 37566 {
        return 4;
    }
    if rate >= 27713 {
        return 5;
    }
    if rate >= 23004 {
        return 6;
    }
    if rate >= 18783 {
        return 7;
    }
    if rate >= 13856 {
        return 8;
    }
    if rate >= 11502 {
        return 9;
    }
    if rate >= 9391 {
        return 10;
    }
    return 11;
}

/// A program configuration element's channel layout (into cfg.elems).
fn decode_pce(b: *mut Bits, cfg: *mut Config, align_ref: usize) !void {
    b.skip(2);
    _ = b.get(4);
    let nfront = b.get(4);
    let nside = b.get(4);
    let nback = b.get(4);
    let nlfe = b.get(2);
    let nassoc = b.get(3);
    let ncc = b.get(4);
    if b.get1() != 0 {
        b.skip(4);
    }
    if b.get1() != 0 {
        b.skip(4);
    }
    if b.get1() != 0 {
        b.skip(3);
    }
    let mut n: usize = 0;
    let counts: [3]u32 = [nfront, nside, nback];
    for c in counts {
        for _i in @as(u32, 0)..c {
            let typ = b.geti(1);
            cfg.elems[n] = Elem.{ typ: typ, id: b.geti(4) };
            n += 1;
        }
    }
    for _i in @as(u32, 0)..nlfe {
        cfg.elems[n] = Elem.{ typ: TYPE_LFE, id: b.geti(4) };
        n += 1;
    }
    b.skip(@as(usize, 4 * nassoc));
    for _i in @as(u32, 0)..ncc {
        b.skip(5); // coupling channels are not output
    }
    // byte alignment relative to the start of the raw data block
    let rel = (b.pos - align_ref) & 7;
    if rel != 0 {
        b.skip(8 - rel);
    }
    let comment = b.get(8);
    b.skip(@as(usize, comment) * 8);
    if b.over {
        return error.InvalidAudio;
    }
    cfg.nelems = n;
}

/// Parses an AudioSpecificConfig (as in an MP4 esds box).
pub fn parse_asc(data: []u8, cfg: *mut Config) !void {
    let mut b = Bits.init(data);
    cfg.* = empty_config();
    let mut aot = get_aot(&mut b);
    let mut idx: i32 = 0;
    cfg.sample_rate = get_rate(&mut b, &mut idx);
    cfg.chan_config = b.geti(4);
    if (aot == 5) or (aot == 29) {
        cfg.sbr = true;
        let mut eidx: i32 = 0;
        _ = get_rate(&mut b, &mut eidx);
        aot = get_aot(&mut b);
    }
    cfg.object_type = aot;
    if (aot != AOT_MAIN) and (aot != AOT_LC) and (aot != AOT_LTP) {
        return error.Unsupported;
    }
    if cfg.sample_rate <= 0 {
        return error.InvalidAudio;
    }
    if idx > 12 {
        idx = rate_index(cfg.sample_rate);
    }
    cfg.sampling_index = idx;
    // GASpecificConfig
    if b.get1() != 0 {
        return error.Unsupported; // 960-sample frames
    }
    if b.get1() != 0 {
        b.skip(14);
    }
    _ = b.get1();
    if cfg.chan_config == 0 {
        b.skip(4);
        decode_pce(&mut b, cfg, 0)!;
    } else if cfg.chan_config > 7 {
        return error.Unsupported;
    }
    if b.over {
        return error.InvalidAudio;
    }
}

// -----------------------------------------------------------------------------
// Huffman codebooks (trees of nodes; a leaf is -(symbol + 1))
// -----------------------------------------------------------------------------

struct Huff {
    nodes: []mut i32,
    root: [12]usize,
    used: usize,
}

fn huff_add(h: *mut Huff, root: usize, code: u32, len: u32, sym: i32) !void {
    let mut node = root;
    let mut i = len;
    while i > 0 {
        i -= 1;
        let bit = us(@bit_cast(i32, (code >> i) & 1));
        let slot = (node * 2) + bit;
        if i == 0 {
            if h.nodes[slot] != 0 {
                return error.InvalidAudio;
            }
            h.nodes[slot] = -(sym + 1);
        } else {
            let v = h.nodes[slot];
            if v < 0 {
                return error.InvalidAudio;
            }
            if v == 0 {
                h.nodes[slot] = si(h.used);
                node = h.used;
                h.used += 1;
            } else {
                node = us(v);
            }
        }
    }
}

fn huff_build(h: *mut Huff) !void {
    let total = t.SF_CODE.len + t.SPECTRAL_CODES.len;
    h.nodes = mem.alloc_zeroed.[i32](2 * (total + 16))!;
    h.used = 0;
    // 0: scalefactors; 1..11: spectral codebooks
    h.root[0] = h.used;
    h.used += 1;
    for i in @as(usize, 0)..t.SF_CODE.len {
        huff_add(h, h.root[0], t.SF_CODE[i], @as(u32, t.SF_BITS[i]), si(i))!;
    }
    for cb in @as(usize, 1)..12 {
        h.root[cb] = h.used;
        h.used += 1;
        let s = @as(usize, t.SPECTRAL_START[cb - 1]);
        let e = @as(usize, t.SPECTRAL_START[cb]);
        for i in s..e {
            huff_add(h, h.root[cb], @as(u32, t.SPECTRAL_CODES[i]), @as(u32, t.SPECTRAL_BITS[i]), si(i - s))!;
        }
    }
}

/// The next symbol, or -1 for an invalid code.
fn huff_decode(h: *Huff, cb: usize, b: *mut Bits) i32 {
    let mut node = h.root[cb];
    for _i in @as(usize, 0)..20 {
        let v = h.nodes[(node * 2) + us(@bit_cast(i32, b.get1()))];
        if v < 0 {
            return -v - 1;
        }
        if v == 0 {
            return -1;
        }
        node = us(v);
    }
    return -1;
}

// -----------------------------------------------------------------------------
// Channel state
// -----------------------------------------------------------------------------

struct Ltp {
    present: bool,
    lag: i32,
    coef: f32,
    used: [40]bool,
}

struct Ics {
    window_sequence: [2]i32,
    use_kb_window: [2]bool,
    max_sfb: i32,
    num_window_groups: i32,
    group_len: [8]i32,
    num_windows: i32,
    num_swb: i32,
    swb: usize, // start of the band offsets in t.SWB_OFFSET_1024 or _128
    short: bool,
    tns_max_bands: i32,
    predictor_present: bool,
    predictor_reset_group: i32,
    prediction_used: [41]bool,
    ltp: Ltp,
}

struct Tns {
    present: bool,
    n_filt: [8]i32,
    length: [8][4]i32,
    order: [8][4]i32,
    direction: [8][4]bool,
    coef: [8][4][20]f32,
}

struct Pred {
    r0: f32,
    r1: f32,
    cor0: f32,
    cor1: f32,
    var0: f32,
    var1: f32,
}

struct Chan {
    ics: Ics,
    tns: Tns,
    band_type: [128]i32,
    run_end: [128]i32,
    sf: [128]f32,
    coeffs: [1024]f32,
    saved: [1536]f32,
    ret: [2048]f32,
    ltp_state: [3072]f32,
    pred: [672]Pred,
    predictor_initialized: bool,
}

fn swb_off(ics: *Ics, i: i32) i32 {
    if ics.short {
        return @as(i32, t.SWB_OFFSET_128[ics.swb + us(i)]);
    }
    return @as(i32, t.SWB_OFFSET_1024[ics.swb + us(i)]);
}

pub struct Decoder {
    cfg: Config,
    huff: Huff,
    chans: []mut Chan,
    nchans: usize,
    // channel of each element: [type (SCE, CPE, LFE)][id]
    slot: [3][16]i32,
    // output order: out channel c comes from chans[order[c]]
    order: [64]usize,
    random_state: i32,
    pow2sf: [428]f32,
    cbrt: []mut f32,
    // scratch
    buf: [1024]f32,
    temp: [128]f32,
    ltp_in: [2048]f32,
    ltp_freq: [1024]f32,
    fft_re: [512]f64,
    fft_im: [512]f64,
    configured: bool,

    pub fn deinit(self: *mut Decoder) void {
        mem.free(self.huff.nodes);
        if self.chans.len > 0 {
            mem.free(self.chans);
        }
        mem.free(self.cbrt);
    }
}

fn reset_pred(p: *mut Pred) void {
    p.r0 = 0.0;
    p.r1 = 0.0;
    p.cor0 = 0.0;
    p.cor1 = 0.0;
    p.var0 = 1.0;
    p.var1 = 1.0;
}

/// A decoder for a configuration (from parse_asc, or an ADTS header).
pub fn new(cfg: Config) ![]mut Decoder {
    let dd = mem.alloc_zeroed.[Decoder](1)!;
    let d = &mut dd[0];
    d.cfg = cfg;
    d.random_state = 0x1f2e3d4c;
    d.cbrt = mem.alloc.[f32](8192) catch |e| {
        mem.free(dd);
        return e;
    };
    huff_build(&mut d.huff) catch |e| {
        mem.free(d.cbrt);
        mem.free(dd);
        return e;
    };
    let lut: [4]f64 = [1.0, 1.18920711500272106672, 1.41421356237309504880, 1.68179283050742908606];
    for i in @as(usize, 0)..428 {
        let base = @as(f64, @float_cast(f32, lut[i % 4]));
        d.pow2sf[i] = @float_cast(f32, math.ldexp(base, si(i / 4) - 50));
    }
    for n in @as(usize, 0)..8192 {
        let x = @float_from_int(f64, n);
        d.cbrt[n] = @float_cast(f32, x * math.cbrt(x));
    }
    configure(d) catch |e| {
        d.deinit();
        mem.free(dd);
        return e;
    };
    return dd;
}

/// The number of output channels.
pub fn channels(d: *Decoder) usize {
    return d.nchans;
}

fn add_chan(d: *mut Decoder, typ: i32, id: i32, n: *mut usize) !void {
    let k = us(@min(typ, 2));
    if (id < 0) or (id > 15) or (d.slot[k][us(id)] >= 0) {
        return error.InvalidAudio;
    }
    let mut w: usize = 1;
    if typ == TYPE_CPE {
        w = 2;
    }
    if (n.* + w) > MAX_CHANNELS {
        return error.Unsupported;
    }
    d.slot[k][us(id)] = si(n.*);
    n.* += w;
}

/// Sets up the channels for the configuration (or the elements of a PCE).
fn configure(d: *mut Decoder) !void {
    for k in @as(usize, 0)..3 {
        for i in @as(usize, 0)..16 {
            d.slot[k][i] = -1;
        }
    }
    let mut n: usize = 0;
    let c = d.cfg.chan_config;
    if c == 0 {
        if d.cfg.nelems == 0 {
            d.nchans = 0;
            return;
        }
        for i in @as(usize, 0)..d.cfg.nelems {
            let e = d.cfg.elems[i];
            // an LFE is stored with type 3; front/side/back elements with 0 or 1
            let mut typ = e.typ;
            if typ == TYPE_LFE {
                typ = 2;
            }
            add_chan(d, typ, e.id, &mut n)!;
        }
        for i in @as(usize, 0)..n {
            d.order[i] = i;
        }
    } else {
        // the elements in bitstream order, and where their channels go
        // (FFmpeg's layouts: FL FR FC LFE BL BR FLC FRC)
        if c == 1 {
            add_chan(d, TYPE_SCE, 0, &mut n)!;
            d.order[0] = 0;
        } else if c == 2 {
            add_chan(d, TYPE_CPE, 0, &mut n)!;
            d.order[0] = 0;
            d.order[1] = 1;
        } else {
            add_chan(d, TYPE_SCE, 0, &mut n)!; // FC: 0
            add_chan(d, TYPE_CPE, 0, &mut n)!; // 1, 2
            if c == 3 {
                let o: [3]usize = [1, 2, 0];
                for i in @as(usize, 0)..3 {
                    d.order[i] = o[i];
                }
            } else if c == 4 {
                add_chan(d, TYPE_SCE, 1, &mut n)!; // BC: 3
                let o: [4]usize = [1, 2, 0, 3];
                for i in @as(usize, 0)..4 {
                    d.order[i] = o[i];
                }
            } else if c == 5 {
                add_chan(d, TYPE_CPE, 1, &mut n)!; // BL BR: 3, 4
                let o: [5]usize = [1, 2, 0, 3, 4];
                for i in @as(usize, 0)..5 {
                    d.order[i] = o[i];
                }
            } else if c == 6 {
                add_chan(d, TYPE_CPE, 1, &mut n)!; // 3, 4
                add_chan(d, 2, 0, &mut n)!; // LFE: 5
                let o: [6]usize = [1, 2, 0, 5, 3, 4];
                for i in @as(usize, 0)..6 {
                    d.order[i] = o[i];
                }
            } else if c == 7 {
                add_chan(d, TYPE_CPE, 1, &mut n)!; // FL FR: 3, 4 (FLC FRC are 1, 2)
                add_chan(d, TYPE_CPE, 2, &mut n)!; // BL BR: 5, 6
                add_chan(d, 2, 0, &mut n)!; // LFE: 7
                let o: [8]usize = [3, 4, 0, 7, 5, 6, 1, 2];
                for i in @as(usize, 0)..8 {
                    d.order[i] = o[i];
                }
            } else {
                return error.Unsupported;
            }
        }
    }
    if d.chans.len > 0 {
        mem.free(d.chans);
        d.chans = d.chans[..0];
    }
    d.chans = mem.alloc_zeroed.[Chan](n)!;
    for i in @as(usize, 0)..n {
        let ch = &mut d.chans[i];
        for k in @as(usize, 0)..MAX_PREDICTORS {
            reset_pred(&mut ch.pred[k]);
        }
    }
    d.nchans = n;
    d.configured = n > 0;
}

// -----------------------------------------------------------------------------
// Individual channel streams
// -----------------------------------------------------------------------------

fn decode_ltp(ltp: *mut Ltp, b: *mut Bits, max_sfb: i32) void {
    ltp.lag = b.geti(11);
    ltp.coef = t.LTP_COEF[@as(usize, b.get(3))];
    for sfb in @as(usize, 0)..us(@min(max_sfb, MAX_LTP_LONG_SFB)) {
        ltp.used[sfb] = b.get1() != 0;
    }
}

fn decode_ics_info(d: *mut Decoder, ics: *mut Ics, b: *mut Bits) !void {
    let aot = d.cfg.object_type;
    let si_ = us(d.cfg.sampling_index);
    _ = b.get1(); // reserved
    ics.window_sequence[1] = ics.window_sequence[0];
    ics.window_sequence[0] = b.geti(2);
    ics.use_kb_window[1] = ics.use_kb_window[0];
    ics.use_kb_window[0] = b.get1() != 0;
    ics.num_window_groups = 1;
    ics.group_len[0] = 1;
    if ics.window_sequence[0] == EIGHT_SHORT {
        ics.max_sfb = b.geti(4);
        for _i in @as(usize, 0)..7 {
            if b.get1() != 0 {
                ics.group_len[us(ics.num_window_groups - 1)] += 1;
            } else {
                ics.num_window_groups += 1;
                ics.group_len[us(ics.num_window_groups - 1)] = 1;
            }
        }
        ics.num_windows = 8;
        ics.short = true;
        ics.swb = @as(usize, t.SWB_OFFSET_128_START[si_]);
        ics.num_swb = @as(i32, t.NUM_SWB_128[si_]);
        ics.tns_max_bands = @as(i32, t.TNS_MAX_BANDS_128[si_]);
        ics.predictor_present = false;
    } else {
        ics.max_sfb = b.geti(6);
        ics.num_windows = 1;
        ics.short = false;
        ics.swb = @as(usize, t.SWB_OFFSET_1024_START[si_]);
        ics.num_swb = @as(i32, t.NUM_SWB_1024[si_]);
        ics.tns_max_bands = @as(i32, t.TNS_MAX_BANDS_1024[si_]);
        ics.predictor_present = b.get1() != 0;
        ics.predictor_reset_group = 0;
        if ics.predictor_present {
            if aot == AOT_MAIN {
                if b.get1() != 0 {
                    ics.predictor_reset_group = b.geti(5);
                    if (ics.predictor_reset_group == 0) or (ics.predictor_reset_group > 30) {
                        ics.max_sfb = 0;
                        return error.InvalidAudio;
                    }
                }
                let n = @min(ics.max_sfb, @as(i32, t.PRED_SFB_MAX[si_]));
                for sfb in @as(usize, 0)..us(n) {
                    ics.prediction_used[sfb] = b.get1() != 0;
                }
            } else if aot == AOT_LC {
                ics.max_sfb = 0;
                return error.InvalidAudio;
            } else {
                ics.ltp.present = b.get1() != 0;
                if ics.ltp.present {
                    decode_ltp(&mut ics.ltp, b, ics.max_sfb);
                }
            }
        }
    }
    if ics.max_sfb > ics.num_swb {
        ics.max_sfb = 0;
        return error.InvalidAudio;
    }
}

fn decode_band_types(ch: *mut Chan, b: *mut Bits) !void {
    let ics = &ch.ics;
    let mut bits: u32 = 5;
    if ics.window_sequence[0] == EIGHT_SHORT {
        bits = 3;
    }
    let esc = si(us(@bit_cast(i32, (@as(u32, 1) << bits) - 1)));
    let mut idx: usize = 0;
    for _g in @as(i32, 0)..ics.num_window_groups {
        let mut k: i32 = 0;
        while k < ics.max_sfb {
            let mut sect_end = k;
            let bt = b.geti(4);
            if bt == 12 {
                return error.InvalidAudio;
            }
            loop {
                let incr = b.geti(bits);
                sect_end += incr;
                if b.over or (sect_end > ics.max_sfb) {
                    return error.InvalidAudio;
                }
                if incr != esc {
                    break;
                }
            }
            while k < sect_end {
                ch.band_type[idx] = bt;
                ch.run_end[idx] = sect_end;
                idx += 1;
                k += 1;
            }
        }
    }
}

fn decode_scalefactors(d: *mut Decoder, ch: *mut Chan, b: *mut Bits, global_gain: i32) !void {
    let ics = &ch.ics;
    let mut off0 = global_gain;
    let mut off1 = global_gain - 90;
    let mut off2: i32 = 0;
    let mut noise_flag: i32 = 1;
    let mut idx: usize = 0;
    for _g in @as(i32, 0)..ics.num_window_groups {
        let mut i: i32 = 0;
        while i < ics.max_sfb {
            let run_end = ch.run_end[idx];
            let bt = ch.band_type[idx];
            while i < run_end {
                if bt == ZERO_BT {
                    ch.sf[idx] = 0.0;
                } else if (bt == INTENSITY_BT) or (bt == INTENSITY_BT2) {
                    let v = huff_decode(&d.huff, 0, b);
                    if v < 0 {
                        return error.InvalidAudio;
                    }
                    off2 += v - 60;
                    let c = @min(@max(off2, -155), 100);
                    ch.sf[idx] = d.pow2sf[us(-c + 200)];
                } else if bt == NOISE_BT {
                    if noise_flag > 0 {
                        off1 += b.geti(9) - 256;
                    } else {
                        let v = huff_decode(&d.huff, 0, b);
                        if v < 0 {
                            return error.InvalidAudio;
                        }
                        off1 += v - 60;
                    }
                    noise_flag -= 1;
                    let c = @min(@max(off1, -100), 155);
                    ch.sf[idx] = -d.pow2sf[us(c + 200)];
                } else {
                    let v = huff_decode(&d.huff, 0, b);
                    if v < 0 {
                        return error.InvalidAudio;
                    }
                    off0 += v - 60;
                    if (off0 < 0) or (off0 > 255) {
                        return error.InvalidAudio;
                    }
                    ch.sf[idx] = -d.pow2sf[us((off0 - 100) + 200)];
                }
                i += 1;
                idx += 1;
            }
        }
    }
}

struct Pulse {
    num: i32,
    pos: [4]i32,
    amp: [4]i32,
}

fn decode_pulses(p: *mut Pulse, b: *mut Bits, ics: *Ics) bool {
    p.num = b.geti(2) + 1;
    let pulse_swb = b.geti(6);
    if pulse_swb >= ics.num_swb {
        return false;
    }
    let end = swb_off(ics, ics.num_swb);
    p.pos[0] = swb_off(ics, pulse_swb) + b.geti(5);
    if p.pos[0] >= end {
        return false;
    }
    p.amp[0] = b.geti(4);
    for i in @as(usize, 1)..us(p.num) {
        p.pos[i] = b.geti(5) + p.pos[i - 1];
        if p.pos[i] >= end {
            return false;
        }
        p.amp[i] = b.geti(4);
    }
    return true;
}

fn decode_tns(d: *Decoder, tns: *mut Tns, b: *mut Bits, ics: *Ics) !void {
    let is8 = ics.window_sequence[0] == EIGHT_SHORT;
    let mut max_order: i32 = 12;
    if is8 {
        max_order = 7;
    } else if d.cfg.object_type == AOT_MAIN {
        max_order = 20;
    }
    let mut e8: u32 = 0;
    if is8 {
        e8 = 1;
    }
    for w in @as(usize, 0)..us(ics.num_windows) {
        tns.n_filt[w] = b.geti(2 - e8);
        if tns.n_filt[w] != 0 {
            let coef_res = b.geti(1);
            for f in @as(usize, 0)..us(tns.n_filt[w]) {
                tns.length[w][f] = b.geti(6 - (2 * e8));
                tns.order[w][f] = b.geti(5 - (2 * e8));
                if tns.order[w][f] > max_order {
                    tns.order[w][f] = 0;
                    return error.InvalidAudio;
                }
                if tns.order[w][f] != 0 {
                    tns.direction[w][f] = b.get1() != 0;
                    let compress = b.geti(1);
                    let coef_len = @bit_cast(u32, (coef_res + 3) - compress);
                    let map = (2 * compress) + coef_res;
                    let mut base: usize = 0;
                    if map == 1 {
                        base = 8;
                    } else if map == 2 {
                        base = 24;
                    } else if map == 3 {
                        base = 28;
                    }
                    for i in @as(usize, 0)..us(tns.order[w][f]) {
                        tns.coef[w][f][i] = t.TNS_COEF[base + @as(usize, b.get(coef_len))];
                    }
                }
            }
        }
    }
}

fn decode_gain_control(ch: *Chan, b: *mut Bits) void {
    let modes: [12]u32 = [1, 0, 5, 2, 1, 2, 8, 0, 2, 2, 1, 5];
    let m = us(ch.ics.window_sequence[0]) * 3;
    let max_band = b.get(2);
    for _bd in @as(u32, 0)..max_band {
        for wd in @as(u32, 0)..modes[m] {
            let adjust = b.get(3);
            for _ad in @as(u32, 0)..adjust {
                let mut n: u32 = modes[m + 2];
                if (wd == 0) and (modes[m + 1] != 0) {
                    n = 4;
                }
                b.skip(@as(usize, 4 + n));
            }
        }
    }
}

fn lcg_random(prev: i32) i32 {
    return @bit_cast(i32, (@bit_cast(u32, prev) *% 1664525) +% 1013904223);
}

fn decode_spectrum(d: *mut Decoder, ch: *mut Chan, b: *mut Bits, pulse_present: bool, pulse: *Pulse) !void {
    let ics = &ch.ics;
    let c = 1024 / us(ics.num_windows);
    let maxo = us(swb_off(ics, ics.max_sfb));
    for g in @as(usize, 0)..us(ics.num_windows) {
        for k in maxo..c {
            ch.coeffs[(g * 128) + k] = 0.0;
        }
    }
    let mut idx: usize = 0;
    let mut base: usize = 0;
    for g in @as(usize, 0)..us(ics.num_window_groups) {
        let g_len = us(ics.group_len[g]);
        for i in @as(usize, 0)..us(ics.max_sfb) {
            let bt = ch.band_type[idx];
            let o0 = us(swb_off(ics, si(i)));
            let off_len = us(swb_off(ics, si(i + 1))) - o0;
            let sf = ch.sf[idx];
            for group in @as(usize, 0)..g_len {
                let cf = base + (group * 128) + o0;
                if (bt == ZERO_BT) or (bt >= INTENSITY_BT2) {
                    for k in @as(usize, 0)..off_len {
                        ch.coeffs[cf + k] = 0.0;
                    }
                } else if bt == NOISE_BT {
                    for k in @as(usize, 0)..off_len {
                        d.random_state = lcg_random(d.random_state);
                        ch.coeffs[cf + k] = @float_from_int(f32, d.random_state);
                    }
                    let mut energy: f32 = 0.0;
                    for k in @as(usize, 0)..off_len {
                        energy += ch.coeffs[cf + k] * ch.coeffs[cf + k];
                    }
                    let scale = sf / @float_cast(f32, @sqrt(@float_cast(f64, energy)));
                    for k in @as(usize, 0)..off_len {
                        ch.coeffs[cf + k] = ch.coeffs[cf + k] * scale;
                    }
                } else {
                    let cb = us(bt);
                    let mut k: usize = 0;
                    while k < off_len {
                        let sym = huff_decode(&d.huff, cb, b);
                        if sym < 0 {
                            return error.InvalidAudio;
                        }
                        let mut q: [4]i32 = [0; 4];
                        let mut dim: usize = 2;
                        if cb <= 4 {
                            dim = 4;
                            if cb <= 2 {
                                q[0] = (sym / 27) - 1;
                                q[1] = ((sym / 9) % 3) - 1;
                                q[2] = ((sym / 3) % 3) - 1;
                                q[3] = (sym % 3) - 1;
                            } else {
                                q[0] = sym / 27;
                                q[1] = (sym / 9) % 3;
                                q[2] = (sym / 3) % 3;
                                q[3] = sym % 3;
                            }
                        } else if cb <= 6 {
                            q[0] = (sym / 9) - 4;
                            q[1] = (sym % 9) - 4;
                        } else {
                            let mut m: i32 = 17;
                            if cb <= 8 {
                                m = 8;
                            } else if cb <= 10 {
                                m = 13;
                            }
                            q[0] = sym / m;
                            q[1] = sym % m;
                        }
                        let unsigned = (cb == 3) or (cb == 4) or (cb >= 7);
                        let mut neg: [4]bool = [false; 4];
                        if unsigned {
                            for j in @as(usize, 0)..dim {
                                if q[j] != 0 {
                                    neg[j] = b.get1() != 0;
                                }
                            }
                        } else {
                            for j in @as(usize, 0)..dim {
                                if q[j] < 0 {
                                    neg[j] = true;
                                    q[j] = -q[j];
                                }
                            }
                        }
                        for j in @as(usize, 0)..dim {
                            let mut v: f32 = 0.0;
                            if (cb == 11) and (q[j] == 16) {
                                let mut n: u32 = 0;
                                while b.get1() != 0 {
                                    n += 1;
                                    if n > 8 {
                                        return error.InvalidAudio;
                                    }
                                }
                                let e = n + 4;
                                let val = (@as(u32, 1) << e) + b.get(e);
                                v = d.cbrt[@as(usize, val)] * sf;
                            } else {
                                v = t.VALS[us(q[j])] * sf;
                            }
                            if neg[j] {
                                v = -v;
                            }
                            ch.coeffs[cf + k + j] = v;
                        }
                        k += dim;
                    }
                }
            }
            idx += 1;
        }
        base += g_len * 128;
    }
    if pulse_present {
        let mut bi: i32 = 0;
        for i in @as(usize, 0)..us(pulse.num) {
            let pos = us(pulse.pos[i]);
            let mut co = ch.coeffs[pos];
            while swb_off(ics, bi + 1) <= pulse.pos[i] {
                bi += 1;
            }
            let sfv = ch.sf[us(bi)];
            if (ch.band_type[us(bi)] != NOISE_BT) and (sfv != 0.0) {
                let mut ico = -@float_from_int(f32, pulse.amp[i]);
                if co != 0.0 {
                    co = co / sfv;
                    let s4 = @float_cast(f32, @sqrt(@float_cast(f64, @float_cast(f32, @sqrt(@float_cast(f64, @abs(co)))))));
                    if co > 0.0 {
                        ico = (co / s4) - ico;
                    } else {
                        ico = (co / s4) + ico;
                    }
                }
                ch.coeffs[pos] = (math.cbrtf(@abs(ico)) * ico) * sfv;
            }
        }
    }
}

fn flt16_round(x: f32) f32 {
    return @bit_cast(f32, (@bit_cast(u32, x) +% 0x8000) & 0xffff0000);
}

fn flt16_even(x: f32) f32 {
    let i = @bit_cast(u32, x);
    return @bit_cast(f32, ((i +% 0x7fff) +% ((i & 0x10000) >> 16)) & 0xffff0000);
}

fn flt16_trunc(x: f32) f32 {
    return @bit_cast(f32, @bit_cast(u32, x) & 0xffff0000);
}

fn predict(ps: *mut Pred, coef: *mut f32, output: bool) void {
    let a: f32 = 0.953125;
    let alpha: f32 = 0.90625;
    let r0 = ps.r0;
    let r1 = ps.r1;
    let cor0 = ps.cor0;
    let cor1 = ps.cor1;
    let var0 = ps.var0;
    let var1 = ps.var1;
    let mut k1: f32 = 0.0;
    if var0 > 1.0 {
        k1 = cor0 * flt16_even(a / var0);
    }
    let mut k2: f32 = 0.0;
    if var1 > 1.0 {
        k2 = cor1 * flt16_even(a / var1);
    }
    let pv = flt16_round((k1 * r0) + (k2 * r1));
    if output {
        coef.* = coef.* + pv;
    }
    let e0 = coef.*;
    let e1 = e0 - (k1 * r0);
    ps.cor1 = flt16_trunc((alpha * cor1) + (r1 * e1));
    ps.var1 = flt16_trunc((alpha * var1) + (0.5 * ((r1 * r1) + (e1 * e1))));
    ps.cor0 = flt16_trunc((alpha * cor0) + (r0 * e0));
    ps.var0 = flt16_trunc((alpha * var0) + (0.5 * ((r0 * r0) + (e0 * e0))));
    ps.r1 = flt16_trunc(a * (r0 - (k1 * e0)));
    ps.r0 = flt16_trunc(a * e0);
}

fn apply_prediction(d: *Decoder, ch: *mut Chan) void {
    if not ch.predictor_initialized {
        for k in @as(usize, 0)..MAX_PREDICTORS {
            reset_pred(&mut ch.pred[k]);
        }
        ch.predictor_initialized = true;
    }
    if ch.ics.window_sequence[0] != EIGHT_SHORT {
        let n = @as(i32, t.PRED_SFB_MAX[us(d.cfg.sampling_index)]);
        for sfb in @as(i32, 0)..n {
            let used = ch.ics.predictor_present and ch.ics.prediction_used[us(sfb)];
            for k in us(swb_off(&ch.ics, sfb))..us(swb_off(&ch.ics, sfb + 1)) {
                predict(&mut ch.pred[k], &mut ch.coeffs[k], used);
            }
        }
        if ch.ics.predictor_reset_group != 0 {
            let mut i = us(ch.ics.predictor_reset_group - 1);
            while i < MAX_PREDICTORS {
                reset_pred(&mut ch.pred[i]);
                i += 30;
            }
        }
    } else {
        for k in @as(usize, 0)..MAX_PREDICTORS {
            reset_pred(&mut ch.pred[k]);
        }
    }
}

fn decode_ics(d: *mut Decoder, ch: *mut Chan, b: *mut Bits, common_window: bool) !void {
    let mut pulse = Pulse.{ num: 0, pos: [0; 4], amp: [0; 4] };
    let global_gain = b.geti(8);
    if not common_window {
        decode_ics_info(d, &mut ch.ics, b) catch |e| {
            ch.tns.present = false;
            return e;
        };
    }
    decode_band_types(ch, b) catch |e| {
        ch.tns.present = false;
        return e;
    };
    decode_scalefactors(d, ch, b, global_gain) catch |e| {
        ch.tns.present = false;
        return e;
    };
    let pulse_present = b.get1() != 0;
    if pulse_present {
        if (ch.ics.window_sequence[0] == EIGHT_SHORT) or not decode_pulses(&mut pulse, b, &ch.ics) {
            ch.tns.present = false;
            return error.InvalidAudio;
        }
    }
    ch.tns.present = b.get1() != 0;
    if ch.tns.present {
        decode_tns(d, &mut ch.tns, b, &ch.ics) catch |e| {
            ch.tns.present = false;
            return e;
        };
    }
    if b.get1() != 0 {
        decode_gain_control(ch, b);
    }
    decode_spectrum(d, ch, b, pulse_present, &pulse) catch |e| {
        ch.tns.present = false;
        return e;
    };
    if b.over {
        ch.tns.present = false;
        return error.InvalidAudio;
    }
    if (d.cfg.object_type == AOT_MAIN) and not common_window {
        apply_prediction(d, ch);
    }
}

fn decode_cpe(d: *mut Decoder, c0: usize, b: *mut Bits) !void {
    let mut ms_mask: [128]bool = [false; 128];
    let common_window = b.get1() != 0;
    let mut ms_present: i32 = 0;
    if common_window {
        decode_ics_info(d, &mut d.chans[c0].ics, b)!;
        let kb = d.chans[c0 + 1].ics.use_kb_window[0];
        d.chans[c0 + 1].ics = d.chans[c0].ics;
        d.chans[c0 + 1].ics.use_kb_window[1] = kb;
        let ics1 = &mut d.chans[c0 + 1].ics;
        if ics1.predictor_present and (d.cfg.object_type != AOT_MAIN) {
            ics1.ltp.present = b.get1() != 0;
            if ics1.ltp.present {
                decode_ltp(&mut ics1.ltp, b, ics1.max_sfb);
            }
        }
        ms_present = b.geti(2);
        if ms_present == 3 {
            return error.InvalidAudio;
        }
        let max_idx = us(d.chans[c0].ics.num_window_groups * d.chans[c0].ics.max_sfb);
        if ms_present == 1 {
            for i in @as(usize, 0)..max_idx {
                ms_mask[i] = b.get1() != 0;
            }
        } else if ms_present == 2 {
            for i in @as(usize, 0)..max_idx {
                ms_mask[i] = true;
            }
        }
    }
    decode_ics(d, &mut d.chans[c0], b, common_window)!;
    decode_ics(d, &mut d.chans[c0 + 1], b, common_window)!;
    let ch0 = &mut d.chans[c0];
    let ch1 = &mut d.chans[c0 + 1];
    if common_window {
        if ms_present != 0 {
            // M/S
            let ics = &ch0.ics;
            let mut idx: usize = 0;
            let mut base: usize = 0;
            for g in @as(usize, 0)..us(ics.num_window_groups) {
                for i in @as(usize, 0)..us(ics.max_sfb) {
                    if ms_mask[idx] and (ch0.band_type[idx] < NOISE_BT) and (ch1.band_type[idx] < NOISE_BT) {
                        let o0 = us(swb_off(ics, si(i)));
                        let o1 = us(swb_off(ics, si(i + 1)));
                        for group in @as(usize, 0)..us(ics.group_len[g]) {
                            for k in o0..o1 {
                                let p = base + (group * 128) + k;
                                let tt = ch0.coeffs[p] - ch1.coeffs[p];
                                ch0.coeffs[p] = ch0.coeffs[p] + ch1.coeffs[p];
                                ch1.coeffs[p] = tt;
                            }
                        }
                    }
                    idx += 1;
                }
                base += us(ics.group_len[g]) * 128;
            }
        }
        if d.cfg.object_type == AOT_MAIN {
            apply_prediction(d, ch0);
            apply_prediction(d, ch1);
        }
    }
    // intensity stereo
    let ics = &ch1.ics;
    let mut idx: usize = 0;
    let mut base: usize = 0;
    for g in @as(usize, 0)..us(ics.num_window_groups) {
        let mut i: i32 = 0;
        while i < ics.max_sfb {
            let bt = ch1.band_type[idx];
            let run_end = ch1.run_end[idx];
            if (bt == INTENSITY_BT) or (bt == INTENSITY_BT2) {
                while i < run_end {
                    let mut c = -1 + (2 * (ch1.band_type[idx] - 14));
                    if (ms_present != 0) and ms_mask[idx] {
                        c = -c;
                    }
                    let scale = @float_from_int(f32, c) * ch1.sf[idx];
                    let o0 = us(swb_off(ics, i));
                    let o1 = us(swb_off(ics, i + 1));
                    for group in @as(usize, 0)..us(ics.group_len[g]) {
                        for k in o0..o1 {
                            let p = base + (group * 128) + k;
                            ch1.coeffs[p] = ch0.coeffs[p] * scale;
                        }
                    }
                    i += 1;
                    idx += 1;
                }
            } else {
                idx += us(run_end - i);
                i = run_end;
            }
        }
        base += us(ics.group_len[g]) * 128;
    }
}

// -----------------------------------------------------------------------------
// TNS, the inverse MDCT and windowing, long-term prediction
// -----------------------------------------------------------------------------

fn apply_tns(coef: []mut f32, tns: *Tns, ics: *Ics, decode: bool) void {
    let mmm = @min(ics.tns_max_bands, ics.max_sfb);
    if mmm == 0 {
        return;
    }
    let mut lpc: [20]f64 = [0.0; 20];
    let mut tmp: [21]f32 = [0.0; 21];
    for w in @as(usize, 0)..us(ics.num_windows) {
        let mut bottom = ics.num_swb;
        for f in @as(usize, 0)..us(tns.n_filt[w]) {
            let top = bottom;
            bottom = @max(0, top - tns.length[w][f]);
            let order = tns.order[w][f];
            if order == 0 {
                continue;
            }
            // the LPC coefficients from the reflection coefficients
            for i in @as(usize, 0)..us(order) {
                let r = -@as(f64, tns.coef[w][f][i]);
                lpc[i] = r;
                for j in @as(usize, 0)..((i + 1) >> 1) {
                    let fj = lpc[j];
                    let bj = lpc[i - 1 - j];
                    lpc[j] = fj + (r * bj);
                    lpc[i - 1 - j] = bj + (r * fj);
                }
            }
            let mut start = swb_off(ics, @min(bottom, mmm));
            let end = swb_off(ics, @min(top, mmm));
            let size = end - start;
            if size <= 0 {
                continue;
            }
            let mut inc: i32 = 1;
            if tns.direction[w][f] {
                inc = -1;
                start = end - 1;
            }
            start += si(w * 128);
            if decode {
                for m in @as(i32, 0)..size {
                    let p = us(start);
                    for i in @as(i32, 1)..(@min(m, order) + 1) {
                        let x = @as(f64, coef[us(start - (i * inc))]) * lpc[us(i - 1)];
                        coef[p] = @float_cast(f32, @as(f64, coef[p]) - x);
                    }
                    start += inc;
                }
            } else {
                for m in @as(i32, 0)..size {
                    let p = us(start);
                    tmp[0] = coef[p];
                    for i in @as(i32, 1)..(@min(m, order) + 1) {
                        coef[p] = @float_cast(f32, @as(f64, coef[p]) + (@as(f64, tmp[us(i)]) * lpc[us(i - 1)]));
                    }
                    let mut i = order;
                    while i > 0 {
                        tmp[us(i)] = tmp[us(i - 1)];
                        i -= 1;
                    }
                    start += inc;
                }
            }
        }
    }
}

/// An in-place complex FFT of n = 2^k points.
fn fft(re: []mut f64, im: []mut f64, n: usize) void {
    let mut j: usize = 0;
    for i in @as(usize, 1)..n {
        let mut bit = n >> 1;
        while (j & bit) != 0 {
            j ^= bit;
            bit = bit >> 1;
        }
        j ^= bit;
        if i < j {
            let tr = re[i];
            re[i] = re[j];
            re[j] = tr;
            let ti = im[i];
            im[i] = im[j];
            im[j] = ti;
        }
    }
    let mut len: usize = 2;
    while len <= n {
        let ang = -6.283185307179586476925 / @float_from_int(f64, len);
        let half = len >> 1;
        for k in @as(usize, 0)..half {
            let a = ang * @float_from_int(f64, k);
            let wr = math.cos(a);
            let wi = math.sin(a);
            let mut i = k;
            while i < n {
                let p = i + half;
                let xr = (re[p] * wr) - (im[p] * wi);
                let xi = (re[p] * wi) + (im[p] * wr);
                re[p] = re[i] - xr;
                im[p] = im[i] - xi;
                re[i] = re[i] + xr;
                im[i] = im[i] + xi;
                i += len;
            }
        }
        len = len << 1;
    }
}

/// u[n] = scale * sum_k x[k] cos(pi/N (n + 1/2)(k + 1/2)), N = x.len.
fn dct4(d: *mut Decoder, x: []f64, u: []mut f64, scale: f64) void {
    let n = x.len;
    let h = n / 2;
    let nf = @float_from_int(f64, n);
    for j in @as(usize, 0)..h {
        let a = (-3.14159265358979323846 * @float_from_int(f64, (4 * j) + 1)) / (4.0 * nf);
        let c = math.cos(a);
        let s = math.sin(a);
        let xr = x[2 * j];
        let xi = x[(n - 1) - (2 * j)];
        d.fft_re[j] = (xr * c) - (xi * s);
        d.fft_im[j] = (xr * s) + (xi * c);
    }
    fft(d.fft_re[..], d.fft_im[..], h);
    for p in @as(usize, 0)..h {
        let a = (-3.14159265358979323846 * @float_from_int(f64, p)) / nf;
        let c = math.cos(a);
        let s = math.sin(a);
        let wr = (d.fft_re[p] * c) - (d.fft_im[p] * s);
        let wi = (d.fft_re[p] * s) + (d.fft_im[p] * c);
        u[2 * p] = wr * scale;
        u[(n - 1) - (2 * p)] = -wi * scale;
    }
}

const IMDCT_SCALE: f64 = 1.0 / (1024.0 * 32768.0);

/// The middle half of the inverse MDCT of in[0..n] (n = 1024 or 128),
/// into out[0..n].
fn imdct_half(d: *mut Decoder, inp: []f32, out: []mut f32, n: usize) void {
    let mut x: [1024]f64 = [0.0; 1024];
    let mut u: [1024]f64 = [0.0; 1024];
    for k in @as(usize, 0)..n {
        x[k] = @as(f64, inp[k]);
    }
    dct4(d, x[..n], u[..n], IMDCT_SCALE * (1024.0 / @float_from_int(f64, n)));
    for m in @as(usize, 0)..n {
        out[m] = @float_cast(f32, u[(n - 1) - m]);
    }
}

/// dst[i] and dst[2 len - 1 - i] from src0, src1 and the rising window.
fn fmul_window(dst: []mut f32, s0: []f32, s1: []f32, win: []f32, len: usize) void {
    for k in @as(usize, 0)..len {
        let i = k;
        let j = ((2 * len) - 1) - k;
        let a = s0[i];
        let bb = s1[(len - 1) - k];
        let wi = win[i];
        let wj = win[j];
        dst[i] = (a * wj) - (bb * wi);
        dst[j] = (a * wi) + (bb * wj);
    }
}

fn long_window(kb: bool) []f32 {
    if kb {
        return t.KBD_1024[..];
    }
    return t.SINE_1024[..];
}

fn short_window(kb: bool) []f32 {
    if kb {
        return t.KBD_128[..];
    }
    return t.SINE_128[..];
}

fn imdct_and_windowing(d: *mut Decoder, ch: *mut Chan) void {
    let ics = &ch.ics;
    let sw = short_window(ics.use_kb_window[0]);
    let swp = short_window(ics.use_kb_window[1]);
    let lwp = long_window(ics.use_kb_window[1]);
    let seq = ics.window_sequence[0];
    let prev = ics.window_sequence[1];
    if seq == EIGHT_SHORT {
        for i in @as(usize, 0)..8 {
            let mut o: [128]f32 = [0.0; 128];
            imdct_half(d, ch.coeffs[i * 128..], o[..], 128);
            for k in @as(usize, 0)..128 {
                d.buf[(i * 128) + k] = o[k];
            }
        }
    } else {
        let mut o: [1024]f32 = [0.0; 1024];
        imdct_half(d, ch.coeffs[..], o[..], 1024);
        d.buf = o;
    }
    let buf = d.buf[..];
    if ((prev == ONLY_LONG) or (prev == LONG_STOP)) and ((seq == ONLY_LONG) or (seq == LONG_START)) {
        fmul_window(ch.ret[..], ch.saved[..], buf, lwp, 512);
    } else {
        for k in @as(usize, 0)..448 {
            ch.ret[k] = ch.saved[k];
        }
        if seq == EIGHT_SHORT {
            fmul_window(ch.ret[448..], ch.saved[448..], buf, swp, 64);
            fmul_window(ch.ret[448 + 128..], buf[64..], buf[128..], sw, 64);
            fmul_window(ch.ret[448 + 256..], buf[128 + 64..], buf[256..], sw, 64);
            fmul_window(ch.ret[448 + 384..], buf[256 + 64..], buf[384..], sw, 64);
            fmul_window(d.temp[..], buf[384 + 64..], buf[512..], sw, 64);
            for k in @as(usize, 0)..64 {
                ch.ret[448 + 512 + k] = d.temp[k];
            }
        } else {
            fmul_window(ch.ret[448..], ch.saved[448..], buf, swp, 64);
            for k in @as(usize, 0)..448 {
                ch.ret[576 + k] = buf[64 + k];
            }
        }
    }
    // buffer update
    if seq == EIGHT_SHORT {
        for k in @as(usize, 0)..64 {
            ch.saved[k] = d.temp[64 + k];
        }
        fmul_window(ch.saved[64..], buf[512 + 64..], buf[640..], sw, 64);
        fmul_window(ch.saved[192..], buf[640 + 64..], buf[768..], sw, 64);
        fmul_window(ch.saved[320..], buf[768 + 64..], buf[896..], sw, 64);
        for k in @as(usize, 0)..64 {
            ch.saved[448 + k] = buf[896 + 64 + k];
        }
    } else if seq == LONG_START {
        for k in @as(usize, 0)..448 {
            ch.saved[k] = buf[512 + k];
        }
        for k in @as(usize, 0)..64 {
            ch.saved[448 + k] = buf[896 + 64 + k];
        }
    } else {
        for k in @as(usize, 0)..512 {
            ch.saved[k] = buf[512 + k];
        }
    }
}

/// The forward MDCT of in[0..2048] into out[0..1024], scaled to match
/// the inverse transform above.
fn mdct_ltp(d: *mut Decoder, inp: []f32, out: []mut f32) void {
    let mut z: [1024]f64 = [0.0; 1024];
    let mut u: [1024]f64 = [0.0; 1024];
    let n: usize = 1024;
    let h = n / 2;
    for q in @as(usize, 0)..h {
        z[q] = -@as(f64, inp[q + (3 * h)]) - @as(f64, inp[((3 * h) - 1) - q]);
    }
    for q in h..n {
        z[q] = @as(f64, inp[q - h]) - @as(f64, inp[((3 * h) - 1) - q]);
    }
    dct4(d, z[..], u[..], -32768.0);
    for k in @as(usize, 0)..n {
        out[k] = @float_cast(f32, u[k]);
    }
}

fn apply_ltp(d: *mut Decoder, ch: *mut Chan) void {
    let ltp = &ch.ics.ltp;
    if ch.ics.window_sequence[0] == EIGHT_SHORT {
        return;
    }
    let mut num: usize = 2048;
    if ltp.lag < 1024 {
        num = us(ltp.lag + 1024);
    }
    for i in @as(usize, 0)..num {
        d.ltp_in[i] = ch.ltp_state[(i + 2048) - us(ltp.lag)] * ltp.coef;
    }
    for i in num..2048 {
        d.ltp_in[i] = 0.0;
    }
    // windowing
    let ics = &ch.ics;
    let lw = long_window(ics.use_kb_window[0]);
    let swn = short_window(ics.use_kb_window[0]);
    let lwp = long_window(ics.use_kb_window[1]);
    let swp = short_window(ics.use_kb_window[1]);
    if ics.window_sequence[0] != LONG_STOP {
        for i in @as(usize, 0)..1024 {
            d.ltp_in[i] = d.ltp_in[i] * lwp[i];
        }
    } else {
        for i in @as(usize, 0)..448 {
            d.ltp_in[i] = 0.0;
        }
        for i in @as(usize, 0)..128 {
            d.ltp_in[448 + i] = d.ltp_in[448 + i] * swp[i];
        }
    }
    if ics.window_sequence[0] != LONG_START {
        for i in @as(usize, 0)..1024 {
            d.ltp_in[1024 + i] = d.ltp_in[1024 + i] * lw[1023 - i];
        }
    } else {
        for i in @as(usize, 0)..128 {
            d.ltp_in[1024 + 448 + i] = d.ltp_in[1024 + 448 + i] * swn[127 - i];
        }
        for i in @as(usize, 0)..448 {
            d.ltp_in[1024 + 576 + i] = 0.0;
        }
    }
    let mut inp: [2048]f32 = d.ltp_in;
    let mut freq: [1024]f32 = [0.0; 1024];
    mdct_ltp(d, inp[..], freq[..]);
    if ch.tns.present {
        apply_tns(freq[..], &ch.tns, &ch.ics, false);
    }
    for sfb in @as(i32, 0)..@min(ics.max_sfb, MAX_LTP_LONG_SFB) {
        if ltp.used[us(sfb)] {
            for i in us(swb_off(ics, sfb))..us(swb_off(ics, sfb + 1)) {
                ch.coeffs[i] = ch.coeffs[i] + freq[i];
            }
        }
    }
    _ = inp;
}

fn update_ltp(d: *mut Decoder, ch: *mut Chan) void {
    let ics = &ch.ics;
    let lw = long_window(ics.use_kb_window[0]);
    let sw = short_window(ics.use_kb_window[0]);
    let mut sl: [1024]f32 = [0.0; 1024];
    let buf = d.buf[..];
    if ics.window_sequence[0] == EIGHT_SHORT {
        for i in @as(usize, 0)..512 {
            sl[i] = ch.saved[i];
        }
        for i in @as(usize, 0)..64 {
            sl[448 + i] = buf[960 + i] * sw[127 - i];
        }
        for i in @as(usize, 0)..64 {
            sl[512 + i] = buf[1023 - i] * sw[63 - i];
        }
    } else if ics.window_sequence[0] == LONG_START {
        for i in @as(usize, 0)..448 {
            sl[i] = buf[512 + i];
        }
        for i in @as(usize, 0)..64 {
            sl[448 + i] = buf[960 + i] * sw[127 - i];
        }
        for i in @as(usize, 0)..64 {
            sl[512 + i] = buf[1023 - i] * sw[63 - i];
        }
    } else {
        for i in @as(usize, 0)..512 {
            sl[i] = buf[512 + i] * lw[1023 - i];
        }
        for i in @as(usize, 0)..512 {
            sl[512 + i] = buf[1023 - i] * lw[511 - i];
        }
    }
    for i in @as(usize, 0)..1024 {
        ch.ltp_state[i] = ch.ltp_state[1024 + i];
    }
    for i in @as(usize, 0)..1024 {
        ch.ltp_state[1024 + i] = ch.ret[i];
    }
    for i in @as(usize, 0)..1024 {
        ch.ltp_state[2048 + i] = sl[i];
    }
}

// -----------------------------------------------------------------------------
// Raw data blocks
// -----------------------------------------------------------------------------

fn skip_dse(b: *mut Bits) !void {
    let align = b.get1() != 0;
    let mut count = @as(usize, b.get(8));
    if count == 255 {
        count += @as(usize, b.get(8));
    }
    if align {
        b.align();
    }
    b.skip(8 * count);
    if b.over {
        return error.InvalidAudio;
    }
}

fn decode_fil(d: *mut Decoder, b: *mut Bits, id: usize) !void {
    let mut count = id;
    if count == 15 {
        count += @as(usize, b.get(8)) - 1;
    }
    if b.left() < @as(i64, @bit_cast(i64, @as(u64, 8 * count))) {
        return error.InvalidAudio;
    }
    if count > 0 {
        let typ = b.get(4);
        if (typ == 13) or (typ == 14) {
            d.cfg.sbr = true; // spectral band replication: not decoded (the core is)
        }
        b.skip((8 * count) - 4);
    }
}

/// Decodes one raw data block (a frame) into out: 1024 interleaved frames
/// of channels(d) samples. Returns false if the block held no audio.
pub fn decode_block(d: *mut Decoder, b: *mut Bits, out: []mut f32) !bool {
    let align_ref = b.pos;
    let mut present: [64]bool = [false; 64];
    let mut seen: [4][16]u8 = [[0; 16]; 4];
    let mut audio = false;
    loop {
        let typ = b.geti(3);
        if typ == TYPE_END {
            break;
        }
        let id = b.geti(4);
        if b.over {
            return error.InvalidAudio;
        }
        if typ < TYPE_DSE {
            if typ == TYPE_CCE {
                return error.Unsupported;
            }
            if not d.configured {
                return error.InvalidAudio;
            }
            if seen[us(typ)][us(id)] > 0 {
                return error.InvalidAudio;
            }
            seen[us(typ)][us(id)] += 1;
            let k = us(@min(typ, 2));
            let c = d.slot[k][us(id)];
            if c < 0 {
                // FFmpeg's fallback: a lone single channel element in a
                // stereo layout, or an LFE numbered differently
                return error.InvalidAudio;
            }
            let cu = us(c);
            present[cu] = true;
            if typ == TYPE_CPE {
                present[cu + 1] = true;
                decode_cpe(d, cu, b)!;
            } else {
                decode_ics(d, &mut d.chans[cu], b, false)!;
            }
            audio = true;
        } else if typ == TYPE_DSE {
            skip_dse(b)!;
        } else if typ == TYPE_PCE {
            let mut cfg = d.cfg;
            decode_pce(b, &mut cfg, align_ref)!;
            if (d.cfg.chan_config == 0) and not d.configured {
                d.cfg = cfg;
                configure(d)!;
            }
        } else if typ == TYPE_FIL {
            decode_fil(d, b, @as(usize, @bit_cast(u32, id)))!;
        }
        if b.over or (b.left() < 3) {
            return error.InvalidAudio;
        }
    }
    if not audio {
        return false;
    }
    // TNS, prediction, the inverse transform
    for c in @as(usize, 0)..d.nchans {
        let ch = &mut d.chans[c];
        if not present[c] {
            @mem_set(ch.coeffs[..], 0.0);
            ch.tns.present = false;
        }
        if d.cfg.object_type == AOT_LTP {
            if ch.ics.predictor_present and ch.ics.ltp.present {
                apply_ltp(d, ch);
            }
        }
        if ch.tns.present {
            apply_tns(ch.coeffs[..], &ch.tns, &ch.ics, true);
        }
        imdct_and_windowing(d, ch);
        if d.cfg.object_type == AOT_LTP {
            update_ltp(d, ch);
        }
    }
    let n = d.nchans;
    for c in @as(usize, 0)..n {
        let src = &d.chans[d.order[c]];
        for i in @as(usize, 0)..1024 {
            out[(i * n) + c] = src.ret[i];
        }
    }
    return true;
}
