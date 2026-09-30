// =============================================================================
// std/audio/formats.ph - reading and writing sound files in any supported
// format.
//
//     let mut s = formats.load("song.flac")!;           // the format from the content
//     defer s.deinit();
//     formats.save("song.wav", &s)!;                   // from the extension
//
//     let f = formats.detect(bytes);                   // WAV, AIFF, ... or UNKNOWN
//     let s2 = formats.decode(bytes)!;
//     let out = formats.encode(&s, formats.FLAC)!;
//
// Formats: WAV, AIFF and FLAC (read and write), Ogg Vorbis, Ogg Opus and MP3
// (read).
// Encoding converts as the format needs (FLAC holds integers of up to 32
// bits: float sounds become S24).
// =============================================================================

import "../audio.ph" as audio;
import "../io.ph" as io;
import "../mem.ph" as mem;
import "../str.ph" as strs;
import "aiff.ph" as aiff;
import "flac.ph" as flac;
import "mp3.ph" as mp3;
import "opus.ph" as opus;
import "vorbis.ph" as vorbis;
import "wav.ph" as wav;

error { Unsupported, InvalidAudio, OutOfMemory }

pub const UNKNOWN: u8 = 0;
pub const WAV: u8 = 1;
pub const AIFF: u8 = 2;
pub const FLAC: u8 = 3;
pub const VORBIS: u8 = 4;
pub const MP3: u8 = 5;
pub const OPUS: u8 = 6;

/// The format of encoded data, from its first bytes.
pub fn detect(data: []u8) u8 {
    if wav.is_wav(data) {
        return WAV;
    }
    if aiff.is_aiff(data) {
        return AIFF;
    }
    if flac.is_flac(data) {
        return FLAC;
    }
    if vorbis.is_vorbis(data) {
        return VORBIS;
    }
    if opus.is_opus(data) {
        return OPUS;
    }
    if mp3.is_mp3(data) {
        return MP3;  // last: its test is the loosest
    }
    return UNKNOWN;
}

/// The format for a file name's extension (case-insensitive), or UNKNOWN.
pub fn from_extension(path: []u8) u8 {
    let dot = strs.last_index_of_byte(path, '.') catch return UNKNOWN;
    let ext = path[dot + 1..];
    let mut buf: [8]u8 = [0; 8];
    if ext.len > buf.len {
        return UNKNOWN;
    }
    for i in @as(usize, 0)..ext.len {
        buf[i] = strs.to_lower_byte(ext[i]);
    }
    let e = buf[..ext.len];
    if @mem_eq(e, "wav") or @mem_eq(e, "wave") {
        return WAV;
    }
    if @mem_eq(e, "aiff") or @mem_eq(e, "aif") or @mem_eq(e, "aifc") {
        return AIFF;
    }
    if @mem_eq(e, "flac") {
        return FLAC;
    }
    if @mem_eq(e, "ogg") or @mem_eq(e, "oga") {
        return VORBIS;
    }
    if @mem_eq(e, "mp3") {
        return MP3;
    }
    if @mem_eq(e, "opus") {
        return OPUS;
    }
    return UNKNOWN;
}

/// Decodes a sound in any supported format.
pub fn decode(data: []u8) !audio.Sound {
    let f = detect(data);
    if f == WAV {
        return wav.decode(data);
    }
    if f == AIFF {
        return aiff.decode(data);
    }
    if f == FLAC {
        return flac.decode(data);
    }
    if f == VORBIS {
        return vorbis.decode(data);
    }
    if f == MP3 {
        return mp3.decode(data);
    }
    if f == OPUS {
        return opus.decode(data);
    }
    return error.Unsupported;
}

/// Encodes a sound (free the result with mem.free); WAV, AIFF or FLAC.
pub fn encode(s: *audio.Sound, format: u8) ![]mut u8 {
    if format == WAV {
        return wav.encode(s);
    }
    if format == AIFF {
        return aiff.encode(s);
    }
    if format == FLAC {
        return flac.encode(s, flac.DEFAULT);
    }
    return error.Unsupported;
}

/// Reads and decodes a sound file.
pub fn load(path: []u8) !audio.Sound {
    let f = io.open(path)!;
    defer f.close();
    let data = f.reader().read_all()!;
    defer mem.free(data);
    return decode(data);
}

/// Encodes a sound in the format its file name's extension names, and
/// writes the file.
pub fn save(path: []u8, s: *audio.Sound) !void {
    let format = from_extension(path);
    if format == UNKNOWN {
        return error.Unsupported;
    }
    let data = encode(s, format)!;
    defer mem.free(data);
    let f = io.create(path)!;
    defer f.close();
    f.writer().write_all(data)!;
}

test "every writable format" {
    let mut s = audio.Sound.init(300, 2, 32000, audio.S16)!;
    defer s.deinit();
    for i in @as(usize, 0)..s.len() {
        s.set_sample(i, @float_from_int(f64, (i * 37) % 200) / 100.0 - 1.0);
    }
    let fmts: [3]u8 = [WAV, AIFF, FLAC];
    for f in fmts {
        let enc = encode(&s, f)!;
        defer mem.free(enc);
        @expect_eq(detect(enc), f);
        let mut back = decode(enc)!;
        defer back.deinit();
        @expect_eq(back.data[..], s.data[..]);
    }
    @expect_eq(from_extension("a/b.Flac"), FLAC);
    @expect_eq(from_extension("x.ogg"), VORBIS);
    @expect_eq(from_extension("x.MP3"), MP3);
    @expect_eq(from_extension("x.opus"), OPUS);
    @expect_eq(from_extension("noext"), UNKNOWN);
}
