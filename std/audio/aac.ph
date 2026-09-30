// =============================================================================
// std/audio/aac.ph - AAC decoding, from ADTS streams (.aac) and MP4 files
// (.m4a, .mp4).
//
//     let mut s = aac.decode(bytes)!;   // F32 samples
//     defer s.deinit();
//
// AAC Main, LC and LTP (aac/decoder.ph), HE-AAC (spectral band
// replication, aac/sbr.ph) and HE-AAC v2 (parametric stereo, aac/ps.ph),
// signalled explicitly or implicitly; up to 64 channels (the standard
// channel configurations, in FFmpeg's channel order, or a program
// configuration element). Mono HE-AAC decodes to stereo, as in FFmpeg
// (parametric stereo may appear). An MP4 file's first audio track is
// decoded and trimmed by its edit list (or iTunes' iTunSMPB gapless
// information), so the output has exactly the encoded length; an ADTS
// stream is decoded whole, including the encoder's priming.
// =============================================================================

import "../audio.ph" as audio;
import "../mem.ph" as mem;
import "../vec.ph" as vec;
import "aac/bits.ph" as bitio;
import "aac/decoder.ph" as dec;

error { InvalidAudio, Unsupported, OutOfMemory, TooLarge }

fn be16(d: []u8, o: usize) u32 {
    return (@as(u32, d[o]) << 8) | @as(u32, d[o + 1]);
}

fn be32(d: []u8, o: usize) u32 {
    return (be16(d, o) << 16) | be16(d, o + 2);
}

fn be64(d: []u8, o: usize) u64 {
    return (@as(u64, be32(d, o)) << 32) | @as(u64, be32(d, o + 4));
}

fn id3_skip(data: []u8) usize {
    if (data.len >= 10) and @mem_eq(data[..3], "ID3") {
        let size = (@as(usize, data[6] & 0x7f) << 21) | (@as(usize, data[7] & 0x7f) << 14) |
                   (@as(usize, data[8] & 0x7f) << 7) | @as(usize, data[9] & 0x7f);
        let mut n = 10 + size;
        if (data[5] & 0x10) != 0 {
            n += 10;
        }
        return @min(n, data.len);
    }
    return 0;
}

fn is_adts_at(data: []u8, o: usize) bool {
    return ((o + 7) <= data.len) and (data[o] == 0xff) and ((data[o + 1] & 0xf6) == 0xf0);
}

/// Whether data is an ADTS stream or an MP4 file with an AAC track.
pub fn is_aac(data: []u8) bool {
    let o = id3_skip(data);
    if is_adts_at(data, o) {
        let len = ((@as(usize, data[o + 3]) & 3) << 11) | (@as(usize, data[o + 4]) << 3) | (@as(usize, data[o + 5]) >> 5);
        // one frame, then another header (or the end)
        return (len >= 7) and (((o + len) == data.len) or is_adts_at(data, o + len) or ((o + len) > data.len));
    }
    if (data.len >= 12) and @mem_eq(data[4..8], "ftyp") {
        // an mp4a sample entry somewhere in the file
        let n = data.len - 4;
        for i in @as(usize, 0)..n {
            if (data[i] == 'm') and @mem_eq(data[i..i + 4], "mp4a") {
                return true;
            }
        }
    }
    return false;
}

struct Out {
    v: vec.Vec.[f32],
    frames: usize,
    channels: usize,
}

fn emit(o: *mut Out, block: []f32, n: usize, frames: usize) !void {
    if ((o.frames + frames) * n) > audio.MAX_SAMPLES {
        return error.TooLarge;
    }
    for i in @as(usize, 0)..(frames * n) {
        o.v.push(block[i])!;
    }
    o.frames += frames;
}

fn finish(o: *mut Out, rate: i32, skip0: usize, total0: usize) !audio.Sound {
    let skip = @min(skip0, o.frames);
    let total = @min(total0, o.frames - skip);
    let n = o.channels;
    let mut s = audio.Sound.init(total, n, @bit_cast(u32, rate), audio.F32) catch |e| {
        if e == error.OutOfMemory {
            return error.OutOfMemory;
        }
        return error.TooLarge;
    };
    @mem_copy(s.samples_f32(), o.v.items()[skip * n..(skip + total) * n]);
    return s;
}

// -----------------------------------------------------------------------------
// ADTS
// -----------------------------------------------------------------------------

fn decode_adts(data: []u8) !audio.Sound {
    let mut o = id3_skip(data);
    let mut d: []mut dec.Decoder = mem.alloc.[dec.Decoder](0)!;
    defer {
        if d.len > 0 {
            d[0].deinit();
        }
        mem.free(d);
    }
    let block = mem.alloc.[f32](2048 * 64)!;
    defer mem.free(block);
    let mut out = Out.{ v: vec.Vec.[f32].init(), frames: 0, channels: 0 };
    defer out.v.deinit();
    let mut first_aot: i32 = 0;
    let mut first_rate: i32 = -1;
    let mut first_chan: i32 = -1;
    while (o + 7) <= data.len {
        if not is_adts_at(data, o) {
            if out.frames > 0 {
                break; // trailing junk (an ID3v1 tag, say)
            }
            return error.InvalidAudio;
        }
        let h = data[o..];
        let protection_absent = (h[1] & 1) != 0;
        let profile = @as(i32, h[2] >> 6);
        let rate_index = @as(i32, (h[2] >> 2) & 15);
        let chan = (@as(i32, h[2] & 1) << 2) | @as(i32, h[3] >> 6);
        let len = ((@as(usize, h[3]) & 3) << 11) | (@as(usize, h[4]) << 3) | (@as(usize, h[5]) >> 5);
        let nblocks = @as(usize, h[6] & 3) + 1;
        if (len < 7) or ((o + len) > data.len) or (rate_index > 12) {
            if (out.frames > 0) and ((o + len) > data.len) {
                break; // a truncated last frame
            }
            return error.InvalidAudio;
        }
        let mut hl: usize = 7;
        if not protection_absent {
            if nblocks > 1 {
                return error.Unsupported;
            }
            hl = 9;
        }
        if hl > len {
            return error.InvalidAudio;
        }
        if d.len == 0 {
            let mut cfg = dec.empty_config();
            cfg.object_type = profile + 1;
            if (cfg.object_type != dec.AOT_MAIN) and (cfg.object_type != dec.AOT_LC) and
               (cfg.object_type != dec.AOT_LTP) {
                return error.Unsupported;
            }
            cfg.sampling_index = rate_index;
            cfg.sample_rate = dec.SAMPLE_RATES[@as(usize, @bit_cast(u32, rate_index))];
            cfg.chan_config = chan;
            if chan > 7 {
                return error.Unsupported;
            }
            mem.free(d);
            d = dec.new(cfg)!;
            first_aot = cfg.object_type;
            first_rate = rate_index;
            first_chan = chan;
        } else if ((profile + 1) != first_aot) or (rate_index != first_rate) or (chan != first_chan) {
            return error.Unsupported; // the format changes mid-stream
        }
        let mut b = bitio.Bits.init(data[o + hl..o + len]);
        for _k in @as(usize, 0)..nblocks {
            let got = dec.decode_block(&mut d[0], &mut b, block)!;
            if got > 0 {
                let n = dec.channels(&d[0]);
                if (out.channels != 0) and (out.channels != n) {
                    return error.Unsupported;
                }
                out.channels = n;
                emit(&mut out, block, n, got)!;
            }
            b.align();
        }
        o += len;
    }
    if (d.len == 0) or (out.channels == 0) {
        return error.InvalidAudio;
    }
    let total = out.frames;
    return finish(&mut out, dec.output_rate(&d[0]), 0, total);
}

// -----------------------------------------------------------------------------
// MP4
// -----------------------------------------------------------------------------

struct Box {
    typ: [4]u8,
    body: usize,
    end: usize,
}

/// The box at o in data[..end], or false.
fn box_at(data: []u8, o: usize, end: usize, bx: *mut Box) bool {
    if (o + 8) > end {
        return false;
    }
    let mut size = @as(u64, be32(data, o));
    let mut hdr: usize = 8;
    if size == 1 {
        if (o + 16) > end {
            return false;
        }
        size = be64(data, o + 8);
        hdr = 16;
    } else if size == 0 {
        size = @as(u64, end - o);
    }
    if (size < @as(u64, hdr)) or (size > @as(u64, end - o)) {
        return false;
    }
    bx.typ = [data[o + 4], data[o + 5], data[o + 6], data[o + 7]];
    bx.body = o + hdr;
    bx.end = o + @truncate(usize, size);
    return true;
}

/// The first child box of the given type in data[o..end].
fn find(data: []u8, o0: usize, end: usize, typ: []u8, bx: *mut Box) bool {
    let mut o = o0;
    while box_at(data, o, end, bx) {
        if @mem_eq(bx.typ[..], typ) {
            return true;
        }
        o = bx.end;
    }
    return false;
}

/// A path of nested boxes.
fn find_path(data: []u8, o: usize, end: usize, path: [][]u8, bx: *mut Box) bool {
    let mut s = o;
    let mut e = end;
    for p in path {
        if not find(data, s, e, p, bx) {
            return false;
        }
        s = bx.body;
        e = bx.end;
    }
    return true;
}

struct Track {
    asc: []u8,
    stsz: Box,
    stsc: Box,
    stco: Box,
    co64: bool,
    timescale: u32,
    duration: u64,
    edit_skip: i64,     // media time of the first edit, or -1
    edit_len: u64,      // its length, in the movie timescale
}

fn desc_len(data: []u8, o: *mut usize, end: usize) usize {
    let mut n: usize = 0;
    for _i in @as(usize, 0)..4 {
        if o.* >= end {
            return 0;
        }
        let b = data[o.*];
        o.* += 1;
        n = (n << 7) | @as(usize, b & 0x7f);
        if (b & 0x80) == 0 {
            break;
        }
    }
    return n;
}

/// The AudioSpecificConfig in an esds box.
fn esds_asc(data: []u8, bx: *Box) ![]u8 {
    let mut o = bx.body + 4;
    let end = bx.end;
    while (o + 2) <= end {
        let tag = data[o];
        o += 1;
        let len = desc_len(data, &mut o, end);
        if tag == 3 {
            if (o + 3) > end {
                return error.InvalidAudio;
            }
            let flags = data[o + 2];
            o += 3;
            if (flags & 0x80) != 0 {
                o += 2;
            }
            if (flags & 0x40) != 0 {
                if o >= end {
                    return error.InvalidAudio;
                }
                o += 1 + @as(usize, data[o]);
            }
            if (flags & 0x20) != 0 {
                o += 2;
            }
        } else if tag == 4 {
            if (o + 13) > end {
                return error.InvalidAudio;
            }
            let oti = data[o];
            if (oti != 0x40) and (oti != 0x66) and (oti != 0x67) and (oti != 0x68) {
                return error.Unsupported; // not AAC
            }
            o += 13;
        } else if tag == 5 {
            if (o + len) > end {
                return error.InvalidAudio;
            }
            return data[o..o + len];
        } else {
            o += len;
        }
    }
    return error.InvalidAudio;
}

fn read_track(data: []u8, trak: *Box, movie_scale: u32, tr: *mut Track) !bool {
    let mut bx = Box.{ typ: [0; 4], body: 0, end: 0 };
    let hp: [2][]u8 = ["mdia", "hdlr"];
    if not find_path(data, trak.body, trak.end, hp[..], &mut bx) or ((bx.body + 12) > bx.end) or
       not @mem_eq(data[bx.body + 8..bx.body + 12], "soun") {
        return false;
    }
    let mp: [2][]u8 = ["mdia", "mdhd"];
    if not find_path(data, trak.body, trak.end, mp[..], &mut bx) or ((bx.body + 24) > bx.end) {
        return error.InvalidAudio;
    }
    if data[bx.body] == 1 {
        if (bx.body + 32) > bx.end {
            return error.InvalidAudio;
        }
        tr.timescale = be32(data, bx.body + 20);
        tr.duration = be64(data, bx.body + 24);
    } else {
        tr.timescale = be32(data, bx.body + 12);
        tr.duration = @as(u64, be32(data, bx.body + 16));
    }
    let sp: [5][]u8 = ["mdia", "minf", "stbl", "stsd", "mp4a"];
    if not find_path(data, trak.body, trak.end, sp[..4], &mut bx) {
        return error.InvalidAudio;
    }
    // stsd: version/flags, count, then the sample entries
    let mut entry = Box.{ typ: [0; 4], body: 0, end: 0 };
    if not box_at(data, bx.body + 8, bx.end, &mut entry) {
        return error.InvalidAudio;
    }
    if not @mem_eq(entry.typ[..], "mp4a") {
        return error.Unsupported;
    }
    let mut version: u32 = 0;
    if (entry.body + 10) <= entry.end {
        version = be16(data, entry.body + 8);
    }
    let mut children = entry.body + 28;
    if version == 1 {
        children += 16;
    } else if version == 2 {
        children += 36;
    }
    let mut esds = Box.{ typ: [0; 4], body: 0, end: 0 };
    if not find(data, children, entry.end, "esds", &mut esds) {
        // QuickTime puts it in a wave box
        let mut wave = Box.{ typ: [0; 4], body: 0, end: 0 };
        if not find(data, children, entry.end, "wave", &mut wave) or
           not find(data, wave.body, wave.end, "esds", &mut esds) {
            return error.InvalidAudio;
        }
    }
    tr.asc = esds_asc(data, &esds)!;
    let st: [3][]u8 = ["mdia", "minf", "stbl"];
    let mut stbl = Box.{ typ: [0; 4], body: 0, end: 0 };
    _ = find_path(data, trak.body, trak.end, st[..], &mut stbl);
    if not find(data, stbl.body, stbl.end, "stsz", &mut tr.stsz) or
       not find(data, stbl.body, stbl.end, "stsc", &mut tr.stsc) {
        return error.InvalidAudio;
    }
    tr.co64 = false;
    if not find(data, stbl.body, stbl.end, "stco", &mut tr.stco) {
        if not find(data, stbl.body, stbl.end, "co64", &mut tr.stco) {
            return error.InvalidAudio;
        }
        tr.co64 = true;
    }
    // the edit list: the first edit that is not empty
    tr.edit_skip = -1;
    let ep: [2][]u8 = ["edts", "elst"];
    if find_path(data, trak.body, trak.end, ep[..], &mut bx) and ((bx.body + 8) <= bx.end) {
        let v = data[bx.body];
        let n = @as(usize, be32(data, bx.body + 4));
        let mut o = bx.body + 8;
        for _i in @as(usize, 0)..n {
            let mut dur: u64 = 0;
            let mut mt: i64 = 0;
            if v == 1 {
                if (o + 20) > bx.end {
                    break;
                }
                dur = be64(data, o);
                mt = @bit_cast(i64, be64(data, o + 8));
                o += 20;
            } else {
                if (o + 12) > bx.end {
                    break;
                }
                dur = @as(u64, be32(data, o));
                mt = @as(i64, @bit_cast(i32, be32(data, o + 4)));
                o += 12;
            }
            if mt >= 0 {
                tr.edit_skip = mt;
                tr.edit_len = dur;
                if movie_scale == 0 {
                    tr.edit_skip = -1;
                }
                break;
            }
        }
    }
    return true;
}

/// iTunes gapless information: the priming and the length, in samples.
fn itunsmpb(data: []u8, moov: *Box, priming: *mut u64, length: *mut u64) bool {
    let key = "iTunSMPB";
    let mut i = moov.body;
    while (i + key.len) <= moov.end {
        if @mem_eq(data[i..i + key.len], key) {
            let mut bx = Box.{ typ: [0; 4], body: 0, end: 0 };
            if find(data, i + key.len, moov.end, "data", &mut bx) and ((bx.body + 8) <= bx.end) {
                // " 00000000 00000840 000001C4 0000000000046A7C ..."
                let s = data[bx.body + 8..bx.end];
                let mut vals: [4]u64 = [0; 4];
                let mut k: usize = 0;
                let mut j: usize = 0;
                while (j < s.len) and (k < 4) {
                    while (j < s.len) and (s[j] == ' ') {
                        j += 1;
                    }
                    let st = j;
                    let mut v: u64 = 0;
                    while j < s.len {
                        let c = s[j];
                        let mut h: u64 = 0;
                        if (c >= '0') and (c <= '9') {
                            h = @as(u64, c - '0');
                        } else if (c >= 'A') and (c <= 'F') {
                            h = @as(u64, c - 'A') + 10;
                        } else if (c >= 'a') and (c <= 'f') {
                            h = @as(u64, c - 'a') + 10;
                        } else {
                            break;
                        }
                        v = (v << 4) | h;
                        j += 1;
                    }
                    if j == st {
                        break;
                    }
                    vals[k] = v;
                    k += 1;
                }
                if (k == 4) and (vals[3] > 0) {
                    priming.* = vals[1];
                    length.* = vals[3];
                    return true;
                }
            }
            return false;
        }
        i += 1;
    }
    return false;
}

fn decode_mp4(data: []u8) !audio.Sound {
    let mut moov = Box.{ typ: [0; 4], body: 0, end: 0 };
    if not find(data, 0, data.len, "moov", &mut moov) {
        return error.InvalidAudio;
    }
    let mut movie_scale: u32 = 0;
    let mut bx = Box.{ typ: [0; 4], body: 0, end: 0 };
    if find(data, moov.body, moov.end, "mvhd", &mut bx) and ((bx.body + 24) <= bx.end) {
        if data[bx.body] == 1 {
            movie_scale = be32(data, bx.body + 20);
        } else {
            movie_scale = be32(data, bx.body + 12);
        }
    }
    // the first audio track
    let mut tr = Track.{ asc: data[..0], stsz: bx, stsc: bx, stco: bx, co64: false, timescale: 0, duration: 0,
                         edit_skip: -1, edit_len: 0 };
    let mut o = moov.body;
    let mut found = false;
    let mut trak = Box.{ typ: [0; 4], body: 0, end: 0 };
    while box_at(data, o, moov.end, &mut trak) {
        if @mem_eq(trak.typ[..], "trak") and read_track(data, &trak, movie_scale, &mut tr)! {
            found = true;
            break;
        }
        o = trak.end;
    }
    if not found {
        return error.InvalidAudio;
    }
    let mut cfg = dec.empty_config();
    dec.parse_asc(tr.asc, &mut cfg)!;
    let d = dec.new(cfg)!;
    defer {
        d[0].deinit();
        mem.free(d);
    }
    let block = mem.alloc.[f32](2048 * 64)!;
    defer mem.free(block);
    let mut out = Out.{ v: vec.Vec.[f32].init(), frames: 0, channels: 0 };
    defer out.v.deinit();
    // the samples: chunks (stco) of runs of samples (stsc) of sizes (stsz)
    if ((tr.stsz.body + 12) > tr.stsz.end) or ((tr.stsc.body + 8) > tr.stsc.end) or ((tr.stco.body + 8) > tr.stco.end) {
        return error.InvalidAudio;
    }
    let fixed = be32(data, tr.stsz.body + 4);
    let nsamples = @as(usize, be32(data, tr.stsz.body + 8));
    if (fixed == 0) and ((tr.stsz.body + 12 + (4 * nsamples)) > tr.stsz.end) {
        return error.InvalidAudio;
    }
    let nchunks = @as(usize, be32(data, tr.stco.body + 4));
    let mut csize: usize = 4;
    if tr.co64 {
        csize = 8;
    }
    if (tr.stco.body + 8 + (csize * nchunks)) > tr.stco.end {
        return error.InvalidAudio;
    }
    let nruns = @as(usize, be32(data, tr.stsc.body + 4));
    if (tr.stsc.body + 8 + (12 * nruns)) > tr.stsc.end {
        return error.InvalidAudio;
    }
    let mut sample: usize = 0;
    let mut run: usize = 0;
    for chunk in @as(usize, 0)..nchunks {
        // the run this chunk is in (first_chunk counts from 1)
        while ((run + 1) < nruns) and (@as(usize, be32(data, tr.stsc.body + 8 + (12 * (run + 1)))) <= (chunk + 1)) {
            run += 1;
        }
        if nruns == 0 {
            break;
        }
        let per = @as(usize, be32(data, tr.stsc.body + 8 + (12 * run) + 4));
        let mut pos: usize = 0;
        if tr.co64 {
            pos = @truncate(usize, be64(data, tr.stco.body + 8 + (8 * chunk)));
        } else {
            pos = @as(usize, be32(data, tr.stco.body + 8 + (4 * chunk)));
        }
        for _k in @as(usize, 0)..per {
            if sample >= nsamples {
                break;
            }
            let mut size = @as(usize, fixed);
            if fixed == 0 {
                size = @as(usize, be32(data, tr.stsz.body + 12 + (4 * sample)));
            }
            if (pos > data.len) or (size > (data.len - pos)) {
                return error.InvalidAudio;
            }
            let mut b = bitio.Bits.init(data[pos..pos + size]);
            let got = dec.decode_block(&mut d[0], &mut b, block)!;
            if got > 0 {
                let n = dec.channels(&d[0]);
                if (out.channels != 0) and (out.channels != n) {
                    return error.Unsupported;
                }
                out.channels = n;
                emit(&mut out, block, n, got)!;
            }
            pos += size;
            sample += 1;
        }
    }
    if out.channels == 0 {
        return error.InvalidAudio;
    }
    // trimming: the edit list, else iTunes' gapless information
    let orate = dec.output_rate(&d[0]);
    let rate = @as(u64, @bit_cast(u32, orate));
    let ts = @as(u64, tr.timescale);
    let mut skip: usize = 0;
    let mut total = out.frames;
    if (tr.edit_skip >= 0) and (ts > 0) {
        skip = @truncate(usize, (@bit_cast(u64, tr.edit_skip) * rate) / ts);
        if (tr.edit_len > 0) and (movie_scale > 0) {
            total = @truncate(usize, (tr.edit_len * rate) / @as(u64, movie_scale));
            // the media's own length is exact where the movie's timescale is coarse
            if tr.duration > @bit_cast(u64, tr.edit_skip) {
                let media = @truncate(usize, ((tr.duration - @bit_cast(u64, tr.edit_skip)) * rate) / ts);
                let diff = @max(media, total) - @min(media, total);
                if (diff * @as(usize, movie_scale)) < @truncate(usize, rate) {
                    total = media;
                }
            }
        }
    } else {
        let mut priming: u64 = 0;
        let mut length: u64 = 0;
        if itunsmpb(data, &moov, &mut priming, &mut length) {
            skip = @truncate(usize, priming);
            total = @truncate(usize, length);
        }
    }
    return finish(&mut out, orate, skip, total);
}

/// Decodes an ADTS stream or an MP4 file's first AAC track to F32 samples.
pub fn decode(data: []u8) !audio.Sound {
    if (data.len >= 12) and @mem_eq(data[4..8], "ftyp") {
        return decode_mp4(data);
    }
    return decode_adts(data);
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

fn num(s: []u8) usize {
    let mut v: usize = 0;
    for c in s {
        v = (v * 10) + @as(usize, c - '0');
    }
    return v;
}

test "decodes like FFmpeg" {
    let list = fs.read_file("std/audio/testdata/aac/cases.txt") catch return;
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
        path.push_str("std/audio/testdata/aac/")!;
        path.push_str(name)!;
        path.push_str(".")!;
        path.push_str(field(line, 1))!;
        let data = fs.read_file(path.bytes())!;
        defer mem.free(data);
        @expect(is_aac(data));
        let mut s = decode(data)!;
        defer s.deinit();
        @expect_eq(s.channels, num(field(line, 2)));
        @expect_eq(@as(usize, s.rate), num(field(line, 4)));
        @expect_eq(s.frames, num(field(line, 5)));
        path.truncate(path.bytes().len - field(line, 1).len);
        path.push_str("pcm")!;
        let want = fs.read_file(path.bytes())!;
        defer mem.free(want);
        let got = s.samples_f32();
        let mut worst: f32 = 0.0;
        for k in @as(usize, 0)..@min(got.len, want.len / 2) {
            let w = @float_from_int(f32, @bit_cast(i16, @as(u16, want[2 * k]) | (@as(u16, want[(2 * k) + 1]) << 8)));
            let g = @min(@max(got[k] * 32768.0, -32768.0), 32767.0);
            worst = @max(worst, @abs(g - w));
        }
        // SBR and PS amplify rounding differences: FFmpeg's own C and SIMD
        // code differ by up to 0.75 of a 16-bit step on these streams
        let mut limit: f32 = 1.0;
        if (name.len >= 2) and @mem_eq(name[..2], "he") {
            limit = 2.0;
        }
        @expect(worst <= limit);
        n += 1;
    }
    @expect_eq(n, 17);
}

test "rejects damaged files" {
    let names: [3][]u8 = ["std/audio/testdata/aac/lc_mono.aac", "std/audio/testdata/aac/main.m4a",
                          "std/audio/testdata/aac/he_v2.aac"];
    let mut seed: u32 = 777;
    for name in names {
        let data = fs.read_file(name) catch return;
        defer mem.free(data);
        let buf = mem.alloc.[u8](data.len)!;
        defer mem.free(buf);
        for _it in @as(usize, 0)..150 {
            @mem_copy(buf, data);
            for _f in @as(usize, 0)..6 {
                seed = (seed *% 1664525) +% 1013904223;
                buf[@as(usize, seed >> 8) % buf.len] ^= @truncate(u8, seed >> 2);
            }
            seed = (seed *% 1664525) +% 1013904223;
            let n = buf.len - (@as(usize, seed >> 8) % (buf.len / 2));
            let mut s = decode(buf[..n]) catch continue;
            s.deinit();
        }
    }
}
