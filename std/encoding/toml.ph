// =============================================================================
// std/encoding/toml.ph - TOML 1.0.0: parsing into a tree, and writing.
//
//     let mut doc = toml.parse(text)!;
//     defer doc.deinit();
//     let port = doc.root.path("server.port")!.as_i64()!;
//     for t in doc.root.get("products")!.as_array()! { ... }
//
//     let s = toml.stringify(doc.root)!;   // mem.free
//
// Tables keep their keys in the order they were first defined. Strings are
// copied out of the input, so the input may be freed. Dates and times are
// DateTime values of four kinds (offset date-time, local date-time, local
// date, local time).
//
// Passes the TOML 1.0.0 cases of toml-test (std/encoding/toml/testdata, from
// tools/gen_toml_tests.py). Nesting deeper than MAX_DEPTH is refused, and
// work is linear in the input.
// =============================================================================

import "../collections/arena.ph" as arena;
import "../collections/map.ph" as maps;
import "../collections/vec.ph" as vec;
import "../fmt.ph" as fmt;
import "../hash.ph" as hash;
import "../io.ph" as io;
import "../mem.ph" as mem;
import "../os/fs.ph" as fs;
import "../str.ph" as strs;
import "json.ph" as json;

error { SyntaxError, TooDeep, NotFound, WrongType, OutOfMemory }

/// Tables and arrays nested deeper than this are refused.
pub const MAX_DEPTH: u32 = 256;

// -----------------------------------------------------------------------------
// Values
// -----------------------------------------------------------------------------

pub const OFFSET_DATETIME: u8 = 0;  // 1979-05-27T07:32:00Z
pub const LOCAL_DATETIME: u8 = 1;   // 1979-05-27T07:32:00
pub const LOCAL_DATE: u8 = 2;       // 1979-05-27
pub const LOCAL_TIME: u8 = 3;       // 07:32:00

pub struct DateTime {
    kind: u8,
    year: u16,
    month: u8,
    day: u8,
    hour: u8,
    minute: u8,
    second: u8,
    nanosecond: u32,
    offset_minutes: i16,  // east of UTC (OFFSET_DATETIME only)

    /// RFC 3339 text: "1979-05-27T07:32:00.5-07:00", or the date or time
    /// alone for local dates and times (fractions without trailing zeros).
    pub fn format(self: DateTime, w: io.Writer) !void {
        if self.kind != LOCAL_TIME {
            fmt.write(w, "{:04}-{:02}-{:02}", [fmt.int(self.year), fmt.int(self.month), fmt.int(self.day)])!;
            if self.kind == LOCAL_DATE {
                return;
            }
            w.write_all("T")!;
        }
        fmt.write(w, "{:02}:{:02}:{:02}", [fmt.int(self.hour), fmt.int(self.minute), fmt.int(self.second)])!;
        if self.nanosecond != 0 {
            let mut digits: [10]u8 = [0; 10];
            let mut ns = self.nanosecond;
            digits[0] = '.';
            let mut i: usize = 9;
            while i > 0 {
                digits[i] = @truncate(u8, ns % 10) + '0';
                ns /= 10;
                i -= 1;
            }
            let mut n: usize = 10;
            while digits[n - 1] == '0' {
                n -= 1;
            }
            w.write_all(digits[..n])!;
        }
        if self.kind == OFFSET_DATETIME {
            if self.offset_minutes == 0 {
                w.write_all("Z")!;
            } else {
                let mut off = @as(i32, self.offset_minutes);
                let mut sign: []u8 = "+";
                if off < 0 {
                    sign = "-";
                    off = -off;
                }
                fmt.write(w, "{}{:02}:{:02}", [fmt.str(sign), fmt.int(off / 60), fmt.int(off % 60)])!;
            }
        }
    }

    pub fn eq(self: DateTime, o: DateTime) bool {
        return (self.kind == o.kind) and (self.year == o.year) and (self.month == o.month) and (self.day == o.day) and
               (self.hour == o.hour) and (self.minute == o.minute) and (self.second == o.second) and
               (self.nanosecond == o.nanosecond) and (self.offset_minutes == o.offset_minutes);
    }
}

pub struct Member {
    key: []u8,
    value: Value,
}

pub union Value {
    String: []u8,
    Int: i64,
    Float: f64,
    Bool: bool,
    DateTime: DateTime,
    Array: []Value,
    Table: []Member,

    /// The value of key in a table: NotFound if absent, WrongType if this is
    /// not a table.
    pub fn get(self: Value, key: []u8) !Value {
        match self {
            .Table(ms) => {
                for m in ms {
                    if @mem_eq(m.key, key) {
                        return m.value;
                    }
                }
                return error.NotFound;
            }
            else => { return error.WrongType; }
        }
    }

    /// Follows a dotted path of bare keys: path("a.b.c").
    pub fn path(self: Value, p: []u8) !Value {
        let mut v = self;
        let mut it = strs.split(p, ".");
        while true {
            let k = it.next() catch break;
            v = v.get(k)!;
        }
        return v;
    }

    pub fn at(self: Value, i: usize) !Value {
        match self {
            .Array(xs) => {
                if i >= xs.len {
                    return error.NotFound;
                }
                return xs[i];
            }
            else => { return error.WrongType; }
        }
    }

    /// Elements of an array or members of a table (0 for anything else).
    pub fn len(self: Value) usize {
        match self {
            .Array(xs) => { return xs.len; }
            .Table(ms) => { return ms.len; }
            else       => { return 0; }
        }
    }

    pub fn as_str(self: Value) ![]u8 {
        match self {
            .String(s) => { return s; }
            else       => { return error.WrongType; }
        }
    }

    pub fn as_i64(self: Value) !i64 {
        match self {
            .Int(x) => { return x; }
            else    => { return error.WrongType; }
        }
    }

    /// A float, or an integer as a float.
    pub fn as_f64(self: Value) !f64 {
        match self {
            .Float(x) => { return x; }
            .Int(x)   => { return @float_from_int(f64, x); }
            else      => { return error.WrongType; }
        }
    }

    pub fn as_bool(self: Value) !bool {
        match self {
            .Bool(b) => { return b; }
            else     => { return error.WrongType; }
        }
    }

    pub fn as_datetime(self: Value) !DateTime {
        match self {
            .DateTime(d) => { return d; }
            else         => { return error.WrongType; }
        }
    }

    pub fn as_array(self: Value) ![]Value {
        match self {
            .Array(xs) => { return xs; }
            else       => { return error.WrongType; }
        }
    }

    pub fn as_table(self: Value) ![]Member {
        match self {
            .Table(ms) => { return ms; }
            else       => { return error.WrongType; }
        }
    }
}

/// A parsed TOML document: the root table and the memory behind it.
pub struct Document {
    root: Value,
    arena: arena.Arena,

    pub fn deinit(self: *mut Document) void {
        self.arena.deinit();
        self.root = Value.Table(@slice_from_ptr(@ptr_from_int(*Member, 8), 0));
    }
}

/// Where and why parsing failed.
pub struct ErrorInfo {
    offset: usize,
    line: u32,    // from 1
    column: u32,  // from 1, in bytes
    message: []u8,
}

// -----------------------------------------------------------------------------
// The tree while parsing: nodes linked by index
// -----------------------------------------------------------------------------

const NIL: u32 = 0xffffffff;
const N_TABLE: u8 = 0;
const N_ARRAY: u8 = 1;
const N_VALUE: u8 = 2;

struct Node {
    kind: u8,
    aot: bool,       // an array of tables ([[x]])
    explicit: bool,  // defined by a [header], or by dotted keys in an earlier section
    frozen: bool,    // an inline table or array: closed to additions
    key: []u8,
    value: Value,  // N_VALUE
    first: u32,    // children, in order
    last: u32,
    next: u32,
    count: u32,
    parent: u32,
    depth: u32,
    chain: u32,  // the next node with the same (parent, key) hash
}

struct Parser {
    s: []u8,
    i: usize,
    arena: *mut arena.Arena,
    nodes: vec.Vec.[Node],
    index: maps.Map.[u64, u32],  // hash of (parent, key) -> the latest node with it
    pending: vec.Vec.[u32],      // containers of dotted keys in this section
    scratch: strs.String,
    msg: []u8,

    fn fail(self: *mut Parser, m: []u8) error { if self.msg.len == 0 {
        self.msg = m;
    }
    return error.SyntaxError; }

    fn peek(self: *Parser) u8 {
        if self.i < self.s.len {
            return self.s[self.i];
        }
        return 0;
    }

    fn at_end(self: *Parser) bool {
        return self.i >= self.s.len;
    }

    // --- nodes ------------------------------------------------------------------

    fn node(self: *mut Parser, n: u32) *mut Node {
        return self.nodes.get_ptr(@as(usize, n));
    }

    fn find(self: *mut Parser, parent: u32, key: []u8) u32 {
        let mut n = self.index.get(key_hash(parent, key)) catch return NIL;
        while n != NIL {
            let nd = self.node(n);
            if (nd.parent == parent) and @mem_eq(nd.key, key) {
                return n;
            }
            n = nd.chain;
        }
        return NIL;
    }

    fn new_node(self: *mut Parser, kind: u8, depth: u32) !u32 {
        if depth > MAX_DEPTH {
            return error.TooDeep;
        }
        let n = @truncate(u32, self.nodes.len);
        self.nodes.push(Node.{ kind: kind, aot: false, explicit: false, frozen: false, key: "",
                               value: Value.Bool(false), first: NIL, last: NIL, next: NIL, count: 0, parent: NIL,
                               depth: depth, chain: NIL })!;
        return n;
    }

    // Links child n under parent with key (for arrays, key is "").
    fn attach(self: *mut Parser, parent: u32, key: []u8, n: u32) !void {
        let pd = self.node(parent).depth;
        self.set_depth(n, pd + 1)!;
        let k = self.arena.dup(key)!;
        {
            let nd = self.node(n);
            nd.key = k;
            nd.parent = parent;
            nd.next = NIL;
        }
        let p = self.node(parent);
        if p.last == NIL {
            p.first = n;
        } else {
            let lastn = p.last;
            self.node(lastn).next = n;
        }
        let p2 = self.node(parent);
        p2.last = n;
        p2.count += 1;
        if p2.kind == N_TABLE {
            let h = key_hash(parent, key);
            let prev = self.index.get(h) catch NIL;
            self.node(n).chain = prev;
            self.index.put(h, n)!;
        }
    }

    // Inline values are built before they are attached: fix their depths.
    fn set_depth(self: *mut Parser, n: u32, d: u32) !void {
        if d > MAX_DEPTH {
            return error.TooDeep;
        }
        let old = self.node(n).depth;
        if old == d {
            return;
        }
        self.node(n).depth = d;
        let mut c = self.node(n).first;
        while c != NIL {
            self.set_depth(c, d + 1)!;
            c = self.node(c).next;
        }
    }

    fn table_child(self: *mut Parser, parent: u32, key: []u8) !u32 {
        let pd = self.node(parent).depth;
        let n = self.new_node(N_TABLE, pd + 1)!;
        self.attach(parent, key, n)!;
        return n;
    }

    // --- white space, comments, line ends -------------------------------------------

    fn skip_ws(self: *mut Parser) void {
        while (self.i < self.s.len) and ((self.s[self.i] == ' ') or (self.s[self.i] == '\t')) {
            self.i += 1;
        }
    }

    // A comment, if there is one, up to (not including) the line end.
    fn skip_comment(self: *mut Parser) !void {
        if self.peek() != '#' {
            return;
        }
        while self.i < self.s.len {
            let c = self.s[self.i];
            if (c == '\n') or ((c == '\r') and ((self.i + 1) < self.s.len) and (self.s[self.i + 1] == '\n')) {
                return;
            }
            if ((c < 0x20) and (c != '\t')) or (c == 0x7f) {
                return self.fail("control character in a comment");
            }
            self.i += 1;
        }
    }

    // A line end (or the end of the input).
    fn newline(self: *mut Parser) !void {
        if self.at_end() {
            return;
        }
        if self.s[self.i] == '\n' {
            self.i += 1;
            return;
        }
        if (self.s[self.i] == '\r') and ((self.i + 1) < self.s.len) and (self.s[self.i + 1] == '\n') {
            self.i += 2;
            return;
        }
        return self.fail("expected the end of the line");
    }

    fn end_of_line(self: *mut Parser) !void {
        self.skip_ws();
        self.skip_comment()!;
        self.newline()!;
    }

    // White space, comments and line ends, inside arrays.
    fn skip_ws_comments_newlines(self: *mut Parser) !void {
        loop {
            self.skip_ws();
            self.skip_comment()!;
            if self.at_end() {
                return;
            }
            let c = self.s[self.i];
            if (c == '\n') or (c == '\r') {
                self.newline()!;
                continue;
            }
            return;
        }
    }

    // --- keys -------------------------------------------------------------------

    // A dotted key, its parts appended to parts (arena copies).
    fn key(self: *mut Parser, parts: *mut vec.Vec.[[]u8]) !void {
        loop {
            self.skip_ws();
            let c = self.peek();
            if ((c == '"') or (c == '\'')) and ((self.i + 2) < self.s.len) and (self.s[self.i + 1] == c) and
               (self.s[self.i + 2] == c) {
                return self.fail("a key cannot be a multi-line string");
            }
            if c == '"' {
                self.basic_string()!;
                parts.push(self.arena.dup(self.scratch.bytes())!)!;
            } else if c == '\'' {
                self.literal_string()!;
                parts.push(self.arena.dup(self.scratch.bytes())!)!;
            } else {
                let st = self.i;
                while (self.i < self.s.len) and is_bare(self.s[self.i]) {
                    self.i += 1;
                }
                if self.i == st {
                    return self.fail("expected a key");
                }
                parts.push(self.s[st..self.i])!;
            }
            self.skip_ws();
            if self.peek() != '.' {
                return;
            }
            self.i += 1;
        }
    }

    // --- strings (into scratch) ---------------------------------------------------

    fn push_char(self: *mut Parser, c: u8) !void {
        self.scratch.push(c)!;
    }

    fn escape(self: *mut Parser) !void {
        // at the backslash
        self.i += 1;
        if self.at_end() {
            return self.fail("unfinished escape");
        }
        let c = self.s[self.i];
        self.i += 1;
        match c {
            'b'  => { self.push_char(0x08)!; }
            't'  => { self.push_char('\t')!; }
            'n'  => { self.push_char('\n')!; }
            'f'  => { self.push_char(0x0c)!; }
            'r'  => { self.push_char('\r')!; }
            '"'  => { self.push_char('"')!; }
            '\\' => { self.push_char('\\')!; }
            'u'  => { self.unicode_escape(4)!; }
            'U'  => { self.unicode_escape(8)!; }
            else => { return self.fail("unknown escape"); }
        }
    }

    fn unicode_escape(self: *mut Parser, n: usize) !void {
        if (self.i + n) > self.s.len {
            return self.fail("short unicode escape");
        }
        let mut v: u32 = 0;
        for k in @as(usize, 0)..n {
            let d = hexv(self.s[self.i + k]);
            if d > 15 {
                return self.fail("bad unicode escape");
            }
            v = (v << 4) | @as(u32, d);
        }
        self.i += n;
        if (v > 0x10ffff) or ((v >= 0xd800) and (v <= 0xdfff)) {
            return self.fail("escape is not a Unicode scalar value");
        }
        let mut buf: [4]u8 = [0; 4];
        self.scratch.push_str(strs.encode_rune(buf[..], v))!;
    }

    // Checks one character of string content (not a quote or backslash).
    fn content_char(self: *mut Parser, multiline: bool) !void {
        let c = self.s[self.i];
        if (c == '\n') and multiline {
            self.push_char('\n')!;
            self.i += 1;
            return;
        }
        if (c == '\r') and multiline and ((self.i + 1) < self.s.len) and (self.s[self.i + 1] == '\n') {
            self.push_char('\n')!;
            self.i += 2;
            return;
        }
        if ((c < 0x20) and (c != '\t')) or (c == 0x7f) {
            return self.fail("control character in a string");
        }
        self.push_char(c)!;
        self.i += 1;
    }

    fn basic_string(self: *mut Parser) !void {
        self.scratch.clear();
        if ((self.i + 2) < self.s.len) and (self.s[self.i + 1] == '"') and (self.s[self.i + 2] == '"') {
            return self.multiline_basic();
        }
        self.i += 1;
        loop {
            if self.at_end() {
                return self.fail("unterminated string");
            }
            let c = self.s[self.i];
            if c == '"' {
                self.i += 1;
                return;
            }
            if c == '\\' {
                self.escape()!;
            } else {
                self.content_char(false)!;
            }
        }
    }

    fn multiline_basic(self: *mut Parser) !void {
        self.i += 3;
        self.first_newline();
        loop {
            if self.at_end() {
                return self.fail("unterminated string");
            }
            let c = self.s[self.i];
            if c == '"' {
                if self.closing_quotes('"')! {
                    return;
                }
                continue;
            }
            if c == '\\' {
                // a backslash at the end of a line trims the line break and
                // the white space after it
                let mut j = self.i + 1;
                while (j < self.s.len) and ((self.s[j] == ' ') or (self.s[j] == '\t')) {
                    j += 1;
                }
                if (j < self.s.len) and ((self.s[j] == '\n') or (self.s[j] == '\r')) {
                    self.i = j;
                    loop {
                        if self.at_end() {
                            break;
                        }
                        let d = self.s[self.i];
                        if (d == ' ') or (d == '\t') or (d == '\n') {
                            self.i += 1;
                        } else if (d == '\r') and ((self.i + 1) < self.s.len) and (self.s[self.i + 1] == '\n') {
                            self.i += 2;
                        } else {
                            break;
                        }
                    }
                    continue;
                }
                self.escape()!;
                continue;
            }
            self.content_char(true)!;
        }
    }

    fn first_newline(self: *mut Parser) void {
        if self.peek() == '\n' {
            self.i += 1;
        } else if (self.peek() == '\r') and ((self.i + 1) < self.s.len) and (self.s[self.i + 1] == '\n') {
            self.i += 2;
        }
    }

    // At a run of quote characters in a multi-line string: whether it closes
    // the string (up to two quotes before the closing three are content).
    fn closing_quotes(self: *mut Parser, q: u8) !bool {
        let mut n: usize = 0;
        while ((self.i + n) < self.s.len) and (self.s[self.i + n] == q) {
            n += 1;
        }
        if n < 3 {
            for _k in @as(usize, 0)..n {
                self.push_char(q)!;
            }
            self.i += n;
            return false;
        }
        if n > 5 {
            return self.fail("too many quotes");
        }
        for _k in @as(usize, 0)..n - 3 {
            self.push_char(q)!;
        }
        self.i += n;
        return true;
    }

    fn literal_string(self: *mut Parser) !void {
        self.scratch.clear();
        if ((self.i + 2) < self.s.len) and (self.s[self.i + 1] == '\'') and (self.s[self.i + 2] == '\'') {
            self.i += 3;
            self.first_newline();
            loop {
                if self.at_end() {
                    return self.fail("unterminated string");
                }
                if self.s[self.i] == '\'' {
                    if self.closing_quotes('\'')! {
                        return;
                    }
                    continue;
                }
                self.content_char(true)!;
            }
        }
        self.i += 1;
        loop {
            if self.at_end() {
                return self.fail("unterminated string");
            }
            if self.s[self.i] == '\'' {
                self.i += 1;
                return;
            }
            self.content_char(false)!;
        }
    }

    // --- values -----------------------------------------------------------------

    // A value: scalars come back as a VALUE node, arrays and inline tables
    // as detached nodes.
    fn value(self: *mut Parser, depth: u32) !u32 {
        let c = self.peek();
        if c == '"' {
            self.basic_string()!;
            return self.leaf(Value.String(self.arena.dup(self.scratch.bytes())!), depth);
        }
        if c == '\'' {
            self.literal_string()!;
            return self.leaf(Value.String(self.arena.dup(self.scratch.bytes())!), depth);
        }
        if c == '[' {
            return self.array(depth);
        }
        if c == '{' {
            return self.inline_table(depth);
        }
        if self.word("true") {
            return self.leaf(Value.Bool(true), depth);
        }
        if self.word("false") {
            return self.leaf(Value.Bool(false), depth);
        }
        return self.leaf(self.number_or_date()!, depth);
    }

    fn leaf(self: *mut Parser, v: Value, depth: u32) !u32 {
        let n = self.new_node(N_VALUE, depth)!;
        self.node(n).value = v;
        return n;
    }

    // Whether the input continues with w (and then a delimiter); if so,
    // skips it.
    fn word(self: *mut Parser, w: []u8) bool {
        if ((self.i + w.len) <= self.s.len) and @mem_eq(self.s[self.i..self.i + w.len], w) {
            let j = self.i + w.len;
            if (j >= self.s.len) or is_delim(self.s[j]) {
                self.i = j;
                return true;
            }
        }
        return false;
    }

    fn array(self: *mut Parser, depth: u32) !u32 {
        self.i += 1;
        let a = self.new_node(N_ARRAY, depth)!;
        loop {
            self.skip_ws_comments_newlines()!;
            if self.peek() == ']' {
                self.i += 1;
                break;
            }
            let v = self.value(depth + 1)!;
            self.attach(a, "", v)!;
            self.skip_ws_comments_newlines()!;
            let c = self.peek();
            if c == ',' {
                self.i += 1;
                continue;
            }
            if c == ']' {
                self.i += 1;
                break;
            }
            return self.fail("expected , or ] in an array");
        }
        self.node(a).frozen = true;
        return a;
    }

    fn inline_table(self: *mut Parser, depth: u32) !u32 {
        self.i += 1;
        let t = self.new_node(N_TABLE, depth)!;
        self.skip_ws();
        if self.peek() == '}' {
            self.i += 1;
            self.node(t).frozen = true;
            return t;
        }
        let mut parts = vec.Vec.[[]u8].init();
        defer parts.deinit();
        loop {
            parts.clear();
            self.key(&mut parts)!;
            if self.peek() != '=' {
                return self.fail("expected = after a key");
            }
            self.i += 1;
            self.skip_ws();
            self.assign(t, parts.items(), false)!;
            self.skip_ws();
            let c = self.peek();
            if c == ',' {
                self.i += 1;
                continue;
            }
            if c == '}' {
                self.i += 1;
                break;
            }
            return self.fail("expected , or } in an inline table");
        }
        self.node(t).frozen = true;
        return t;
    }

    // key = value under table t (the value is next in the input). In a
    // section (not an inline table), the containers the dotted key passes
    // become explicit at the next header.
    fn assign(self: *mut Parser, t: u32, parts: [][]u8, section: bool) !void {
        let mut cur = t;
        for k in @as(usize, 0)..parts.len - 1 {
            let mut n = self.find(cur, parts[k]);
            if n == NIL {
                n = self.table_child(cur, parts[k])!;
            } else {
                let nd = self.node(n);
                if (nd.kind != N_TABLE) or nd.frozen or (section and nd.explicit) {
                    return self.fail("cannot add to this key with a dotted key");
                }
            }
            if section {
                self.pending.push(n)!;
            }
            cur = n;
        }
        let stem = parts[parts.len - 1];
        if self.find(cur, stem) != NIL {
            return self.fail("duplicate key");
        }
        let v = self.value(self.node(cur).depth + 1)!;
        self.attach(cur, stem, v)!;
    }

    // A number, date or time.
    fn number_or_date(self: *mut Parser) !Value {
        let st = self.i;
        // dates and times: digits then - or :
        if ((st + 4) < self.s.len) and is_digit4(self.s[st..st + 4]) and (self.s[st + 4] == '-') {
            return Value.DateTime(self.datetime()!);
        }
        if ((st + 2) < self.s.len) and is_digit(self.s[st]) and is_digit(self.s[st + 1]) and (self.s[st + 2] == ':') {
            let mut d = DateTime.{ kind: LOCAL_TIME, year: 0, month: 0, day: 0, hour: 0, minute: 0, second: 0,
                                   nanosecond: 0, offset_minutes: 0 };
            self.time(&mut d)!;
            return Value.DateTime(d);
        }
        while (self.i < self.s.len) and not is_delim(self.s[self.i]) {
            self.i += 1;
        }
        let tok = self.s[st..self.i];
        if tok.len == 0 {
            return self.fail("expected a value");
        }
        return parse_number(tok) catch return self.fail("invalid number");
    }

    fn digits(self: *mut Parser, n: usize) !u32 {
        if (self.i + n) > self.s.len {
            return self.fail("invalid date or time");
        }
        let mut v: u32 = 0;
        for k in @as(usize, 0)..n {
            let c = self.s[self.i + k];
            if not is_digit(c) {
                return self.fail("invalid date or time");
            }
            v = (v * 10) + @as(u32, c - '0');
        }
        self.i += n;
        return v;
    }

    fn expect(self: *mut Parser, c: u8) !void {
        if self.peek() != c {
            return self.fail("invalid date or time");
        }
        self.i += 1;
    }

    fn datetime(self: *mut Parser) !DateTime {
        let mut d = DateTime.{ kind: LOCAL_DATE, year: 0, month: 0, day: 0, hour: 0, minute: 0, second: 0,
                               nanosecond: 0, offset_minutes: 0 };
        d.year = @truncate(u16, self.digits(4)!);
        self.expect('-')!;
        d.month = @truncate(u8, self.digits(2)!);
        self.expect('-')!;
        d.day = @truncate(u8, self.digits(2)!);
        if (d.month < 1) or (d.month > 12) or (d.day < 1) or (d.day > days_in(d.year, d.month)) {
            return self.fail("invalid date");
        }
        // a time follows T (or a space, if a digit comes next)
        let c = self.peek();
        let space_time = (c == ' ') and ((self.i + 1) < self.s.len) and is_digit(self.s[self.i + 1]);
        if (c == 'T') or (c == 't') or space_time {
            self.i += 1;
            d.kind = LOCAL_DATETIME;
            self.time(&mut d)!;
            let o = self.peek();
            if (o == 'Z') or (o == 'z') {
                self.i += 1;
                d.kind = OFFSET_DATETIME;
            } else if (o == '+') or (o == '-') {
                self.i += 1;
                let h = self.digits(2)!;
                self.expect(':')!;
                let m = self.digits(2)!;
                if (h > 23) or (m > 59) {
                    return self.fail("invalid offset");
                }
                let mut off = @bit_cast(i16, @truncate(u16, (h * 60) + m));
                if o == '-' {
                    off = -off;
                }
                d.offset_minutes = off;
                d.kind = OFFSET_DATETIME;
            }
        }
        if (self.i < self.s.len) and not is_delim(self.s[self.i]) {
            return self.fail("invalid date or time");
        }
        return d;
    }

    fn time(self: *mut Parser, d: *mut DateTime) !void {
        d.hour = @truncate(u8, self.digits(2)!);
        self.expect(':')!;
        d.minute = @truncate(u8, self.digits(2)!);
        self.expect(':')!;
        d.second = @truncate(u8, self.digits(2)!);
        if (d.hour > 23) or (d.minute > 59) or (d.second > 60) {
            return self.fail("invalid time");
        }
        if self.peek() == '.' {
            self.i += 1;
            let st = self.i;
            let mut ns: u32 = 0;
            let mut n: usize = 0;
            while (self.i < self.s.len) and is_digit(self.s[self.i]) {
                if n < 9 {
                    ns = (ns * 10) + @as(u32, self.s[self.i] - '0');
                    n += 1;
                }
                self.i += 1;
            }
            if self.i == st {
                return self.fail("invalid fraction of a second");
            }
            while n < 9 {
                ns *= 10;
                n += 1;
            }
            d.nanosecond = ns;
        }
        if (d.kind == LOCAL_TIME) and (self.i < self.s.len) and not is_delim(self.s[self.i]) {
            return self.fail("invalid time");
        }
    }

    // --- the document ---------------------------------------------------------------

    fn header_path(self: *mut Parser, parts: [][]u8) !u32 {
        // the containers on the way: create them, or go into them
        let mut cur: u32 = 0;
        for k in @as(usize, 0)..parts.len - 1 {
            let mut n = self.find(cur, parts[k]);
            if n == NIL {
                n = self.table_child(cur, parts[k])!;
            } else {
                let nd = self.node(n);
                if nd.frozen or (nd.kind == N_VALUE) or ((nd.kind == N_ARRAY) and not nd.aot) {
                    return self.fail("cannot extend this key");
                }
                if nd.kind == N_ARRAY {
                    n = nd.last;  // the latest table of the array
                }
            }
            cur = n;
        }
        return cur;
    }

    fn document(self: *mut Parser) !void {
        let mut parts = vec.Vec.[[]u8].init();
        defer parts.deinit();
        let mut cur: u32 = 0;
        loop {
            self.skip_ws_comments_newlines()!;
            if self.at_end() {
                return;
            }
            parts.clear();
            if self.peek() == '[' {
                for n in self.pending.items() {
                    self.node(n).explicit = true;
                }
                self.pending.clear();
                let aot = ((self.i + 1) < self.s.len) and (self.s[self.i + 1] == '[');
                self.i += 1;
                if aot {
                    self.i += 1;
                }
                self.key(&mut parts)!;
                self.expect_close(aot)!;
                let p = parts.items();
                let parent = self.header_path(p)!;
                let stem = p[p.len - 1];
                let existing = self.find(parent, stem);
                if aot {
                    let mut arr = existing;
                    if arr == NIL {
                        let pd = self.node(parent).depth;
                        arr = self.new_node(N_ARRAY, pd + 1)!;
                        self.node(arr).aot = true;
                        self.attach(parent, stem, arr)!;
                    } else if not ((self.node(arr).kind == N_ARRAY) and self.node(arr).aot) {
                        return self.fail("not an array of tables");
                    }
                    self.node(arr).explicit = true;
                    let ad = self.node(arr).depth;
                    let t = self.new_node(N_TABLE, ad + 1)!;
                    self.attach(arr, "", t)!;
                    cur = t;
                } else {
                    let mut t = existing;
                    if t == NIL {
                        t = self.table_child(parent, stem)!;
                    } else {
                        let nd = self.node(t);
                        if (nd.kind != N_TABLE) or nd.explicit or nd.frozen {
                            return self.fail("table defined twice");
                        }
                    }
                    self.node(t).explicit = true;
                    cur = t;
                }
                self.end_of_line()!;
                continue;
            }
            self.key(&mut parts)!;
            if self.peek() != '=' {
                return self.fail("expected = after a key");
            }
            self.i += 1;
            self.skip_ws();
            self.assign(cur, parts.items(), true)!;
            self.end_of_line()!;
        }
    }

    fn expect_close(self: *mut Parser, aot: bool) !void {
        if self.peek() != ']' {
            return self.fail("expected ] after a table name");
        }
        self.i += 1;
        if aot {
            if self.peek() != ']' {
                return self.fail("expected ]] after an array of tables");
            }
            self.i += 1;
        }
    }

    // --- freezing the tree into Values ------------------------------------------------

    fn build(self: *mut Parser, n: u32) !Value {
        let nd = self.node(n).*;
        if nd.kind == N_VALUE {
            return nd.value;
        }
        if nd.kind == N_ARRAY {
            let xs = @cast_slice(Value, self.arena.bytes(@as(usize, nd.count) * @size_of(Value))!);
            let mut c = nd.first;
            let mut k: usize = 0;
            while c != NIL {
                xs[k] = self.build(c)!;
                k += 1;
                c = self.node(c).next;
            }
            return Value.Array(xs);
        }
        let ms = @cast_slice(Member, self.arena.bytes(@as(usize, nd.count) * @size_of(Member))!);
        let mut c = nd.first;
        let mut k: usize = 0;
        while c != NIL {
            ms[k] = Member.{ key: self.node(c).key, value: self.build(c)! };
            k += 1;
            c = self.node(c).next;
        }
        return Value.Table(ms);
    }
}

fn key_hash(parent: u32, key: []u8) u64 {
    return hash.hash_bytes(key) ^ (@as(u64, parent) *% 0x9E3779B97F4A7C15);
}

fn is_bare(c: u8) bool {
    return ((c >= 'a') and (c <= 'z')) or ((c >= 'A') and (c <= 'Z')) or ((c >= '0') and (c <= '9')) or (c == '_') or
           (c == '-');
}

fn is_digit(c: u8) bool {
    return (c >= '0') and (c <= '9');
}

fn is_digit4(s: []u8) bool {
    return is_digit(s[0]) and is_digit(s[1]) and is_digit(s[2]) and is_digit(s[3]);
}

fn is_delim(c: u8) bool {
    return (c == ' ') or (c == '\t') or (c == '\n') or (c == '\r') or (c == ',') or (c == ']') or (c == '}') or
           (c == '#');
}

fn hexv(c: u8) u8 {
    if is_digit(c) {
        return c - '0';
    }
    if (c >= 'a') and (c <= 'f') {
        return (c - 'a') + 10;
    }
    if (c >= 'A') and (c <= 'F') {
        return (c - 'A') + 10;
    }
    return 255;
}

fn days_in(y: u16, m: u8) u8 {
    if m == 2 {
        if (((y % 4) == 0) and ((y % 100) != 0)) or ((y % 400) == 0) {
            return 29;
        }
        return 28;
    }
    if (m == 4) or (m == 6) or (m == 9) or (m == 11) {
        return 30;
    }
    return 31;
}

// Underscores must sit between two digits (of the given base).
fn strip_underscores(tok: []u8, out: []mut u8, base: u32) ![]u8 {
    let mut n: usize = 0;
    for k in @as(usize, 0)..tok.len {
        let c = tok[k];
        if c == '_' {
            if (k == 0) or ((k + 1) == tok.len) or (digit_value(tok[k - 1]) >= base) or
               (digit_value(tok[k + 1]) >= base) {
                return error.SyntaxError;
            }
            continue;
        }
        if n >= out.len {
            return error.SyntaxError;
        }
        out[n] = c;
        n += 1;
    }
    return out[..n];
}

fn digit_value(c: u8) u32 {
    let v = hexv(c);
    if v > 15 {
        return 99;
    }
    return @as(u32, v);
}

fn parse_number(tok: []u8) !Value {
    let mut buf: [128]u8 = [0; 128];
    // special floats
    let mut body = tok;
    let mut sign: u8 = 0;
    if (tok[0] == '+') or (tok[0] == '-') {
        sign = tok[0];
        body = tok[1..];
    }
    if body.len == 0 {
        return error.SyntaxError;
    }
    if strs.eq(body, "inf") {
        if sign == '-' {
            return Value.Float(-1.0 / 0.0);
        }
        return Value.Float(1.0 / 0.0);
    }
    if strs.eq(body, "nan") {
        return Value.Float(0.0 / 0.0);
    }
    // prefixed integers (no sign)
    if (body.len > 2) and (body[0] == '0') and ((body[1] == 'x') or (body[1] == 'o') or (body[1] == 'b')) {
        if sign != 0 {
            return error.SyntaxError;
        }
        let mut base: u32 = 16;
        if body[1] == 'o' {
            base = 8;
        } else if body[1] == 'b' {
            base = 2;
        }
        let d = strip_underscores(body[2..], buf[..], base)!;
        if d.len == 0 {
            return error.SyntaxError;
        }
        let mut v: u64 = 0;
        for c in d {
            let x = digit_value(c);
            if x >= base {
                return error.SyntaxError;
            }
            v = @mul_checked(v, @as(u64, base)) catch return error.SyntaxError;
            v = @add_checked(v, @as(u64, x)) catch return error.SyntaxError;
        }
        if v > 0x7fffffffffffffff {
            return error.SyntaxError;
        }
        return Value.Int(@bit_cast(i64, v));
    }
    // decimal: the integer part has no leading zeros
    let mut k: usize = 0;
    while (k < body.len) and (is_digit(body[k]) or (body[k] == '_')) {
        k += 1;
    }
    let int_part = body[..k];
    if (int_part.len == 0) or (int_part[0] == '_') {
        return error.SyntaxError;
    }
    if (int_part.len > 1) and (int_part[0] == '0') {
        return error.SyntaxError;
    }
    if k == body.len {
        let d = strip_underscores(int_part, buf[..], 10)!;
        let mut v: u64 = 0;
        for c in d {
            v = @mul_checked(v, 10) catch return error.SyntaxError;
            v = @add_checked(v, @as(u64, c - '0')) catch return error.SyntaxError;
        }
        if sign == '-' {
            if v > 0x8000000000000000 {
                return error.SyntaxError;
            }
            return Value.Int(@bit_cast(i64, 0 -% v));
        }
        if v > 0x7fffffffffffffff {
            return error.SyntaxError;
        }
        return Value.Int(@bit_cast(i64, v));
    }
    // a float: fraction and/or exponent, digits on both sides of the point
    let rest = body[k..];
    let mut j: usize = 0;
    if rest[0] == '.' {
        j = 1;
        let fst = j;
        while (j < rest.len) and (is_digit(rest[j]) or (rest[j] == '_')) {
            j += 1;
        }
        if (j == fst) or (rest[fst] == '_') {
            return error.SyntaxError;
        }
    }
    if (j < rest.len) and ((rest[j] == 'e') or (rest[j] == 'E')) {
        j += 1;
        if (j < rest.len) and ((rest[j] == '+') or (rest[j] == '-')) {
            j += 1;
        }
        let es = j;
        while (j < rest.len) and (is_digit(rest[j]) or (rest[j] == '_')) {
            j += 1;
        }
        if (j == es) or (rest[es] == '_') {
            return error.SyntaxError;
        }
    }
    if (j != rest.len) or (j == 0) {
        return error.SyntaxError;
    }
    // underscores only between digits, then an ordinary float
    for q in @as(usize, 0)..tok.len {
        if tok[q] == '_' {
            if (q == 0) or ((q + 1) == tok.len) or not is_digit(tok[q - 1]) or not is_digit(tok[q + 1]) {
                return error.SyntaxError;
            }
        }
    }
    let mut n: usize = 0;
    for c in tok {
        if c != '_' {
            if n >= buf.len {
                return error.SyntaxError;
            }
            buf[n] = c;
            n += 1;
        }
    }
    let f = fmt.parse_float(buf[..n]) catch return error.SyntaxError;
    return Value.Float(f);
}

/// Parses a TOML document.
pub fn parse(text: []u8) !Document {
    let mut info = ErrorInfo.{ offset: 0, line: 0, column: 0, message: "" };
    return parse_with(text, &mut info);
}

/// Parses a TOML document; on SyntaxError (or TooDeep) info says where.
pub fn parse_with(text: []u8, info: *mut ErrorInfo) !Document {
    let mut doc = Document.{ root: Value.Bool(false), arena: arena.Arena.init() };
    let r = parse_into(text, &mut doc, info);
    r catch |e| {
        doc.arena.deinit();
        return e;
    };
    return doc;
}

fn parse_into(text: []u8, doc: *mut Document, info: *mut ErrorInfo) !void {
    let mut p = Parser.{
        s: text,
        i: 0,
        arena: &mut doc.arena,
        nodes: vec.Vec.[Node].init(),
        index: maps.Map.[u64, u32].init(hash.hash_u64, hash.eq_u64),
        pending: vec.Vec.[u32].init(),
        scratch: strs.String.init(),
        msg: "",
    };
    defer p.nodes.deinit();
    defer p.index.deinit();
    defer p.pending.deinit();
    defer p.scratch.deinit();
    let r = parse_doc(&mut p);
    r catch |e| {
        if (e == error.SyntaxError) or (e == error.TooDeep) {
            set_info(info, text, p.i, p.msg);
        }
        return e;
    };
    doc.root = p.build(0)!;
}

fn parse_doc(p: *mut Parser) !void {
    if not strs.valid_utf8(p.s) {
        // find the first bad byte, for the message's position
        let mut k: usize = 0;
        while k < p.s.len {
            let r = strs.decode_rune(p.s[k..]);
            if (r.value == 0xfffd) and not ((r.len == 3) and (p.s[k] == 0xef)) {
                break;
            }
            k += r.len;
        }
        p.i = k;
        return p.fail("invalid UTF-8");
    }
    _ = p.new_node(N_TABLE, 0)!;
    p.document()!;
}

fn set_info(info: *mut ErrorInfo, text: []u8, off: usize, msg: []u8) void {
    let o = @min(off, text.len);
    let mut line: u32 = 1;
    let mut col: u32 = 1;
    for k in @as(usize, 0)..o {
        if text[k] == '\n' {
            line += 1;
            col = 1;
        } else {
            col += 1;
        }
    }
    info.offset = o;
    info.line = line;
    info.column = col;
    info.message = msg;
    if msg.len == 0 {
        info.message = "nesting too deep";
    }
}

// -----------------------------------------------------------------------------
// Writing
// -----------------------------------------------------------------------------

/// A table as a TOML document (free it with mem.free). Scalars and arrays
/// of scalars are written as key = value lines, subtables as [sections],
/// arrays of tables as [[sections]].
pub fn stringify(root: Value) ![]mut u8 {
    let ms = root.as_table()!;
    let mut s = strs.String.init();
    let mut path = vec.Vec.[[]u8].init();
    defer path.deinit();
    write_table(&mut s, ms, &mut path) catch |e| {
        s.deinit();
        return e;
    };
    return s.take();
}

fn is_table_array(v: Value) bool {
    let xs = v.as_array() catch return false;
    if xs.len == 0 {
        return false;
    }
    for x in xs {
        _ = x.as_table() catch return false;
    }
    return true;
}

fn is_section(v: Value) bool {
    match v {
        .Table(t) => { return t.len >= 0; }
        else      => { return is_table_array(v); }
    }
}

fn write_table(s: *mut strs.String, ms: []Member, path: *mut vec.Vec.[[]u8]) !void {
    let w = s.writer();
    for m in ms {
        if not is_section(m.value) {
            write_key(w, m.key)!;
            w.write_all(" = ")!;
            write_value(w, m.value)!;
            w.write_all("\n")!;
        }
    }
    for m in ms {
        if not is_section(m.value) {
            continue;
        }
        path.push(m.key)!;
        match m.value {
            .Table(sub) => {
                w.write_all("\n[")!;
                write_path(w, path.items())!;
                w.write_all("]\n")!;
                write_table(s, sub, path)!;
            }
            .Array(xs) => {
                for x in xs {
                    w.write_all("\n[[")!;
                    write_path(w, path.items())!;
                    w.write_all("]]\n")!;
                    write_table(s, x.as_table()!, path)!;
                }
            }
            else => {}
        }
        path.truncate(path.len - 1);
    }
}

fn write_path(w: io.Writer, parts: [][]u8) !void {
    for k in @as(usize, 0)..parts.len {
        if k > 0 {
            w.write_all(".")!;
        }
        write_key(w, parts[k])!;
    }
}

fn write_key(w: io.Writer, k: []u8) !void {
    let mut bare = k.len > 0;
    for c in k {
        if not is_bare(c) {
            bare = false;
        }
    }
    if bare {
        w.write_all(k)!;
    } else {
        write_string(w, k)!;
    }
}

fn write_string(w: io.Writer, v: []u8) !void {
    w.write_all("\"")!;
    let digits = "0123456789ABCDEF";
    for c in v {
        if c == '"' {
            w.write_all("\\\"")!;
        } else if c == '\\' {
            w.write_all("\\\\")!;
        } else if c == '\n' {
            w.write_all("\\n")!;
        } else if c == '\t' {
            w.write_all("\\t")!;
        } else if c == '\r' {
            w.write_all("\\r")!;
        } else if (c < 0x20) or (c == 0x7f) {
            let e: [6]u8 = ['\\', 'u', '0', '0', digits[@as(usize, c >> 4)], digits[@as(usize, c & 15)]];
            w.write_all(e[..])!;
        } else {
            let one: [1]u8 = [c];
            w.write_all(one[..])!;
        }
    }
    w.write_all("\"")!;
}

fn write_value(w: io.Writer, v: Value) !void {
    match v {
        .String(x) => { write_string(w, x)!; }
        .Int(x)    => { fmt.write(w, "{}", [fmt.int(x)])!; }
        .Float(x) => {
            if x != x {
                w.write_all("nan")!;
            } else if x == (1.0 / 0.0) {
                w.write_all("inf")!;
            } else if x == (-1.0 / 0.0) {
                w.write_all("-inf")!;
            } else {
                let mut buf: [40]u8 = [0; 40];
                let t = fmt.format_float(buf[..], x)!;
                w.write_all(t)!;
                // TOML floats need a point or an exponent
                if (strs.index_of_any(t, ".eE") catch 999) == 999 {
                    w.write_all(".0")!;
                }
            }
        }
        .Bool(b) => {
            if b {
                w.write_all("true")!;
            } else {
                w.write_all("false")!;
            }
        }
        .DateTime(d) => { d.format(w)!; }
        .Array(xs) => {
            w.write_all("[")!;
            for k in @as(usize, 0)..xs.len {
                if k > 0 {
                    w.write_all(", ")!;
                }
                write_value(w, xs[k])!;
            }
            w.write_all("]")!;
        }
        .Table(ms) => {
            w.write_all("{")!;
            for k in @as(usize, 0)..ms.len {
                if k > 0 {
                    w.write_all(",")!;
                }
                w.write_all(" ")!;
                write_key(w, ms[k].key)!;
                w.write_all(" = ")!;
                write_value(w, ms[k].value)!;
            }
            if ms.len > 0 {
                w.write_all(" ")!;
            }
            w.write_all("}")!;
        }
    }
}

// -----------------------------------------------------------------------------
// Tests
// -----------------------------------------------------------------------------

// Compares a parsed value with toml-test's tagged JSON for it.
fn matches(v: Value, j: json.Value) bool {
    let ty = j.get("type") catch json.Value.Null;
    let tag = ty.as_str() catch "";
    if (tag.len > 0) and (j.len() == 2) {
        let want = (j.get("value") catch return false).as_str() catch return false;
        return leaf_matches(v, tag, want);
    }
    match j {
        .Array(xs) => {
            let ys = v.as_array() catch return false;
            if xs.len != ys.len {
                return false;
            }
            for k in @as(usize, 0)..xs.len {
                if not matches(ys[k], xs[k]) {
                    return false;
                }
            }
            return true;
        }
        .Object(ms) => {
            let ts = v.as_table() catch return false;
            if ts.len != ms.len {
                return false;
            }
            for m in ms {
                let x = v.get(m.key) catch return false;
                if not matches(x, m.value) {
                    return false;
                }
            }
            return true;
        }
        else => { return false; }
    }
}

fn leaf_matches(v: Value, tag: []u8, want: []u8) bool {
    if strs.eq(tag, "string") {
        return strs.eq(v.as_str() catch return false, want);
    }
    if strs.eq(tag, "integer") {
        return (v.as_i64() catch return false) == (fmt.parse_int.[i64](want) catch return false);
    }
    if strs.eq(tag, "bool") {
        return (v.as_bool() catch return false) == strs.eq(want, "true");
    }
    if strs.eq(tag, "float") {
        let mut got: f64 = 0.0;
        match v {
            .Float(x) => { got = x; }
            else      => { return false; }
        }
        if strs.eq(want, "nan") or strs.eq(want, "+nan") or strs.eq(want, "-nan") {
            return got != got;
        }
        let mut w: f64 = 0.0;
        if strs.eq(want, "inf") or strs.eq(want, "+inf") {
            w = 1.0 / 0.0;
        } else if strs.eq(want, "-inf") {
            w = -1.0 / 0.0;
        } else {
            w = fmt.parse_float(want) catch return false;
        }
        return @bit_cast(u64, got) == @bit_cast(u64, w);
    }
    // dates and times: read the expected text as TOML too
    let d = v.as_datetime() catch return false;
    let mut src = strs.String.init();
    defer src.deinit();
    src.push_str("x = ") catch return false;
    src.push_str(want) catch return false;
    let mut doc = parse(src.bytes()) catch return false;
    defer doc.deinit();
    let e = (doc.root.get("x") catch return false).as_datetime() catch return false;
    let kinds: [4][]u8 = ["datetime", "datetime-local", "date-local", "time-local"];
    return d.eq(e) and strs.eq(kinds[@as(usize, d.kind)], tag);
}

fn next_line(data: []u8, at: *mut usize) []u8 {
    let st = at.*;
    let nl = strs.index_of_from(data, "\n", st) catch data.len;
    at.* = nl + 1;
    return data[st..nl];
}

test "toml-test: the TOML 1.0.0 cases" {
    let data = fs.read_file("std/encoding/toml/testdata/toml-test.txt") catch return;
    defer mem.free(data);
    let mut at: usize = 0;
    _ = next_line(data, &mut at);
    let mut passed: usize = 0;
    let mut failed: usize = 0;
    while at < data.len {
        let head = next_line(data, &mut at);
        let mut f = strs.fields(head);
        let kind = f.next() catch break;
        let name = f.next() catch break;
        let tlen = fmt.parse_int.[usize](f.next() catch break) catch break;
        let text = data[at..at + tlen];
        at += tlen + 1;
        let mut ok = false;
        if strs.eq(kind, "valid") {
            let jlen = fmt.parse_int.[usize](f.next() catch break) catch break;
            let jtext = data[at..at + jlen];
            at += jlen + 1;
            let mut want = json.parse(jtext) catch @trap();
            defer want.deinit();
            let mut info = ErrorInfo.{ offset: 0, line: 0, column: 0, message: "" };
            let got = parse_with(text, &mut info) catch |e| {
                fmt.print("{}:{}: {} ({})\n", [fmt.str(name), fmt.int(info.line), fmt.str(info.message),
                                               fmt.str(@error_name(e))]) catch {};
                failed += 1;
                continue;
            };
            let mut doc = got;
            ok = matches(doc.root, want.root);
            doc.deinit();
        } else {
            let got = parse(text) catch |e| {
                _ = e;
                passed += 1;
                continue;
            };
            let mut doc = got;
            doc.deinit();
        }
        if ok {
            passed += 1;
        } else {
            failed += 1;
            if failed <= 40 {
                fmt.print("FAIL {}\n", [fmt.str(name)]) catch {};
            }
        }
    }
    fmt.print("toml-test: {} passed, {} failed\n", [fmt.int(passed), fmt.int(failed)]) catch {};
    @expect_eq(failed, 0);
    @expect(passed > 700);
}

test "values, paths and writing" {
    let text =
        "title = \"TOML \\u00e9\"\n[owner]\nname = 'Tom'\ndob = 1979-05-27T07:32:00-08:00\n[database]\nports = [ 8000, 8001 ]\nenabled = true\ntemp = { cpu = 79.5 }\n[[products]]\nname = \"Hammer\"\n[[products]]\nname = \"Nail\"\nsku = 0x10\n";
    let mut doc = parse(text)!;
    defer doc.deinit();
    @expect_eq(doc.root.get("title")!.as_str()!, "TOML \xc3\xa9");
    @expect_eq(doc.root.path("database.temp.cpu")!.as_f64()!, 79.5);
    @expect_eq(doc.root.path("database.ports")!.at(1)!.as_i64()!, 8001);
    let d = doc.root.path("owner.dob")!.as_datetime()!;
    @expect_eq(d.offset_minutes, -480);
    @expect_eq(doc.root.get("products")!.at(1)!.get("sku")!.as_i64()!, 16);
    let out = stringify(doc.root)!;
    defer mem.free(out);
    let mut again = parse(out)!;
    defer again.deinit();
    let out2 = stringify(again.root)!;
    defer mem.free(out2);
    @expect_eq(out, out2);
    @expect(strs.contains(out, "[[products]]\nname = \"Nail\"\nsku = 16\n"));
    @expect(strs.contains(out, "dob = 1979-05-27T07:32:00-08:00"));
    let mut info = ErrorInfo.{ offset: 0, line: 0, column: 0, message: "" };
    _ = parse_with("a = 1\nb = @\n", &mut info) catch |e| {
        @expect_eq(e, error.SyntaxError);
        @expect_eq(info.line, 2);
        return;
    };
    @trap();
}

test "hostile nesting" {
    let mut s = strs.String.init();
    defer s.deinit();
    s.push_str("x = ")!;
    for _k in @as(usize, 0)..10000 {
        s.push('[')!;
    }
    _ = parse(s.bytes()) catch |e| {
        @expect_eq(e, error.TooDeep);
        return;
    };
    @trap();
}
