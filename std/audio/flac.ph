// =============================================================================
// std/audio/flac.ph - FLAC, the Free Lossless Audio Codec (RFC 9639).
//
//     let mut s = flac.decode(bytes)!;
//     defer s.deinit();
//     let out = flac.encode(&s, flac.DEFAULT)!;   // mem.free it
//
// Decoding reads every FLAC stream: 4 to 32 bits per sample, 1 to 8
// channels, fixed and variable block sizes, all subframe types and stereo
// decorrelations. Samples come out left-justified in the smallest format
// that holds them (a 12-bit stream as S16, 20-bit as S24). Frame CRCs, and
// the MD5 signature of the audio when the stream has one, are checked.
//
// Encoding takes integer sounds (a float sound is written as S24) and, per
// block, keeps the smallest of constant, verbatim, fixed-polynomial and
// linear-predictive subframes, with the best stereo decorrelation and Rice
// partitioning. Levels 0 to 8 trade speed for size as in the reference
// encoder: DEFAULT (5) is about as small as `flac -5`.
// =============================================================================

import "../audio.ph" as audio;
import "../crypto/md5.ph" as md5;
import "../math.ph" as math;
import "../mem.ph" as mem;

error { InvalidAudio, Unsupported, OutOfMemory, TooLarge }

pub const FAST: u32 = 0;
pub const DEFAULT: u32 = 5;
pub const BEST: u32 = 8;

const MAX_BLOCK: usize = 65536;  // (a frame header can say 65536)
const MAX_LPC: usize = 32;

/// Where the "fLaC" marker is (after an ID3v2 tag, if any), or none.
fn start_of(d: []u8) !usize {
    let mut p: usize = 0;
    if (d.len >= 10) and @mem_eq(d[..3], "ID3") {
        let size = (
            ((@as(usize, d[6] & 0x7f) << 21) | (@as(usize, d[7] & 0x7f) << 14)) | (
                @as(usize, d[8] & 0x7f) << 7)) | @as(usize, d[9] & 0x7f);
        p = 10 + size;
        if (d[5] & 0x10) != 0 {
            p += 10;  // footer
        }
    }
    if ((p + 4) > d.len) or not @mem_eq(d[p..p + 4], "fLaC") {
        return error.InvalidAudio;
    }
    return p;
}

pub fn is_flac(data: []u8) bool {
    _ = start_of(data) catch return false;
    return true;
}

// -----------------------------------------------------------------------------
// CRCs
// -----------------------------------------------------------------------------

fn crc8(d: []u8) u8 {
    let mut c: u32 = 0;
    for b in d {
        c ^= @as(u32, b);
        for _k in @as(u32, 0)..8 {
            if (c & 0x80) != 0 {
                c = ((c << 1) ^ 0x07) & 0xff;
            } else {
                c = (c << 1) & 0xff;
            }
        }
    }
    return @truncate(u8, c);
}

let mut crc16_table: [256]u16 = [0; 256];
let mut crc16_ready: bool = false;

fn crc16(d: []u8) u16 {
    if not crc16_ready {
        for i in @as(u32, 0)..256 {
            let mut c = i << 8;
            for _k in @as(u32, 0)..8 {
                if (c & 0x8000) != 0 {
                    c = ((c << 1) ^ 0x8005) & 0xffff;
                } else {
                    c = (c << 1) & 0xffff;
                }
            }
            crc16_table[@as(usize, i)] = @truncate(u16, c);
        }
        crc16_ready = true;
    }
    let mut c: u32 = 0;
    for b in d {
        c = ((c << 8) & 0xffff) ^ @as(u32, crc16_table[@as(usize, (c >> 8) ^ @as(u32, b))]);
    }
    return @truncate(u16, c);
}

// -----------------------------------------------------------------------------
// Decoding
// -----------------------------------------------------------------------------

/// A most-significant-bit-first reader.
struct Bits {
    d: []u8,
    pos: usize,  // the next byte to load
    cache: u64,  // unread bits, at the top
    n: u32,      // how many
    over: bool,  // loaded past the end (zeros)

    fn refill(self: *mut Bits) void {
        while self.n <= 56 {
            let mut b: u64 = 0;
            if self.pos < self.d.len {
                b = @as(u64, self.d[self.pos]);
            } else {
                self.over = true;
            }
            self.pos += 1;
            self.cache |= b << @as(u64, 56 - self.n);
            self.n += 8;
        }
    }

    /// k bits (k <= 56) as an unsigned number.
    fn read(self: *mut Bits, k: u32) u64 {
        if k == 0 {
            return 0;
        }
        if self.n < k {
            self.refill();
        }
        let v = self.cache >> @as(u64, 64 - k);
        self.cache = self.cache << @as(u64, k);
        self.n -= k;
        return v;
    }

    /// k bits (k <= 56) as a two's complement number.
    fn signed(self: *mut Bits, k: u32) i64 {
        if k == 0 {
            return 0;
        }
        let sh = @as(u64, 64 - k);
        return @bit_cast(i64, self.read(k) << sh) >> sh;
    }

    /// Counts zero bits up to a one (which is consumed).
    fn unary(self: *mut Bits) u64 {
        let mut count: u64 = 0;
        loop {
            if self.cache != 0 {
                let z = @as(u32, @truncate(u8, @clz(self.cache)));
                if z < self.n {
                    self.cache = self.cache << @as(u64, z + 1);
                    self.n -= z + 1;
                    return count + @as(u64, z);
                }
            }
            count += @as(u64, self.n);
            self.cache = 0;
            self.n = 0;
            if self.over {
                return count;
            }
            self.refill();
        }
    }

    /// Whether more bits were consumed than there are.
    fn past_end(self: *Bits) bool {
        return ((self.pos * 8) - @as(usize, self.n)) > (self.d.len * 8);
    }

    fn align(self: *mut Bits) void {
        let r = self.n % 8;
        self.cache = self.cache << @as(u64, r);
        self.n -= r;
    }

    /// The offset of the next unread byte (when aligned).
    fn byte_pos(self: *Bits) usize {
        return self.pos - @as(usize, self.n / 8);
    }

    fn seek(self: *mut Bits, p: usize) void {
        self.pos = p;
        self.cache = 0;
        self.n = 0;
    }
}

struct Info {
    rate: u32,
    channels: usize,
    bps: u32,
    total: u64,
    max_block: usize,
    md5: [16]u8,
}

fn read_streaminfo(d: []u8, at: usize) !Info {
    if (at + 34) > d.len {
        return error.InvalidAudio;
    }
    let mut b = Bits.{ d: d[at..at + 34], pos: 0, cache: 0, n: 0, over: false };
    _ = b.read(16);  // minimum block size
    let max_block = @truncate(usize, b.read(16));
    _ = b.read(24);
    _ = b.read(24);
    let rate = @truncate(u32, b.read(20));
    let channels = @truncate(usize, b.read(3)) + 1;
    let bps = @truncate(u32, b.read(5)) + 1;
    let total = b.read(36);
    let mut info = Info.{ rate: rate, channels: channels, bps: bps, total: total, max_block: max_block, md5: [0; 16] };
    @mem_copy(info.md5[..], d[at + 18..at + 34]);
    if (bps < 4) or (rate == 0) {
        return error.InvalidAudio;
    }
    return info;
}

/// The frame being decoded.
struct Frame {
    block: usize,
    rate: u32,
    channels: usize,
    assign: u32,  // 0-7 independent, 8 left/side, 9 side/right, 10 mid/side
    bps: u32,
}

let RATES: [12]u32 = [0, 88200, 176400, 192000, 8000, 16000, 22050, 24000, 32000, 44100, 48000, 96000];
let SIZES: [8]u32 = [0, 8, 12, 0, 16, 20, 24, 32];

/// Reads a frame header at byte p; the header's CRC must match.
fn frame_header(d: []u8, p: usize, info: *Info) !Frame {
    if ((p + 6) > d.len) or (d[p] != 0xff) or ((d[p + 1] & 0xfe) != 0xf8) {
        return error.InvalidAudio;
    }
    let bs_code = d[p + 2] >> 4;
    let sr_code = d[p + 2] & 15;
    let ch_code = @as(u32, d[p + 3] >> 4);
    let ss_code = (d[p + 3] >> 1) & 7;
    if ((d[p + 3] & 1) != 0) or (bs_code == 0) or (sr_code == 15) or (ch_code > 10) or (ss_code == 3) {
        return error.InvalidAudio;
    }
    // the frame or sample number, UTF-8 style
    let mut q = p + 4;
    let lead = d[q];
    let mut extra: usize = 0;
    if lead >= 0x80 {
        if (lead & 0xc0) == 0x80 or (lead == 0xff) {
            return error.InvalidAudio;
        }
        let ones = @as(usize, @clz(@as(u32, ~lead) << 24));
        extra = ones - 1;
    }
    q += 1 + extra;
    let mut block: usize = 0;
    if bs_code == 1 {
        block = 192;
    } else if bs_code <= 5 {
        block = @as(usize, 576) << @as(usize, bs_code - 2);
    } else if bs_code == 6 {
        if q >= d.len {
            return error.InvalidAudio;
        }
        block = @as(usize, d[q]) + 1;
        q += 1;
    } else if bs_code == 7 {
        if (q + 2) > d.len {
            return error.InvalidAudio;
        }
        block = ((@as(usize, d[q]) << 8) | @as(usize, d[q + 1])) + 1;
        q += 2;
    } else {
        block = @as(usize, 256) << @as(usize, bs_code - 8);
    }
    let mut rate = info.rate;
    if (sr_code >= 1) and (sr_code <= 11) {
        rate = RATES[@as(usize, sr_code)];
    } else if sr_code == 12 {
        if q >= d.len {
            return error.InvalidAudio;
        }
        rate = @as(u32, d[q]) * 1000;
        q += 1;
    } else if (sr_code == 13) or (sr_code == 14) {
        if (q + 2) > d.len {
            return error.InvalidAudio;
        }
        rate = (@as(u32, d[q]) << 8) | @as(u32, d[q + 1]);
        if sr_code == 14 {
            rate *= 10;
        }
        q += 2;
    }
    if q >= d.len {
        return error.InvalidAudio;
    }
    if crc8(d[p..q]) != d[q] {
        return error.InvalidAudio;
    }
    let mut bps = info.bps;
    if ss_code != 0 {
        bps = SIZES[@as(usize, ss_code)];
    }
    let mut channels = @as(usize, ch_code) + 1;
    if ch_code >= 8 {
        channels = 2;
    }
    return Frame.{ block: block, rate: rate, channels: channels, assign: ch_code, bps: bps };
}

/// The length of the frame header at p (it has been checked).
fn header_len(d: []u8, p: usize) usize {
    let bs_code = d[p + 2] >> 4;
    let sr_code = d[p + 2] & 15;
    let lead = d[p + 4];
    let mut n: usize = 5;
    if lead >= 0x80 {
        n += @as(usize, @clz(@as(u32, ~lead) << 24)) - 1;
    }
    if bs_code == 6 {
        n += 1;
    } else if bs_code == 7 {
        n += 2;
    }
    if sr_code == 12 {
        n += 1;
    } else if (sr_code == 13) or (sr_code == 14) {
        n += 2;
    }
    return n + 1;  // CRC-8
}

fn fits(v: i64, bps: u32) bool {
    let lim = @as(i64, 1) << @as(u64, bps - 1);
    return (v >= -lim) and (v < lim);
}

fn residual(b: *mut Bits, block: usize, order: usize, out: []mut i64) !void {
    let method = b.read(2);
    if method > 1 {
        return error.InvalidAudio;
    }
    let mut pbits: u32 = 4;
    let mut esc: u64 = 15;
    if method == 1 {
        pbits = 5;
        esc = 31;
    }
    let porder = @truncate(u32, b.read(4));
    let parts = @as(usize, 1) << @as(usize, porder);
    let psize = block >> @as(usize, porder);
    if ((psize << @as(usize, porder)) != block) or (psize < order) {
        return error.InvalidAudio;
    }
    let mut i = order;
    for part in @as(usize, 0)..parts {
        let mut end = i + psize;
        if part == 0 {
            end = psize;
        }
        let k = b.read(pbits);
        if k == esc {
            let nb = @truncate(u32, b.read(5));
            while i < end {
                out[i] = b.signed(nb);
                i += 1;
            }
            continue;
        }
        let kk = @as(u64, k);
        while i < end {
            let q = b.unary();
            if q >= 0x100000000 {
                return error.InvalidAudio;
            }
            let v = (q << kk) | b.read(@truncate(u32, k));
            out[i] = @bit_cast(i64, v >> 1) ^ -@bit_cast(i64, v & 1);
            i += 1;
        }
    }
    if b.past_end() {
        return error.InvalidAudio;
    }
}

fn subframe(b: *mut Bits, block: usize, bps0: u32, out: []mut i64) !void {
    if b.read(1) != 0 {
        return error.InvalidAudio;
    }
    let t = b.read(6);
    let mut wasted: u32 = 0;
    if b.read(1) == 1 {
        let w = b.unary() + 1;
        if w >= @as(u64, bps0) {
            return error.InvalidAudio;
        }
        wasted = @truncate(u32, w);
    }
    let bps = bps0 - wasted;
    if t == 0 {
        let v = b.signed(bps);
        for i in @as(usize, 0)..block {
            out[i] = v;
        }
    } else if t == 1 {
        for i in @as(usize, 0)..block {
            out[i] = b.signed(bps);
        }
    } else if (t >= 8) and (t <= 12) {
        let order = @truncate(usize, t - 8);
        if order > block {
            return error.InvalidAudio;
        }
        for i in @as(usize, 0)..order {
            out[i] = b.signed(bps);
        }
        residual(b, block, order, out)!;
        for i in order..block {
            let r = out[i];
            let mut v: i64 = r;
            if order == 1 {
                v = r + out[i - 1];
            } else if order == 2 {
                v = (r + (2 * out[i - 1])) - out[i - 2];
            } else if order == 3 {
                v = ((r + (3 * out[i - 1])) - (3 * out[i - 2])) + out[i - 3];
            } else if order == 4 {
                v = (((r + (4 * out[i - 1])) - (6 * out[i - 2])) + (4 * out[i - 3])) - out[i - 4];
            }
            if not fits(v, bps) {
                return error.InvalidAudio;
            }
            out[i] = v;
        }
    } else if t >= 32 {
        let order = @truncate(usize, t & 31) + 1;
        if order > block {
            return error.InvalidAudio;
        }
        for i in @as(usize, 0)..order {
            out[i] = b.signed(bps);
        }
        let prec = @truncate(u32, b.read(4)) + 1;
        if prec == 16 {
            return error.InvalidAudio;
        }
        let shift = b.signed(5);
        if shift < 0 {
            return error.InvalidAudio;
        }
        let mut coef: [32]i64 = [0; 32];
        for j in @as(usize, 0)..order {
            coef[j] = b.signed(prec);
        }
        residual(b, block, order, out)!;
        let sh = @bit_cast(u64, shift);
        let mut rc: [32]i64 = [0; 32];
        for j in @as(usize, 0)..order {
            rc[j] = coef[order - 1 - j];
        }
        let cs = rc[..order];
        let lim = @as(i64, 1) << @as(u64, bps - 1);
        for i in order..block {
            let h = out[i - order..i];
            let mut sum: i64 = 0;
            for j in @as(usize, 0)..cs.len {
                sum += cs[j] * h[j];
            }
            let v = out[i] + (sum >> sh);
            if (v < -lim) or (v >= lim) {
                return error.InvalidAudio;
            }
            out[i] = v;
        }
    } else {
        return error.InvalidAudio;
    }
    if b.past_end() {
        return error.InvalidAudio;
    }
    if wasted > 0 {
        for i in @as(usize, 0)..block {
            out[i] = out[i] << @as(u64, wasted);
        }
    }
}

/// The format samples of bps bits are held in.
fn format_for(bps: u32) u8 {
    if bps <= 8 {
        return audio.U8;
    }
    if bps <= 16 {
        return audio.S16;
    }
    if bps <= 24 {
        return audio.S24;
    }
    return audio.S32;
}

/// The next frame's sync code at or after p (with a valid header), or
/// d.len.
fn find_sync(d: []u8, p: usize, info: *Info) usize {
    let mut q = p;
    while (q + 1) < d.len {
        if (d[q] == 0xff) and ((d[q + 1] & 0xfe) == 0xf8) {
            let ok = frame_header(d, q, info) catch Frame.{ block: 0, rate: 0, channels: 0, assign: 0, bps: 0 };
            if ok.block > 0 {
                return q;
            }
        }
        q += 1;
    }
    return d.len;
}

pub fn decode(data: []u8) !audio.Sound {
    let start = start_of(data)!;
    let mut p = start + 4;
    let mut info = Info.{ rate: 0, channels: 0, bps: 0, total: 0, max_block: 0, md5: [0; 16] };
    let mut have_info = false;
    loop {
        if (p + 4) > data.len {
            return error.InvalidAudio;
        }
        let last = (data[p] & 0x80) != 0;
        let kind = data[p] & 0x7f;
        let len = ((@as(usize, data[p + 1]) << 16) | (@as(usize, data[p + 2]) << 8)) | @as(usize, data[p + 3]);
        if (kind == 0) and (len >= 34) {
            info = read_streaminfo(data, p + 4)!;
            have_info = true;
        }
        p += 4 + len;
        if last {
            break;
        }
    }
    if not have_info {
        return error.InvalidAudio;
    }
    let format = format_for(info.bps);
    let nch = info.channels;
    // the declared length sizes the buffer, within reason: a tiny file can
    // claim a billion samples (it grows as frames really arrive)
    let mut cap: usize = 4096;
    if (info.total > 0) and (info.total < @as(u64, audio.MAX_SAMPLES)) {
        cap = @min(@truncate(usize, info.total), 1 << 20);
    }
    let mut s = audio.Sound.init(cap, nch, info.rate, format)!;
    decode_frames(data, p, &info, &mut s) catch |e| {
        s.deinit();
        return e;
    };
    return s;
}

/// Adds a block's samples (channel c at chans[c * stride..]) to the MD5
/// signature: interleaved, little-endian, width bytes each.
fn md5_block(sig: *mut md5.Md5, chans: []i64, stride: usize, block: usize, nch: usize, width: usize,
             buf: []mut u8) void {
    let fb = nch * width;
    for c in @as(usize, 0)..nch {
        let src = chans[c * stride..(c * stride) + block];
        let mut at = c * width;
        for i in @as(usize, 0)..block {
            let u = @bit_cast(u64, src[i]);
            for k in @as(usize, 0)..width {
                buf[at + k] = @truncate(u8, u >> @as(u64, 8 * k));
            }
            at += fb;
        }
    }
    sig.update(buf[..block * fb]);
}

fn decode_frames(data: []u8, p0: usize, info: *Info, s: *mut audio.Sound) !void {
    let nch = info.channels;
    let chans = mem.alloc.[i64](MAX_BLOCK * 8)!;
    defer mem.free(chans);
    let width = @as(usize, (info.bps + 7) / 8);
    let mut sig = md5.Md5.init();
    let zero: [16]u8 = [0; 16];
    let check_md5 = not @mem_eq(info.md5[..], zero[..]);
    let mbuf = mem.alloc.[u8](MAX_BLOCK * 8 * 4)!;
    defer mem.free(mbuf);
    let cap_frames = s.frames;
    let mut frames: usize = 0;
    let mut p = p0;
    let fmt = s.format;
    let shift = audio.bits(fmt) - info.bps;
    let bpsample = audio.bytes_per_sample(fmt);
    while p < data.len {
        let fr = frame_header(data, p, info) catch {
            let q = find_sync(data, p + 1, info);
            if q >= data.len {
                break;  // trailing data (an ID3v1 tag, say)
            }
            return error.InvalidAudio;
        };
        if (fr.channels != nch) or (fr.bps != info.bps) {
            return error.Unsupported;  // a change of format mid-stream
        }
        let block = fr.block;
        let mut b = Bits.{ d: data, pos: p + header_len(data, p), cache: 0, n: 0, over: false };
        for c in @as(usize, 0)..nch {
            let mut bps = fr.bps;
            if ((fr.assign == 8) and (c == 1)) or ((fr.assign == 9) and (c == 0)) or ((fr.assign == 10) and (c == 1)) {
                bps += 1;  // the side channel
            }
            subframe(&mut b, block, bps, chans[c * MAX_BLOCK..(c * MAX_BLOCK) + block])!;
        }
        b.align();
        let end = b.byte_pos();
        if (end + 2) > data.len {
            return error.InvalidAudio;
        }
        let crc = (@as(u16, data[end]) << 8) | @as(u16, data[end + 1]);
        if crc16(data[p..end]) != crc {
            return error.InvalidAudio;
        }
        p = end + 2;
        // undo the stereo decorrelation
        let l = chans[..MAX_BLOCK];
        let r = chans[MAX_BLOCK..2 * MAX_BLOCK];
        if fr.assign == 8 {
            for i in @as(usize, 0)..block {
                r[i] = l[i] - r[i];
            }
        } else if fr.assign == 9 {
            for i in @as(usize, 0)..block {
                l[i] = l[i] + r[i];
            }
        } else if fr.assign == 10 {
            for i in @as(usize, 0)..block {
                let side = r[i];
                let mid = (l[i] << 1) | (side & 1);
                l[i] = (mid + side) >> 1;
                r[i] = (mid - side) >> 1;
            }
        }
        // store
        if (frames + block) > s.frames {
            let want = @max(s.frames * 2, frames + block);
            if (want * nch) > audio.MAX_SAMPLES {
                return error.TooLarge;
            }
            s.data = mem.realloc(s.data, want * nch * bpsample)!;
            s.frames = want;
        }
        if check_md5 {
            md5_block(&mut sig, chans, MAX_BLOCK, block, nch, width, mbuf);
        }
        let sh = @as(u64, shift);
        let base = frames * nch;
        if fmt == audio.U8 {
            let out = s.data;
            for c in @as(usize, 0)..nch {
                let src = chans[c * MAX_BLOCK..(c * MAX_BLOCK) + block];
                for i in @as(usize, 0)..block {
                    out[base + (i * nch) + c] = @truncate(u8, @bit_cast(u64, (src[i] << sh) + 128));
                }
            }
        } else if fmt == audio.S16 {
            let out = @cast_slice(i16, s.data[..s.frames * nch * 2]);
            for c in @as(usize, 0)..nch {
                let src = chans[c * MAX_BLOCK..(c * MAX_BLOCK) + block];
                for i in @as(usize, 0)..block {
                    out[base + (i * nch) + c] = @truncate(i16, src[i] << sh);
                }
            }
        } else {
            let out = @cast_slice(i32, s.data[..s.frames * nch * 4]);
            for c in @as(usize, 0)..nch {
                let src = chans[c * MAX_BLOCK..(c * MAX_BLOCK) + block];
                for i in @as(usize, 0)..block {
                    out[base + (i * nch) + c] = @truncate(i32, src[i] << sh);
                }
            }
        }
        frames += block;
    }
    if (info.total > 0) and (@as(u64, frames) > info.total) {
        frames = @truncate(usize, info.total);  // the last block may be padded
    }
    if frames != s.frames {
        s.data = mem.realloc(s.data, @max(frames * nch * bpsample, 1))!;
        s.frames = frames;
    }
    _ = cap_frames;
    if check_md5 {
        let mut got: [16]u8 = [0; 16];
        sig.final(got[..]);
        if not @mem_eq(got[..], info.md5[..]) {
            return error.InvalidAudio;
        }
    }
}

// -----------------------------------------------------------------------------
// Encoding
// -----------------------------------------------------------------------------

/// A most-significant-bit-first writer into growing memory.
struct Out {
    buf: []mut u8,
    len: usize,  // whole bytes written
    acc: u64,    // pending bits, at the bottom
    nacc: u32,

    fn init() !Out {
        return Out.{ buf: mem.alloc.[u8](1 << 16)!, len: 0, acc: 0, nacc: 0 };
    }

    fn reserve(self: *mut Out, n: usize) !void {
        if (self.len + n + 16) > self.buf.len {
            let want = @max(self.buf.len * 2, self.len + n + 16);
            self.buf = mem.realloc(self.buf, want)!;
        }
    }

    /// Writes the low k bits of v (k <= 32).
    fn bits(self: *mut Out, v: u64, k: u32) void {
        if k == 0 {
            return;
        }
        self.acc = (self.acc << @as(u64, k)) | (v & ((@as(u64, 1) << @as(u64, k)) - 1));
        self.nacc += k;
        while self.nacc >= 8 {
            self.nacc -= 8;
            self.buf[self.len] = @truncate(u8, self.acc >> @as(u64, self.nacc));
            self.len += 1;
        }
    }

    fn signed(self: *mut Out, v: i64, k: u32) void {
        if k > 32 {
            self.bits(@bit_cast(u64, v) >> 32, k - 32);
            self.bits(@bit_cast(u64, v), 32);
            return;
        }
        self.bits(@bit_cast(u64, v), k);
    }

    fn zeros(self: *mut Out, n: u64) void {
        let mut left = n;
        while left > 0 {
            let k = @truncate(u32, @min(left, 32));
            self.bits(0, k);
            left -= @as(u64, k);
        }
    }

    fn align(self: *mut Out) void {
        if self.nacc > 0 {
            self.bits(0, 8 - self.nacc);
        }
    }

    fn bytes(self: *mut Out, s: []u8) void {
        @mem_copy(self.buf[self.len..self.len + s.len], s);
        self.len += s.len;
    }
}

/// How a channel of a block is coded.
struct Plan {
    kind: u8,  // 0 constant, 1 verbatim, 2 fixed, 3 LPC
    order: usize,
    wasted: u32,
    bps: u32,  // after removing wasted bits
    prec: u32,
    shift: u32,
    coef: [32]i64,
    porder: u32,
    params: [256]u8,  // Rice parameter per partition
    wide: bool,       // 5-bit parameters
    cost: u64,        // bits
}

fn zigzag(r: i64) u64 {
    return @bit_cast(u64, (r << 1) ^ (r >> 63));
}

struct Settings {
    block: usize,
    max_lpc: usize,
    max_porder: u32,
    stereo: bool,
    lpc: bool,
    exhaustive: bool,  // try every LPC order
}

fn settings(level: u32) Settings {
    if level <= 2 {
        return Settings.{ block: 1152, max_lpc: 0, max_porder: 3, stereo: level > 0, lpc: false, exhaustive: false };
    }
    if level <= 5 {
        return Settings.{ block: 4096, max_lpc: 8, max_porder: 5, stereo: true, lpc: true, exhaustive: false };
    }
    return Settings.{ block: 4096, max_lpc: 12, max_porder: 6, stereo: true, lpc: true, exhaustive: level >= 8 };
}

/// Chooses Rice partitioning and parameters for residuals r[order..n];
/// fills the plan's porder, params, wide, and returns the bits the
/// residual takes (or none if it cannot be coded).
fn rice_plan(r: []i64, order: usize, max_porder: u32, sums: []mut u64, plan: *mut Plan) !u64 {
    let n = r.len;
    // the finest usable partition order
    let mut top: u32 = 0;
    while (top < max_porder) and ((n % (@as(usize, 2) << @as(usize, top))) == 0) and
          ((n >> @as(usize, top + 1)) > order) {
        top += 1;
    }
    let parts_top = @as(usize, 1) << @as(usize, top);
    let psize = n >> @as(usize, top);
    for part in @as(usize, 0)..parts_top {
        let mut sum: u64 = 0;
        let mut i = part * psize;
        if part == 0 {
            i = order;
        }
        let end = (part + 1) * psize;
        while i < end {
            let u = zigzag(r[i]);
            if u >= 0x100000000 {
                return error.Unsupported;  // a residual beyond 32 bits
            }
            sum += u;
            i += 1;
        }
        sums[part] = sum;
    }
    let mut best: u64 = @max_value(u64);
    let mut po = top;
    loop {
        let parts = @as(usize, 1) << @as(usize, po);
        let mut total: u64 = 0;
        let mut wide = false;
        let mut params: [256]u8 = [0; 256];
        for part in @as(usize, 0)..parts {
            let mut cnt = @as(u64, n >> @as(usize, po));
            if part == 0 {
                cnt -= @as(u64, order);
            }
            let sum = sums[part];
            // the best k by the estimate cnt * (k + 1) + sum >> k
            let mut k: u32 = 0;
            let mut kcost = cnt + sum;
            let mut t: u32 = 1;
            while t <= 30 {
                let c = (cnt * @as(u64, t + 1)) + (sum >> @as(u64, t));
                if c < kcost {
                    kcost = c;
                    k = t;
                } else if c > kcost {
                    break;
                }
                t += 1;
            }
            if k > 14 {
                wide = true;
            }
            params[part] = @truncate(u8, k);
            total += kcost;
        }
        let mut pbits: u64 = 4;
        if wide {
            pbits = 5;
        }
        total += (pbits * @as(u64, parts)) + 6;
        if total < best {
            best = total;
            plan.porder = po;
            plan.wide = wide;
            @mem_copy(plan.params[..parts], params[..parts]);
        }
        if po == 0 {
            break;
        }
        // merge pairs of partitions for the next coarser order
        for part in @as(usize, 0)..(parts / 2) {
            sums[part] = sums[2 * part] + sums[(2 * part) + 1];
        }
        po -= 1;
    }
    return best;
}

/// Fixed-polynomial residuals of the given order.
fn fixed_residual(x: []i64, order: usize, r: []mut i64) void {
    let n = x.len;
    if order == 0 {
        @mem_copy(r[..n], x);
    } else if order == 1 {
        for i in @as(usize, 1)..n {
            r[i] = x[i] - x[i - 1];
        }
    } else if order == 2 {
        for i in @as(usize, 2)..n {
            r[i] = (x[i] - (2 * x[i - 1])) + x[i - 2];
        }
    } else if order == 3 {
        for i in @as(usize, 3)..n {
            r[i] = ((x[i] - (3 * x[i - 1])) + (3 * x[i - 2])) - x[i - 3];
        }
    } else {
        for i in @as(usize, 4)..n {
            r[i] = (((x[i] - (4 * x[i - 1])) + (6 * x[i - 2])) - (4 * x[i - 3])) + x[i - 4];
        }
    }
}

/// The fixed order whose residuals look smallest (by their absolute sums,
/// all orders in one pass).
fn best_fixed_order(x: []i64) usize {
    let n = x.len;
    if n < 5 {
        return 0;
    }
    let mut sums: [5]u64 = [0; 5];
    let mut a = x[3];
    let mut b = a - x[2];
    let mut c = b - (x[2] - x[1]);
    let mut d = c - ((x[2] - x[1]) - (x[1] - x[0]));
    for i in @as(usize, 4)..n {
        let e0 = x[i];
        let e1 = e0 - a;
        let e2 = e1 - b;
        let e3 = e2 - c;
        let e4 = e3 - d;
        sums[0] += @bit_cast(u64, @abs(e0));
        sums[1] += @bit_cast(u64, @abs(e1));
        sums[2] += @bit_cast(u64, @abs(e2));
        sums[3] += @bit_cast(u64, @abs(e3));
        sums[4] += @bit_cast(u64, @abs(e4));
        a = e0;
        b = e1;
        c = e2;
        d = e3;
    }
    let mut best: usize = 0;
    for k in @as(usize, 1)..5 {
        if sums[k] < sums[best] {
            best = k;
        }
    }
    return best;
}

fn lpc_residual(x: []i64, order: usize, coef: []i64, shift: u32, r: []mut i64) void {
    // the coefficients reversed, to run forwards over the history
    let mut rc: [32]i64 = [0; 32];
    for j in @as(usize, 0)..order {
        rc[j] = coef[order - 1 - j];
    }
    let cs = rc[..order];
    let sh = @as(u64, shift);
    for i in order..x.len {
        let h = x[i - order..i];
        let mut sum: i64 = 0;
        for j in @as(usize, 0)..cs.len {
            sum += cs[j] * h[j];
        }
        r[i] = x[i] - (sum >> sh);
    }
}

/// Scratch memory for planning a channel.
struct Work {
    r: []mut i64,     // residuals being tried
    best: []mut i64,  // the chosen plan's residuals
    sums: []mut u64,
    win: []mut f64,
    xs: []mut f64,
}

/// The cheapest coding of one channel's block x (samples of bps bits).
fn plan_channel(x0: []i64, bps0: u32, st: *Settings, w: *mut Work, x: []mut i64) Plan {
    let n = x0.len;
    let mut plan = Plan.{
        kind: 1,
        order: 0,
        wasted: 0,
        bps: bps0,
        prec: 0,
        shift: 0,
        coef: [0; 32],
        porder: 0,
        params: [0; 256],
        wide: false,
        cost: 0,
    };
    // constant?
    let mut all_same = true;
    let mut or_bits: u64 = 0;
    for i in @as(usize, 0)..n {
        if x0[i] != x0[0] {
            all_same = false;
        }
        or_bits |= @bit_cast(u64, x0[i]);
    }
    if all_same {
        plan.kind = 0;
        plan.cost = @as(u64, bps0) + 8;
        return plan;
    }
    // wasted low bits
    let mut wasted: u32 = 0;
    while ((or_bits >> @as(u64, wasted)) & 1) == 0 {
        wasted += 1;
    }
    let bps = bps0 - wasted;
    for i in @as(usize, 0)..n {
        x[i] = x0[i] >> @as(u64, wasted);
    }
    plan.wasted = wasted;
    plan.bps = bps;
    let header = @as(u64, 8 + wasted);
    // verbatim
    plan.cost = header + (@as(u64, n) * @as(u64, bps));
    let xs = x[..n];
    // fixed orders
    let mut trial = plan;
    let mut lo_fixed: usize = 0;
    let mut hi_fixed: usize = 4;
    if not st.exhaustive {
        lo_fixed = best_fixed_order(xs);
        hi_fixed = lo_fixed;
    }
    for order in lo_fixed..hi_fixed + 1 {
        if order >= n {
            break;
        }
        fixed_residual(xs, order, w.r);
        let rc = rice_plan(w.r[..n], order, st.max_porder, w.sums, &mut trial) catch continue;
        let cost = (header + (@as(u64, order) * @as(u64, bps))) + rc;
        if cost < plan.cost {
            trial.kind = 2;
            trial.order = order;
            trial.cost = cost;
            plan = trial;
            @mem_copy(w.best[..n], w.r[..n]);
        }
    }
    if not st.lpc or (n <= st.max_lpc) or (n < 32) {
        return plan;
    }
    // LPC: Levinson-Durbin on the autocorrelation of the windowed block
    let maxo = st.max_lpc;
    for i in @as(usize, 0)..n {
        w.xs[i] = @float_from_int(f64, xs[i]) * w.win[i];
    }
    let mut ac: [33]f64 = [0.0; 33];
    for lag in @as(usize, 0)..maxo + 1 {
        let mut s: f64 = 0.0;
        for i in lag..n {
            s += w.xs[i] * w.xs[i - lag];
        }
        ac[lag] = s;
    }
    if ac[0] == 0.0 {
        return plan;
    }
    ac[0] *= 1.0 + 1e-9;  // a touch of white noise for stability
    let mut a: [33]f64 = [0.0; 33];
    let mut err = ac[0];
    let mut lpcs: [33][33]f64 = [[0.0; 33]; 33];
    let mut errs: [33]f64 = [0.0; 33];
    for m in @as(usize, 1)..maxo + 1 {
        let mut acc = ac[m];
        for j in @as(usize, 1)..m {
            acc -= a[j] * ac[m - j];
        }
        let k = acc / err;
        let mut na = a;
        na[m] = k;
        for j in @as(usize, 1)..m {
            na[j] = a[j] - (k * a[m - j]);
        }
        a = na;
        err *= 1.0 - (k * k);
        lpcs[m] = a;
        errs[m] = err;
        if err <= 0.0 {
            break;
        }
    }
    let mut prec: u32 = 15;
    if bps <= 16 {
        prec = 13;
        if n <= 1152 {
            prec = 11;
        }
    }
    // which orders to try: all of them at the top levels, else the one the
    // prediction errors suggest
    let mut lo_order: usize = 1;
    let mut hi_order = maxo;
    if not st.exhaustive {
        let mut best_est: f64 = 1e300;
        for m in @as(usize, 1)..maxo + 1 {
            if (errs[m] <= 0.0) or (lpcs[m][m] == 0.0) {
                break;
            }
            let bits_per = @max(0.5 * math.log2((errs[m] * 0.5) / @float_from_int(f64, n)), 0.0);
            let est = (bits_per * @float_from_int(f64, n - m)) + @float_from_int(f64, m * @as(usize, prec + bps));
            if est < best_est {
                best_est = est;
                lo_order = m;
            }
        }
        hi_order = lo_order;
    }
    for m in lo_order..hi_order + 1 {
        // quantize the coefficients
        let mut cmax: f64 = 0.0;
        for j in @as(usize, 1)..m + 1 {
            cmax = @max(cmax, @abs(lpcs[m][j]));
        }
        if (cmax == 0.0) or (cmax != cmax) {
            continue;
        }
        let e = math.frexp(cmax).exponent;
        let mut sh = @as(i32, @bit_cast(i32, prec)) - 1 - e;
        if sh > 15 {
            sh = 15;
        }
        if sh < 0 {
            continue;
        }
        let limit = @as(i64, 1) << @as(u64, prec - 1);
        let mut q: [32]i64 = [0; 32];
        let mut carry: f64 = 0.0;
        let scale = @float_from_int(f64, @as(i64, 1) << @as(u64, @bit_cast(u32, sh)));
        for j in @as(usize, 0)..m {
            carry += lpcs[m][j + 1] * scale;
            let v = @max(@min(@int_from_float(i64, @nearest(carry)), limit - 1), -limit);
            q[j] = v;
            carry -= @float_from_int(f64, v);
        }
        let shift = @bit_cast(u32, sh);
        lpc_residual(xs, m, q[..], shift, w.r);
        let rc = rice_plan(w.r[..n], m, st.max_porder, w.sums, &mut trial) catch continue;
        let cost = (((header + (@as(u64, m) * @as(u64, bps))) + 4) + 5) + (@as(u64, m) * @as(u64, prec)) + rc;
        if cost < plan.cost {
            trial.kind = 3;
            trial.order = m;
            trial.prec = prec;
            trial.shift = shift;
            trial.coef = q;
            trial.cost = cost;
            plan = trial;
            @mem_copy(w.best[..n], w.r[..n]);
        }
    }
    return plan;
}

fn write_subframe(o: *mut Out, plan: *Plan, x: []i64, res: []i64) void {
    let n = x.len;
    let mut t: u64 = 1;
    if plan.kind == 0 {
        t = 0;
    } else if plan.kind == 2 {
        t = 8 + @as(u64, plan.order);
    } else if plan.kind == 3 {
        t = 32 + @as(u64, plan.order - 1);
    }
    o.bits(t, 7);  // a zero bit, then the type
    if plan.wasted > 0 {
        o.bits(1, 1);
        o.zeros(@as(u64, plan.wasted - 1));
        o.bits(1, 1);
    } else {
        o.bits(0, 1);
    }
    let bps = plan.bps;
    let wsh = @as(u64, plan.wasted);
    if plan.kind == 0 {
        o.signed(x[0] >> wsh, bps);
        return;
    }
    if plan.kind == 1 {
        for i in @as(usize, 0)..n {
            o.signed(x[i] >> wsh, bps);
        }
        return;
    }
    for i in @as(usize, 0)..plan.order {
        o.signed(x[i] >> wsh, bps);
    }
    if plan.kind == 3 {
        o.bits(@as(u64, plan.prec - 1), 4);
        o.bits(@as(u64, plan.shift), 5);
        for j in @as(usize, 0)..plan.order {
            o.signed(plan.coef[j], plan.prec);
        }
    }
    let mut pbits: u32 = 4;
    if plan.wide {
        o.bits(1, 2);
        pbits = 5;
    } else {
        o.bits(0, 2);
    }
    o.bits(@as(u64, plan.porder), 4);
    let parts = @as(usize, 1) << @as(usize, plan.porder);
    let psize = n >> @as(usize, plan.porder);
    for part in @as(usize, 0)..parts {
        let k = @as(u32, plan.params[part]);
        o.bits(@as(u64, k), pbits);
        let mut i = part * psize;
        if part == 0 {
            i = plan.order;
        }
        let end = (part + 1) * psize;
        let kk = @as(u64, k);
        while i < end {
            let u = zigzag(res[i]);
            o.zeros(u >> kk);
            o.bits(1, 1);
            o.bits(u, k);
            i += 1;
        }
    }
}

fn utf8_number(o: *mut Out, v: u64) void {
    if v < 0x80 {
        o.bits(v, 8);
        return;
    }
    let mut n: u32 = 2;
    while (n < 7) and (v >= (@as(u64, 1) << @as(u64, (5 * n) + 1))) {
        n += 1;
    }
    let lead_bits = 7 - n;
    let lead = ((@as(u64, 0xff) << @as(u64, 8 - n)) & 0xff) | (v >> @as(u64, 6 * (n - 1)));
    o.bits(lead, 8);
    _ = lead_bits;
    let mut k = n - 1;
    while k > 0 {
        k -= 1;
        o.bits(0x80 | ((v >> @as(u64, 6 * k)) & 0x3f), 8);
    }
}

pub fn encode(s: *audio.Sound, level: u32) ![]mut u8 {
    if audio.is_float(s.format) {
        let mut c = s.convert(audio.S24)!;
        defer c.deinit();
        return encode(&c, level);
    }
    if (s.channels > 8) or (s.rate >= 1048576) {
        return error.Unsupported;
    }
    let mut o = Out.init()!;
    encode_into(s, level, &mut o) catch |e| {
        mem.free(o.buf);
        return e;
    };
    return mem.realloc(o.buf, o.len) catch |e| {
        mem.free(o.buf);
        return e;
    };
}

fn encode_into(s: *audio.Sound, level: u32, o: *mut Out) !void {
    let st = settings(level);
    let nch = s.channels;
    let bps = audio.bits(s.format);
    let total = s.frames;
    let mut block = st.block;
    if (total < block) and (total >= 16) {
        block = total;
    }
    // the header, with STREAMINFO to fill in at the end
    o.bytes("fLaC");
    o.bits(0x80, 8);  // last metadata block, STREAMINFO
    o.bits(34, 24);
    let info_at = o.len;
    o.zeros(34 * 8);
    // scratch
    let nbuf = (block * @max(nch, 4)) + 1;  // the channels, and mid and side
    let chans = mem.alloc.[i64](nbuf)!;
    defer mem.free(chans);
    let shifted = mem.alloc.[i64](nbuf)!;
    defer mem.free(shifted);
    let res = mem.alloc.[i64](nbuf)!;
    defer mem.free(res);
    let r = mem.alloc.[i64](block + 1)!;
    defer mem.free(r);
    let sums = mem.alloc.[u64](256)!;
    defer mem.free(sums);
    let win = mem.alloc.[f64](block + 1)!;
    defer mem.free(win);
    let xs = mem.alloc.[f64](block + 1)!;
    defer mem.free(xs);
    let mut w = Work.{ r: r, best: res[..block], sums: sums, win: win, xs: xs };
    let mut sig = md5.Md5.init();
    let width = @as(usize, (bps + 7) / 8);
    let mbuf = mem.alloc.[u8]((block * nch * width) + 1)!;
    defer mem.free(mbuf);
    let mut min_frame: usize = @max_value(usize);
    let mut max_frame: usize = 0;
    let mut win_n: usize = 0;
    let mut start: usize = 0;
    let mut index: u64 = 0;
    let mut plans: [8]Plan = [Plan.{
        kind: 0,
        order: 0,
        wasted: 0,
        bps: 0,
        prec: 0,
        shift: 0,
        coef: [0; 32],
        porder: 0,
        params: [0; 256],
        wide: false,
        cost: 0,
    }; 8];
    while start < total {
        let n = @min(block, total - start);
        if n != win_n {
            // a Tukey(0.5) window
            for i in @as(usize, 0)..n {
                let t = @float_from_int(f64, i) / @float_from_int(f64, @max(n, 2) - 1);
                let mut v: f64 = 1.0;
                if t < 0.25 {
                    v = 0.5 * (1.0 - math.cos(4.0 * math.PI * t));
                } else if t > 0.75 {
                    v = 0.5 * (1.0 - math.cos(4.0 * math.PI * (1.0 - t)));
                }
                win[i] = v;
            }
            win_n = n;
        }
        // gather the channels (and the signature)
        let base = start * nch;
        for c in @as(usize, 0)..nch {
            let dst = chans[c * block..(c * block) + n];
            if s.format == audio.U8 {
                for i in @as(usize, 0)..n {
                    dst[i] = @as(i64, s.data[base + (i * nch) + c]) - 128;
                }
            } else if s.format == audio.S16 {
                let src = s.samples_i16();
                for i in @as(usize, 0)..n {
                    dst[i] = @as(i64, src[base + (i * nch) + c]);
                }
            } else {
                let src = s.samples_i32();
                for i in @as(usize, 0)..n {
                    dst[i] = @as(i64, src[base + (i * nch) + c]);
                }
            }
        }
        md5_block(&mut sig, chans, block, n, nch, width, mbuf);
        let frame_start = o.len;
        o.reserve(((n * nch) * 8) + 64)!;
        // plan each channel; for stereo, mid and side too
        let stereo = (nch == 2) and st.stereo;
        let mut assign: u32 = @truncate(u32, nch - 1);
        let mut coded: usize = nch;
        if stereo {
            for i in @as(usize, 0)..n {
                let l = chans[i];
                let rr = chans[block + i];
                chans[(2 * block) + i] = (l + rr) >> 1;
                chans[(3 * block) + i] = l - rr;
            }
            coded = 4;
        }
        for c in @as(usize, 0)..coded {
            let mut cb = bps;
            if stereo and (c == 3) {
                cb += 1;
            }
            w.best = res[c * block..(c * block) + n];
            plans[c] = plan_channel(chans[c * block..(c * block) + n], cb, &st, &mut w,
                                    shifted[c * block..(c * block) + n]);
        }
        let mut pick: [2]usize = [0, 1];
        if stereo {
            let lr = plans[0].cost + plans[1].cost;
            let ls = plans[0].cost + plans[3].cost;
            let rs = plans[1].cost + plans[3].cost;
            let ms = plans[2].cost + plans[3].cost;
            assign = 1;
            if (ls < lr) and (ls <= rs) and (ls <= ms) {
                assign = 8;
                pick = [0, 3];
            } else if (rs < lr) and (rs <= ms) {
                assign = 9;
                pick = [3, 1];
            } else if ms < lr {
                assign = 10;
                pick = [2, 3];
            }
        }
        // the frame header
        let hstart = o.len;
        o.bits(0xfff8, 16);
        let mut bs_code: u64 = 7;
        if n == 4096 {
            bs_code = 12;
        } else if n == 1152 {
            bs_code = 3;
        } else if n <= 256 {
            bs_code = 6;
        }
        let mut sr_code: u64 = 0;
        for k in @as(usize, 1)..12 {
            if RATES[k] == s.rate {
                sr_code = @as(u64, k);
            }
        }
        if sr_code == 0 {
            if ((s.rate % 1000) == 0) and (s.rate <= 255000) {
                sr_code = 12;
            } else if s.rate <= 65535 {
                sr_code = 13;
            } else if ((s.rate % 10) == 0) and (s.rate <= 655350) {
                sr_code = 14;
            }
        }
        o.bits((bs_code << 4) | sr_code, 8);
        let mut ss_code: u64 = 0;
        for k in @as(usize, 1)..8 {
            if SIZES[k] == bps {
                ss_code = @as(u64, k);
            }
        }
        o.bits((@as(u64, assign) << 4) | (ss_code << 1), 8);
        utf8_number(o, index);
        if bs_code == 6 {
            o.bits(@as(u64, n - 1), 8);
        } else if bs_code == 7 {
            o.bits(@as(u64, n - 1), 16);
        }
        if sr_code == 12 {
            o.bits(@as(u64, s.rate / 1000), 8);
        } else if sr_code == 13 {
            o.bits(@as(u64, s.rate), 16);
        } else if sr_code == 14 {
            o.bits(@as(u64, s.rate / 10), 16);
        }
        o.bits(@as(u64, crc8(o.buf[hstart..o.len])), 8);
        // the subframes
        for c in @as(usize, 0)..nch {
            let mut src = c;
            if nch == 2 {
                src = pick[c];
            }
            write_subframe(o, &plans[src], chans[src * block..(src * block) + n], res[src * block..(src * block) + n]);
        }
        o.align();
        let crc = crc16(o.buf[frame_start..o.len]);
        o.bits(@as(u64, crc), 16);
        let flen = o.len - frame_start;
        min_frame = @min(min_frame, flen);
        max_frame = @max(max_frame, flen);
        start += n;
        index += 1;
    }
    // STREAMINFO
    let end = o.len;
    let saved_len = o.len;
    o.len = info_at;
    let mut bs16 = @max(block, 16);
    if total == 0 {
        bs16 = 16;
        min_frame = 0;
    }
    o.bits(@as(u64, bs16), 16);
    o.bits(@as(u64, bs16), 16);
    o.bits(@as(u64, min_frame), 24);
    o.bits(@as(u64, max_frame), 24);
    o.bits(@as(u64, s.rate), 20);
    o.bits(@as(u64, nch - 1), 3);
    o.bits(@as(u64, bps - 1), 5);
    o.bits(@as(u64, total) >> 32, 4);
    o.bits(@as(u64, total) & 0xffffffff, 32);
    let mut digest: [16]u8 = [0; 16];
    sig.final(digest[..]);
    o.bytes(digest[..]);
    o.len = saved_len;
    _ = end;
}

// -----------------------------------------------------------------------------
// Tests
// -----------------------------------------------------------------------------

fn test_sound(frames: usize, channels: usize, format: u8, seed: u64) !audio.Sound {
    let mut s = audio.Sound.init(frames, channels, 44100, format)!;
    let mut x = seed;
    for i in @as(usize, 0)..frames {
        for c in @as(usize, 0)..channels {
            x = (x *% 6364136223846793005) +% 1442695040888963407;
            let noise = @float_from_int(f64, @truncate(u32, x >> 40)) / 16777216.0 - 0.5;
            let t = @float_from_int(f64, i) / 44100.0;
            let v = (0.4 * math.sin(2.0 * math.PI * (440.0 + (110.0 * @float_from_int(f64, c))) * t)) + (0.01 * noise);
            s.set(i, c, v);
        }
    }
    return s;
}

test "round trips" {
    let fmts: [4]u8 = [audio.U8, audio.S16, audio.S24, audio.S32];
    let chans: [3]usize = [1, 2, 5];
    let levels: [3]u32 = [FAST, DEFAULT, BEST];
    for f in fmts {
        for ch in chans {
            for lv in levels {
                let mut s = test_sound(5000, ch, f, 7)!;
                defer s.deinit();
                let enc = encode(&s, lv)!;
                defer mem.free(enc);
                @expect(is_flac(enc));
                let mut back = decode(enc)!;
                defer back.deinit();
                @expect_eq(back.format, f);
                @expect_eq(back.channels, ch);
                @expect_eq(back.frames, s.frames);
                @expect_eq(back.data[..], s.data[..]);
            }
        }
    }
}

test "compresses" {
    let mut s = audio.Sound.init(44100, 2, 44100, audio.S16)!;
    defer s.deinit();
    for i in @as(usize, 0)..44100 {
        let t = @float_from_int(f64, i) / 44100.0;
        s.set(i, 0, 0.5 * math.sin(2.0 * math.PI * 440.0 * t));
        s.set(i, 1, 0.5 * math.sin(2.0 * math.PI * 440.0 * t) + (0.1 * math.sin(2.0 * math.PI * 1000.0 * t)));
    }
    let enc = encode(&s, DEFAULT)!;
    defer mem.free(enc);
    @expect(enc.len < (s.data.len / 4));
    // silence and short sounds
    let mut z = audio.Sound.init(10000, 1, 8000, audio.S16)!;
    defer z.deinit();
    let ez = encode(&z, DEFAULT)!;
    defer mem.free(ez);
    @expect(ez.len < 200);
    let mut e = audio.Sound.init(0, 2, 8000, audio.S16)!;
    defer e.deinit();
    let ee = encode(&e, DEFAULT)!;
    defer mem.free(ee);
    let mut eb = decode(ee)!;
    defer eb.deinit();
    @expect_eq(eb.frames, 0);
    let mut one = audio.Sound.init(1, 1, 8000, audio.S16)!;
    defer one.deinit();
    one.samples_i16()[0] = -5;
    let eo = encode(&one, DEFAULT)!;
    defer mem.free(eo);
    let mut ob = decode(eo)!;
    defer ob.deinit();
    @expect_eq(ob.samples_i16()[0], -5);
}

test "rejects corruption" {
    let mut s = test_sound(3000, 1, audio.S16, 5)!;
    defer s.deinit();
    let enc = encode(&s, DEFAULT)!;
    defer mem.free(enc);
    let bad = mem.dup(enc[..])!;
    defer mem.free(bad);
    bad[bad.len - 100] ^= 0x10;
    _ = decode(bad) catch |e| {
        @expect_eq(e, error.InvalidAudio);
        return;
    };
    @trap();
}
