// =============================================================================
// std/audio/opus.ph - Ogg Opus decoding (RFC 6716, RFC 7845).
//
//     let mut s = opus.decode(bytes)!;   // F32 samples at 48 kHz
//     defer s.deinit();
//
// Every mode of Opus: SILK (narrow, medium and wide band, 10-60 ms frames,
// stereo prediction, low bit-rate redundancy), CELT (2.5-20 ms frames,
// every band layout), hybrid, redundant frames and the transitions between
// modes; multistream (channel mapping families 0, 1 and 255, up to 255
// channels). The pre-skip and the last page's granule position trim the
// output, and the header's output gain is applied. Ported from libopus
// (opus/): the output follows libopus's floating point decoder.
// =============================================================================

import "../audio.ph" as audio;
import "../mem.ph" as mem;
import "../vec.ph" as vec;
import "ogg.ph" as ogg;
import "opus/decoder.ph" as dec;

error { InvalidAudio, Unsupported, OutOfMemory, TooLarge }

/// Whether data is an Ogg stream holding Opus.
pub fn is_opus(data: []u8) bool {
    if (data.len < 36) or not ogg.is_ogg(data) {
        return false;
    }
    let nseg = @as(usize, data[26]);
    let o = 27 + nseg;
    return (data.len >= (o + 8)) and @mem_eq(data[o..o + 8], "OpusHead");
}

struct Head {
    channels: usize,
    pre_skip: usize,
    gain: i32,
    streams: usize,
    coupled: usize,
    mapping: [255]u8,
}

fn read_head(p: []u8) !Head {
    if (p.len < 19) or not @mem_eq(p[..8], "OpusHead") or ((p[8] >> 4) != 0) {
        return error.InvalidAudio;
    }
    let mut h = Head.{ channels: @as(usize, p[9]), pre_skip: @as(usize, p[10]) | (@as(usize, p[11]) << 8),
                       gain: @as(i32, @bit_cast(i16, @as(u16, p[16]) | (@as(u16, p[17]) << 8))), streams: 1,
                       coupled: 0, mapping: [0; 255] };
    if h.channels == 0 {
        return error.InvalidAudio;
    }
    let family = p[18];
    if family == 0 {
        if h.channels > 2 {
            return error.InvalidAudio;
        }
        h.coupled = h.channels - 1;
        for i in @as(usize, 0)..h.channels {
            h.mapping[i] = @truncate(u8, i);
        }
    } else {
        if p.len < (21 + h.channels) {
            return error.InvalidAudio;
        }
        h.streams = @as(usize, p[19]);
        h.coupled = @as(usize, p[20]);
        if (h.streams == 0) or (h.coupled > h.streams) or ((h.streams + h.coupled) > 255) {
            return error.InvalidAudio;
        }
        for i in @as(usize, 0)..h.channels {
            h.mapping[i] = p[21 + i];
            let m = @as(usize, p[21 + i]);
            if (m != 255) and (m >= (h.streams + h.coupled)) {
                return error.InvalidAudio;
            }
        }
    }
    return h;
}

struct Streams {
    s: vec.Vec.[[]mut dec.Stream],

    fn deinit(self: *mut Streams) void {
        for x in self.s.items() {
            x[0].deinit();
            mem.free(x);
        }
        self.s.deinit();
    }
}

/// Decodes an Ogg Opus stream to interleaved F32 samples at 48 kHz.
pub fn decode(data: []u8) !audio.Sound {
    let serial = ogg.find_stream(data, "OpusHead")!;
    let mut ps = ogg.packets(data, serial)!;
    defer ps.deinit();
    let pk = ps.items.items();
    if pk.len < 2 {
        return error.InvalidAudio;
    }
    let h = read_head(pk[0].data)!;
    if (pk[1].data.len < 8) or not @mem_eq(pk[1].data[..8], "OpusTags") {
        return error.InvalidAudio;
    }
    let nch = h.channels;
    let mut st = Streams.{ s: vec.Vec.[[]mut dec.Stream].init() };
    defer st.deinit();
    for i in @as(usize, 0)..h.streams {
        let mut c: i32 = 1;
        if i < h.coupled {
            c = 2;
        }
        let s = dec.new_stream(c)!;
        st.s.push(s) catch |e| {
            s[0].deinit();
            mem.free(s);
            return e;
        };
        s[0].decode_gain = h.gain;
    }
    let buf = mem.alloc.[f32](5760 * 2)!;
    defer mem.free(buf);
    let mut out = vec.Vec.[f32].init();
    defer out.deinit();
    let mut frames: usize = 0;
    let mut last_granule: i64 = -1;
    for pi in @as(usize, 2)..pk.len {
        let p = pk[pi].data;
        if pk[pi].granule >= 0 {
            last_granule = pk[pi].granule;
        }
        if p.len == 0 {
            continue;
        }
        // each stream's packet (all but the last self-delimited)
        let mut o: usize = 0;
        let mut n: i32 = -1;
        let start = frames;
        for s in @as(usize, 0)..h.streams {
            if o >= p.len {
                return error.InvalidAudio;
            }
            let last = s == (h.streams - 1);
            let mut used: usize = 0;
            let sd = &mut st.s.items()[s][0];
            let got = dec.decode_packet(sd, p[o..], not last, buf, &mut used);
            if (got <= 0) or ((n >= 0) and (got != n)) {
                return error.InvalidAudio;
            }
            if n < 0 {
                n = got;
                if ((frames + @as(usize, @bit_cast(u32, n))) * nch) > audio.MAX_SAMPLES {
                    return error.TooLarge;
                }
                for _k in @as(usize, 0)..(@as(usize, @bit_cast(u32, n)) * nch) {
                    out.push(0.0)!;
                }
                frames += @as(usize, @bit_cast(u32, n));
            }
            if last {
                o = p.len;
            } else {
                o += used;
            }
            // the stream's channels go where the mapping puts them
            let sc = @as(usize, @bit_cast(u32, sd.channels));
            let ov = out.items();
            for c in @as(usize, 0)..nch {
                let m = @as(usize, h.mapping[c]);
                if m == 255 {
                    continue;
                }
                let mut src_s: usize = 0;
                let mut src_c: usize = 0;
                if m < (2 * h.coupled) {
                    src_s = m / 2;
                    src_c = m % 2;
                } else {
                    src_s = h.coupled + (m - (2 * h.coupled));
                }
                if src_s != s {
                    continue;
                }
                for f in @as(usize, 0)..@as(usize, @bit_cast(u32, n)) {
                    ov[((start + f) * nch) + c] = buf[(f * sc) + src_c];
                }
            }
        }
    }
    // trimming
    let skip = @min(h.pre_skip, frames);
    let mut end = frames;
    if last_granule >= 0 {
        let g = @truncate(usize, @bit_cast(u64, last_granule));
        if g < h.pre_skip {
            end = skip;
        } else {
            end = @min(frames, g);
        }
    }
    let total = end - @min(skip, end);
    let mut s = audio.Sound.init(total, nch, 48000, audio.F32) catch |e| {
        if e == error.OutOfMemory {
            return error.OutOfMemory;
        }
        return error.TooLarge;
    };
    let dst = s.samples_f32();
    @mem_copy(dst, out.items()[skip * nch..(skip + total) * nch]);
    return s;
}

// -----------------------------------------------------------------------------
// Tests
// -----------------------------------------------------------------------------

import "../fs.ph" as fs;
import "../str.ph" as strs;

fn field(line: []u8, n: usize) []u8 {
    let mut k: usize = 0;
    let mut s: usize = 0;
    for i in @as(usize, 0)..(line.len + 1) {
        if (i == line.len) or (line[i] == ' ') {
            if k == n {
                return line[s..i];
            }
            k += 1;
            s = i + 1;
        }
    }
    return line[..0];
}

test "decodes like libopus" {
    let list = fs.read_file("std/audio/testdata/opus/cases.txt") catch return;
    defer mem.free(list);
    let mut start: usize = 0;
    let mut n: usize = 0;
    for i in @as(usize, 0)..list.len {
        if list[i] != '\n' {
            continue;
        }
        let line = list[start..i];
        start = i + 1;
        let name = field(line, 0);
        let mut path = strs.String.init();
        defer path.deinit();
        path.push_str("std/audio/testdata/opus/")!;
        path.push_str(name)!;
        path.push_str(".opus")!;
        let data = fs.read_file(path.bytes())!;
        defer mem.free(data);
        @expect(is_opus(data));
        let mut s = decode(data)!;
        defer s.deinit();
        path.truncate(path.bytes().len - 4);
        path.push_str("pcm")!;
        let want = fs.read_file(path.bytes())!;
        defer mem.free(want);
        @expect_eq(s.frames * s.channels * 2, want.len);
        let got = s.samples_f32();
        let mut worst: f32 = 0.0;
        for k in @as(usize, 0)..@min(got.len, want.len / 2) {
            let w = @float_from_int(f32, @bit_cast(i16, @as(u16, want[2 * k]) | (@as(u16, want[(2 * k) + 1]) << 8)));
            worst = @max(worst, @abs((got[k] * 32768.0) - w));
        }
        @expect(worst <= 1.0);
        n += 1;
    }
    @expect_eq(n, 18);
}

test "survives damaged packets" {
    let data = fs.read_file("std/audio/testdata/opus/switching.opus") catch return;
    defer mem.free(data);
    let serial = ogg.find_stream(data, "OpusHead")!;
    let mut ps = ogg.packets(data, serial)!;
    defer ps.deinit();
    let pk = ps.items.items();
    let mut st = Streams.{ s: vec.Vec.[[]mut dec.Stream].init() };
    defer st.deinit();
    st.s.push(dec.new_stream(1)!)!;
    st.s.push(dec.new_stream(2)!)!;
    let buf = mem.alloc.[f32](5760 * 2)!;
    defer mem.free(buf);
    let mut pb: [1500]u8 = [0; 1500];
    let mut seed: u32 = 12345;
    for it in @as(usize, 0)..3000 {
        seed = (seed *% 1664525) +% 1013904223;
        let src = pk[2 + (@as(usize, seed >> 8) % (pk.len - 2))].data;
        let n = @min(src.len, pb.len);
        @mem_copy(pb[..n], src[..n]);
        for _f in @as(usize, 0)..4 {
            seed = (seed *% 1664525) +% 1013904223;
            pb[@as(usize, seed >> 8) % n] ^= @truncate(u8, seed >> 3);
        }
        let mut used: usize = 0;
        _ = dec.decode_packet(&mut st.s.items()[it % 2][0], pb[..n], false, buf, &mut used);
    }
}
