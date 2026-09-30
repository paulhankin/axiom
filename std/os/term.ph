// =============================================================================
// std/os/term.ph - terminals: detection, size, raw mode, styles, keys.
//
//     let color = term.color_enabled(term.STDOUT);
//     let red = term.Style.fg(term.Color.basic(term.BasicColor.RED)).bold();
//     red.write(w, "error", color)!;                 // plain when !color
//     let sz = term.size(term.STDOUT)!;              // sz.cols, sz.rows
//
//     term.enable_raw(term.STDIN)!;                  // keys as they are typed
//     defer term.disable_raw(term.STDIN);
//     let n = stdin.read(buf[..])!;
//     let k = term.decode_key(buf[..n])!;            // k.code (a KeyCode), k.rune, k.mods, k.len
//
// Output uses ANSI (ECMA-48) escape sequences, which every current
// terminal understands. color_enabled follows the usual conventions:
// NO_COLOR set (to anything) turns colour off, FORCE_COLOR turns it on,
// and otherwise it needs a terminal whose TERM is not "dumb".
//
// On the node host only stdout's and stderr's size is known, and only the
// main thread can put stdin in raw mode.
// =============================================================================

import "../io.ph" as io;
import "../mem.ph" as mem;
import "../os.ph" as os;
import "../str.ph" as strs;

error { NotATerminal, Incomplete, OutOfMemory }

import host "phalange" {
    fn ph_isatty(fd: i32) i32;
    fn ph_term_size(fd: i32, out: *mut u32) i32;
    fn ph_term_raw(fd: i32, enable: u32) i32;
}

pub const STDIN: i32 = 0;
pub const STDOUT: i32 = 1;
pub const STDERR: i32 = 2;

/// Whether the file descriptor is a terminal.
pub fn is_tty(fd: i32) bool {
    return ph_isatty(fd) == 1;
}

pub struct Size {
    cols: u32,
    rows: u32,
}

/// The terminal's size in character cells.
pub fn size(fd: i32) !Size {
    let mut v: [2]u32 = [0, 0];
    if ph_term_size(fd, &mut v[0]) != 0 {
        return error.NotATerminal;
    }
    return Size.{ cols: v[0], rows: v[1] };
}

/// Puts the terminal in raw mode: input arrives a byte at a time, without
/// echo or line editing, and Ctrl-C is a byte rather than a signal.
pub fn enable_raw(fd: i32) !void {
    if ph_term_raw(fd, 1) != 0 {
        return error.NotATerminal;
    }
}

/// Restores the settings from before enable_raw.
pub fn disable_raw(fd: i32) void {
    _ = ph_term_raw(fd, 0);
}

/// Whether to write colour to fd (see the header).
pub fn color_enabled(fd: i32) bool {
    if os.has_env("NO_COLOR") {
        return false;
    }
    if os.has_env("FORCE_COLOR") {
        return true;
    }
    if not is_tty(fd) {
        return false;
    }
    let t = os.getenv("TERM") catch return true;
    defer mem.free(t);
    return not strs.eq(t, "dumb");
}

// -----------------------------------------------------------------------------
// Styles
// -----------------------------------------------------------------------------

/// The 8 basic colours (each also has a bright form: Color.bright).
pub enum BasicColor: u8 {
    BLACK = 0,
    RED = 1,
    GREEN = 2,
    YELLOW = 3,
    BLUE = 4,
    MAGENTA = 5,
    CYAN = 6,
    WHITE = 7,
}

/// Which kind of colour a Color is.
pub enum ColorKind: u8 {
    NONE = 0,  // the terminal's default
    BASIC = 1,
    INDEXED = 2,
    RGB = 3,
}

/// A colour: none (the terminal's default), one of the 16 basic ones (a
/// BasicColor or its bright form), one of the 256 indexed ones, or RGB.
pub struct Color {
    kind: ColorKind,
    r: u8,  // the index (bright basic colours are 8-15), or red
    g: u8,
    b: u8,

    pub fn none() Color {
        return Color.{ kind: ColorKind.NONE, r: 0, g: 0, b: 0 };
    }

    pub fn basic(c: BasicColor) Color {
        return Color.{ kind: ColorKind.BASIC, r: @int_from_enum(c), g: 0, b: 0 };
    }

    pub fn bright(c: BasicColor) Color {
        return Color.{ kind: ColorKind.BASIC, r: @int_from_enum(c) + 8, g: 0, b: 0 };
    }

    pub fn indexed(n: u8) Color {
        return Color.{ kind: ColorKind.INDEXED, r: n, g: 0, b: 0 };
    }

    pub fn rgb(r: u8, g: u8, b: u8) Color {
        return Color.{ kind: ColorKind.RGB, r: r, g: g, b: b };
    }
}

/// Text attributes and colours.
pub struct Style {
    fg_: Color,
    bg_: Color,
    attrs: u8,  // 1 bold, 2 dim, 4 italic, 8 underline, 16 inverse, 32 strike

    pub fn plain() Style {
        return Style.{ fg_: Color.none(), bg_: Color.none(), attrs: 0 };
    }

    pub fn fg(c: Color) Style {
        return Style.{ fg_: c, bg_: Color.none(), attrs: 0 };
    }

    pub fn on(self: Style, c: Color) Style {
        return Style.{ fg_: self.fg_, bg_: c, attrs: self.attrs };
    }

    pub fn bold(self: Style) Style {
        return self.with(1);
    }

    pub fn dim(self: Style) Style {
        return self.with(2);
    }

    pub fn italic(self: Style) Style {
        return self.with(4);
    }

    pub fn underline(self: Style) Style {
        return self.with(8);
    }

    pub fn inverse(self: Style) Style {
        return self.with(16);
    }

    pub fn strike(self: Style) Style {
        return self.with(32);
    }

    fn with(self: Style, a: u8) Style {
        return Style.{ fg_: self.fg_, bg_: self.bg_, attrs: self.attrs | a };
    }

    /// Writes the escape sequence that starts the style.
    pub fn start(self: Style, w: io.Writer) !void {
        let mut buf: [64]u8 = [0; 64];
        let mut n: usize = 0;
        put(buf[..], &mut n, "\x1b[");
        let mut first = true;
        let codes: [6]u8 = [1, 2, 3, 4, 7, 9];
        for k in @as(usize, 0)..6 {
            if (self.attrs & (@as(u8, 1) << @truncate(u8, k))) != 0 {
                sep(buf[..], &mut n, &mut first);
                num(buf[..], &mut n, @as(u32, codes[k]));
            }
        }
        color_codes(buf[..], &mut n, &mut first, self.fg_, 30);
        color_codes(buf[..], &mut n, &mut first, self.bg_, 40);
        if first {
            return;  // nothing to set
        }
        put(buf[..], &mut n, "m");
        w.write_all(buf[..n])!;
    }

    /// Writes text in the style (and resets it after), or just the text
    /// when enabled is false.
    pub fn write(self: Style, w: io.Writer, text: []u8, enabled: bool) !void {
        if enabled and ((self.attrs != 0) or (self.fg_.kind != ColorKind.NONE) or (self.bg_.kind != ColorKind.NONE)) {
            self.start(w)!;
            w.write_all(text)!;
            w.write_all(RESET)!;
        } else {
            w.write_all(text)!;
        }
    }
}

/// Back to the default style.
pub let RESET: []u8 = "\x1b[0m";

fn put(buf: []mut u8, n: *mut usize, s: []u8) void {
    @mem_copy(buf[n.*..n.* + s.len], s);
    n.* += s.len;
}

fn sep(buf: []mut u8, n: *mut usize, first: *mut bool) void {
    if not first.* {
        put(buf, n, ";");
    }
    first.* = false;
}

fn num(buf: []mut u8, n: *mut usize, v: u32) void {
    let mut tmp: [10]u8 = [0; 10];
    let mut k: usize = 0;
    let mut x = v;
    loop {
        tmp[k] = @truncate(u8, x % 10) + '0';
        k += 1;
        x /= 10;
        if x == 0 {
            break;
        }
    }
    while k > 0 {
        k -= 1;
        buf[n.*] = tmp[k];
        n.* += 1;
    }
}

// 30-37/90-97 (40-47/100-107), 38;5;n (48;5;n), 38;2;r;g;b (48;2;r;g;b)
fn color_codes(buf: []mut u8, n: *mut usize, first: *mut bool, c: Color, base: u32) void {
    if c.kind == ColorKind.NONE {
        return;
    }
    sep(buf, n, first);
    if c.kind == ColorKind.BASIC {
        if c.r < 8 {
            num(buf, n, base + @as(u32, c.r));
        } else {
            num(buf, n, (base + 60) + @as(u32, c.r - 8));
        }
    } else if c.kind == ColorKind.INDEXED {
        num(buf, n, base + 8);
        put(buf, n, ";5;");
        num(buf, n, @as(u32, c.r));
    } else {
        num(buf, n, base + 8);
        put(buf, n, ";2;");
        num(buf, n, @as(u32, c.r));
        put(buf, n, ";");
        num(buf, n, @as(u32, c.g));
        put(buf, n, ";");
        num(buf, n, @as(u32, c.b));
    }
}

// -----------------------------------------------------------------------------
// The cursor and the screen
// -----------------------------------------------------------------------------

fn csi(w: io.Writer, a: u32, final: []u8) !void {
    let mut buf: [24]u8 = [0; 24];
    let mut n: usize = 0;
    put(buf[..], &mut n, "\x1b[");
    num(buf[..], &mut n, a);
    put(buf[..], &mut n, final);
    w.write_all(buf[..n])!;
}

/// Moves the cursor to a row and column, counting from 1.
pub fn move_to(w: io.Writer, row: u32, col: u32) !void {
    let mut buf: [32]u8 = [0; 32];
    let mut n: usize = 0;
    put(buf[..], &mut n, "\x1b[");
    num(buf[..], &mut n, row);
    put(buf[..], &mut n, ";");
    num(buf[..], &mut n, col);
    put(buf[..], &mut n, "H");
    w.write_all(buf[..n])!;
}

pub fn move_up(w: io.Writer, n: u32) !void {
    csi(w, n, "A")!;
}

pub fn move_down(w: io.Writer, n: u32) !void {
    csi(w, n, "B")!;
}

pub fn move_right(w: io.Writer, n: u32) !void {
    csi(w, n, "C")!;
}

pub fn move_left(w: io.Writer, n: u32) !void {
    csi(w, n, "D")!;
}

/// Clears the screen and puts the cursor at the top left.
pub fn clear_screen(w: io.Writer) !void {
    w.write_all("\x1b[2J\x1b[H")!;
}

/// Clears the line the cursor is on, and returns to its start.
pub fn clear_line(w: io.Writer) !void {
    w.write_all("\x1b[2K\r")!;
}

pub fn hide_cursor(w: io.Writer) !void {
    w.write_all("\x1b[?25l")!;
}

pub fn show_cursor(w: io.Writer) !void {
    w.write_all("\x1b[?25h")!;
}

/// Switches to the alternate screen (as full-screen programs do), and back.
pub fn enter_alternate_screen(w: io.Writer) !void {
    w.write_all("\x1b[?1049h")!;
}

pub fn leave_alternate_screen(w: io.Writer) !void {
    w.write_all("\x1b[?1049l")!;
}

/// Sets the window's title.
pub fn set_title(w: io.Writer, title: []u8) !void {
    w.write_all("\x1b]0;")!;
    for c in title {
        // (no control characters, which would end the sequence early)
        if (c >= 0x20) and (c != 0x7f) {
            let one: [1]u8 = [c];
            w.write_all(one[..])!;
        }
    }
    w.write_all("\x07")!;
}

// -----------------------------------------------------------------------------
// Keys
// -----------------------------------------------------------------------------

/// Key codes: CHAR (a character, in rune), or a named key.
pub enum KeyCode: u16 {
    CHAR = 0,
    ENTER = 1,
    TAB = 2,
    BACKSPACE = 3,
    ESCAPE = 4,
    UP = 5,
    DOWN = 6,
    RIGHT = 7,
    LEFT = 8,
    HOME = 9,
    END = 10,
    PAGE_UP = 11,
    PAGE_DOWN = 12,
    INSERT = 13,
    DELETE = 14,
    F1 = 21,
    F2 = 22,
    F3 = 23,
    F4 = 24,
    F5 = 25,
    F6 = 26,
    F7 = 27,
    F8 = 28,
    F9 = 29,
    F10 = 30,
    F11 = 31,
    F12 = 32,
    UNKNOWN = 99,
}

// F1 to F12, in order.
let FKEYS: [12]KeyCode = [
    KeyCode.F1,
    KeyCode.F2,
    KeyCode.F3,
    KeyCode.F4,
    KeyCode.F5,
    KeyCode.F6,
    KeyCode.F7,
    KeyCode.F8,
    KeyCode.F9,
    KeyCode.F10,
    KeyCode.F11,
    KeyCode.F12,
];

/// Modifier bits.
pub const SHIFT: u8 = 1;
pub const ALT: u8 = 2;
pub const CTRL: u8 = 4;

pub struct Key {
    code: KeyCode,
    rune: u32,  // for KeyCode.CHAR: the character (with CTRL, the letter: Ctrl-A is 'a')
    mods: u8,
    len: usize,  // the bytes it took
}

/// The first key in bytes read from a terminal in raw mode. Incomplete if
/// they end in the middle of one (read more and try again; a lone ESC is
/// the Escape key, so call with the bytes so far after a short wait).
pub fn decode_key(b: []u8) !Key {
    if b.len == 0 {
        return error.Incomplete;
    }
    let c = b[0];
    if c == 0x1b {
        if b.len == 1 {
            return Key.{ code: KeyCode.ESCAPE, rune: 0, mods: 0, len: 1 };
        }
        if (b[1] == '[') or (b[1] == 'O') {
            return escape_sequence(b);
        }
        // Alt and a key
        let mut k = decode_key(b[1..])!;
        k.mods |= ALT;
        k.len += 1;
        return k;
    }
    if (c == '\r') or (c == '\n') {
        return Key.{ code: KeyCode.ENTER, rune: 0, mods: 0, len: 1 };
    }
    if c == '\t' {
        return Key.{ code: KeyCode.TAB, rune: 0, mods: 0, len: 1 };
    }
    if (c == 0x7f) or (c == 0x08) {
        return Key.{ code: KeyCode.BACKSPACE, rune: 0, mods: 0, len: 1 };
    }
    if c == 0 {
        return Key.{ code: KeyCode.CHAR, rune: ' ', mods: CTRL, len: 1 };
    }
    if c < 0x20 {
        // Ctrl-A to Ctrl-Z (and a few more)
        return Key.{ code: KeyCode.CHAR, rune: @as(u32, c) + 0x60, mods: CTRL, len: 1 };
    }
    // a UTF-8 character
    let mut need: usize = 1;
    if c >= 0xf0 {
        need = 4;
    } else if c >= 0xe0 {
        need = 3;
    } else if c >= 0xc0 {
        need = 2;
    }
    if b.len < need {
        return error.Incomplete;
    }
    let r = strs.decode_rune(b[..need]);
    return Key.{ code: KeyCode.CHAR, rune: r.value, mods: 0, len: @max(r.len, 1) };
}

// ESC [ n ~: 1 and 7 Home, 2 Insert, 3 Delete, 4 and 8 End, 5 PgUp, 6 PgDn,
// 11-15 F1-F5, 17-21 F6-F10, 23-24 F11-F12
fn tilde_key(n: u32) KeyCode {
    if (n == 1) or (n == 7) {
        return KeyCode.HOME;
    }
    if (n == 4) or (n == 8) {
        return KeyCode.END;
    }
    if (n >= 2) and (n <= 6) {
        let ks: [5]KeyCode = [KeyCode.INSERT, KeyCode.DELETE, KeyCode.END, KeyCode.PAGE_UP, KeyCode.PAGE_DOWN];
        return ks[@as(usize, n - 2)];
    }
    if (n >= 11) and (n <= 15) {
        return FKEYS[@as(usize, n - 11)];
    }
    if (n >= 17) and (n <= 21) {
        return FKEYS[5 + @as(usize, n - 17)];
    }
    if (n == 23) or (n == 24) {
        return FKEYS[10 + @as(usize, n - 23)];
    }
    return KeyCode.UNKNOWN;
}

fn escape_sequence(b: []u8) !Key {
    // ESC [ params final, or ESC O final
    let ss3 = b[1] == 'O';
    let mut i: usize = 2;
    let mut params: [2]u32 = [0, 0];
    let mut np: usize = 0;
    let mut have = false;
    while i < b.len {
        let c = b[i];
        if (c >= '0') and (c <= '9') {
            if np < 2 {
                params[np] = @min((params[np] * 10) + @as(u32, c - '0'), 1000);
            }
            have = true;
            i += 1;
            continue;
        }
        if c == ';' {
            np += 1;
            i += 1;
            continue;
        }
        break;
    }
    if i >= b.len {
        return error.Incomplete;
    }
    let final = b[i];
    let len = i + 1;
    // modifiers: parameter 2 is 1 + bits (shift 1, alt 2, ctrl 4)
    let mut mods: u8 = 0;
    if (np >= 1) and (params[1] > 1) {
        mods = @truncate(u8, (params[1] - 1) & 7);
    }
    let mut code = KeyCode.UNKNOWN;
    if final == 'A' {
        code = KeyCode.UP;
    } else if final == 'B' {
        code = KeyCode.DOWN;
    } else if final == 'C' {
        code = KeyCode.RIGHT;
    } else if final == 'D' {
        code = KeyCode.LEFT;
    } else if final == 'H' {
        code = KeyCode.HOME;
    } else if final == 'F' {
        code = KeyCode.END;
    } else if (final == 'Z') and not ss3 {
        return Key.{ code: KeyCode.TAB, rune: 0, mods: SHIFT, len: len };
    } else if (final >= 'P') and (final <= 'S') {
        code = FKEYS[@as(usize, final - 'P')];
    } else if (final == '~') and have {
        code = tilde_key(params[0]);
    }
    return Key.{ code: code, rune: 0, mods: mods, len: len };
}

// -----------------------------------------------------------------------------
// Tests
// -----------------------------------------------------------------------------

fn styled(s: Style, text: []u8, enabled: bool) ![]mut u8 {
    let mut out = strs.String.init();
    s.write(out.writer(), text, enabled) catch |e| {
        out.deinit();
        return e;
    };
    return out.take();
}

test "styles" {
    let a = styled(Style.fg(Color.basic(BasicColor.RED)).bold(), "hi", true)!;
    defer mem.free(a);
    @expect_eq(a, "\x1b[1;31mhi\x1b[0m");
    let b = styled(Style.fg(Color.bright(BasicColor.RED)).on(Color.indexed(236)).underline(), "x", true)!;
    defer mem.free(b);
    @expect_eq(b, "\x1b[4;91;48;5;236mx\x1b[0m");
    let c = styled(Style.fg(Color.rgb(255, 128, 0)), "x", true)!;
    defer mem.free(c);
    @expect_eq(c, "\x1b[38;2;255;128;0mx\x1b[0m");
    let d = styled(Style.fg(Color.basic(BasicColor.RED)).bold(), "hi", false)!;
    defer mem.free(d);
    @expect_eq(d, "hi");
    let e = styled(Style.plain(), "hi", true)!;
    defer mem.free(e);
    @expect_eq(e, "hi");
    let mut s = strs.String.init();
    defer s.deinit();
    move_to(s.writer(), 3, 14)!;
    move_up(s.writer(), 2)!;
    clear_line(s.writer())!;
    set_title(s.writer(), "a\x07b")!;
    @expect_eq(s.bytes(), "\x1b[3;14H\x1b[2A\x1b[2K\r\x1b]0;ab\x07");
}

fn key(b: []u8) Key {
    return decode_key(b) catch Key.{ code: KeyCode.UNKNOWN, rune: 0, mods: 0, len: 0 };
}

test "keys" {
    let k1 = key("a");
    @expect((k1.code == KeyCode.CHAR) and (k1.rune == 'a') and (k1.len == 1));
    let k2 = key("\x03");
    @expect((k2.code == KeyCode.CHAR) and (k2.rune == 'c') and (k2.mods == CTRL));
    let k3 = key("\x1b[A");
    @expect((k3.code == KeyCode.UP) and (k3.len == 3));
    let k4 = key("\x1b[1;5C");
    @expect((k4.code == KeyCode.RIGHT) and (k4.mods == CTRL) and (k4.len == 6));
    let k5 = key("\x1bOP");
    @expect(k5.code == KeyCode.F1);
    let k6 = key("\x1b[15~");
    @expect(k6.code == KeyCode.F5);
    let k7 = key("\x1b[3~x");
    @expect((k7.code == KeyCode.DELETE) and (k7.len == 4));
    let k8 = key("\x1bx");
    @expect((k8.code == KeyCode.CHAR) and (k8.rune == 'x') and (k8.mods == ALT) and (k8.len == 2));
    let k9 = key("\xc3\xa9!");
    @expect((k9.code == KeyCode.CHAR) and (k9.rune == 0xe9) and (k9.len == 2));
    let k10 = key("\x1b");
    @expect(k10.code == KeyCode.ESCAPE);
    let k11 = key("\x7f");
    @expect(k11.code == KeyCode.BACKSPACE);
    let k12 = key("\x1b[Z");
    @expect((k12.code == KeyCode.TAB) and (k12.mods == SHIFT));
    let k13 = key("\x1b[24~");
    @expect(k13.code == KeyCode.F12);
    @expect(incomplete(decode_key("\x1b[1;5")) and incomplete(decode_key("\xe2\x82")) and incomplete(decode_key("")));
}

fn incomplete(r: !Key) bool {
    _ = r catch |e| return e == error.Incomplete;
    return false;
}

test "not a terminal" {
    // (the tests' output is captured, not a terminal)
    let f = io.open("std/os/term.ph") catch return;
    defer f.close();
    @expect(not is_tty(f.fd));
    @expect(not_tty(size(f.fd)));
    @expect(raw_fails(enable_raw(f.fd)));
}

fn not_tty(r: !Size) bool {
    _ = r catch |e| return e == error.NotATerminal;
    return false;
}

fn raw_fails(r: !void) bool {
    r catch |e| return e == error.NotATerminal;
    return false;
}
