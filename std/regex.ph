// =============================================================================
// std/regex.ph - regular expressions that run in linear time.
//
//     let mut re = regex.compile("(?P<key>\\w+)=(\\d+)")!;
//     defer re.deinit();
//     if re.is_match(line)! { ... }
//     let m = re.find(line)!;                     // m.start, m.end
//     let mut caps = re.captures(line)!;          // caps.get(2), caps.name("key")
//     defer caps.deinit();
//     let mut it = re.find_all(text);
//     loop { let m = it.next() catch break; ... }
//     let out = re.replace_all(text, "$key: ${2}")!;   // mem.free(out)
//
// The syntax is RE2's (and mostly Perl's), without backreferences and
// lookaround, which cannot be matched in linear time:
//
//   x y  x|y  x* x+ x? x{n} x{n,} x{n,m}   (add ? for the lazy forms)
//   .  [abc]  [^a-z]  [[:alpha:]]  \d \w \s \D \W \S
//   ^ $ \A \z \b \B
//   (x)  (?:x)  (?P<name>x)  (?<name>x)  (?flags)  (?flags:x)
//   \n \t \r \f \v \a \x7F \x{10FFFF}, and \ before any punctuation
//
// Flags: i (case-insensitive, for ASCII letters), m (^ and $ match at line
// boundaries), s (. matches \n), U (swaps greedy and lazy).
//
// Matching works on UTF-8 text: . and classes match whole code points, and
// positions are byte offsets. \d, \w, \s and \b are ASCII. Among matches
// starting at the leftmost position, the one a backtracking engine would
// find first is chosen (leftmost-first, as in Perl, RE2 and Go). The
// matcher is a Pike VM: time is proportional to the length of the text
// times the size of the pattern, whatever either contains. (Each search
// is linear; find_all and replace_all search again after each match, so
// like RE2's, they can take time quadratic in the text for patterns such
// as `a*b|a` on long runs of a.)
// =============================================================================

import "mem.ph" as mem;
import "str.ph" as strs;
import "sync.ph" as sync;
import "vec.ph" as vec;

error { SyntaxError, TooBig, NotFound, Done, OutOfMemory }

/// Patterns that would compile to more instructions than this fail with
/// TooBig (x{1000}{1000} and the like).
pub const MAX_PROGRAM: usize = 100000;

// -----------------------------------------------------------------------------
// Character classes
// -----------------------------------------------------------------------------

struct Range {
    lo: u32,
    hi: u32,
}

fn range_less(a: Range, b: Range) bool {
    return a.lo < b.lo;
}

/// A set of code points under construction.
struct Set {
    rs: vec.Vec.[Range],

    fn init() Set {
        return Set.{ rs: vec.Vec.[Range].init() };
    }

    fn deinit(self: *mut Set) void {
        self.rs.deinit();
    }

    fn add(self: *mut Set, lo: u32, hi: u32) !void {
        self.rs.push(Range.{ lo: lo, hi: hi })!;
    }

    fn add_set(self: *mut Set, o: *Set) !void {
        self.rs.extend(o.rs.items())!;
    }

    /// Sorts and merges overlapping and adjacent ranges.
    fn normalize(self: *mut Set) void {
        let xs = self.rs.items();
        if xs.len < 2 {
            return;
        }
        vec.sort(xs, range_less);
        let mut n: usize = 1;
        for i in 1..xs.len {
            if xs[i].lo <= (xs[n - 1].hi + 1) {
                xs[n - 1].hi = @max(xs[n - 1].hi, xs[i].hi);
            } else {
                xs[n] = xs[i];
                n += 1;
            }
        }
        self.rs.truncate(n);
    }

    /// Replaces the set by its complement in [0, 0x10FFFF].
    fn negate(self: *mut Set) !void {
        self.normalize();
        let mut out = vec.Vec.[Range].init();
        let mut next: u32 = 0;
        for r in self.rs.items() {
            if r.lo > next {
                out.push(Range.{ lo: next, hi: r.lo - 1 }) catch |e| {
                    out.deinit();
                    return e;
                };
            }
            next = r.hi + 1;
        }
        if next <= 0x10FFFF {
            out.push(Range.{ lo: next, hi: 0x10FFFF }) catch |e| {
                out.deinit();
                return e;
            };
        }
        self.rs.deinit();
        self.rs = out;
    }

    /// Adds the other ASCII case of every ASCII letter in the set.
    fn fold(self: *mut Set) !void {
        let n = self.rs.len;
        for i in 0..n {
            let r = self.rs.get(i);
            // A-Z -> a-z and a-z -> A-Z, for the part of r inside each
            let ulo = @max(r.lo, 'A');
            let uhi = @min(r.hi, 'Z');
            if ulo <= uhi {
                self.add(ulo + 32, uhi + 32)!;
            }
            let llo = @max(r.lo, 'a');
            let lhi = @min(r.hi, 'z');
            if llo <= lhi {
                self.add(llo - 32, lhi - 32)!;
            }
        }
    }
}

fn add_perl(s: *mut Set, c: u8) !void {
    match c | 0x20 {
        'd' => { s.add('0', '9')!; }
        'w' => {
            s.add('0', '9')!;
            s.add('A', 'Z')!;
            s.add('_', '_')!;
            s.add('a', 'z')!;
        }
        's' => {
            s.add(9, 13)!;  // \t \n \v \f \r
            s.add(' ', ' ')!;
        }
        else => { @trap(); }
    }
}

fn is_word_byte(c: u8) bool {
    return strs.is_alnum(c) or (c == '_');
}

// -----------------------------------------------------------------------------
// Syntax tree
// -----------------------------------------------------------------------------

const N_EMPTY: u8 = 0;
const N_RUNE: u8 = 1;    // a: code point
const N_CLASS: u8 = 2;   // a: class
const N_ANY: u8 = 3;     // . without s
const N_ANY_NL: u8 = 4;  // . with s
const N_CAT: u8 = 5;     // kids
const N_ALT: u8 = 6;     // kids
const N_REPEAT: u8 = 7;  // a: child; min, max (INF), greedy
const N_GROUP: u8 = 8;   // a: child; b: capture index, or NONE
const N_ASSERT: u8 = 9;  // a: assertion

const INF: u32 = 0xFFFFFFFF;
const NONE: u32 = 0xFFFFFFFF;

const A_BEGIN_LINE: u32 = 0;
const A_END_LINE: u32 = 1;
const A_BEGIN_TEXT: u32 = 2;
const A_END_TEXT: u32 = 3;
const A_WORD: u32 = 4;
const A_NOT_WORD: u32 = 5;

struct Node {
    kind: u8,
    greedy: bool,
    a: u32,
    b: u32,
    min: u32,
    max: u32,
    first: u32,  // children: kids[first..first + count]
    count: u32,
}

struct Flags {
    fold: bool,       // i
    multiline: bool,  // m
    dotall: bool,     // s
    ungreedy: bool,   // U
}

pub struct ErrorInfo {
    offset: usize,
    message: []u8,
}

struct Name {
    name: []u8,
    index: u32,
}

struct Parser {
    s: []u8,
    i: usize,
    nodes: vec.Vec.[Node],
    kids: vec.Vec.[u32],
    ranges: vec.Vec.[Range],
    classes: vec.Vec.[Range],  // lo: first range, hi: count
    ncap: u32,
    names: vec.Vec.[Name],
    message: []u8,
    depth: u32,

    fn fail(self: *mut Parser, msg: []u8) error { if self.message.len == 0 {
        self.message = msg;
    }
    return error.SyntaxError; }

    fn node(self: *mut Parser, n: Node) !u32 {
        self.nodes.push(n)!;
        return @int_cast(u32, self.nodes.len - 1);
    }

    fn leaf(self: *mut Parser, kind: u8, a: u32) !u32 {
        return self.node(Node.{ kind: kind, greedy: false, a: a, b: 0, min: 0, max: 0, first: 0, count: 0 });
    }

    fn list(self: *mut Parser, kind: u8, items: []u32) !u32 {
        let first = @int_cast(u32, self.kids.len);
        self.kids.extend(items)!;
        return self.node(Node.{ kind: kind, greedy: false, a: 0, b: 0, min: 0, max: 0, first: first,
                                count: @int_cast(u32, items.len) });
    }

    /// Stores a finished set as a class and returns its index.
    fn class_of(self: *mut Parser, set: *mut Set) !u32 {
        set.normalize();
        let first = @int_cast(u32, self.ranges.len);
        self.ranges.extend(set.rs.items())!;
        self.classes.push(Range.{ lo: first, hi: @int_cast(u32, set.rs.len) })!;
        return @int_cast(u32, self.classes.len - 1);
    }

    fn peek(self: *Parser) u8 {
        if self.i >= self.s.len {
            return 0;
        }
        return self.s[self.i];
    }

    fn more(self: *Parser) bool {
        return self.i < self.s.len;
    }

    /// The next code point of the pattern.
    fn next_rune(self: *mut Parser) u32 {
        let r = strs.decode_rune(self.s[self.i..]);
        self.i += r.len;
        return r.value;
    }

    fn alternation(self: *mut Parser, f: *mut Flags) !u32 {
        let mut alts = vec.Vec.[u32].init();
        defer alts.deinit();
        alts.push(self.concatenation(f)!)!;
        while self.peek() == '|' {
            self.i += 1;
            alts.push(self.concatenation(f)!)!;
        }
        if alts.len == 1 {
            return alts.get(0);
        }
        return self.list(N_ALT, alts.items());
    }

    fn concatenation(self: *mut Parser, f: *mut Flags) !u32 {
        let mut items = vec.Vec.[u32].init();
        defer items.deinit();
        while self.more() and (self.peek() != '|') and (self.peek() != ')') {
            let before = self.i;
            let a = self.atom(f)!;
            if a == NONE {
                continue;  // a flag group such as (?i)
            }
            let r = self.repetition(a, f, before)!;
            items.push(r)!;
        }
        if items.len == 0 {
            return self.leaf(N_EMPTY, 0);
        }
        if items.len == 1 {
            return items.get(0);
        }
        return self.list(N_CAT, items.items());
    }

    fn number(self: *mut Parser) !u32 {
        if not strs.is_digit(self.peek()) {
            return self.fail("expected a number");
        }
        let mut v: u32 = 0;
        while strs.is_digit(self.peek()) {
            v = (v * 10) + @as(u32, self.peek() - '0');
            self.i += 1;
            if v > 1000 {
                return self.fail("repetition count above 1000");
            }
        }
        return v;
    }

    fn repetition(self: *mut Parser, atom: u32, f: *mut Flags, start: usize) !u32 {
        let mut a = atom;
        loop {
            let c = self.peek();
            let mut min: u32 = 0;
            let mut max: u32 = INF;
            if c == '*' {
                self.i += 1;
            } else if c == '+' {
                min = 1;
                self.i += 1;
            } else if c == '?' {
                max = 1;
                self.i += 1;
            } else if c == '{' {
                // {n}, {n,}, {n,m}; anything else is a literal '{'
                let save = self.i;
                self.i += 1;
                if not strs.is_digit(self.peek()) {
                    self.i = save;
                    return a;
                }
                min = self.number()!;
                max = min;
                if self.peek() == ',' {
                    self.i += 1;
                    max = INF;
                    if strs.is_digit(self.peek()) {
                        max = self.number()!;
                    }
                }
                if self.peek() != '}' {
                    self.i = save;
                    return a;
                }
                self.i += 1;
                if max < min {
                    return self.fail("bad repetition range");
                }
            } else {
                return a;
            }
            let k = self.nodes.get(@as(usize, a)).kind;
            if (k == N_EMPTY) or (k == N_ASSERT) {
                return self.fail("missing argument to repetition operator");
            }
            let mut greedy = true;
            if self.peek() == '?' {
                greedy = false;
                self.i += 1;
            }
            if f.ungreedy {
                greedy = not greedy;
            }
            let nc = self.peek();
            if (nc == '*') or (nc == '+') or (nc == '?') {
                return self.fail("bad repetition operator");
            }
            a = self.node(Node.{ kind: N_REPEAT, greedy: greedy, a: a, b: 0, min: min, max: max, first: 0, count: 0 })!;
        }
    }

    fn literal(self: *mut Parser, r: u32, f: *mut Flags) !u32 {
        if f.fold and (r < 128) and strs.is_alpha(@truncate(u8, r)) {
            let mut set = Set.init();
            defer set.deinit();
            set.add(r, r)!;
            set.fold()!;
            return self.leaf(N_CLASS, self.class_of(&mut set)!);
        }
        return self.leaf(N_RUNE, r);
    }

    /// One atom; NONE for a flag group, which matches nothing.
    fn atom(self: *mut Parser, f: *mut Flags) !u32 {
        let c = self.peek();
        match c {
            '(' => { return self.group(f); }
            '[' => {
                let mut set = Set.init();
                defer set.deinit();
                self.class(&mut set, f)!;
                return self.leaf(N_CLASS, self.class_of(&mut set)!);
            }
            '.' => {
                self.i += 1;
                if f.dotall {
                    return self.leaf(N_ANY_NL, 0);
                }
                return self.leaf(N_ANY, 0);
            }
            '^' => {
                self.i += 1;
                if f.multiline {
                    return self.leaf(N_ASSERT, A_BEGIN_LINE);
                }
                return self.leaf(N_ASSERT, A_BEGIN_TEXT);
            }
            '$' => {
                self.i += 1;
                if f.multiline {
                    return self.leaf(N_ASSERT, A_END_LINE);
                }
                return self.leaf(N_ASSERT, A_END_TEXT);
            }
            '\\' => { return self.escape(f); }
            '*'  => { return self.fail("missing argument to repetition operator"); }
            '+'  => { return self.fail("missing argument to repetition operator"); }
            '?'  => { return self.fail("missing argument to repetition operator"); }
            else => {
                let r = self.next_rune();
                return self.literal(r, f);
            }
        }
    }

    fn escape(self: *mut Parser, f: *mut Flags) !u32 {
        self.i += 1;
        if not self.more() {
            return self.fail("trailing backslash");
        }
        let c = self.peek();
        match c {
            'A' => {
                self.i += 1;
                return self.leaf(N_ASSERT, A_BEGIN_TEXT);
            }
            'z' => {
                self.i += 1;
                return self.leaf(N_ASSERT, A_END_TEXT);
            }
            'b' => {
                self.i += 1;
                return self.leaf(N_ASSERT, A_WORD);
            }
            'B' => {
                self.i += 1;
                return self.leaf(N_ASSERT, A_NOT_WORD);
            }
            else => {}
        }
        let lc = c | 0x20;
        if (lc == 'd') or (lc == 'w') or (lc == 's') {
            self.i += 1;
            let mut set = Set.init();
            defer set.deinit();
            add_perl(&mut set, c)!;
            if strs.is_upper(c) {
                set.negate()!;
            }
            return self.leaf(N_CLASS, self.class_of(&mut set)!);
        }
        let r = self.escaped_rune()!;
        return self.literal(r, f);
    }

    /// The code point of a character escape (after the backslash).
    fn escaped_rune(self: *mut Parser) !u32 {
        let c = self.peek();
        self.i += 1;
        match c {
            'n' => { return '\n'; }
            't' => { return '\t'; }
            'r' => { return '\r'; }
            'f' => { return 12; }
            'v' => { return 11; }
            'a' => { return 7; }
            'x' => {
                if self.peek() == '{' {
                    self.i += 1;
                    let mut v: u32 = 0;
                    let mut n: usize = 0;
                    while self.more() and (self.peek() != '}') {
                        let d = hex_digit(self.peek());
                        if (d > 15) or (n >= 6) {
                            return self.fail("bad \\x{...} escape");
                        }
                        v = (v << 4) | d;
                        n += 1;
                        self.i += 1;
                    }
                    if (n == 0) or (self.peek() != '}') or (v > 0x10FFFF) {
                        return self.fail("bad \\x{...} escape");
                    }
                    self.i += 1;
                    return v;
                }
                if (self.i + 2) > self.s.len {
                    return self.fail("bad \\x escape");
                }
                let hi = hex_digit(self.s[self.i]);
                let lo = hex_digit(self.s[self.i + 1]);
                if (hi > 15) or (lo > 15) {
                    return self.fail("bad \\x escape");
                }
                self.i += 2;
                return (hi << 4) | lo;
            }
            else => {
                if (c < 0x80) and not strs.is_alnum(c) {
                    return @as(u32, c);  // escaped punctuation
                }
                self.i -= 1;
                return self.fail("unknown escape");
            }
        }
    }

    fn group(self: *mut Parser, f: *mut Flags) !u32 {
        let open = self.i;
        self.i += 1;
        self.depth += 1;
        if self.depth > 1000 {
            return self.fail("nested too deeply");
        }
        let mut cap = NONE;
        let mut inner = f.*;
        if self.peek() == '?' {
            self.i += 1;
            let c = self.peek();
            if (c == 'P') or (c == '<') {
                // (?P<name>...) or (?<name>...)
                if c == 'P' {
                    self.i += 1;
                }
                if self.peek() != '<' {
                    return self.fail("bad named group");
                }
                self.i += 1;
                let start = self.i;
                while self.more() and (strs.is_alnum(self.peek()) or (self.peek() == '_')) {
                    self.i += 1;
                }
                let name = self.s[start..self.i];
                if (name.len == 0) or (self.peek() != '>') {
                    return self.fail("bad named group");
                }
                self.i += 1;
                for nm in self.names.items() {
                    if @mem_eq(nm.name, name) {
                        return self.fail("duplicate group name");
                    }
                }
                self.ncap += 1;
                cap = self.ncap;
                self.names.push(Name.{ name: name, index: cap })!;
            } else if c == ':' {
                self.i += 1;
            } else {
                // a flag group: (?imsU-imsU) or (?imsU-imsU:...)
                let mut on = true;
                let mut any = false;
                loop {
                    let fc = self.peek();
                    if fc == 'i' {
                        inner.fold = on;
                    } else if fc == 'm' {
                        inner.multiline = on;
                    } else if fc == 's' {
                        inner.dotall = on;
                    } else if fc == 'U' {
                        inner.ungreedy = on;
                    } else if (fc == '-') and on {
                        on = false;
                        self.i += 1;
                        continue;
                    } else {
                        break;
                    }
                    any = true;
                    self.i += 1;
                }
                if (not any) and on {
                    return self.fail("bad group flags");
                }
                if self.peek() == ')' {
                    // (?flags): for the rest of the enclosing group
                    self.i += 1;
                    self.depth -= 1;
                    f.* = inner;
                    return NONE;
                }
                if self.peek() != ':' {
                    return self.fail("bad group flags");
                }
                self.i += 1;
            }
        } else {
            self.ncap += 1;
            cap = self.ncap;
        }
        let body = self.alternation(&mut inner)!;
        if self.peek() != ')' {
            self.i = open;
            return self.fail("missing )");
        }
        self.i += 1;
        self.depth -= 1;
        return self.node(Node.{ kind: N_GROUP, greedy: false, a: body, b: cap, min: 0, max: 0, first: 0, count: 0 });
    }

    /// A bracketed class, from '[' to ']'.
    fn class(self: *mut Parser, set: *mut Set, f: *mut Flags) !void {
        let open = self.i;
        self.i += 1;
        let mut negated = false;
        if self.peek() == '^' {
            negated = true;
            self.i += 1;
        }
        let mut first = true;
        loop {
            if not self.more() {
                self.i = open;
                return self.fail("missing ]");
            }
            let c = self.peek();
            if (c == ']') and not first {
                self.i += 1;
                break;
            }
            first = false;
            if (c == '[') and ((self.i + 1) < self.s.len) and (self.s[self.i + 1] == ':') {
                self.posix(set)!;
                continue;
            }
            if (c == '\\') and ((self.i + 1) < self.s.len) {
                let e = self.s[self.i + 1];
                let le = e | 0x20;
                if (le == 'd') or (le == 'w') or (le == 's') {
                    self.i += 2;
                    let mut sub = Set.init();
                    defer sub.deinit();
                    add_perl(&mut sub, e)!;
                    if strs.is_upper(e) {
                        sub.negate()!;
                    }
                    set.add_set(&sub)!;
                    continue;
                }
            }
            let lo = self.class_rune()!;
            let mut hi = lo;
            if (self.peek() == '-') and ((self.i + 1) < self.s.len) and (self.s[self.i + 1] != ']') {
                self.i += 1;
                hi = self.class_rune()!;
                if hi < lo {
                    return self.fail("bad class range");
                }
            }
            set.add(lo, hi)!;
        }
        if f.fold {
            set.fold()!;
        }
        if negated {
            set.negate()!;
        }
    }

    fn class_rune(self: *mut Parser) !u32 {
        if self.peek() == '\\' {
            self.i += 1;
            if not self.more() {
                return self.fail("trailing backslash");
            }
            return self.escaped_rune();
        }
        return self.next_rune();
    }

    /// [:name:] inside a class.
    fn posix(self: *mut Parser, set: *mut Set) !void {
        let start = self.i + 2;
        let end = strs.index_of_from(self.s, ":]", start) catch return self.fail("bad POSIX class");
        let raw = self.s[start..end];
        self.i = end + 2;
        let mut negate = false;
        let mut name = raw;
        if (raw.len > 0) and (raw[0] == '^') {
            negate = true;
            name = raw[1..];
        }
        let mut sub = Set.init();
        defer sub.deinit();
        if @mem_eq(name, "alpha") {
            sub.add('A', 'Z')!;
            sub.add('a', 'z')!;
        } else if @mem_eq(name, "digit") {
            sub.add('0', '9')!;
        } else if @mem_eq(name, "alnum") {
            sub.add('0', '9')!;
            sub.add('A', 'Z')!;
            sub.add('a', 'z')!;
        } else if @mem_eq(name, "upper") {
            sub.add('A', 'Z')!;
        } else if @mem_eq(name, "lower") {
            sub.add('a', 'z')!;
        } else if @mem_eq(name, "space") {
            sub.add(9, 13)!;
            sub.add(' ', ' ')!;
        } else if @mem_eq(name, "xdigit") {
            sub.add('0', '9')!;
            sub.add('A', 'F')!;
            sub.add('a', 'f')!;
        } else if @mem_eq(name, "punct") {
            sub.add('!', '/')!;
            sub.add(':', '@')!;
            sub.add('[', '`')!;
            sub.add('{', '~')!;
        } else if @mem_eq(name, "word") {
            add_perl(&mut sub, 'w')!;
        } else {
            return self.fail("unknown POSIX class");
        }
        if negate {
            sub.negate()!;
        }
        set.add_set(&sub)!;
    }
}

fn hex_digit(c: u8) u32 {
    if (c >= '0') and (c <= '9') {
        return @as(u32, c - '0');
    }
    let l = c | 0x20;
    if (l >= 'a') and (l <= 'f') {
        return @as(u32, l - 'a') + 10;
    }
    return 16;
}

// -----------------------------------------------------------------------------
// Compilation
// -----------------------------------------------------------------------------

const OP_RUNE: u8 = 0;    // a: code point
const OP_CLASS: u8 = 1;   // a: class
const OP_ANY: u8 = 2;     // any code point but \n
const OP_ANY_NL: u8 = 3;  // any code point
const OP_SPLIT: u8 = 4;   // try a, then b
const OP_JMP: u8 = 5;     // a
const OP_SAVE: u8 = 6;    // a: capture slot
const OP_ASSERT: u8 = 7;  // a: assertion
const OP_MATCH: u8 = 8;

struct Inst {
    op: u8,
    a: u32,
    b: u32,
}

struct Compiler {
    p: *Parser,
    prog: vec.Vec.[Inst],
    reverse: bool,  // for the reversed text: concatenations backwards, no captures
    // (repetitions of empty things emit nothing, and stacked ones nest:
    // bound the work and the recursion, not just the instructions)
    work: usize,
    depth: u32,

    fn emit(self: *mut Compiler, op: u8, a: u32, b: u32) !u32 {
        if self.prog.len >= MAX_PROGRAM {
            return error.TooBig;
        }
        self.prog.push(Inst.{ op: op, a: a, b: b })!;
        return @int_cast(u32, self.prog.len - 1);
    }

    fn pc(self: *Compiler) u32 {
        return @int_cast(u32, self.prog.len);
    }

    fn patch_b(self: *mut Compiler, at: u32, target: u32) void {
        self.prog.get_ptr(@as(usize, at)).b = target;
    }

    fn patch_a(self: *mut Compiler, at: u32, target: u32) void {
        self.prog.get_ptr(@as(usize, at)).a = target;
    }

    fn compile(self: *mut Compiler, ni: u32) !void {
        self.work += 1;
        if (self.work > (4 * MAX_PROGRAM)) or (self.depth >= 1000) {
            return error.TooBig;
        }
        self.depth += 1;
        let r = self.compile_node(ni);
        self.depth -= 1;
        return r;
    }

    fn compile_node(self: *mut Compiler, ni: u32) !void {
        let n = self.p.nodes.get(@as(usize, ni));
        match n.kind {
            0 => {}
            1 => { _ = self.emit(OP_RUNE, n.a, 0)!; }
            2 => { _ = self.emit(OP_CLASS, n.a, 0)!; }
            3 => { _ = self.emit(OP_ANY, 0, 0)!; }
            4 => { _ = self.emit(OP_ANY_NL, 0, 0)!; }
            5 => {
                for j in 0..n.count {
                    let mut k = n.first + j;
                    if self.reverse {
                        k = ((n.first + n.count) - 1) - j;
                    }
                    self.compile(self.p.kids.get(@as(usize, k)))!;
                }
            }
            6 => {
                // split L1, next; L1: x1; jmp end; next: split L2, ...
                let mut jumps = vec.Vec.[u32].init();
                defer jumps.deinit();
                for k in n.first..n.first + n.count {
                    let child = self.p.kids.get(@as(usize, k));
                    if k == ((n.first + n.count) - 1) {
                        self.compile(child)!;
                        break;
                    }
                    let sp = self.emit(OP_SPLIT, 0, 0)!;
                    self.patch_a(sp, self.pc());
                    self.compile(child)!;
                    jumps.push(self.emit(OP_JMP, 0, 0)!)!;
                    self.patch_b(sp, self.pc());
                }
                let end = self.pc();
                for j in jumps.items() {
                    self.patch_a(j, end);
                }
            }
            7 => { self.repeat(n)!; }
            8 => {
                if (n.b != NONE) and not self.reverse {
                    _ = self.emit(OP_SAVE, 2 * n.b, 0)!;
                    self.compile(n.a)!;
                    _ = self.emit(OP_SAVE, (2 * n.b) + 1, 0)!;
                } else {
                    self.compile(n.a)!;
                }
            }
            9    => { _ = self.emit(OP_ASSERT, n.a, 0)!; }
            else => { @trap(); }
        }
    }

    /// A split whose targets set_split fills in.
    fn split(self: *mut Compiler) !u32 {
        return self.emit(OP_SPLIT, 0, 0);
    }

    fn set_split(self: *mut Compiler, at: u32, into: u32, out: u32, greedy: bool) void {
        if greedy {
            self.patch_a(at, into);
            self.patch_b(at, out);
        } else {
            self.patch_a(at, out);
            self.patch_b(at, into);
        }
    }

    fn repeat(self: *mut Compiler, n: Node) !void {
        if n.max == INF {
            if n.min == 0 {
                // L1: split L2, L3; L2: x; jmp L1; L3:
                let l1 = self.split()!;
                self.compile(n.a)!;
                _ = self.emit(OP_JMP, l1, 0)!;
                self.set_split(l1, l1 + 1, self.pc(), n.greedy);
                return;
            }
            // x{n,} = x{n-1} x+, and x+ = L1: x; split L1, L2; L2:
            for i in 0..n.min - 1 {
                self.compile(n.a)!;
            }
            let l1 = self.pc();
            self.compile(n.a)!;
            let sp = self.split()!;
            self.set_split(sp, l1, self.pc(), n.greedy);
            return;
        }
        for i in 0..n.min {
            self.compile(n.a)!;
        }
        // (x(x(x)?)?)? for the optional copies
        let mut splits = vec.Vec.[u32].init();
        defer splits.deinit();
        for i in n.min..n.max {
            let sp = self.split()!;
            splits.push(sp)!;
            self.compile(n.a)!;
        }
        let end = self.pc();
        for sp in splits.items() {
            self.set_split(sp, sp + 1, end, n.greedy);
        }
    }
}

// -----------------------------------------------------------------------------
// Regex
// -----------------------------------------------------------------------------

/// A match: text[start..end].
pub struct Match {
    start: usize,
    end: usize,
}

pub struct Regex {
    prog: vec.Vec.[Inst],
    ranges: vec.Vec.[Range],
    classes: vec.Vec.[Range],  // lo: first range, hi: count
    ngroups: usize,            // capture groups, not counting the whole match
    names: vec.Vec.[Name],
    pattern: []mut u8,  // a copy: the names point into it
    prefix: []mut u8,   // a literal every match starts with
    anchored: bool,     // matches only at the start of the text
    first_bytes: [256]bool,
    use_first: bool,        // first_bytes lists every byte a match can start with
    rprog: vec.Vec.[Inst],  // the reversed pattern, for finding where matches start
    use_dfa: bool,
    cache: *mut DfaCache,  // the lazily built DFAs, when use_dfa

    pub fn deinit(self: *mut Regex) void {
        self.prog.deinit();
        self.ranges.deinit();
        self.classes.deinit();
        self.names.deinit();
        self.rprog.deinit();
        if self.use_dfa {
            self.cache.fwd.deinit();
            self.cache.rev.deinit();
            mem.destroy(self.cache);
        }
        mem.free(self.pattern);
        mem.free(self.prefix);
    }

    /// Drops what build() had assembled, which compile_with frees, and
    /// frees the prefix.
    fn forget_parts(self: *mut Regex) void {
        self.prog = vec.Vec.[Inst].init();
        self.ranges = vec.Vec.[Range].init();
        self.classes = vec.Vec.[Range].init();
        self.names = vec.Vec.[Name].init();
        mem.free(self.prefix);
    }

    /// The number of capture groups (not counting group 0, the whole match).
    pub fn group_count(self: *Regex) usize {
        return self.ngroups;
    }

    /// The index of a named group.
    pub fn group_index(self: *Regex, name: []u8) !usize {
        for nm in self.names.items() {
            if @mem_eq(nm.name, name) {
                return @as(usize, nm.index);
            }
        }
        return error.NotFound;
    }

    pub fn is_match(self: *Regex, text: []u8) !bool {
        let mut m = Match.{ start: 0, end: 0 };
        let r = dfa_find(self, text, 0, &mut m, true);
        if r >= 0 {
            return r == 1;
        }
        let mut slots: [2]usize = [0; 2];
        return run(self, text, 0, slots[..0], true);
    }

    /// The leftmost match, or NotFound.
    pub fn find(self: *Regex, text: []u8) !Match {
        return self.find_at(text, 0);
    }

    /// The leftmost match starting at or after byte `pos`. Assertions such
    /// as ^ and \b still see the text before pos.
    pub fn find_at(self: *Regex, text: []u8, pos: usize) !Match {
        let mut m = Match.{ start: 0, end: 0 };
        let r = dfa_find(self, text, pos, &mut m, false);
        if r == 1 {
            return m;
        }
        if r == 0 {
            return error.NotFound;
        }
        let mut slots: [2]usize = [0; 2];
        if not run(self, text, pos, slots[..], false)! {
            return error.NotFound;
        }
        return Match.{ start: slots[0], end: slots[1] };
    }

    /// The leftmost match and its groups, or NotFound.
    pub fn captures(self: *Regex, text: []u8) !Captures {
        return self.captures_at(text, 0);
    }

    pub fn captures_at(self: *Regex, text: []u8, pos: usize) !Captures {
        // the DFA narrows the search down to the match, the VM fills in the
        // groups
        let mut m = Match.{ start: pos, end: text.len };
        let r = dfa_find(self, text, pos, &mut m, false);
        if r == 0 {
            return error.NotFound;
        }
        let mut from = pos;
        if r == 1 {
            from = m.start;
        }
        let slots = mem.alloc.[usize](2 * (self.ngroups + 1))!;
        let found = run(self, text, from, slots, false) catch |e| {
            mem.free(slots);
            return e;
        };
        if not found {
            mem.free(slots);
            return error.NotFound;
        }
        return Captures.{ re: self, text: text, slots: slots };
    }

    /// Successive non-overlapping matches. An empty match right where the
    /// previous match ended is skipped.
    pub fn find_all(self: *Regex, text: []u8) Matches {
        return Matches.{ re: self, text: text, pos: 0, last_end: NO_POS, done: false };
    }

    /// text with every match replaced by `repl`, in which $1 or ${1} is a
    /// group, $name or ${name} a named group, $0 the whole match and $$ a
    /// dollar sign. Free the result with mem.free.
    pub fn replace_all(self: *Regex, text: []u8, repl: []u8) ![]mut u8 {
        let mut out = strs.String.init();
        let mut it = self.find_all(text);
        let mut last: usize = 0;
        loop {
            let m = it.next() catch |e| {
                if e == error.Done {
                    break;
                }
                out.deinit();
                return e;
            };
            let mut caps = self.captures_at(text, m.start) catch |e| {
                out.deinit();
                return e;
            };
            defer caps.deinit();
            out.push_str(text[last..m.start]) catch |e| {
                out.deinit();
                return e;
            };
            expand(&mut out, &caps, repl) catch |e| {
                out.deinit();
                return e;
            };
            last = m.end;
        }
        out.push_str(text[last..]) catch |e| {
            out.deinit();
            return e;
        };
        return out.take();
    }

    /// text with every match replaced by `repl` taken literally.
    pub fn replace_all_literal(self: *Regex, text: []u8, repl: []u8) ![]mut u8 {
        let mut out = strs.String.init();
        let mut it = self.find_all(text);
        let mut last: usize = 0;
        loop {
            let m = it.next() catch |e| {
                if e == error.Done {
                    break;
                }
                out.deinit();
                return e;
            };
            out.push_str(text[last..m.start]) catch |e| {
                out.deinit();
                return e;
            };
            out.push_str(repl) catch |e| {
                out.deinit();
                return e;
            };
            last = m.end;
        }
        out.push_str(text[last..]) catch |e| {
            out.deinit();
            return e;
        };
        return out.take();
    }
}

const NO_POS: usize = @max_value(usize);

/// A match and its groups. Call deinit.
pub struct Captures {
    re: *Regex,
    text: []u8,
    slots: []mut usize,  // start and end of each group; NO_POS if unused

    pub fn deinit(self: *mut Captures) void {
        mem.free(self.slots);
    }

    /// The number of groups including group 0.
    pub fn len(self: *Captures) usize {
        return self.slots.len / 2;
    }

    /// Where group i matched; NotFound if it took no part in the match.
    pub fn span(self: *Captures, i: usize) !Match {
        if (i >= self.len()) or (self.slots[2 * i] == NO_POS) or (self.slots[(2 * i) + 1] == NO_POS) {
            return error.NotFound;
        }
        return Match.{ start: self.slots[2 * i], end: self.slots[(2 * i) + 1] };
    }

    /// The text of group i (0 is the whole match).
    pub fn get(self: *Captures, i: usize) ![]u8 {
        let m = self.span(i)!;
        return self.text[m.start..m.end];
    }

    /// The text of a named group.
    pub fn name(self: *Captures, n: []u8) ![]u8 {
        return self.get(self.re.group_index(n)!);
    }
}

fn expand(out: *mut strs.String, caps: *Captures, repl: []u8) !void {
    let mut i: usize = 0;
    while i < repl.len {
        let c = repl[i];
        if (c != '$') or ((i + 1) >= repl.len) {
            out.push(c)!;
            i += 1;
            continue;
        }
        let d = repl[i + 1];
        if d == '$' {
            out.push('$')!;
            i += 2;
            continue;
        }
        let mut name: []u8 = "";
        if d == '{' {
            let close = strs.index_of_from(repl, "}", i + 2) catch {
                out.push(c)!;
                i += 1;
                continue;
            };
            name = repl[i + 2..close];
            i = close + 1;
        } else {
            let mut j = i + 1;
            while (j < repl.len) and (strs.is_alnum(repl[j]) or (repl[j] == '_')) {
                j += 1;
            }
            name = repl[i + 1..j];
            if name.len == 0 {
                out.push(c)!;
                i += 1;
                continue;
            }
            i = j;
        }
        // a number is a group index, anything else a group name; groups
        // that did not take part expand to nothing
        let mut idx: usize = 0;
        let mut numeric = name.len > 0;
        for ch in name {
            if not strs.is_digit(ch) {
                numeric = false;
            }
        }
        if numeric {
            for ch in name {
                idx = @min((idx * 10) + @as(usize, ch - '0'), 1000000);
            }
        } else {
            idx = caps.re.group_index(name) catch continue;
        }
        let t = caps.get(idx) catch "";
        out.push_str(t)!;
    }
}

/// See Regex.find_all.
pub struct Matches {
    re: *Regex,
    text: []u8,
    pos: usize,
    last_end: usize,
    done: bool,

    pub fn next(self: *mut Matches) !Match {
        loop {
            if self.done or (self.pos > self.text.len) {
                self.done = true;
                return error.Done;
            }
            let m = self.re.find_at(self.text, self.pos) catch |e| {
                self.done = true;
                if e == error.NotFound {
                    return error.Done;
                }
                return e;
            };
            if m.end > m.start {
                self.pos = m.end;
            } else {
                self.pos = m.end + char_len(self.text, m.end);
                if m.start == self.last_end {
                    continue;  // empty, right after the previous match
                }
            }
            self.last_end = m.end;
            return m;
        }
    }
}

/// The length of the code point at text[i] (1 at the end).
fn char_len(text: []u8, i: usize) usize {
    if (i >= text.len) or (text[i] < 0x80) {
        return 1;
    }
    return strs.decode_rune(text[i..]).len;
}

/// Compiles a pattern; see the top of this file for the syntax.
pub fn compile(pattern: []u8) !Regex {
    let mut info = ErrorInfo.{ offset: 0, message: "" };
    return compile_with(pattern, &mut info);
}

/// Like compile, and on a syntax error fills in where and why.
pub fn compile_with(pattern: []u8, info: *mut ErrorInfo) !Regex {
    let pat = strs.dup(pattern)!;
    let mut p = Parser.{
        s: pat,
        i: 0,
        nodes: vec.Vec.[Node].init(),
        kids: vec.Vec.[u32].init(),
        ranges: vec.Vec.[Range].init(),
        classes: vec.Vec.[Range].init(),
        ncap: 0,
        names: vec.Vec.[Name].init(),
        message: "",
        depth: 0,
    };
    defer p.nodes.deinit();
    defer p.kids.deinit();
    let mut c = Compiler.{ p: &p, prog: vec.Vec.[Inst].init(), reverse: false, work: 0, depth: 0 };
    let re = build(&mut p, &mut c, pat) catch |e| {
        info.offset = p.i;
        info.message = p.message;
        if e == error.TooBig {
            info.message = "pattern too big";
        }
        c.prog.deinit();
        p.ranges.deinit();
        p.classes.deinit();
        p.names.deinit();
        mem.free(pat);
        return e;
    };
    return re;
}

fn build(p: *mut Parser, c: *mut Compiler, pat: []mut u8) !Regex {
    let mut flags = Flags.{ fold: false, multiline: false, dotall: false, ungreedy: false };
    let root = p.alternation(&mut flags)!;
    if p.more() {
        return p.fail("unmatched )");
    }
    _ = c.emit(OP_SAVE, 0, 0)!;
    c.compile(root)!;
    _ = c.emit(OP_SAVE, 1, 0)!;
    _ = c.emit(OP_MATCH, 0, 0)!;
    let prefix = literal_prefix(p, root)!;
    let mut re = Regex.{
        prog: c.prog,
        ranges: p.ranges,
        classes: p.classes,
        ngroups: @as(usize, p.ncap),
        names: p.names,
        pattern: pat,
        prefix: prefix,
        anchored: starts_anchored(p, root),
        first_bytes: [false; 256],
        use_first: false,
        rprog: vec.Vec.[Inst].init(),
        use_dfa: false,
        cache: @ptr_from_int(*mut DfaCache, 8),
    };
    re.use_first = first_bytes(&mut re);
    // the DFAs handle patterns without assertions that are not anchored
    let mut plain = not re.anchored;
    for inst in re.prog.items() {
        if inst.op == OP_ASSERT {
            plain = false;
        }
    }
    if plain {
        let mut rc = Compiler.{ p: p, prog: vec.Vec.[Inst].init(), reverse: true, work: 0, depth: 0 };
        rc.compile(root) catch |e| {
            rc.prog.deinit();
            re.forget_parts();
            return e;
        };
        _ = rc.emit(OP_MATCH, 0, 0) catch |e| {
            rc.prog.deinit();
            re.forget_parts();
            return e;
        };
        re.cache = mem.create(DfaCache.{ fwd: Dfa.init(false), rev: Dfa.init(true),
                                         lock: sync.Mutex.init() }) catch |e| {
            rc.prog.deinit();
            re.forget_parts();
            return e;
        };
        re.rprog = rc.prog;
        re.use_dfa = true;
    }
    return re;
}

/// The node with any non-capturing groups around it removed.
fn unwrap(p: *Parser, ni: u32) Node {
    let mut n = p.nodes.get(@as(usize, ni));
    while (n.kind == N_GROUP) and (n.b == NONE) {
        n = p.nodes.get(@as(usize, n.a));
    }
    return n;
}

/// The literal text every match must start with (possibly empty).
fn literal_prefix(p: *Parser, root: u32) ![]mut u8 {
    let mut out = strs.String.init();
    let n = unwrap(p, root);
    if n.kind == N_RUNE {
        out.push_rune(n.a) catch |e| {
            out.deinit();
            return e;
        };
    } else if n.kind == N_CAT {
        for k in n.first..n.first + n.count {
            let child = unwrap(p, p.kids.get(@as(usize, k)));
            if child.kind != N_RUNE {
                break;
            }
            out.push_rune(child.a) catch |e| {
                out.deinit();
                return e;
            };
        }
    }
    return out.take();
}

/// Whether the pattern starts with \A (or ^ without the m flag).
fn starts_anchored(p: *Parser, root: u32) bool {
    let mut n = unwrap(p, root);
    if n.kind == N_CAT {
        n = unwrap(p, p.kids.get(@as(usize, n.first)));
    }
    return (n.kind == N_ASSERT) and (n.a == A_BEGIN_TEXT);
}

/// Fills re.first_bytes with every byte a match can start with; false when
/// that is not useful (the pattern can match the empty string, starts with
/// an assertion, or can start with almost anything).
fn first_bytes(re: *mut Regex) bool {
    let prog = re.prog.items();
    // an explicit depth-first walk over the instructions reachable without
    // consuming input
    let mut seen = mem.alloc_zeroed.[bool](prog.len) catch return false;
    defer mem.free(seen);
    let mut stack = vec.Vec.[u32].init();
    defer stack.deinit();
    stack.push(0) catch return false;
    loop {
        let pc = stack.pop() catch break;
        if seen[@as(usize, pc)] {
            continue;
        }
        seen[@as(usize, pc)] = true;
        let inst = prog[@as(usize, pc)];
        match inst.op {
            4 => {
                stack.push(inst.a) catch return false;
                stack.push(inst.b) catch return false;
            }
            5 => { stack.push(inst.a) catch return false; }
            6 => { stack.push(pc + 1) catch return false; }
            0 => {
                let mut b: [4]u8 = [0; 4];
                let enc = strs.encode_rune(b[..], inst.a);
                re.first_bytes[@as(usize, enc[0])] = true;
            }
            1 => {
                let cls = re.classes.get(@as(usize, inst.a));
                for r in re.ranges.items()[@as(usize, cls.lo)..@as(usize, cls.lo + cls.hi)] {
                    if r.lo < 0x80 {
                        for c in @as(usize, r.lo)..@as(usize, @min(r.hi, 0x7F)) + 1 {
                            re.first_bytes[c] = true;
                        }
                    }
                    if r.hi >= 0x80 {
                        // any lead byte (and invalid bytes, which read as U+FFFD)
                        for c in @as(usize, 0x80)..256 {
                            re.first_bytes[c] = true;
                        }
                    }
                }
            }
            else => {
                return false;  // ., an assertion, or an empty match
            }
        }
    }
    let mut count: u32 = 0;
    for x in re.first_bytes {
        if x {
            count += 1;
        }
    }
    return count < 200;
}

// -----------------------------------------------------------------------------
// The Pike VM
// -----------------------------------------------------------------------------

/// Threads waiting at consuming instructions, in priority order.
struct List {
    pcs: []mut u32,
    caps: []mut usize,  // ncap slots per thread
    n: usize,
}

struct Vm {
    re: *Regex,
    text: []u8,
    ncap: usize,
    stamp: []mut u32,  // the generation in which each pc was last visited
    gen: u32,
    stack: []mut usize,  // pc << 1, or [old value, (slot << 1) | 1]
    tmp: []mut usize,    // capture slots along the current path
}

fn class_has(re: *Regex, ci: u32, r: u32) bool {
    let cls = re.classes.get(@as(usize, ci));
    let rs = re.ranges.items()[@as(usize, cls.lo)..@as(usize, cls.lo + cls.hi)];
    // binary search over the sorted ranges
    let mut lo: usize = 0;
    let mut hi = rs.len;
    while lo < hi {
        let mid = lo + ((hi - lo) / 2);
        if r < rs[mid].lo {
            hi = mid;
        } else if r > rs[mid].hi {
            lo = mid + 1;
        } else {
            return true;
        }
    }
    return false;
}

fn assertion_holds(kind: u32, text: []u8, i: usize) bool {
    match kind {
        0 => { return (i == 0) or (text[i - 1] == '\n'); }
        1 => { return (i == text.len) or (text[i] == '\n'); }
        2 => { return i == 0; }
        3 => { return i == text.len; }
        else => {
            let before = (i > 0) and is_word_byte(text[i - 1]);
            let after = (i < text.len) and is_word_byte(text[i]);
            if kind == A_WORD {
                return before != after;
            }
            return before == after;
        }
    }
}

/// Adds the thread at pc0 (with capture slots vm.tmp) and every thread it
/// reaches without consuming input to `l`, in priority order: a depth-first
/// walk that tries a split's first branch first.
fn add_thread(vm: *mut Vm, l: *mut List, pc0: u32, i: usize) void {
    let prog = vm.re.prog.items();
    vm.stack[0] = @as(usize, pc0) << 1;
    let mut sp: usize = 1;
    while sp > 0 {
        sp -= 1;
        let e = vm.stack[sp];
        if (e & 1) == 1 {
            // back out of a Save: restore the slot
            sp -= 1;
            vm.tmp[e >> 1] = vm.stack[sp];
            continue;
        }
        let pc = e >> 1;
        if vm.stamp[pc] == vm.gen {
            continue;
        }
        vm.stamp[pc] = vm.gen;
        let inst = prog[pc];
        match inst.op {
            4 => {
                vm.stack[sp] = @as(usize, inst.b) << 1;
                vm.stack[sp + 1] = @as(usize, inst.a) << 1;
                sp += 2;
            }
            5 => {
                vm.stack[sp] = @as(usize, inst.a) << 1;
                sp += 1;
            }
            6 => {
                let slot = @as(usize, inst.a);
                if slot < vm.ncap {
                    vm.stack[sp] = vm.tmp[slot];
                    vm.stack[sp + 1] = (slot << 1) | 1;
                    sp += 2;
                    vm.tmp[slot] = i;
                }
                vm.stack[sp] = (pc + 1) << 1;
                sp += 1;
            }
            7 => {
                if assertion_holds(inst.a, vm.text, i) {
                    vm.stack[sp] = (pc + 1) << 1;
                    sp += 1;
                }
            }
            else => {
                let k = l.n;
                l.pcs[k] = @int_cast(u32, pc);
                if vm.ncap > 0 {
                    @mem_copy(l.caps[k * vm.ncap..(k + 1) * vm.ncap], vm.tmp);
                }
                l.n += 1;
            }
        }
    }
}

fn new_list(ninst: usize, ncap: usize) !List {
    let pcs = mem.alloc.[u32](ninst)!;
    let caps = mem.alloc.[usize](@max(ninst * ncap, 1)) catch |e| {
        mem.free(pcs);
        return e;
    };
    return List.{ pcs: pcs, caps: caps, n: 0 };
}

fn free_list(l: *mut List) void {
    mem.free(l.pcs);
    mem.free(l.caps);
}

/// Runs the program on text from `pos`. With slots.len > 0, fills in the
/// capture slots (NO_POS for groups not taking part) of the leftmost-first
/// match; with `earliest`, stops at the first match found.
fn run(re: *Regex, text: []u8, pos: usize, slots: []mut usize, earliest: bool) !bool {
    if pos > text.len {
        return false;
    }
    let ninst = re.prog.len;
    let ncap = slots.len;
    let mut clist = new_list(ninst, ncap)!;
    defer free_list(&mut clist);
    let mut nlist = new_list(ninst, ncap)!;
    defer free_list(&mut nlist);
    let stamp = mem.alloc.[u32](ninst)!;
    defer mem.free(stamp);
    let stack = mem.alloc.[usize]((4 * ninst) + 4)!;
    defer mem.free(stack);
    let tmp = mem.alloc.[usize](@max(ncap, 1))!;
    defer mem.free(tmp);
    @mem_set(stamp, 0);
    let mut vm = Vm.{ re: re, text: text, ncap: ncap, stamp: stamp, gen: 1, stack: stack, tmp: tmp[..ncap] };
    let prog = re.prog.items();
    let mut matched = false;
    let mut i = pos;
    loop {
        // a new thread at the lowest priority, while nothing has matched
        if not matched {
            if clist.n == 0 {
                // skip ahead to where a match could start
                if re.anchored and (i > 0) {
                    break;
                }
                if re.prefix.len > 0 {
                    i = strs.index_of_from(text, re.prefix, i) catch break;
                } else if re.use_first {
                    while (i < text.len) and not re.first_bytes[@as(usize, text[i])] {
                        i += 1;
                    }
                    if i == text.len {
                        break;
                    }
                }
            }
            if (not re.anchored) or (i == 0) {
                @mem_set(vm.tmp, NO_POS);
                add_thread(&mut vm, &mut clist, 0, i);
            }
        }
        if clist.n == 0 {
            // nothing can match from here: try the next position
            if matched or re.anchored or (i >= text.len) {
                break;
            }
            i += char_len(text, i);
            vm.gen += 1;
            continue;
        }
        // the code point at i
        let mut r: u32 = 0xFFFFFFFF;
        let mut len: usize = 0;
        if i < text.len {
            let b = text[i];
            if b < 0x80 {
                r = @as(u32, b);
                len = 1;
            } else {
                let d = strs.decode_rune(text[i..]);
                r = d.value;
                len = d.len;
            }
        }
        vm.gen += 1;
        nlist.n = 0;
        for k in 0..clist.n {
            let pc = @as(usize, clist.pcs[k]);
            let inst = prog[pc];
            let mut ok = false;
            match inst.op {
                0 => { ok = r == inst.a; }
                1 => { ok = (len > 0) and class_has(re, inst.a, r); }
                2 => { ok = (len > 0) and (r != '\n'); }
                3 => { ok = len > 0; }
                8 => {
                    matched = true;
                    if ncap > 0 {
                        @mem_copy(slots, clist.caps[k * ncap..(k + 1) * ncap]);
                    }
                    if earliest {
                        return true;
                    }
                    break;  // lower-priority threads are cut off
                }
                else => {}
            }
            if ok {
                if ncap > 0 {
                    @mem_copy(vm.tmp, clist.caps[k * ncap..(k + 1) * ncap]);
                }
                add_thread(&mut vm, &mut nlist, @int_cast(u32, pc + 1), i + len);
            }
        }
        if i >= text.len {
            break;
        }
        let t = clist;
        clist = nlist;
        nlist = t;
        i += len;
    }
    return matched;
}

// -----------------------------------------------------------------------------
// The lazy DFA
//
// A DFA state is the ordered list of instructions the Pike VM's threads
// would be at, so the DFA finds the same matches; states and their
// transitions for ASCII are built on demand and cached. The forward DFA
// finds where the leftmost-first match ends (a list is cut after a Match,
// as the VM cuts lower-priority threads, and new threads stop starting once
// something has matched); the reverse DFA, run backwards from there over
// the reversed pattern, finds the earliest start of a match ending there,
// which is where the leftmost-first match starts. The Pike VM is still used
// for captures (over just the match), for assertions, and when the cache
// grows too big.
// -----------------------------------------------------------------------------

const DFA_MAX_STATES: usize = 4096;
// (and at most this many program counters in all its states: 16 MiB)
const DFA_MAX_PCS: usize = 1 << 22;

/// The DFAs of a Regex. The lock keeps threads that share the Regex from
/// using them at once (a thread that finds them busy uses the Pike VM).
struct DfaCache {
    fwd: Dfa,
    rev: Dfa,
    lock: sync.Mutex,
}
const DEAD: i32 = -2;
const UNKNOWN: i32 = -1;
const GAVE_UP: i64 = -2;

struct DState {
    first: u32,  // pcs[first..first + len]
    len: u32,
    has_match: bool,  // a match ends here
    matched: bool,    // a match ended earlier (no new threads start)
    hash: u64,
}

struct Dfa {
    longest: bool,  // reverse: no cutting, anchored
    states: vec.Vec.[DState],
    pcs: vec.Vec.[u32],
    trans: vec.Vec.[i32],  // 128 per state, for ASCII
    buckets: []mut u32,    // state index + 1, open addressing
    stamp: []mut u32,
    gen: u32,
    stack: vec.Vec.[u32],
    work: vec.Vec.[u32],
    start: i32,

    fn init(longest: bool) Dfa {
        return Dfa.{
            longest: longest,
            states: vec.Vec.[DState].init(),
            pcs: vec.Vec.[u32].init(),
            trans: vec.Vec.[i32].init(),
            buckets: @slice_from_ptr(@ptr_from_int(*mut u32, 4), 0),
            stamp: @slice_from_ptr(@ptr_from_int(*mut u32, 4), 0),
            gen: 0,
            stack: vec.Vec.[u32].init(),
            work: vec.Vec.[u32].init(),
            start: UNKNOWN,
        };
    }

    fn deinit(self: *mut Dfa) void {
        self.states.deinit();
        self.pcs.deinit();
        self.trans.deinit();
        mem.free(self.buckets);
        mem.free(self.stamp);
        self.stack.deinit();
        self.work.deinit();
    }

    fn reset(self: *mut Dfa) void {
        self.states.clear();
        self.pcs.clear();
        self.trans.clear();
        @mem_set(self.buckets, 0);
        self.start = UNKNOWN;
    }

    /// Appends to `work` the consuming instructions reachable from pc
    /// without consuming input, in priority order. Returns true once a
    /// Match was appended and (leftmost-first) the rest must be cut.
    fn closure(self: *mut Dfa, prog: []Inst, pc0: u32) !bool {
        self.stack.clear();
        self.stack.push(pc0)!;
        loop {
            let pc = self.stack.pop() catch break;
            let upc = @as(usize, pc);
            if self.stamp[upc] == self.gen {
                continue;
            }
            self.stamp[upc] = self.gen;
            let inst = prog[upc];
            match inst.op {
                4 => {
                    self.stack.push(inst.b)!;
                    self.stack.push(inst.a)!;
                }
                5 => { self.stack.push(inst.a)!; }
                6 => { self.stack.push(pc + 1)!; }
                8 => {
                    self.work.push(pc)!;
                    if not self.longest {
                        return true;
                    }
                }
                else => { self.work.push(pc)!; }
            }
        }
        return false;
    }

    /// The state for the list in `work`, added if new.
    fn intern(self: *mut Dfa, prog: []Inst, matched: bool) !i32 {
        let list = self.work.items();
        if list.len == 0 {
            return DEAD;
        }
        let mut h: u64 = 0xcbf29ce484222325;
        let mut has_match = false;
        for pc in list {
            h = (h ^ @as(u64, pc)) *% 0x100000001b3;
            if prog[@as(usize, pc)].op == OP_MATCH {
                has_match = true;
            }
        }
        if matched {
            h = h ^ 0x9E3779B97F4A7C15;
        }
        let mask = self.buckets.len - 1;
        let mut b = @truncate(usize, h) & mask;
        loop {
            let e = self.buckets[b];
            if e == 0 {
                break;
            }
            let st = self.states.get(@as(usize, e - 1));
            if (st.hash == h) and (st.matched == matched) and (st.len == @int_cast(u32, list.len)) {
                if @mem_eq(self.pcs.items()[@as(usize, st.first)..@as(usize, st.first + st.len)], list) {
                    return @int_cast(i32, e - 1);
                }
            }
            b = (b + 1) & mask;
        }
        if (self.states.len >= DFA_MAX_STATES) or ((self.pcs.len + list.len) > DFA_MAX_PCS) {
            return error.TooBig;
        }
        let idx = self.states.len;
        self.states.push(DState.{ first: @int_cast(u32, self.pcs.len), len: @int_cast(u32, list.len),
                                  has_match: has_match, matched: matched, hash: h })!;
        self.pcs.extend(list)!;
        for k in @as(usize, 0)..128 {
            self.trans.push(UNKNOWN)!;
        }
        self.buckets[b] = @int_cast(u32, idx + 1);
        return @int_cast(i32, idx);
    }

    fn ensure(self: *mut Dfa, ninst: usize) !void {
        if self.stamp.len < ninst {
            mem.free(self.stamp);
            self.stamp = mem.alloc_zeroed.[u32](ninst)!;
            self.gen = 0;
        }
        if self.buckets.len == 0 {
            self.buckets = mem.alloc_zeroed.[u32](2 * DFA_MAX_STATES)!;
        }
    }

    fn start_state(self: *mut Dfa, prog: []Inst) !i32 {
        if self.start == UNKNOWN {
            self.gen += 1;
            self.work.clear();
            _ = self.closure(prog, 0)!;
            self.start = self.intern(prog, false)!;
        }
        return self.start;
    }

    /// The state after reading r in state si.
    fn step(self: *mut Dfa, re: *Regex, prog: []Inst, si: i32, r: u32) !i32 {
        let st = self.states.get(@int_cast(usize, si));
        self.gen += 1;
        self.work.clear();
        let mut cut = false;
        let base = @as(usize, st.first);
        for k in base..base + @as(usize, st.len) {
            let pc = self.pcs.get(k);
            let inst = prog[@as(usize, pc)];
            let mut ok = false;
            match inst.op {
                0 => { ok = r == inst.a; }
                1 => { ok = class_has(re, inst.a, r); }
                2 => { ok = r != '\n'; }
                3 => { ok = true; }
                8 => {
                    if not self.longest {
                        break;
                    }
                }
                else => {}
            }
            if ok {
                if self.closure(prog, pc + 1)! {
                    cut = true;
                    break;
                }
            }
        }
        let matched = st.matched or st.has_match;
        if (not cut) and (not self.longest) and not matched {
            // a new thread at the lowest priority (an unanchored search)
            _ = self.closure(prog, 0)!;
        }
        return self.intern(prog, matched);
    }

    /// step, through the cache for ASCII.
    fn next(self: *mut Dfa, re: *Regex, prog: []Inst, si: i32, r: u32) !i32 {
        if r < 128 {
            let slot = (@as(usize, @int_cast(u32, si)) * 128) + @as(usize, r);
            let t = self.trans.get(slot);
            if t != UNKNOWN {
                return t;
            }
            let n = self.step(re, prog, si, r)!;
            self.trans.set(slot, n);
            return n;
        }
        return self.step(re, prog, si, r);
    }
}

/// Where the leftmost-first match starting at or after pos ends, -1 if
/// there is none, or GAVE_UP if the DFA grew too big.
fn dfa_forward(re: *Regex, text: []u8, pos: usize, earliest: bool) i64 {
    let prog = re.prog.items();
    let d = &mut re.cache.fwd;
    d.ensure(prog.len) catch return GAVE_UP;
    let start = d.start_state(prog) catch {
        d.reset();
        return GAVE_UP;
    };
    let mut si = start;
    let mut last: i64 = -1;
    if d.states.get(@int_cast(usize, si)).has_match {
        last = @int_cast(i64, pos);
        if earliest {
            return last;
        }
    }
    let mut i = pos;
    while i < text.len {
        if si == start {
            // nothing in progress: skip to where a match could start
            if re.prefix.len > 0 {
                i = strs.index_of_from(text, re.prefix, i) catch break;
            } else if re.use_first {
                while (i < text.len) and not re.first_bytes[@as(usize, text[i])] {
                    i += 1;
                }
                if i == text.len {
                    break;
                }
            }
        }
        let b = text[i];
        let mut r = @as(u32, b);
        let mut len: usize = 1;
        if b >= 0x80 {
            let dr = strs.decode_rune(text[i..]);
            r = dr.value;
            len = dr.len;
        }
        si = d.next(re, prog, si, r) catch {
            d.reset();
            return GAVE_UP;
        };
        i += len;
        if si == DEAD {
            break;
        }
        if d.states.get(@int_cast(usize, si)).has_match {
            last = @int_cast(i64, i);
            if earliest {
                return last;
            }
        }
    }
    return last;
}

/// The earliest start at or after pos of a match of the reversed pattern
/// ending at `end`.
fn dfa_reverse(re: *Regex, text: []u8, pos: usize, end: usize) i64 {
    let prog = re.rprog.items();
    let d = &mut re.cache.rev;
    d.ensure(prog.len) catch return GAVE_UP;
    let mut si = d.start_state(prog) catch {
        d.reset();
        return GAVE_UP;
    };
    let mut best: i64 = -1;
    if d.states.get(@int_cast(usize, si)).has_match {
        best = @int_cast(i64, end);
    }
    let mut i = end;
    while i > pos {
        // the code point ending at i, decoded as the forward direction does
        let mut j = i - 1;
        let mut r = @as(u32, text[j]);
        if r >= 0x80 {
            while (j > pos) and ((text[j] & 0xC0) == 0x80) and ((i - j) < 4) {
                j -= 1;
            }
            let dr = strs.decode_rune(text[j..]);
            if dr.len == (i - j) {
                r = dr.value;
            } else {
                j = i - 1;
                r = strs.REPLACEMENT;
            }
        }
        si = d.next(re, prog, si, r) catch {
            d.reset();
            return GAVE_UP;
        };
        i = j;
        if si == DEAD {
            break;
        }
        if d.states.get(@int_cast(usize, si)).has_match {
            best = @int_cast(i64, i);
        }
    }
    return best;
}

/// find_at through the DFAs: 1 found (in m), 0 none, -1 not possible.
fn dfa_find(re: *Regex, text: []u8, pos: usize, m: *mut Match, earliest: bool) i32 {
    if not re.use_dfa {
        return -1;
    }
    if not re.cache.lock.try_lock() {
        return -1;  // in use by another thread
    }
    defer re.cache.lock.unlock();
    let e = dfa_forward(re, text, pos, earliest);
    if e == GAVE_UP {
        return -1;
    }
    if e < 0 {
        return 0;
    }
    if earliest {
        m.start = pos;
        m.end = @int_cast(usize, e);
        return 1;
    }
    let s = dfa_reverse(re, text, pos, @int_cast(usize, e));
    if s < 0 {
        return -1;
    }
    m.start = @int_cast(usize, s);
    m.end = @int_cast(usize, e);
    return 1;
}

// -----------------------------------------------------------------------------
// Tests (phalange test std/regex.ph)
// -----------------------------------------------------------------------------

/// The span of the leftmost match as "start-end", or "none".
fn span_of(pattern: []u8, text: []u8, out: []mut u8) []u8 {
    let mut re = compile(pattern) catch return "error";
    defer re.deinit();
    let m = re.find(text) catch return "none";
    let mut b = BufOut.{ buf: out, n: 0 };
    b.num(m.start);
    b.put('-');
    b.num(m.end);
    return out[..b.n];
}

struct BufOut {
    buf: []mut u8,
    n: usize,

    fn put(self: *mut BufOut, c: u8) void {
        self.buf[self.n] = c;
        self.n += 1;
    }

    fn num(self: *mut BufOut, x: usize) void {
        if x >= 10 {
            self.num(x / 10);
        }
        self.put('0' + @truncate(u8, x % 10));
    }
}

fn check(pattern: []u8, text: []u8, want: []u8) void {
    let mut b: [32]u8 = [0; 32];
    @expect_eq(span_of(pattern, text, b[..]), want);
}

test "matching" {
    check("abc", "xxabcxx", "2-5");
    check("a|b|c", "zzcb", "2-3");
    check("a*", "baaa", "0-0");
    check("a+", "baaa", "1-4");
    check("a+?", "baaa", "1-2");
    check("a{2,3}", "aaaa", "0-3");
    check("a{2,3}?", "aaaa", "0-2");
    check("a{2}", "a", "none");
    check("a{2,}", "aaaaa", "0-5");
    check("x{0}y", "xy", "1-2");
    check("(a|ab)(c|bcd)", "abcd", "0-4");
    check("(a+|b+)*c", "aabbc", "0-5");
    check("^abc$", "abc", "0-3");
    check("^abc$", "xabc", "none");
    check("(?m)^b$", "a\nb\nc", "2-3");
    check("abc$", "abc\n", "none");
    check("\\bfoo\\b", "a foo b", "2-5");
    check("\\Bfoo", "a foo xfoo", "7-10");
    check("[a-c]+", "xxcabz", "2-5");
    check("[^a-c]+", "abxyzc", "2-5");
    check("[]a]+", "x]a]", "1-4");
    check("[a-]+", "x-a-", "1-4");
    check("\\d+\\.\\d*", "v 12.5x", "2-6");
    check("\\w+", "  hi_there!", "2-10");
    check("\\s+", "a \t\nb", "1-4");
    check("[[:alpha:]]+", "12abC3", "2-5");
    check("[\\d\\s]+", "ab1 2c", "2-5");
    check("(?i)hello", "say HeLLo", "4-9");
    check("(?i:a)b", "Ab AB", "0-2");
    check(".+", "ab\ncd", "0-2");
    check("(?s).+", "ab\ncd", "0-5");
    check("\u{e9}.", "caf\u{e9}\u{e8}!", "3-7");
    check("[\u{e0}-\u{ff}]+", "abc\u{e9}\u{e8}d", "3-7");
    check("\\x41\\x{1F600}", "A\u{1F600}", "0-5");
    check("a\\.b", "axb a.b", "4-7");
    check("", "abc", "0-0");
    check("x*", "", "0-0");
    check("(|a)+", "aa", "0-0");
    check("(a*)+$", "aab", "3-3");
    check("\\A\\d", "1a", "0-1");
    check("\\d\\z", "a1", "1-2");
    // a pattern that is exponential for backtracking engines
    let mut long: [40]u8 = ['a'; 40];
    let mut re = compile("(a*)*b")!;
    defer re.deinit();
    @expect(not re.is_match(long[..])!);
}

test "syntax errors" {
    let bad: [15][]u8 = ["(", ")", "a)", "[a", "*", "a**", "x{2,1}", "\\", "\\q", "(?P<n>a)(?P<n>b)", "(?z)", "a{1001}",
                         "[z-a]", "\\x{110000}", "[[:nope:]]"];
    for p in bad {
        let mut re = compile(p) catch continue;
        re.deinit();
        @trap();
    }
    let mut info = ErrorInfo.{ offset: 0, message: "" };
    _ = compile_with("ab(cd", &mut info) catch |e| {
        @expect_eq(e, error.SyntaxError);
        @expect_eq(info.offset, 2);
        @expect_eq(info.message, "missing )");
        return;
    };
    @trap();
}

test "captures" {
    let mut re = compile("(?P<key>\\w+)=(\\d+)?(x)?")!;
    defer re.deinit();
    @expect_eq(re.group_count(), 3);
    let mut c = re.captures("  width=640;")!;
    defer c.deinit();
    @expect_eq(c.get(0)!, "width=640");
    @expect_eq(c.name("key")!, "width");
    @expect_eq(c.get(2)!, "640");
    _ = c.get(3) catch |e| {
        @expect_eq(e, error.NotFound);
        let s = c.span(2)!;
        @expect_eq(s.start, 8);
        return;
    };
    @trap();
}

test "find_all and replace" {
    let mut re = compile("a*")!;
    defer re.deinit();
    let mut it = re.find_all("baaac");
    let mut spans: [8]Match = [Match.{ start: 0, end: 0 }; 8];
    let mut n: usize = 0;
    loop {
        let m = it.next() catch break;
        spans[n] = m;
        n += 1;
    }
    @expect_eq(n, 3);
    @expect_eq(spans[1].start, 1);
    @expect_eq(spans[1].end, 4);
    @expect_eq(spans[2].start, 5);
    let mut kv = compile("(?P<k>\\w+)=(?P<v>\\w+)")!;
    defer kv.deinit();
    let out = kv.replace_all("a=1, b=22", "$v:${k}$$")!;
    defer mem.free(out);
    @expect_eq(out, "1:a$, 22:b$");
    let out2 = kv.replace_all_literal("a=1, b=22", "$0")!;
    defer mem.free(out2);
    @expect_eq(out2, "$0, $0");
    let mut words = compile("\\w+")!;
    defer words.deinit();
    let mut wi = words.find_all("one two  three");
    let mut total: usize = 0;
    loop {
        let m = wi.next() catch break;
        total += m.end - m.start;
    }
    @expect_eq(total, 11);
}

test "hostile patterns are refused quickly" {
    // empty things repeated, stacked repetitions, huge counts
    let bad: [3][]u8 = ["(?:){1000}{1000}{1000}{1000}", "(?:a{0}){1000}{1000}{1000}", "a{1000}{1000}"];
    for p in bad {
        let mut r = compile(p) catch continue;
        r.deinit();
        @expect(false);
    }
    let mut deep = strs.String.init();
    defer deep.deinit();
    deep.push('a')!;
    for _k in @as(usize, 0)..8000 {
        deep.push_str("{1}")!;
    }
    let mut r2 = compile(deep.bytes()) catch return;
    r2.deinit();
    @expect(false);
}
