// =============================================================================
// std/audio/opus/decoder.ph - decoding Opus packets (RFC 6716): the table of
// contents, frame packing, SILK, CELT and hybrid frames, redundant CELT
// frames and the transitions between modes, as libopus's
// src/opus_decoder.c does them; and multistream packets (RFC 7845).
// =============================================================================

import "../../mem.ph" as mem;
import "celt.ph" as celt;
import "range.ph" as rc;
import "silk.ph" as silk;

error { InvalidAudio, OutOfMemory }

pub const MODE_SILK: i32 = 1000;
pub const MODE_HYBRID: i32 = 1001;
pub const MODE_CELT: i32 = 1002;

pub const BW_NB: i32 = 1101;
pub const BW_MB: i32 = 1102;
pub const BW_WB: i32 = 1103;
pub const BW_SWB: i32 = 1104;
pub const BW_FB: i32 = 1105;

const FS: i32 = 48000;
const F20: i32 = 960;
const F10: i32 = 480;
const F5: i32 = 240;
const F2_5: i32 = 120;

fn us(v: i32) usize {
    return @as(usize, @bit_cast(u32, v));
}

// -----------------------------------------------------------------------------
// Packets
// -----------------------------------------------------------------------------

pub fn packet_mode(toc: u8) i32 {
    if (toc & 0x80) != 0 {
        return MODE_CELT;
    }
    if (toc & 0x60) == 0x60 {
        return MODE_HYBRID;
    }
    return MODE_SILK;
}

pub fn packet_bandwidth(toc: u8) i32 {
    if (toc & 0x80) != 0 {
        let bw = BW_MB + @as(i32, (toc >> 5) & 3);
        if bw == BW_MB {
            return BW_NB;
        }
        return bw;
    }
    if (toc & 0x60) == 0x60 {
        if (toc & 0x10) != 0 {
            return BW_FB;
        }
        return BW_SWB;
    }
    return BW_NB + @as(i32, (toc >> 5) & 3);
}

/// Samples per frame at 48 kHz.
pub fn samples_per_frame(toc: u8) i32 {
    if (toc & 0x80) != 0 {
        return (FS << @as(u32, (toc >> 3) & 3)) / 400;
    }
    if (toc & 0x60) == 0x60 {
        if (toc & 0x08) != 0 {
            return FS / 50;
        }
        return FS / 100;
    }
    let a = (toc >> 3) & 3;
    if a == 3 {
        return (FS * 60) / 1000;
    }
    return (FS << @as(u32, a)) / 100;
}

pub struct Frames {
    toc: u8,
    count: usize,
    offset: [48]usize,  // of each frame in the packet
    size: [48]usize,
    packet_len: usize,  // the bytes the packet takes (with padding)
}

fn parse_size(d: []u8, o: usize, len: i32, size: *mut i32) i32 {
    if len < 1 {
        size.* = -1;
        return -1;
    }
    if d[o] < 252 {
        size.* = @as(i32, d[o]);
        return 1;
    }
    if len < 2 {
        size.* = -1;
        return -1;
    }
    size.* = (4 * @as(i32, d[o + 1])) + @as(i32, d[o]);
    return 2;
}

/// Splits a packet into its frames (self_delimited for all but the last
/// stream of a multistream packet).
pub fn parse_packet(data: []u8, self_delimited: bool, f: *mut Frames) !void {
    let mut len = @bit_cast(i32, @truncate(u32, data.len));
    if len == 0 {
        return error.InvalidAudio;
    }
    let framesize = samples_per_frame(data[0]);
    let mut cbr = false;
    let toc = data[0];
    let mut o: usize = 1;
    len -= 1;
    let mut last_size = len;
    let mut count: i32 = 1;
    let mut size: [48]i32 = [0; 48];
    let code = toc & 3;
    if code == 1 {
        count = 2;
        cbr = true;
        if not self_delimited {
            if (len & 1) != 0 {
                return error.InvalidAudio;
            }
            last_size = len / 2;
            size[0] = last_size;
        }
    } else if code == 2 {
        count = 2;
        let bytes = parse_size(data, o, len, &mut size[0]);
        len -= bytes;
        if (size[0] < 0) or (size[0] > len) {
            return error.InvalidAudio;
        }
        o += us(bytes);
        last_size = len - size[0];
    } else if code == 3 {
        if len < 1 {
            return error.InvalidAudio;
        }
        let ch = data[o];
        o += 1;
        count = @as(i32, ch & 0x3f);
        if (count <= 0) or ((framesize * count) > 5760) {
            return error.InvalidAudio;
        }
        len -= 1;
        if (ch & 0x40) != 0 {
            loop {
                if len <= 0 {
                    return error.InvalidAudio;
                }
                let p = @as(i32, data[o]);
                o += 1;
                len -= 1;
                let mut tmp = p;
                if p == 255 {
                    tmp = 254;
                }
                len -= tmp;
                if p != 255 {
                    break;
                }
            }
        }
        if len < 0 {
            return error.InvalidAudio;
        }
        cbr = (ch & 0x80) == 0;
        if not cbr {
            last_size = len;
            for i in @as(usize, 0)..us(count - 1) {
                let bytes = parse_size(data, o, len, &mut size[i]);
                len -= bytes;
                if (size[i] < 0) or (size[i] > len) {
                    return error.InvalidAudio;
                }
                o += us(bytes);
                last_size -= bytes + size[i];
            }
            if last_size < 0 {
                return error.InvalidAudio;
            }
        } else if not self_delimited {
            last_size = len / count;
            if (last_size * count) != len {
                return error.InvalidAudio;
            }
            for i in @as(usize, 0)..us(count - 1) {
                size[i] = last_size;
            }
        }
    }
    let n = us(count);
    if self_delimited {
        let bytes = parse_size(data, o, len, &mut size[n - 1]);
        len -= bytes;
        if (size[n - 1] < 0) or (size[n - 1] > len) {
            return error.InvalidAudio;
        }
        o += us(bytes);
        if cbr {
            if (size[n - 1] * count) > len {
                return error.InvalidAudio;
            }
            for i in @as(usize, 0)..(n - 1) {
                size[i] = size[n - 1];
            }
        } else if (bytes + size[n - 1]) > last_size {
            return error.InvalidAudio;
        }
    } else {
        if last_size > 1275 {
            return error.InvalidAudio;
        }
        size[n - 1] = last_size;
    }
    f.toc = toc;
    f.count = n;
    for i in @as(usize, 0)..n {
        if (size[i] > 1275) or ((o + us(size[i])) > data.len) {
            return error.InvalidAudio;
        }
        f.offset[i] = o;
        f.size[i] = us(size[i]);
        o += us(size[i]);
    }
    // the padding of code 3 packets ends the packet (in a multistream packet
    // the next stream follows it)
    if code == 3 {
        let ch = data[1];
        if (ch & 0x40) != 0 {
            let mut q: usize = 2;
            let mut pad: usize = 0;
            loop {
                let p = @as(usize, data[q]);
                q += 1;
                if p == 255 {
                    pad += 254;
                } else {
                    pad += p;
                    break;
                }
            }
            o += pad;
        }
    }
    f.packet_len = o;
}

// -----------------------------------------------------------------------------
// A decoder of one stream (1 or 2 channels)
// -----------------------------------------------------------------------------

pub struct Stream {
    channels: i32,
    celt: []mut celt.Celt,
    silk: []mut silk.Silk,
    ctl: silk.Control,
    stream_channels: i32,
    bandwidth: i32,
    mode: i32,
    prev_mode: i32,
    frame_size: i32,
    prev_redundancy: bool,
    last_packet_duration: i32,
    decode_gain: i32,
    pcm_silk: [5760]i16,       // up to 60 ms of 48 kHz stereo
    transition: [480]f32,
    redundant: [480]f32,

    pub fn deinit(self: *mut Stream) void {
        mem.free(self.celt);
        mem.free(self.silk);
    }
}

pub fn new_stream(channels: i32) ![]mut Stream {
    let s = mem.alloc_zeroed.[Stream](1)!;
    let st = &mut s[0];
    st.celt = celt.new(channels) catch |e| {
        mem.free(s);
        return e;
    };
    st.silk = silk.new() catch |e| {
        mem.free(st.celt);
        mem.free(s);
        return e;
    };
    st.channels = channels;
    st.stream_channels = channels;
    st.ctl.api_rate = FS;
    st.ctl.channels_api = channels;
    st.prev_mode = 0;
    st.frame_size = FS / 400;
    return s;
}

fn smooth_fade(in1: []f32, in2: []f32, out: []mut f32, overlap: i32, channels: i32) void {
    let win = celt.window();
    for c in @as(usize, 0)..us(channels) {
        for i in @as(usize, 0)..us(overlap) {
            let w = win[i] * win[i];
            let k = (i * us(channels)) + c;
            out[k] = (w * in2[k]) + ((1.0 - w) * in1[k]);
        }
    }
}

// one frame of a packet (data empty: concealment)
fn decode_frame(st: *mut Stream, data: []u8, len0: i32, pcm: []mut f32, frame_size0: i32) i32 {
    let mut len = len0;
    let mut frame_size = frame_size0;
    let cc = us(st.channels);
    let mut have_data = data.len > 0;
    if frame_size < F2_5 {
        return -1;
    }
    frame_size = @min(frame_size, (FS / 25) * 3);
    if len <= 1 {
        have_data = false;
        frame_size = @min(frame_size, st.frame_size);
    }
    let mut dec = rc.Dec.init(data[..0]);
    let mut audiosize: i32 = 0;
    let mut mode: i32 = 0;
    let mut bandwidth: i32 = 0;
    if have_data {
        audiosize = st.frame_size;
        mode = st.mode;
        bandwidth = st.bandwidth;
        dec = rc.Dec.init(data[..us(len)]);
    } else {
        audiosize = frame_size;
        mode = st.prev_mode;
        bandwidth = 0;
        if mode == 0 {
            @mem_set(pcm[..us(audiosize) * cc], 0.0);
            return audiosize;
        }
        if audiosize > F20 {
            let mut done: i32 = 0;
            loop {
                let ret = decode_frame(st, data[..0], 0, pcm[us(done) * cc..], @min(audiosize - done, F20));
                if ret < 0 {
                    return ret;
                }
                done += ret;
                if done >= audiosize {
                    break;
                }
            }
            return frame_size;
        } else if audiosize < F20 {
            if audiosize > F10 {
                audiosize = F10;
            } else if (mode != MODE_SILK) and (audiosize > F5) and (audiosize < F10) {
                audiosize = F5;
            }
        }
    }
    let mut transition = false;
    if have_data and (st.prev_mode > 0) and
       (((mode == MODE_CELT) and (st.prev_mode != MODE_CELT) and not st.prev_redundancy) or
        ((mode != MODE_CELT) and (st.prev_mode == MODE_CELT))) {
        transition = true;
    }
    let mut trans = st.transition[..];
    if transition and (mode == MODE_CELT) {
        _ = decode_frame(st, data[..0], 0, trans, @min(F5, audiosize));
    }
    if audiosize > frame_size {
        return -1;
    }
    frame_size = audiosize;
    let pcm_silk = st.pcm_silk[..];
    if mode != MODE_CELT {
        if st.prev_mode == MODE_CELT {
            silk.reset(&mut st.silk[0]);
        }
        st.ctl.payload_ms = @max(10, (1000 * audiosize) / FS);
        if have_data {
            st.ctl.channels_internal = st.stream_channels;
            if mode == MODE_SILK {
                if bandwidth == BW_NB {
                    st.ctl.internal_rate = 8000;
                } else if bandwidth == BW_MB {
                    st.ctl.internal_rate = 12000;
                } else {
                    st.ctl.internal_rate = 16000;
                }
            } else {
                st.ctl.internal_rate = 16000;
            }
        }
        let mut lost: i32 = 0;
        if not have_data {
            lost = 1;
        }
        let mut decoded: i32 = 0;
        loop {
            let mut n: i32 = 0;
            let ok = silk.decode(&mut st.silk[0], &mut st.ctl, lost, decoded == 0, &mut dec,
                                 pcm_silk[us(decoded) * cc..], &mut n);
            if not ok {
                if lost != 0 {
                    n = frame_size;
                    @mem_set(pcm_silk[us(decoded) * cc..us(decoded + n) * cc], 0);
                } else {
                    return -1;
                }
            }
            decoded += n;
            if decoded >= frame_size {
                break;
            }
        }
    }
    let mut start_band: i32 = 0;
    let mut redundancy = false;
    let mut redundancy_bytes: i32 = 0;
    let mut celt_to_silk = false;
    let mut hyb: i32 = 0;
    if st.mode == MODE_HYBRID {
        hyb = 1;
    }
    if (mode != MODE_CELT) and have_data and ((dec.tell() + 17 + (20 * hyb)) <= (8 * len)) {
        if mode == MODE_HYBRID {
            redundancy = dec.bit_logp(12);
        } else {
            redundancy = true;
        }
        if redundancy {
            celt_to_silk = dec.bit_logp(1);
            if mode == MODE_HYBRID {
                redundancy_bytes = @bit_cast(i32, dec.uint(256)) + 2;
            } else {
                redundancy_bytes = len - ((dec.tell() + 7) >> 3);
            }
            len -= redundancy_bytes;
            if (len * 8) < dec.tell() {
                len = 0;
                redundancy_bytes = 0;
                redundancy = false;
            }
            dec.storage -= @bit_cast(u32, redundancy_bytes);
        }
    }
    if mode != MODE_CELT {
        start_band = 17;
    }
    if redundancy {
        transition = false;
    }
    if transition and (mode != MODE_CELT) {
        _ = decode_frame(st, data[..0], 0, trans, @min(F5, audiosize));
    }
    let cs = &mut st.celt[0];
    if bandwidth != 0 {
        let mut endband: i32 = 21;
        if bandwidth == BW_NB {
            endband = 13;
        } else if (bandwidth == BW_MB) or (bandwidth == BW_WB) {
            endband = 17;
        } else if bandwidth == BW_SWB {
            endband = 19;
        }
        cs.end = endband;
    }
    cs.stream_channels = st.stream_channels;
    let red = st.redundant[..];
    let mut redundant_rng: u32 = 0;
    if redundancy and celt_to_silk {
        cs.start = 0;
        let mut rd = rc.Dec.init(data[us(len)..us(len + redundancy_bytes)]);
        _ = celt.decode(cs, redundancy_bytes, red, F5, &mut rd, 1);
        redundant_rng = cs.rng;
    }
    cs.start = start_band;
    let mut celt_ok = true;
    if mode != MODE_SILK {
        let celt_frame_size = @min(F20, frame_size);
        if (mode != st.prev_mode) and (st.prev_mode > 0) and not st.prev_redundancy {
            cs.reset();
        }
        if have_data {
            celt_ok = celt.decode(cs, len, pcm, celt_frame_size, &mut dec, 1);
        } else {
            celt.decode_lost(cs, pcm, celt_frame_size);
        }
    } else {
        @mem_set(pcm[..us(frame_size) * cc], 0.0);
        if (st.prev_mode == MODE_HYBRID) and not (redundancy and celt_to_silk and st.prev_redundancy) {
            cs.start = 0;
            let silence: [2]u8 = [0xff, 0xff];
            let mut sd = rc.Dec.init(silence[..]);
            _ = celt.decode(cs, 2, pcm, F2_5, &mut sd, 1);
        }
    }
    if mode != MODE_CELT {
        for i in @as(usize, 0)..(us(frame_size) * cc) {
            pcm[i] = pcm[i] + ((1.0 / 32768.0) * @float_from_int(f32, pcm_silk[i]));
        }
    }
    let c2 = us(st.channels);
    if redundancy and not celt_to_silk {
        cs.reset();
        cs.start = 0;
        let mut rd = rc.Dec.init(data[us(len)..us(len + redundancy_bytes)]);
        _ = celt.decode(cs, redundancy_bytes, red, F5, &mut rd, 1);
        redundant_rng = cs.rng;
        let o = c2 * us(frame_size - F2_5);
        let mut tmp: [240]f32 = [0.0; 240];
        smooth_fade(pcm[o..], red[c2 * us(F2_5)..], tmp[..], F2_5, st.channels);
        @mem_copy(pcm[o..o + (c2 * us(F2_5))], tmp[..c2 * us(F2_5)]);
    }
    if redundancy and celt_to_silk {
        for i in @as(usize, 0)..(c2 * us(F2_5)) {
            pcm[i] = red[i];
        }
        let o = c2 * us(F2_5);
        let mut tmp: [240]f32 = [0.0; 240];
        smooth_fade(red[o..], pcm[o..], tmp[..], F2_5, st.channels);
        @mem_copy(pcm[o..o + (c2 * us(F2_5))], tmp[..c2 * us(F2_5)]);
    }
    if transition {
        if audiosize >= F5 {
            for i in @as(usize, 0)..(c2 * us(F2_5)) {
                pcm[i] = trans[i];
            }
            let o = c2 * us(F2_5);
            let mut tmp: [240]f32 = [0.0; 240];
            smooth_fade(trans[o..], pcm[o..], tmp[..], F2_5, st.channels);
            @mem_copy(pcm[o..o + (c2 * us(F2_5))], tmp[..c2 * us(F2_5)]);
        } else {
            let mut tmp: [240]f32 = [0.0; 240];
            smooth_fade(trans, pcm, tmp[..], F2_5, st.channels);
            @mem_copy(pcm[..c2 * us(F2_5)], tmp[..c2 * us(F2_5)]);
        }
    }
    if st.decode_gain != 0 {
        let gain = celt.exp2_gain(st.decode_gain);
        for i in @as(usize, 0)..(us(frame_size) * c2) {
            pcm[i] = pcm[i] * gain;
        }
    }
    _ = redundant_rng;
    st.prev_mode = mode;
    st.prev_redundancy = redundancy and not celt_to_silk;
    if not celt_ok {
        return -1;
    }
    return audiosize;
}

/// Decodes a packet (of this stream) into pcm (interleaved, up to 5760
/// frames); returns the frames decoded, or -1.
pub fn decode_packet(st: *mut Stream, data: []u8, self_delimited: bool, pcm: []mut f32, used: *mut usize) i32 {
    let mut f = Frames.{ toc: 0, count: 0, offset: [0; 48], size: [0; 48], packet_len: 0 };
    parse_packet(data, self_delimited, &mut f) catch return -1;
    used.* = f.packet_len;
    let toc = data[0];
    st.mode = packet_mode(toc);
    st.bandwidth = packet_bandwidth(toc);
    st.frame_size = samples_per_frame(toc);
    st.stream_channels = 1;
    if (toc & 4) != 0 {
        st.stream_channels = 2;
    }
    if (@bit_cast(i32, @truncate(u32, f.count)) * st.frame_size) > 5760 {
        return -1;
    }
    let cc = us(st.channels);
    let mut nb: i32 = 0;
    for i in @as(usize, 0)..f.count {
        let fr = data[f.offset[i]..f.offset[i] + f.size[i]];
        let ret = decode_frame(st, fr, @bit_cast(i32, @truncate(u32, f.size[i])), pcm[us(nb) * cc..], 5760 - nb);
        if ret < 0 {
            return -1;
        }
        nb += ret;
    }
    st.last_packet_duration = nb;
    return nb;
}
