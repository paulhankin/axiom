// =============================================================================
// std/log.ph - structured logging: levels, key-value fields, text (logfmt)
// or JSON lines, loggers with fields of their own, rotating log files.
//
//     log.info("server started", [log.str("addr", "0.0.0.0"), log.int("port", 8080)]);
//     // time=2026-09-29T06:40:12.5Z level=INFO msg="server started" addr=0.0.0.0 port=8080
//
//     let mut lg = log.Logger.init(some_writer, log.DEBUG, log.JSON);
//     let mut req = lg.with([log.str("request", id)][..])!;    // adds fields to every line
//     defer req.deinit();
//     req.warn("slow", [log.duration("took", d)]);
//     // {"time":"...","level":"WARN","msg":"slow","request":"...","took":"1.5s"}
//
// The module-level functions (debug, info, warn, err) use the default
// logger: standard error, level INFO, text; set_level, set_format,
// set_output and level_from_env configure it. Each line is formatted in
// memory and written with one call under a lock, so lines from different
// threads never interleave. Logging never fails: a line that cannot be
// written is dropped.
// =============================================================================

import "collections/vec.ph" as vec;
import "fmt.ph" as fmt;
import "io.ph" as io;
import "mem.ph" as mem;
import "os.ph" as os;
import "os/fs.ph" as fs;
import "str.ph" as strs;
import "sync.ph" as sync;
import "thread.ph" as thread;
import "time.ph" as time;

error { OutOfMemory, IoError, BadLevel }

/// Levels. Others between them are allowed (INFO + 2 prints as INFO+2).
pub const DEBUG: i32 = -4;
pub const INFO: i32 = 0;
pub const WARN: i32 = 4;
pub const ERROR: i32 = 8;

/// Output formats.
pub const TEXT: u8 = 0;
pub const JSON: u8 = 1;

union Value {
    Str: []u8,
    Int: i64,
    Uint: u64,
    Float: f64,
    Bool: bool,
    Err: error,
    Dur: time.Duration,
    Time: time.Time,
}

/// A key and a value. Fields borrow their strings: they must live until the
/// line is logged (for a Logger's own fields, as long as the Logger).
pub struct Field {
    key: []u8,
    value: Value,
}

pub fn str(key: []u8, v: []u8) Field {
    return Field.{ key: key, value: Value.Str(v) };
}

pub fn int(key: []u8, v: i64) Field {
    return Field.{ key: key, value: Value.Int(v) };
}

pub fn uint(key: []u8, v: u64) Field {
    return Field.{ key: key, value: Value.Uint(v) };
}

pub fn float(key: []u8, v: f64) Field {
    return Field.{ key: key, value: Value.Float(v) };
}

pub fn boolean(key: []u8, v: bool) Field {
    return Field.{ key: key, value: Value.Bool(v) };
}

/// An error, under the key "error".
pub fn error_value(e: error) Field {
    return Field.{ key: "error", value: Value.Err(e) };
}

pub fn duration(key: []u8, d: time.Duration) Field {
    return Field.{ key: key, value: Value.Dur(d) };
}

pub fn at(key: []u8, t: time.Time) Field {
    return Field.{ key: key, value: Value.Time(t) };
}

/// A level's name: DEBUG, INFO, WARN, ERROR, or one of them with an
/// offset (INFO+2, DEBUG-1).
pub fn level_name(level: i32, buf: []mut u8) []u8 {
    let mut base = ERROR;
    let mut name = "ERROR";
    if level < INFO {
        base = DEBUG;
        name = "DEBUG";
    } else if level < WARN {
        base = INFO;
        name = "INFO";
    } else if level < ERROR {
        base = WARN;
        name = "WARN";
    }
    if level == base {
        return name;
    }
    let mut bw = io.BufferWriter.init(buf);
    let mut sign = "+";
    if level < base {
        sign = "-";
    }
    let off = @abs(level - base);
    fmt.write(bw.writer(), "{}{}{}", [fmt.str(name), fmt.str(sign), fmt.int(off)]) catch return name;
    return bw.written();
}

/// A level from its name (any case; "warning" too) or a number.
pub fn parse_level(s: []u8) !i32 {
    if strs.eq_ignore_case(s, "debug") {
        return DEBUG;
    }
    if strs.eq_ignore_case(s, "info") {
        return INFO;
    }
    if strs.eq_ignore_case(s, "warn") or strs.eq_ignore_case(s, "warning") {
        return WARN;
    }
    if strs.eq_ignore_case(s, "error") {
        return ERROR;
    }
    return fmt.parse_int.[i32](s) catch return error.BadLevel;
}

// One lock for all output: a line is written whole.
let mut out_lock: sync.Mutex = sync.Mutex.{ state: 0 };

pub struct Logger {
    out: io.Writer,
    level: i32,
    format: u8,
    with_time: bool,      // start each line with the time (default true)
    fields: []mut Field,  // added to every line (owned: see with)

    pub fn init(out: io.Writer, level: i32, format: u8) Logger {
        return Logger.{ out: out, level: level, format: format, with_time: true, fields: empty_fields() };
    }

    /// Frees the fields array of a logger made by with.
    pub fn deinit(self: *mut Logger) void {
        if self.fields.len > 0 {
            mem.free(self.fields);
        }
        self.fields = empty_fields();
    }

    pub fn enabled(self: *Logger, level: i32) bool {
        return level >= self.level;
    }

    /// A logger that adds fields to every line (after this logger's own);
    /// free it with deinit. It borrows the fields' strings.
    pub fn with(self: *Logger, fields: []Field) !Logger {
        let all = mem.alloc.[Field](self.fields.len + fields.len)!;
        @mem_copy(all[..self.fields.len], self.fields);
        @mem_copy(all[self.fields.len..], fields);
        let mut l = self.*;
        l.fields = all;
        return l;
    }

    pub fn log[const N: usize](self: *Logger, level: i32, msg: []u8, fields: [N]Field) void {
        if level < self.level {
            return;
        }
        self.emit(level, msg, fields[..]);
    }

    pub fn debug[const N: usize](self: *Logger, msg: []u8, fields: [N]Field) void {
        self.log(DEBUG, msg, fields);
    }

    pub fn info[const N: usize](self: *Logger, msg: []u8, fields: [N]Field) void {
        self.log(INFO, msg, fields);
    }

    pub fn warn[const N: usize](self: *Logger, msg: []u8, fields: [N]Field) void {
        self.log(WARN, msg, fields);
    }

    pub fn err[const N: usize](self: *Logger, msg: []u8, fields: [N]Field) void {
        self.log(ERROR, msg, fields);
    }

    /// Logs a line with fields given as a slice.
    pub fn emit(self: *Logger, level: i32, msg: []u8, fields: []Field) void {
        if level < self.level {
            return;
        }
        let mut line = strs.String.init();
        defer line.deinit();
        format_line(&mut line, self, level, msg, fields) catch return;
        out_lock.lock();
        self.out.write_all(line.bytes()) catch {};
        out_lock.unlock();
    }
}

fn empty_fields() []mut Field {
    return @slice_from_ptr(@ptr_from_int(*mut Field, 8), 0);
}

fn format_line(line: *mut strs.String, l: *Logger, level: i32, msg: []u8, fields: []Field) !void {
    let w = line.writer();
    let mut lb: [24]u8 = [0; 24];
    let lname = level_name(level, lb[..]);
    let mut tb: [48]u8 = [0; 48];
    let mut ts: []u8 = tb[..0];
    if l.with_time {
        ts = time.now().format_rfc3339(tb[..]);
    }
    if l.format == JSON {
        line.push('{')!;
        let mut first = true;
        if ts.len > 0 {
            line.push_str("\"time\":")!;
            json_string(line, ts)!;
            first = false;
        }
        if not first {
            line.push(',')!;
        }
        line.push_str("\"level\":")!;
        json_string(line, lname)!;
        line.push_str(",\"msg\":")!;
        json_string(line, msg)!;
        for f in l.fields {
            json_field(line, &f)!;
        }
        for f in fields {
            json_field(line, &f)!;
        }
        line.push_str("}\n")!;
        return;
    }
    if ts.len > 0 {
        line.push_str("time=")!;
        line.push_str(ts)!;
        line.push(' ')!;
    }
    line.push_str("level=")!;
    line.push_str(lname)!;
    line.push_str(" msg=")!;
    text_value(line, msg)!;
    for f in l.fields {
        text_field(line, &f)!;
    }
    for f in fields {
        text_field(line, &f)!;
    }
    line.push('\n')!;
    _ = w;
}

/// The value as text (strings unquoted; the caller quotes).
fn value_text(v: Value, line: *mut strs.String, raw: *mut bool) !void {
    let w = line.writer();
    raw.* = false;
    match v {
        .Str(s) => { line.push_str(s)!; }
        .Int(x) => {
            fmt.write(w, "{}", [fmt.int(x)])!;
            raw.* = true;
        }
        .Uint(x) => {
            fmt.write(w, "{}", [fmt.int(x)])!;
            raw.* = true;
        }
        .Float(x) => {
            fmt.write(w, "{}", [fmt.float(x)])!;
            raw.* = (x == x) and (@abs(x) != (1.0 / 0.0));  // JSON has no NaN or infinity
        }
        .Bool(b) => {
            if b {
                line.push_str("true")!;
            } else {
                line.push_str("false")!;
            }
            raw.* = true;
        }
        .Err(e) => { line.push_str(@error_name(e))!; }
        .Dur(d) => {
            let mut b: [64]u8 = [0; 64];
            line.push_str(d.format(b[..]))!;
        }
        .Time(t) => {
            let mut b: [48]u8 = [0; 48];
            line.push_str(t.format_rfc3339(b[..]))!;
        }
    }
}

fn text_field(line: *mut strs.String, f: *Field) !void {
    line.push(' ')!;
    text_key(line, f.key)!;
    line.push('=')!;
    let start = line.len;
    let mut raw = false;
    value_text(f.value, line, &mut raw)!;
    if not raw and needs_quotes(line.bytes()[start..]) {
        let copy = mem.dup(line.bytes()[start..])!;
        defer mem.free(copy);
        line.truncate(start);
        quote(line, copy)!;
    }
}

/// A key: characters that would break the key=value syntax become '_'.
fn text_key(line: *mut strs.String, k: []u8) !void {
    if k.len == 0 {
        line.push('_')!;
        return;
    }
    let mut i: usize = 0;
    while i < k.len {
        let c = k[i];
        if c >= 0x80 {
            // (invalid UTF-8 and line-breaking characters become '_' too)
            let r = strs.decode_rune(k[i..]);
            if ((r.value == strs.REPLACEMENT) and (r.len == 1)) or breaks_lines(r.value) {
                line.push('_')!;
            } else {
                line.push_str(k[i..i + r.len])!;
            }
            i += r.len;
            continue;
        }
        if (c <= ' ') or (c == '=') or (c == '"') or (c == 127) {
            line.push('_')!;
        } else {
            line.push(c)!;
        }
        i += 1;
    }
}

fn text_value(line: *mut strs.String, s: []u8) !void {
    if needs_quotes(s) {
        quote(line, s)!;
    } else {
        line.push_str(s)!;
    }
}

fn needs_quotes(s: []u8) bool {
    if s.len == 0 {
        return true;
    }
    let mut high = false;
    for c in s {
        if (c <= ' ') or (c == '=') or (c == '"') or (c == 127) or (c == '\\') {
            return true;
        }
        if c >= 0x80 {
            high = true;
        }
    }
    if not high {
        return false;
    }
    if not strs.valid_utf8(s) {
        return true;
    }
    let mut it = strs.runes(s);
    while true {
        let r = it.next() catch break;
        if breaks_lines(r) {
            return true;
        }
    }
    return false;
}

/// C1 controls (NEL, CSI, ...) and the Unicode line and paragraph
/// separators: terminals and log viewers act on them, so they are escaped.
fn breaks_lines(cp: u32) bool {
    return ((cp >= 0x80) and (cp <= 0x9f)) or (cp == 0x2028) or (cp == 0x2029);
}

/// A double-quoted string with JSON's escapes (invalid UTF-8 as �).
fn quote(line: *mut strs.String, s: []u8) !void {
    json_string(line, s)!;
}

fn json_string(line: *mut strs.String, s: []u8) !void {
    let hex = "0123456789abcdef";
    line.push('"')!;
    let mut i: usize = 0;
    while i < s.len {
        let c = s[i];
        if c >= 0x80 {
            let r = strs.decode_rune(s[i..]);
            if (r.value == strs.REPLACEMENT) and (r.len == 1) {
                line.push_str("\\ufffd")!;
            } else if breaks_lines(r.value) {
                line.push_str("\\u")!;
                for k in @as(usize, 0)..4 {
                    line.push(hex[@as(usize, (r.value >> @truncate(u32, 12 - (4 * k))) & 15)])!;
                }
            } else {
                line.push_str(s[i..i + r.len])!;
            }
            i += r.len;
            continue;
        }
        if c == '"' {
            line.push_str("\\\"")!;
        } else if c == '\\' {
            line.push_str("\\\\")!;
        } else if c == '\n' {
            line.push_str("\\n")!;
        } else if c == '\r' {
            line.push_str("\\r")!;
        } else if c == '\t' {
            line.push_str("\\t")!;
        } else if (c < 0x20) or (c == 0x7f) {
            line.push_str("\\u00")!;
            line.push(hex[@as(usize, c >> 4)])!;
            line.push(hex[@as(usize, c & 15)])!;
        } else {
            line.push(c)!;
        }
        i += 1;
    }
    line.push('"')!;
}

fn json_field(line: *mut strs.String, f: *Field) !void {
    line.push(',')!;
    json_string(line, f.key)!;
    line.push(':')!;
    let start = line.len;
    let mut raw = false;
    value_text(f.value, line, &mut raw)!;
    if not raw {
        let copy = mem.dup(line.bytes()[start..])!;
        defer mem.free(copy);
        line.truncate(start);
        json_string(line, copy)!;
    }
}

// -----------------------------------------------------------------------------
// The default logger
// -----------------------------------------------------------------------------

let mut default_level: i32 = INFO;
let mut default_format: u8 = TEXT;
let mut default_time: bool = true;
let mut default_out: io.Writer = io.Writer.{ ctx: @ptr_from_int(*mut void, 0), write_fn: stderr_write };

fn stderr_write(ctx: *mut void, bytes: []u8) !usize {
    _ = ctx;
    return io.stderr().write(bytes);
}

/// The default logger (writing to standard error unless set_output).
pub fn default() Logger {
    let mut l = Logger.init(default_out, default_level, default_format);
    l.with_time = default_time;
    return l;
}

pub fn set_level(level: i32) void {
    default_level = level;
}

pub fn set_format(format: u8) void {
    default_format = format;
}

/// Whether default lines start with the time.
pub fn set_time(on: bool) void {
    default_time = on;
}

pub fn set_output(w: io.Writer) void {
    default_out = w;
}

/// Sets the default level from an environment variable (such as
/// LOG_LEVEL), if it is set and valid; returns whether it was.
pub fn level_from_env(name: []u8) bool {
    let v = os.getenv(name) catch return false;
    defer mem.free(v);
    let l = parse_level(v) catch return false;
    default_level = l;
    return true;
}

pub fn debug[const N: usize](msg: []u8, fields: [N]Field) void {
    let l = default();
    l.log(DEBUG, msg, fields);
}

pub fn info[const N: usize](msg: []u8, fields: [N]Field) void {
    let l = default();
    l.log(INFO, msg, fields);
}

pub fn warn[const N: usize](msg: []u8, fields: [N]Field) void {
    let l = default();
    l.log(WARN, msg, fields);
}

pub fn err[const N: usize](msg: []u8, fields: [N]Field) void {
    let l = default();
    l.log(ERROR, msg, fields);
}

// -----------------------------------------------------------------------------
// Rotating log files
// -----------------------------------------------------------------------------

/// A log file that is rotated when it would grow past max_bytes: path
/// becomes path.1, path.1 becomes path.2, ..., keeping `keep` old files.
pub struct FileSink {
    path: []mut u8,
    file: io.File,
    size: u64,
    max_bytes: u64,  // 0: never rotate
    keep: u32,
    lock: sync.Mutex,
    is_open: bool,  // false after a rotation failed to create the new file

    pub fn writer(self: *mut FileSink) io.Writer {
        return io.Writer.{ ctx: @ptr_cast(*mut void, self), write_fn: sink_write };
    }

    pub fn close(self: *mut FileSink) void {
        if self.is_open {
            self.file.close();
        }
        mem.free(self.path);
        mem.free(@slice_from_ptr(self, 1));
    }

    fn rotate(self: *mut FileSink) !void {
        if self.is_open {
            self.file.close();
            self.is_open = false;  // (its descriptor may be reused by anyone now)
        }
        let mut k = self.keep;
        while k > 0 {
            let from = numbered(self.path, k - 1)!;
            defer mem.free(from);
            let to = numbered(self.path, k)!;
            defer mem.free(to);
            rename_quiet(from, to);
            k -= 1;
        }
        if self.keep == 0 {
            remove_quiet(self.path);
        }
        self.file = io.create(self.path)!;
        self.is_open = true;
        self.size = 0;
    }
}

import host "phalange" {
    fn ph_rename(from: *u8, flen: usize, to: *u8, tlen: usize) i32;
    fn ph_remove(path: *u8, len: usize, dir: u32) i32;
}

fn rename_quiet(from: []u8, to: []u8) void {
    _ = ph_rename(from.ptr, from.len, to.ptr, to.len);
}

fn remove_quiet(p: []u8) void {
    _ = ph_remove(p.ptr, p.len, 0);
}

/// path.k (path itself for k = 0).
fn numbered(p: []u8, k: u32) ![]mut u8 {
    if k == 0 {
        return mem.dup(p);
    }
    return fmt.alloc("{}.{}", [fmt.str(p), fmt.int(k)]);
}

fn sink_write(ctx: *mut void, bytes: []u8) !usize {
    let s = @ptr_cast(*mut FileSink, ctx);
    s.lock.lock();
    defer s.lock.unlock();
    if not s.is_open {
        // (the last rotation could not create the file: try again)
        s.file = io.append(s.path)!;
        s.is_open = true;
        s.size = s.file.size() catch 0;
    }
    if (s.max_bytes > 0) and (s.size > 0) and ((s.size + @as(u64, bytes.len)) > s.max_bytes) {
        s.rotate()!;
    }
    s.file.writer().write_all(bytes)!;
    s.size += @as(u64, bytes.len);
    return bytes.len;
}

/// Opens (appending to) a log file; close it with close().
pub fn open_file(p: []u8, max_bytes: u64, keep: u32) !*mut FileSink {
    let f = io.append(p) catch return error.IoError;
    let size = f.size() catch 0;
    let mem_ = mem.alloc.[FileSink](1) catch |e| {
        f.close();
        return e;
    };
    let path_copy = mem.dup(p) catch |e| {
        f.close();
        mem.free(mem_);
        return e;
    };
    mem_[0] = FileSink.{
        path: path_copy,
        file: f,
        size: size,
        max_bytes: max_bytes,
        keep: keep,
        lock: sync.Mutex.init(),
        is_open: true,
    };
    return &mut mem_[0];
}

// -----------------------------------------------------------------------------
// Tests
// -----------------------------------------------------------------------------

error { Boom }

test "text lines" {
    let mut out = strs.String.init();
    defer out.deinit();
    let mut l = Logger.init(out.writer(), INFO, TEXT);
    l.with_time = false;
    l.info("server started", [str("addr", "0.0.0.0"), int("port", 8080), boolean("tls", true)]);
    l.debug("hidden", [int("x", 1)]);
    l.warn("odd \"value\"", [str("path", "a b"), str("empty", ""), float("ratio", 0.25), error_value(error.Boom)]);
    l.log(INFO + 2, "between", [uint("n", 3)]);
    let want =
        "level=INFO msg=\"server started\" addr=0.0.0.0 port=8080 tls=true\nlevel=WARN msg=\"odd \\\"value\\\"\" path=\"a b\" empty=\"\" ratio=0.25 error=Boom\nlevel=INFO+2 msg=between n=3\n";
    @expect_eq(out.bytes(), want);
}

test "json lines and loggers with fields" {
    let mut out = strs.String.init();
    defer out.deinit();
    let mut l = Logger.init(out.writer(), DEBUG, JSON);
    l.with_time = false;
    let extra: [1]Field = [str("request", "r-1")];
    let mut req = l.with(extra[..])!;
    defer req.deinit();
    req.debug(
        "step", [duration("took", time.Duration.milliseconds(1500)), float("nan", 0.0 / 0.0), str("ctl", "a\nb\x01")]);
    l.err("plain", [int("code", -3)]);
    let want =
        "{\"level\":\"DEBUG\",\"msg\":\"step\",\"request\":\"r-1\",\"took\":\"1.5s\",\"nan\":\"NaN\",\"ctl\":\"a\\nb\\u0001\"}\n{\"level\":\"ERROR\",\"msg\":\"plain\",\"code\":-3}\n";
    @expect_eq(out.bytes(), want);
}

test "levels" {
    @expect_eq(parse_level("Warning")!, WARN);
    @expect_eq(parse_level("-4")!, DEBUG);
    let mut b: [24]u8 = [0; 24];
    @expect_eq(level_name(DEBUG - 1, b[..]), "DEBUG-1");
    @expect_eq(level_name(12, b[..]), "ERROR+4");
    let mut out = strs.String.init();
    defer out.deinit();
    set_output(out.writer());
    set_time(false);
    set_level(WARN);
    info("no", [int("x", 1)]);
    warn("yes", [int("x", 2)]);
    set_level(INFO);
    set_time(true);
    @expect_eq(out.bytes(), "level=WARN msg=yes x=2\n");
}

test "rotating files" {
    let dir = fs.make_temp_dir("phalange-log-")!;
    defer mem.free(dir);
    defer fs.remove_all(dir) catch {};
    let p = fmt.alloc("{}/app.log", [fmt.str(dir)])!;
    defer mem.free(p);
    let sink = open_file(p, 100, 2)!;
    let mut l = Logger.init(sink.writer(), INFO, TEXT);
    l.with_time = false;
    for i in @as(i64, 0)..12 {
        l.info("line", [int("i", i)]);  // 22 bytes: four to a file
    }
    sink.close();
    let cur = fs.read_file(p)!;
    defer mem.free(cur);
    @expect_eq(cur,
               "level=INFO msg=line i=8\nlevel=INFO msg=line i=9\nlevel=INFO msg=line i=10\nlevel=INFO msg=line i=11\n");
    let p1 = fmt.alloc("{}.1", [fmt.str(p)])!;
    defer mem.free(p1);
    let one = fs.read_file(p1)!;
    defer mem.free(one);
    @expect(strs.starts_with(one, "level=INFO msg=line i=4\n"));
    let p3 = fmt.alloc("{}.3", [fmt.str(p)])!;
    defer mem.free(p3);
    @expect(not fs.exists(p3));
}

fn log_many(l: *Logger) void {
    for i in @as(i64, 0)..200 {
        l.info("from a thread", [int("i", i), str("pad", "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx")]);
    }
}

test "lines from threads do not interleave" {
    let mut out = strs.String.init();
    defer out.deinit();
    let mut l = Logger.init(out.writer(), INFO, TEXT);
    l.with_time = false;
    let mut ts = vec.Vec.[thread.Thread.[*Logger]].init();
    defer ts.deinit();
    for _k in @as(usize, 0)..4 {
        ts.push(thread.spawn(log_many, &l)!)!;
    }
    for t in ts.items() {
        t.join();
    }
    let mut it = strs.lines(out.bytes());
    let mut n: usize = 0;
    loop {
        let line = it.next() catch break;
        @expect(strs.starts_with(line, "level=INFO msg=\"from a thread\" i="));
        @expect(strs.ends_with(line, " pad=xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"));
        n += 1;
    }
    @expect_eq(n, 800);
}

test "line-breaking characters are escaped" {
    let mut buf = strs.String.init();
    defer buf.deinit();
    let mut lg = Logger.init(buf.writer(), DEBUG, TEXT);
    lg.with_time = false;
    lg.info("m", [str("k\xe2\x80\xa8", "a\xc2\x85b\xe2\x80\xa9c")]);
    @expect(strs.contains(buf.bytes(), "k_=\"a\\u0085b\\u2029c\""));
    lg.format = JSON;
    buf.clear();
    lg.info("m", [str("k", "\xc2\x9b31m")]);
    @expect(strs.contains(buf.bytes(), "\"k\":\"\\u009b31m\""));
}
