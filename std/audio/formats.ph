// =============================================================================
// std/audio/formats.ph - reading and writing sound files in any supported
// format.
//
//     let mut s = formats.load("song.flac")!;           // the format from the content
//     defer s.deinit();
//     formats.save("song.wav", &s)!;                   // from the extension
//
//     let f = formats.detect(bytes);                   // Format.WAV, ... or Format.UNKNOWN
//     let s2 = formats.decode(bytes)!;                 // or decode_from(reader)
//     let out = formats.encode(&s, formats.Format.FLAC)!;
//     formats.encode_to(w, &s, formats.Format.WAV)!;   // to an io.Writer
//
// Formats: WAV, AIFF and FLAC (read and write), Ogg Vorbis, Ogg Opus, MP3 and
// AAC (ADTS and MP4; read).
// Encoding converts as the format needs (FLAC holds integers of up to 32
// bits: float sounds become S24).
// =============================================================================

import "../audio.ph" as audio;
import "../io.ph" as io;
import "../mem.ph" as mem;
import "../os/fs.ph" as fs;
import "../str.ph" as strs;
import "aac.ph" as aac;
import "aiff.ph" as aiff;
import "flac.ph" as flac;
import "mp3.ph" as mp3;
import "opus.ph" as opus;
import "vorbis.ph" as vorbis;
import "wav.ph" as wav;

error { Unsupported, InvalidAudio, OutOfMemory }

/// A sound file format.
pub enum Format: u8 {
    UNKNOWN,
    WAV,
    AIFF,
    FLAC,
    VORBIS,
    MP3,
    OPUS,
    AAC,
}

/// The format of encoded data, from its first bytes.
pub fn detect(data: []u8) Format {
    if wav.is_wav(data) {
        return Format.WAV;
    }
    if aiff.is_aiff(data) {
        return Format.AIFF;
    }
    if flac.is_flac(data) {
        return Format.FLAC;
    }
    if vorbis.is_vorbis(data) {
        return Format.VORBIS;
    }
    if opus.is_opus(data) {
        return Format.OPUS;
    }
    if aac.is_aac(data) {
        return Format.AAC;
    }
    if mp3.is_mp3(data) {
        return Format.MP3;  // last: its test is the loosest
    }
    return Format.UNKNOWN;
}

/// The format for a file name's extension (case-insensitive), or Format.UNKNOWN.
pub fn from_extension(path: []u8) Format {
    let dot = strs.last_index_of_byte(path, '.') catch return Format.UNKNOWN;
    let ext = path[dot + 1..];
    let mut buf: [8]u8 = [0; 8];
    if ext.len > buf.len {
        return Format.UNKNOWN;
    }
    for i in @as(usize, 0)..ext.len {
        buf[i] = strs.to_lower_byte(ext[i]);
    }
    let e = buf[..ext.len];
    if @mem_eq(e, "wav") or @mem_eq(e, "wave") {
        return Format.WAV;
    }
    if @mem_eq(e, "aiff") or @mem_eq(e, "aif") or @mem_eq(e, "aifc") {
        return Format.AIFF;
    }
    if @mem_eq(e, "flac") {
        return Format.FLAC;
    }
    if @mem_eq(e, "ogg") or @mem_eq(e, "oga") {
        return Format.VORBIS;
    }
    if @mem_eq(e, "mp3") {
        return Format.MP3;
    }
    if @mem_eq(e, "opus") {
        return Format.OPUS;
    }
    if @mem_eq(e, "aac") or @mem_eq(e, "adts") or @mem_eq(e, "m4a") or @mem_eq(e, "m4b") or @mem_eq(e, "mp4") {
        return Format.AAC;
    }
    return Format.UNKNOWN;
}

/// Decodes a sound in any supported format.
pub fn decode(data: []u8) !audio.Sound {
    let f = detect(data);
    if f == Format.WAV {
        return wav.decode(data);
    }
    if f == Format.AIFF {
        return aiff.decode(data);
    }
    if f == Format.FLAC {
        return flac.decode(data);
    }
    if f == Format.VORBIS {
        return vorbis.decode(data);
    }
    if f == Format.MP3 {
        return mp3.decode(data);
    }
    if f == Format.OPUS {
        return opus.decode(data);
    }
    if f == Format.AAC {
        return aac.decode(data);
    }
    return error.Unsupported;
}

/// Decodes a sound in any supported format, read to the end from r.
pub fn decode_from(r: io.Reader) !audio.Sound {
    let data = r.read_all()!;
    defer mem.free(data);
    return decode(data);
}

/// Encodes a sound (free the result with mem.free); WAV, AIFF or FLAC.
pub fn encode(s: *audio.Sound, format: Format) ![]mut u8 {
    if format == Format.WAV {
        return wav.encode(s);
    }
    if format == Format.AIFF {
        return aiff.encode(s);
    }
    if format == Format.FLAC {
        return flac.encode(s, flac.DEFAULT);
    }
    return error.Unsupported;
}

/// Encodes a sound to w; WAV, AIFF or FLAC (see each module's encode_to
/// for how much it holds in memory).
pub fn encode_to(w: io.Writer, s: *audio.Sound, format: Format) !void {
    if format == Format.WAV {
        return wav.encode_to(w, s);
    }
    if format == Format.AIFF {
        return aiff.encode_to(w, s);
    }
    if format == Format.FLAC {
        return flac.encode_to(w, s, flac.DEFAULT);
    }
    return error.Unsupported;
}

/// Reads and decodes a sound file.
pub fn load(path: []u8) !audio.Sound {
    let f = io.open(path)!;
    defer f.close();
    return decode_from(f.reader());
}

/// Encodes a sound in the format its file name's extension names, and
/// writes the file as it is encoded. If that fails, the file is removed.
pub fn save(path: []u8, s: *audio.Sound) !void {
    let format = from_extension(path);
    if (format != Format.WAV) and (format != Format.AIFF) and (format != Format.FLAC) {
        return error.Unsupported;
    }
    let f = io.create(path)!;
    write_file(f, s, format) catch |e| {
        f.close();
        fs.remove(path) catch {};
        return e;
    };
    f.close();
}

fn write_file(f: io.File, s: *audio.Sound, format: Format) !void {
    if format == Format.FLAC {
        // flac.encode_to encodes twice (its header needs the frame sizes
        // and MD5 first); in memory it is one pass
        let b = flac.encode(s, flac.DEFAULT)!;
        defer mem.free(b);
        f.writer().write_all(b)!;
        return;
    }
    let mut bw = io.BufWriter.init(f.writer());
    encode_to(bw.writer(), s, format)!;
    bw.flush()!;
}

test "every writable format" {
    let mut s = audio.Sound.init(300, 2, 32000, audio.SampleFormat.S16)!;
    defer s.deinit();
    for i in @as(usize, 0)..s.len() {
        s.set_sample(i, @float_from_int(f64, (i * 37) % 200) / 100.0 - 1.0);
    }
    let fmts: [3]Format = [Format.WAV, Format.AIFF, Format.FLAC];
    for f in fmts {
        let enc = encode(&s, f)!;
        defer mem.free(enc);
        @expect(detect(enc) == f);
        let mut back = decode(enc)!;
        defer back.deinit();
        @expect_eq(back.data[..], s.data[..]);
        let mut sw = strs.String.init();
        defer sw.deinit();
        encode_to(sw.writer(), &s, f)!;
        @expect_eq(sw.bytes(), enc);
        let mut sr = io.SliceReader.init(enc);
        let mut back2 = decode_from(sr.reader())!;
        defer back2.deinit();
        @expect_eq(back2.data[..], s.data[..]);
    }
    @expect(from_extension("a/b.Flac") == Format.FLAC);
    @expect(from_extension("x.ogg") == Format.VORBIS);
    @expect(from_extension("x.MP3") == Format.MP3);
    @expect(from_extension("x.opus") == Format.OPUS);
    @expect(from_extension("x.m4a") == Format.AAC);
    @expect(from_extension("noext") == Format.UNKNOWN);
}

test "save and load" {
    let mut s = audio.Sound.init(20000, 2, 32000, audio.SampleFormat.S24)!;
    defer s.deinit();
    for i in @as(usize, 0)..s.len() {
        s.set_sample(i, @float_from_int(f64, (i * 37) % 200) / 100.0 - 1.0);
    }
    let dir = fs.make_temp_dir("audio")!;
    defer mem.free(dir);
    let names: [3][]u8 = ["/a.wav", "/a.aiff", "/a.flac"];
    let fmts: [3]Format = [Format.WAV, Format.AIFF, Format.FLAC];
    for k in @as(usize, 0)..3 {
        let mut p = strs.String.from(dir)!;
        defer p.deinit();
        p.push_str(names[k])!;
        save(p.bytes(), &s)!;
        let file = fs.read_file(p.bytes())!;
        defer mem.free(file);
        let enc = encode(&s, fmts[k])!;
        defer mem.free(enc);
        @expect_eq(file, enc);
        let mut back = load(p.bytes())!;
        defer back.deinit();
        @expect_eq(back.data[..], s.data[..]);
    }
    fs.remove_all(dir)!;
}
