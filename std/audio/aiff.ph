// =============================================================================
// std/audio/aiff.ph - AIFF and AIFF-C files.
//
//     let mut s = aiff.decode(bytes)!;
//     defer s.deinit();
//     let out = aiff.encode(&s)!;   // mem.free it
//
// Decoding reads AIFF (big-endian integers of 1 to 32 bits) and AIFF-C
// with the NONE, twos, sowt (little-endian), in24, in32, 42ni, 23ni, raw
// (unsigned 8-bit), fl32, fl64, ulaw and alaw encodings. Encoding writes
// integer sounds as AIFF (U8 as signed 8-bit samples) and float sounds as
// AIFF-C (fl32 or fl64).
// =============================================================================

import "../audio.ph" as audio;
import "../mem.ph" as mem;
import "wav.ph" as wav;

error { InvalidAudio, Unsupported, OutOfMemory, TooLarge }

pub fn is_aiff(data: []u8) bool {
    return (data.len >= 12) and @mem_eq(data[..4], "FORM") and
           (@mem_eq(data[8..12], "AIFF") or @mem_eq(data[8..12], "AIFC"));
}

fn be16(b: []u8, i: usize) u32 {
    return (@as(u32, b[i]) << 8) | @as(u32, b[i + 1]);
}

fn be32(b: []u8, i: usize) u32 {
    return (be16(b, i) << 16) | be16(b, i + 2);
}

/// An 80-bit IEEE extended float (the sample rate) as a whole number, or
/// 0 if it is not a positive one below 2^32.
fn ext80_rate(b: []u8) u32 {
    let e = @as(i32, @bit_cast(i32, be16(b, 0) & 0x7fff)) - 16383;
    let mant = (@as(u64, be32(b, 2)) << 32) | @as(u64, be32(b, 6));
    if ((be16(b, 0) & 0x8000) != 0) or (e < 0) or (e > 31) or (mant == 0) {
        return 0;
    }
    // round the value mant * 2^(e - 63) to an integer
    let sh = @truncate(u32, @bit_cast(u32, 63 - e));
    let v = (mant >> sh) + ((mant >> (sh - 1)) & 1);
    return @truncate(u32, @min(v, 0xffffffff));
}

fn ext80(rate: u32, out: []mut u8) void {
    let lz = @clz(rate);
    let e = 16383 + 31 - lz;
    let mant = @as(u64, rate) << @as(u64, 32 + lz);
    out[0] = @truncate(u8, e >> 8);
    out[1] = @truncate(u8, e);
    for i in @as(usize, 0)..8 {
        out[2 + i] = @truncate(u8, mant >> @truncate(u64, 56 - (8 * i)));
    }
}

pub fn decode(data: []u8) !audio.Sound {
    if not is_aiff(data) {
        return error.InvalidAudio;
    }
    let aifc = @mem_eq(data[8..12], "AIFC");
    let mut have_comm = false;
    let mut channels: usize = 0;
    let mut frames: usize = 0;
    let mut bits: u32 = 0;
    let mut rate: u32 = 0;
    let mut comp: [4]u8 = ['N', 'O', 'N', 'E'];
    let mut ssnd: []u8 = data[..0];
    let mut have_ssnd = false;
    let mut p: usize = 12;
    while (p + 8) <= data.len {
        let id = data[p..p + 4];
        let size = @as(usize, be32(data, p + 4));
        let body = p + 8;
        let avail = data.len - body;
        if @mem_eq(id, "COMM") {
            if (size < 18) or (avail < 18) {
                return error.InvalidAudio;
            }
            channels = @as(usize, be16(data, body));
            frames = @as(usize, be32(data, body + 2));
            bits = be16(data, body + 6);
            rate = ext80_rate(data[body + 8..body + 18]);
            if aifc and (size >= 22) and (avail >= 22) {
                @mem_copy(comp[..], data[body + 18..body + 22]);
            }
            have_comm = true;
        } else if @mem_eq(id, "SSND") and (size >= 8) and (avail >= 8) {
            let off = @as(usize, be32(data, body));
            let len = @min(size, avail);
            if (8 + off) <= len {
                ssnd = data[body + 8 + off..body + len];
                have_ssnd = true;
            }
        }
        if size >= avail {
            break;
        }
        p = body + size + (size & 1);
    }
    if not have_comm or (channels == 0) or (channels > 255) or (rate == 0) or (bits == 0) or (bits > 64) {
        return error.InvalidAudio;
    }
    if not have_ssnd {
        frames = 0;
    }
    return decode_samples(ssnd, comp, channels, frames, bits, rate);
}

fn decode_samples(d: []u8, comp: [4]u8, channels: usize, frames_hint: usize, bits: u32, rate: u32) !audio.Sound {
    let c = comp[..];
    let mut width = (@as(usize, bits) + 7) / 8;
    let mut little = false;
    let mut unsigned = false;
    let mut float = false;
    let mut law: u8 = 0;
    if @mem_eq(c, "NONE") or @mem_eq(c, "twos") {
        // big-endian integers
    } else if @mem_eq(c, "sowt") {
        little = true;
    } else if @mem_eq(c, "in24") {
        width = 3;
    } else if @mem_eq(c, "in32") {
        width = 4;
    } else if @mem_eq(c, "42ni") {
        width = 3;
        little = true;
    } else if @mem_eq(c, "23ni") {
        width = 4;
        little = true;
    } else if @mem_eq(c, "raw ") {
        width = 1;
        unsigned = true;
    } else if @mem_eq(c, "fl32") or @mem_eq(c, "FL32") {
        width = 4;
        float = true;
    } else if @mem_eq(c, "fl64") or @mem_eq(c, "FL64") {
        width = 8;
        float = true;
    } else if @mem_eq(c, "ulaw") or @mem_eq(c, "ULAW") {
        width = 1;
        law = 'u';
    } else if @mem_eq(c, "alaw") or @mem_eq(c, "ALAW") {
        width = 1;
        law = 'a';
    } else {
        return error.Unsupported;
    }
    if width > 8 or (not float and (width > 4)) {
        return error.Unsupported;
    }
    let mut format = audio.SampleFormat.S32;
    if law != 0 {
        format = audio.SampleFormat.S16;
    } else if float {
        format = audio.SampleFormat.F32;
        if width == 8 {
            format = audio.SampleFormat.F64;
        }
    } else if width == 1 {
        format = audio.SampleFormat.U8;
    } else if width == 2 {
        format = audio.SampleFormat.S16;
    } else if width == 3 {
        format = audio.SampleFormat.S24;
    }
    let frames = @min(frames_hint, d.len / (width * channels));
    let mut s = audio.Sound.init(frames, channels, rate, format)!;
    let n = frames * channels;
    for i in @as(usize, 0)..n {
        let at = i * width;
        if law != 0 {
            if law == 'u' {
                s.samples_i16()[i] = wav.mulaw_decode(d[at]);
            } else {
                s.samples_i16()[i] = wav.alaw_decode(d[at]);
            }
            continue;
        }
        // gather the sample's bytes as a big-endian number
        let mut v: u64 = 0;
        for k in @as(usize, 0)..width {
            let mut b = d[at + k];
            if little {
                b = d[at + width - 1 - k];
            }
            v = (v << 8) | @as(u64, b);
        }
        if format == audio.SampleFormat.U8 {
            let mut x = @truncate(u8, v);
            if not unsigned {
                x ^= 0x80;  // AIFF's 8-bit samples are signed
            }
            s.data[i] = x;
        } else if format == audio.SampleFormat.S16 {
            s.samples_i16()[i] = @bit_cast(i16, @truncate(u16, v));
        } else if format == audio.SampleFormat.S24 {
            s.samples_i32()[i] = @bit_cast(i32, @truncate(u32, v << 8)) >> 8;
        } else if format == audio.SampleFormat.S32 {
            s.samples_i32()[i] = @bit_cast(i32, @truncate(u32, v));
        } else if format == audio.SampleFormat.F32 {
            s.samples_f32()[i] = @bit_cast(f32, @truncate(u32, v));
        } else {
            s.samples_f64()[i] = @bit_cast(f64, v);
        }
    }
    return s;
}

fn put32(b: []mut u8, at: usize, v: u32) void {
    b[at] = @truncate(u8, v >> 24);
    b[at + 1] = @truncate(u8, v >> 16);
    b[at + 2] = @truncate(u8, v >> 8);
    b[at + 3] = @truncate(u8, v);
}

pub fn encode(s: *audio.Sound) ![]mut u8 {
    let width = @as(usize, audio.bits(s.format)) / 8;
    let n = s.len();
    let data_len = @as(u64, n) * @as(u64, width);
    let float = audio.is_float(s.format);
    let mut comm_len: usize = 18;
    let mut head: usize = 12;
    if float {
        comm_len = 24;  // + compression type and an empty name, padded
        head += 12;     // FVER
    }
    head += 8 + comm_len + 16;  // COMM, SSND header
    let pad = @truncate(usize, data_len & 1);
    let total64 = (@as(u64, head) + data_len) + @as(u64, pad);
    if (total64 > 0xffffffff) or (s.frames > 0xffffffff) {
        return error.TooLarge;
    }
    let total = @truncate(usize, total64);
    let b = mem.alloc_zeroed.[u8](total)!;
    let mut o: usize = 0;
    @mem_copy(b[0..4], "FORM");
    put32(b, 4, @truncate(u32, total - 8));
    if float {
        @mem_copy(b[8..12], "AIFC");
        @mem_copy(b[12..16], "FVER");
        put32(b, 16, 4);
        put32(b, 20, 0xa2805140);
        o = 24;
    } else {
        @mem_copy(b[8..12], "AIFF");
        o = 12;
    }
    @mem_copy(b[o..o + 4], "COMM");
    put32(b, o + 4, @truncate(u32, comm_len));
    b[o + 8] = @truncate(u8, s.channels >> 8);
    b[o + 9] = @truncate(u8, s.channels);
    put32(b, o + 10, @truncate(u32, s.frames));
    let bits = audio.bits(s.format);
    b[o + 14] = @truncate(u8, bits >> 8);
    b[o + 15] = @truncate(u8, bits);
    ext80(s.rate, b[o + 16..o + 26]);
    if float {
        if s.format == audio.SampleFormat.F32 {
            @mem_copy(b[o + 26..o + 30], "fl32");
        } else {
            @mem_copy(b[o + 26..o + 30], "fl64");
        }
        // an empty Pascal string, padded to even length: two zero bytes
    }
    o += 8 + comm_len;
    @mem_copy(b[o..o + 4], "SSND");
    put32(b, o + 4, @truncate(u32, data_len + 8));
    o += 16;  // offset and block size: 0
    for i in @as(usize, 0)..n {
        if s.format == audio.SampleFormat.U8 {
            b[o] = s.data[i] ^ 0x80;
            o += 1;
            continue;
        }
        let mut v: u64 = 0;
        if s.format == audio.SampleFormat.S16 {
            v = @as(u64, @bit_cast(u16, s.samples_i16()[i]));
        } else if (s.format == audio.SampleFormat.S24) or (s.format == audio.SampleFormat.S32) {
            v = @as(u64, @bit_cast(u32, s.samples_i32()[i]));
        } else if s.format == audio.SampleFormat.F32 {
            v = @as(u64, @bit_cast(u32, s.samples_f32()[i]));
        } else {
            v = @bit_cast(u64, s.samples_f64()[i]);
        }
        for k in @as(usize, 0)..width {
            b[o + k] = @truncate(u8, v >> @truncate(u64, 8 * (width - 1 - k)));
        }
        o += width;
    }
    return b;
}

test "round trips every format" {
    let fmts: [6]audio.SampleFormat = [audio.SampleFormat.U8, audio.SampleFormat.S16, audio.SampleFormat.S24,
                                       audio.SampleFormat.S32, audio.SampleFormat.F32, audio.SampleFormat.F64];
    let chans: [2]usize = [1, 3];
    for f in fmts {
        for ch in chans {
            let mut s = audio.Sound.init(51, ch, 44100, f)!;
            defer s.deinit();
            for i in @as(usize, 0)..s.len() {
                s.set_sample(i, @float_from_int(f64, (i * 7919) % 2001) / 1000.0 - 1.0);
            }
            let enc = encode(&s)!;
            defer mem.free(enc);
            @expect(is_aiff(enc));
            let mut back = decode(enc)!;
            defer back.deinit();
            @expect(back.format == f);
            @expect_eq(back.channels, ch);
            @expect_eq(back.rate, 44100);
            @expect_eq(back.frames, 51);
            @expect_eq(back.data[..], s.data[..]);
        }
    }
}

test "sample rates" {
    let rates: [6]u32 = [1, 8000, 11025, 44100, 96000, 4000000000];
    let mut buf: [10]u8 = [0; 10];
    for r in rates {
        ext80(r, buf[..]);
        @expect_eq(ext80_rate(buf[..]), r);
    }
    // 44100 as written by other programs
    let std: [10]u8 = [0x40, 0x0e, 0xac, 0x44, 0, 0, 0, 0, 0, 0];
    @expect_eq(ext80_rate(std[..]), 44100);
}
