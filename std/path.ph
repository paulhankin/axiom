// =============================================================================
// std/path.ph - file paths, as text: joining, cleaning, splitting, relative
// paths and glob patterns. No file system access (that is std/fs).
//
//     let p = path.join(["a", "b/../c", "file.txt"][..])!;   // "a/c/file.txt"; mem.free it
//     path.base("a/c/file.txt")                            // "file.txt"
//     path.ext("file.tar.gz")                              // ".gz"
//     path.matches("src/**/*.ph", "src/a/b/x.ph")          // true
//
// Paths are POSIX paths: '/' separates elements, a leading '/' makes a path
// absolute. Functions that build a new path return memory to free with
// mem.free; the others return slices of their argument.
// =============================================================================

import "mem.ph" as mem;
import "str.ph" as strs;

error { OutOfMemory, BadPattern }

pub const SEP: u8 = '/';

pub fn is_abs(p: []u8) bool {
    return (p.len > 0) and (p[0] == '/');
}

/// The shortest path equivalent to p, by pure text rules: repeated
/// separators become one, "." elements go, ".." removes the element before
/// it (at the root it is dropped), and there is no trailing '/'. The empty
/// path cleans to ".".
pub fn clean(p: []u8) ![]mut u8 {
    let mut out = strs.String.init();
    clean_into(p, &mut out) catch |e| {
        out.deinit();
        return e;
    };
    return out.take();
}

fn clean_into(p: []u8, out: *mut strs.String) !void {
    let abs = is_abs(p);
    if abs {
        out.push('/')!;
    }
    let root = out.len;
    let mut dots: usize = 0;  // how many leading ".." a relative path keeps
    let mut i: usize = 0;
    while i < p.len {
        while (i < p.len) and (p[i] == '/') {
            i += 1;
        }
        let start = i;
        while (i < p.len) and (p[i] != '/') {
            i += 1;
        }
        let el = p[start..i];
        if (el.len == 0) or ((el.len == 1) and (el[0] == '.')) {
            continue;
        }
        if (el.len == 2) and (el[0] == '.') and (el[1] == '.') {
            if out.len > (root + (dots * 3)) {
                // drop the last element
                let b = out.bytes();
                let mut k = out.len;
                while (k > root) and (b[k - 1] != '/') {
                    k -= 1;
                }
                if k > root {
                    k -= 1;  // its separator
                }
                out.truncate(k);
                continue;
            }
            if abs {
                continue;  // ".." at the root is the root
            }
            if out.len > root {
                out.push('/')!;
            }
            out.push_str("..")!;
            dots += 1;
            continue;
        }
        if out.len > root {
            out.push('/')!;
        }
        out.push_str(el)!;
    }
    if out.len == 0 {
        out.push('.')!;
    }
}

/// The elements joined with '/', then cleaned; empty elements are skipped
/// (joining nothing gives "").
pub fn join(parts: [][]u8) ![]mut u8 {
    let mut raw = strs.String.init();
    defer raw.deinit();
    let mut any = false;
    for part in parts {
        if part.len == 0 {
            continue;
        }
        if any {
            raw.push('/')!;
        }
        raw.push_str(part)!;
        any = true;
    }
    if not any {
        return mem.alloc.[u8](0);
    }
    return clean(raw.bytes());
}

/// a and b joined (see join).
pub fn join2(a: []u8, b: []u8) ![]mut u8 {
    let parts: [2][]u8 = [a, b];
    return join(parts[..]);
}

/// The last element of p, without trailing separators: "/a/b/" gives "b",
/// "/" gives "/", "" gives ".".
pub fn base(p: []u8) []u8 {
    if p.len == 0 {
        return ".";
    }
    let mut end = p.len;
    while (end > 0) and (p[end - 1] == '/') {
        end -= 1;
    }
    if end == 0 {
        return "/";
    }
    let mut start = end;
    while (start > 0) and (p[start - 1] != '/') {
        start -= 1;
    }
    return p[start..end];
}

/// Everything before the last element, without its trailing separators:
/// "a/b/c" gives "a/b", "a" gives ".", "/a" gives "/". Not cleaned.
pub fn dir(p: []u8) []u8 {
    let mut end = p.len;
    while (end > 0) and (p[end - 1] == '/') {
        end -= 1;
    }
    while (end > 0) and (p[end - 1] != '/') {
        end -= 1;
    }
    // end is just after the separator before the last element
    if end == 0 {
        if is_abs(p) {
            return "/";
        }
        return ".";
    }
    let mut d = end;
    while (d > 1) and (p[d - 1] == '/') {
        d -= 1;
    }
    return p[..d];
}

/// The extension of the last element, from its last '.' ("" if none; a
/// leading dot, as in ".profile", is not one).
pub fn ext(p: []u8) []u8 {
    let b = base(p);
    let mut i = b.len;
    while i > 1 {
        i -= 1;
        if b[i] == '.' {
            return b[i..];
        }
    }
    return b[b.len..];
}

/// The last element without its extension: "dir/file.tar.gz" gives
/// "file.tar".
pub fn stem(p: []u8) []u8 {
    let b = base(p);
    return b[..b.len - ext(p).len];
}

/// p with its extension replaced by e (which should start with '.', or be
/// empty to remove it).
pub fn with_ext(p: []u8, e: []u8) ![]mut u8 {
    let old = ext(p);
    let mut end = p.len;
    while (end > 0) and (p[end - 1] == '/') {
        end -= 1;
    }
    let keep = p[..end - old.len];
    let out = mem.alloc.[u8](keep.len + e.len)!;
    @mem_copy(out[..keep.len], keep);
    @mem_copy(out[keep.len..], e);
    return out;
}

/// The path that leads from directory `from` to `to` (both cleaned first;
/// both must be absolute or both relative), such as "../b/c".
pub fn rel(from: []u8, to: []u8) ![]mut u8 {
    if is_abs(from) != is_abs(to) {
        return error.BadPattern;
    }
    let a = clean(from)!;
    defer mem.free(a);
    let b = clean(to)!;
    defer mem.free(b);
    let ae = elements(a);
    let be = elements(b);
    // the common prefix, in whole elements
    let mut i: usize = 0;
    let mut j: usize = 0;
    let mut common: usize = 0;
    let mut bcut: usize = 0;
    loop {
        let x = next_element(ae, &mut i);
        let y = next_element(be, &mut j);
        if (x.len == 0) or (y.len == 0) or not @mem_eq(x, y) {
            break;
        }
        common = i;
        bcut = j;
    }
    // a relative `from` that starts with ".." past the common part cannot
    // be climbed out of
    let rest = ae[common..];
    let mut out = strs.String.init();
    let mut k: usize = 0;
    loop {
        let x = next_element(rest, &mut k);
        if x.len == 0 {
            break;
        }
        if (x.len == 2) and (x[0] == '.') and (x[1] == '.') {
            out.deinit();
            return error.BadPattern;
        }
        if out.len > 0 {
            out.push('/') catch |e| {
                out.deinit();
                return e;
            };
        }
        out.push_str("..") catch |e| {
            out.deinit();
            return e;
        };
    }
    let tail = trim_seps(be[bcut..]);
    if tail.len > 0 {
        if out.len > 0 {
            out.push('/') catch |e| {
                out.deinit();
                return e;
            };
        }
        out.push_str(tail) catch |e| {
            out.deinit();
            return e;
        };
    }
    if out.len == 0 {
        out.push('.') catch |e| {
            out.deinit();
            return e;
        };
    }
    return out.take();
}

/// A cleaned path's elements, without "." for the empty relative path.
fn elements(p: []u8) []u8 {
    if (p.len == 1) and (p[0] == '.') {
        return p[..0];
    }
    return p;
}

fn trim_seps(p: []u8) []u8 {
    let mut s: usize = 0;
    while (s < p.len) and (p[s] == '/') {
        s += 1;
    }
    return p[s..];
}

/// The next element of p from position *i (empty at the end).
fn next_element(p: []u8, i: *mut usize) []u8 {
    while (i.* < p.len) and (p[i.*] == '/') {
        i.* += 1;
    }
    let s = i.*;
    while (i.* < p.len) and (p[i.*] != '/') {
        i.* += 1;
    }
    return p[s..i.*];
}

/// An iterator over the elements of a path (see elements()).
pub struct Elements {
    p: []u8,
    i: usize,

    /// The next element, or error.Done.
    pub fn next(self: *mut Elements) ![]u8 {
        let e = next_element(self.p, &mut self.i);
        if e.len == 0 {
            return error.Done;
        }
        return e;
    }
}

error { Done }

/// The elements of p ("/a//b/" gives "a" and "b").
pub fn each(p: []u8) Elements {
    return Elements.{ p: p, i: 0 };
}

// -----------------------------------------------------------------------------
// Glob patterns
// -----------------------------------------------------------------------------

/// Whether name matches the glob pattern within one element: '*' matches
/// any run of characters but '/', '?' any one character but '/', '[abc]',
/// '[a-z]' and '[!a-z]' (or '[^a-z]') a character class, '\\' escapes.
/// An ill-formed class matches nothing.
pub fn match_element(pattern: []u8, name: []u8) bool {
    // iterative, with backtracking to the last '*'
    let mut p: usize = 0;
    let mut n: usize = 0;
    let mut star_p: usize = @max_value(usize);
    let mut star_n: usize = 0;
    while n < name.len {
        if p < pattern.len {
            let c = pattern[p];
            if c == '*' {
                star_p = p;
                star_n = n;
                p += 1;
                continue;
            }
            if (c == '?') and (name[n] != '/') {
                p += 1;
                n += 1;
                continue;
            }
            if c == '[' {
                let r = match_class(pattern, p, name[n]);
                if r > 0 {
                    p = r;
                    n += 1;
                    continue;
                }
            } else if c == '\\' {
                if ((p + 1) < pattern.len) and (pattern[p + 1] == name[n]) {
                    p += 2;
                    n += 1;
                    continue;
                }
            } else if (c != '?') and (c == name[n]) {
                p += 1;
                n += 1;
                continue;
            }
        }
        // mismatch: let the last '*' take one more character
        if (star_p != @max_value(usize)) and (name[star_n] != '/') {
            star_n += 1;
            n = star_n;
            p = star_p + 1;
            continue;
        }
        return false;
    }
    while (p < pattern.len) and (pattern[p] == '*') {
        p += 1;
    }
    return p == pattern.len;
}

/// If the class at pattern[p] ('[') matches c: the position after it; else 0.
fn match_class(pattern: []u8, p0: usize, c: u8) usize {
    let mut p = p0 + 1;
    let mut negate = false;
    if (p < pattern.len) and ((pattern[p] == '!') or (pattern[p] == '^')) {
        negate = true;
        p += 1;
    }
    let mut hit = false;
    let mut first = true;
    while p < pattern.len {
        let mut lo = pattern[p];
        if (lo == ']') and not first {
            if hit != negate {
                return p + 1;
            }
            return 0;
        }
        first = false;
        if (lo == '\\') and ((p + 1) < pattern.len) {
            p += 1;
            lo = pattern[p];
        }
        p += 1;
        let mut hi = lo;
        if ((p + 1) < pattern.len) and (pattern[p] == '-') and (pattern[p + 1] != ']') {
            hi = pattern[p + 1];
            if (hi == '\\') and ((p + 2) < pattern.len) {
                p += 1;
                hi = pattern[p + 1];
            }
            p += 2;
        }
        if (c >= lo) and (c <= hi) {
            hit = true;
        }
    }
    return 0;  // unterminated
}

/// Whether the path matches the pattern, element by element (see
/// match_element); an element "**" matches any number of elements
/// (including none).
pub fn matches(pattern: []u8, p: []u8) bool {
    return match_from(pattern, 0, p, 0);
}

/// Elements of the pattern from pi0 against the path from xi0. "**" first
/// matches no elements; on a mismatch only the most recent "**" takes one
/// more (the others need not: it can absorb whatever they would), so the
/// work is at most elements times elements, however many "**" there are.
fn match_from(pat: []u8, pi0: usize, p: []u8, xi0: usize) bool {
    let mut pi = pi0;
    let mut xi = xi0;
    let mut star = false;
    let mut star_pi: usize = 0;  // the pattern just after the last "**"
    let mut star_xi: usize = 0;  // where the path resumes after what it took
    loop {
        let mut pj = pi;
        let pe = next_element(pat, &mut pj);
        if (pe.len == 2) and (pe[0] == '*') and (pe[1] == '*') {
            star = true;
            star_pi = pj;
            star_xi = xi;
            pi = pj;
            continue;
        }
        let mut xj = xi;
        let xe = next_element(p, &mut xj);
        if (pe.len == 0) and (xe.len == 0) {
            return true;
        }
        if (pe.len != 0) and (xe.len != 0) and match_element(pe, xe) {
            pi = pj;
            xi = xj;
            continue;
        }
        if not star {
            return false;
        }
        let mut at = star_xi;
        let skipped = next_element(p, &mut at);
        if skipped.len == 0 {
            return false;
        }
        star_xi = at;
        pi = star_pi;
        xi = at;
    }
}

/// Whether a pattern has any glob characters.
pub fn has_glob(pattern: []u8) bool {
    for c in pattern {
        if (c == '*') or (c == '?') or (c == '[') or (c == '\\') {
            return true;
        }
    }
    return false;
}

// -----------------------------------------------------------------------------
// Tests
// -----------------------------------------------------------------------------

fn expect_clean(p: []u8, want: []u8) !void {
    let c = clean(p)!;
    defer mem.free(c);
    @expect_eq(c, want);
}

test "clean" {
    let cases: [22][2][]u8 = [
        ["", "."],
        [".", "."],
        ["a", "a"],
        ["/", "/"],
        ["//", "/"],
        ["a/", "a"],
        ["a//b", "a/b"],
        ["a/./b", "a/b"],
        ["a/../b", "b"],
        ["a/..", "."],
        ["../a", "../a"],
        ["../../a/..", "../.."],
        ["/..", "/"],
        ["/../a", "/a"],
        ["a/b/../../..", ".."],
        ["./a/./b/.", "a/b"],
        ["abc/def/../ghi", "abc/ghi"],
        ["/a/b/c/../../d", "/a/d"],
        ["..", ".."],
        ["a/../..", ".."],
        ["a/b/c/..", "a/b"],
        ["...", "..."],
    ];
    for c in cases {
        expect_clean(c[0], c[1])!;
    }
}

test "join, base, dir, ext" {
    let parts: [4][]u8 = ["a", "", "b/../c", "file.txt"];
    let j = join(parts[..])!;
    defer mem.free(j);
    @expect_eq(j, "a/c/file.txt");
    let j2 = join2("/usr/", "/lib")!;
    defer mem.free(j2);
    @expect_eq(j2, "/usr/lib");
    @expect_eq(base("/a/b/"), "b");
    @expect_eq(base("/"), "/");
    @expect_eq(base(""), ".");
    @expect_eq(dir("a/b/c"), "a/b");
    @expect_eq(dir("a"), ".");
    @expect_eq(dir("/a"), "/");
    @expect_eq(dir("a//b"), "a");
    @expect_eq(ext("x/file.tar.gz"), ".gz");
    @expect_eq(ext(".profile"), "");
    @expect_eq(ext("a.b/c"), "");
    @expect_eq(stem("dir/file.tar.gz"), "file.tar");
    let w = with_ext("a/b.txt", ".md")!;
    defer mem.free(w);
    @expect_eq(w, "a/b.md");
    let mut it = each("/a//b/");
    @expect_eq(it.next()!, "a");
    @expect_eq(it.next()!, "b");
    _ = it.next() catch |e| {
        @expect_eq(e, error.Done);
        return;
    };
    @trap();
}

fn expect_rel(a: []u8, b: []u8, want: []u8) !void {
    let r = rel(a, b)!;
    defer mem.free(r);
    @expect_eq(r, want);
}

test "rel" {
    expect_rel("/a/b", "/a/b/c/d", "c/d")!;
    expect_rel("/a/b/c", "/a/x", "../../x")!;
    expect_rel("/a", "/a", ".")!;
    expect_rel("a/b", "a/c", "../c")!;
    expect_rel(".", "x/y", "x/y")!;
    expect_rel("/", "/x", "x")!;
    expect_rel("/x", "/", "..")!;
    _ = rel("/a", "b") catch |e| {
        @expect_eq(e, error.BadPattern);
        return;
    };
    @trap();
}

test "globs" {
    @expect(match_element("*.ph", "mem.ph"));
    @expect(not match_element("*.ph", "mem.c"));
    @expect(match_element("a?c", "abc"));
    @expect(match_element("[a-c]x", "bx"));
    @expect(not match_element("[!a-c]x", "bx"));
    @expect(match_element("[^a-c]x", "dx"));
    @expect(match_element("*a*b*c*", "xxaxxbxxcxx"));
    @expect(not match_element("*a*b*c*", "xxaxxcxxbxx"));
    @expect(match_element("\\*", "*"));
    @expect(not match_element("\\*", "x"));
    @expect(match_element("[]]", "]"));
    @expect(not match_element("[ab", "a"));
    @expect(match_element("", ""));
    @expect(match_element("*", ""));
    @expect(matches("src/*.ph", "src/mem.ph"));
    @expect(not matches("src/*.ph", "src/a/mem.ph"));
    @expect(matches("src/**/*.ph", "src/a/b/mem.ph"));
    @expect(matches("src/**/*.ph", "src/mem.ph"));
    @expect(matches("**", "a/b/c"));
    @expect(matches("a/**", "a"));
    @expect(not matches("a/*", "a"));
    @expect(has_glob("a/*.c"));
    @expect(not has_glob("a/b.c"));
}

test "many double stars stay fast" {
    let mut pat = strs.String.init();
    defer pat.deinit();
    for _k in @as(usize, 0)..12 {
        pat.push_str("**/")!;
    }
    pat.push_str("z")!;
    let mut p = strs.String.init();
    defer p.deinit();
    for _k in @as(usize, 0)..60 {
        p.push_str("a/")!;
    }
    p.push_str("y")!;
    @expect(not matches(pat.bytes(), p.bytes()));
    p.push_str("/z")!;
    @expect(matches(pat.bytes(), p.bytes()));
}
