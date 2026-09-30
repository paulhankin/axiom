// =============================================================================
// std/text/html.ph - HTML: the WHATWG parsing algorithm, a tree, CSS selectors
// and serialization.
//
//     let mut doc = html.parse(text)!;
//     defer doc.deinit();
//     let links = doc.query_all("article a[href]")!;      // CSS selectors
//     defer mem.free(links);
//     for id in links { let href = doc.attr(id, "href")!; ... }
//     let title = doc.title()!;                           // (the caller frees
//     let text = doc.text_content(doc.body())!;           //  these three)
//     let out = doc.to_html(0)!;                          // serialization
//
//     let mut frag = html.parse_fragment("<td>x", "tr", html.Options.default())!;
//
//     let mut t = html.Tokenizer.init(text)!;             // just the tokens
//     defer t.deinit();
//     loop { let tok = t.next()!; if tok.kind == html.TK_EOF { break; } ... }
//
// parse() implements the HTML Living Standard's parser: the tokenizer with
// every character reference, and tree construction with all its insertion
// modes, the adoption agency algorithm for misnested formatting, foster
// parenting out of tables, templates, the relaxed <select> content model,
// and SVG and MathML (foreign content). Every input yields a document, as
// in browsers; there are no parse errors to fail on. The trees match what
// browsers build: the parser passes all 1,792 tree construction tests of
// html5lib-tests (with scripting both off and on) and all 7,028 of its
// tokenizer tests (std/text/html/testdata, from tools/gen_html_tests.py).
// Scripts never run; Options.scripting only changes how <noscript> parses.
// Processing instructions are read as bogus comments, as browsers have
// always done.
//
// Nodes are numbered: node 0 is the document, and each node has parent,
// first/last child and previous/next sibling links (NONE when absent);
// template contents hang off `contents`. Input to parse() is UTF-8
// (invalid bytes become U+FFFD; a byte-order mark is dropped);
// parse_bytes() sniffs UTF-16 byte-order marks and windows-1252.
//
// Untrusted input: nothing recurses, and the work per token is bounded, so
// time grows linearly with the input whatever its shape. That takes two
// departures from the standard, which only very unusual documents notice:
// elements nested more than MAX_DEPTH deep are not opened (later content
// goes to their parent), and at most MAX_ACTIVE_FORMATTING formatting
// elements stay active at once.
// =============================================================================

import "../collections/arena.ph" as arena;
import "../collections/vec.ph" as vec;
import "../fmt.ph" as fmt;
import "../io.ph" as io;
import "../mem.ph" as mem;
import "../os/fs.ph" as fs;
import "../str.ph" as strs;
import "html/entities.ph" as ent;

error { OutOfMemory, NotFound, SyntaxError }

pub const NONE: u32 = 0xffffffff;

/// Elements nested deeper than this are not opened: later content goes to
/// their parent.
pub const MAX_DEPTH: usize = 512;

// Node kinds
pub const DOCUMENT: u8 = 0;
pub const DOCTYPE: u8 = 1;
pub const ELEMENT: u8 = 2;
pub const TEXT: u8 = 3;
pub const COMMENT: u8 = 4;
pub const FRAGMENT: u8 = 5;  // template contents; the result of parse_fragment

// Element namespaces
pub const NS_HTML: u8 = 0;
pub const NS_SVG: u8 = 1;
pub const NS_MATHML: u8 = 2;

// Attribute namespaces (from SVG and MathML: xlink:href, xml:lang, xmlns)
pub const A_NONE: u8 = 0;
pub const A_XLINK: u8 = 1;
pub const A_XML: u8 = 2;
pub const A_XMLNS: u8 = 3;

// Document modes
pub const NO_QUIRKS: u8 = 0;
pub const QUIRKS: u8 = 1;
pub const LIMITED_QUIRKS: u8 = 2;

pub let NAMESPACE_URIS: [3][]u8 = ["http://www.w3.org/1999/xhtml", "http://www.w3.org/2000/svg",
                                   "http://www.w3.org/1998/Math/MathML"];

// -----------------------------------------------------------------------------
// Tag names (generated: the names the parser treats specially)
// -----------------------------------------------------------------------------

const T_A: u16 = 1;
const T_ADDRESS: u16 = 2;
const T_ANNOTATION_XML: u16 = 3;
const T_APPLET: u16 = 4;
const T_AREA: u16 = 5;
const T_ARTICLE: u16 = 6;
const T_ASIDE: u16 = 7;
const T_B: u16 = 8;
const T_BASE: u16 = 9;
const T_BASEFONT: u16 = 10;
const T_BGSOUND: u16 = 11;
const T_BIG: u16 = 12;
const T_BLOCKQUOTE: u16 = 13;
const T_BODY: u16 = 14;
const T_BR: u16 = 15;
const T_BUTTON: u16 = 16;
const T_CAPTION: u16 = 17;
const T_CENTER: u16 = 18;
const T_CODE: u16 = 19;
const T_COL: u16 = 20;
const T_COLGROUP: u16 = 21;
const T_DD: u16 = 22;
const T_DESC: u16 = 23;
const T_DETAILS: u16 = 24;
const T_DIALOG: u16 = 25;
const T_DIR: u16 = 26;
const T_DIV: u16 = 27;
const T_DL: u16 = 28;
const T_DT: u16 = 29;
const T_EM: u16 = 30;
const T_EMBED: u16 = 31;
const T_FIELDSET: u16 = 32;
const T_FIGCAPTION: u16 = 33;
const T_FIGURE: u16 = 34;
const T_FONT: u16 = 35;
const T_FOOTER: u16 = 36;
const T_FOREIGNOBJECT: u16 = 37;
const T_FORM: u16 = 38;
const T_FRAME: u16 = 39;
const T_FRAMESET: u16 = 40;
const T_H1: u16 = 41;
const T_H2: u16 = 42;
const T_H3: u16 = 43;
const T_H4: u16 = 44;
const T_H5: u16 = 45;
const T_H6: u16 = 46;
const T_HEAD: u16 = 47;
const T_HEADER: u16 = 48;
const T_HGROUP: u16 = 49;
const T_HR: u16 = 50;
const T_HTML: u16 = 51;
const T_I: u16 = 52;
const T_IFRAME: u16 = 53;
const T_IMAGE: u16 = 54;
const T_IMG: u16 = 55;
const T_INPUT: u16 = 56;
const T_KEYGEN: u16 = 57;
const T_LI: u16 = 58;
const T_LINK: u16 = 59;
const T_LISTING: u16 = 60;
const T_MAIN: u16 = 61;
const T_MALIGNMARK: u16 = 62;
const T_MARQUEE: u16 = 63;
const T_MATH: u16 = 64;
const T_MENU: u16 = 65;
const T_META: u16 = 66;
const T_MGLYPH: u16 = 67;
const T_MI: u16 = 68;
const T_MN: u16 = 69;
const T_MO: u16 = 70;
const T_MS: u16 = 71;
const T_MTEXT: u16 = 72;
const T_NAV: u16 = 73;
const T_NOBR: u16 = 74;
const T_NOEMBED: u16 = 75;
const T_NOFRAMES: u16 = 76;
const T_NOSCRIPT: u16 = 77;
const T_OBJECT: u16 = 78;
const T_OL: u16 = 79;
const T_OPTGROUP: u16 = 80;
const T_OPTION: u16 = 81;
const T_P: u16 = 82;
const T_PARAM: u16 = 83;
const T_PLAINTEXT: u16 = 84;
const T_PRE: u16 = 85;
const T_RB: u16 = 86;
const T_RP: u16 = 87;
const T_RT: u16 = 88;
const T_RTC: u16 = 89;
const T_RUBY: u16 = 90;
const T_S: u16 = 91;
const T_SCRIPT: u16 = 92;
const T_SEARCH: u16 = 93;
const T_SECTION: u16 = 94;
const T_SELECT: u16 = 95;
const T_SELECTEDCONTENT: u16 = 96;
const T_SMALL: u16 = 97;
const T_SOURCE: u16 = 98;
const T_SPAN: u16 = 99;
const T_STRIKE: u16 = 100;
const T_STRONG: u16 = 101;
const T_STYLE: u16 = 102;
const T_SUB: u16 = 103;
const T_SUMMARY: u16 = 104;
const T_SUP: u16 = 105;
const T_SVG: u16 = 106;
const T_TABLE: u16 = 107;
const T_TBODY: u16 = 108;
const T_TD: u16 = 109;
const T_TEMPLATE: u16 = 110;
const T_TEXTAREA: u16 = 111;
const T_TFOOT: u16 = 112;
const T_TH: u16 = 113;
const T_THEAD: u16 = 114;
const T_TITLE: u16 = 115;
const T_TR: u16 = 116;
const T_TRACK: u16 = 117;
const T_TT: u16 = 118;
const T_U: u16 = 119;
const T_UL: u16 = 120;
const T_VAR: u16 = 121;
const T_WBR: u16 = 122;
const T_XMP: u16 = 123;

// The names of the T_ constants, sorted (T_x is index + 1).
let TAG_NAMES: [123][]u8 = [
    "a",
    "address",
    "annotation-xml",
    "applet",
    "area",
    "article",
    "aside",
    "b",
    "base",
    "basefont",
    "bgsound",
    "big",
    "blockquote",
    "body",
    "br",
    "button",
    "caption",
    "center",
    "code",
    "col",
    "colgroup",
    "dd",
    "desc",
    "details",
    "dialog",
    "dir",
    "div",
    "dl",
    "dt",
    "em",
    "embed",
    "fieldset",
    "figcaption",
    "figure",
    "font",
    "footer",
    "foreignobject",
    "form",
    "frame",
    "frameset",
    "h1",
    "h2",
    "h3",
    "h4",
    "h5",
    "h6",
    "head",
    "header",
    "hgroup",
    "hr",
    "html",
    "i",
    "iframe",
    "image",
    "img",
    "input",
    "keygen",
    "li",
    "link",
    "listing",
    "main",
    "malignmark",
    "marquee",
    "math",
    "menu",
    "meta",
    "mglyph",
    "mi",
    "mn",
    "mo",
    "ms",
    "mtext",
    "nav",
    "nobr",
    "noembed",
    "noframes",
    "noscript",
    "object",
    "ol",
    "optgroup",
    "option",
    "p",
    "param",
    "plaintext",
    "pre",
    "rb",
    "rp",
    "rt",
    "rtc",
    "ruby",
    "s",
    "script",
    "search",
    "section",
    "select",
    "selectedcontent",
    "small",
    "source",
    "span",
    "strike",
    "strong",
    "style",
    "sub",
    "summary",
    "sup",
    "svg",
    "table",
    "tbody",
    "td",
    "template",
    "textarea",
    "tfoot",
    "th",
    "thead",
    "title",
    "tr",
    "track",
    "tt",
    "u",
    "ul",
    "var",
    "wbr",
    "xmp",
];

const F_SPECIAL: u8 = 1;  // in the "special" category (HTML namespace)
const F_FORMATTING: u8 = 2;
const F_IMPLIED: u8 = 4;   // closed by "generate implied end tags"
const F_THOROUGH: u8 = 8;  // ... and by "generate all implied end tags thoroughly"
const F_SCOPE: u8 = 16;    // bounds "in scope" (HTML namespace)
const F_VOID: u8 = 32;     // serializes without an end tag
const F_RAW: u8 = 64;      // text children serialize unescaped

// The F_ flags of each T_ constant (0 for unknown names).
let TAG_FLAGS: [124]u8 = [
    0,
    2,
    1,
    0,
    17,
    33,
    1,
    1,
    2,
    33,
    33,
    33,
    2,
    1,
    1,
    33,
    1,
    25,
    1,
    2,
    33,
    9,
    13,
    0,
    1,
    0,
    1,
    1,
    1,
    13,
    2,
    33,
    1,
    1,
    1,
    2,
    1,
    0,
    1,
    33,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    33,
    17,
    2,
    65,
    0,
    33,
    33,
    33,
    13,
    33,
    1,
    1,
    0,
    17,
    0,
    1,
    33,
    0,
    0,
    0,
    0,
    0,
    0,
    1,
    2,
    65,
    65,
    1,
    17,
    1,
    12,
    12,
    13,
    33,
    65,
    1,
    12,
    12,
    12,
    12,
    0,
    2,
    65,
    1,
    1,
    17,
    0,
    2,
    33,
    0,
    2,
    2,
    65,
    0,
    1,
    0,
    0,
    17,
    9,
    25,
    17,
    1,
    9,
    25,
    9,
    1,
    9,
    33,
    2,
    2,
    1,
    0,
    33,
    65,
];

/// The T_ constant for a (lowercase) tag name, or 0.
fn tag_of(name: []u8) u16 {
    let mut lo: usize = 0;
    let mut hi: usize = TAG_NAMES.len;
    while lo < hi {
        let mid = (lo + hi) / 2;
        let c = strs.compare(TAG_NAMES[mid], name);
        if c == 0 {
            return @truncate(u16, mid + 1);
        }
        if c < 0 {
            lo = mid + 1;
        } else {
            hi = mid;
        }
    }
    return 0;
}

fn is_ws(c: u8) bool {
    return (c == ' ') or (c == '\t') or (c == '\n') or (c == 12) or (c == '\r');
}

fn is_alpha(c: i32) bool {
    return ((c >= 'a') and (c <= 'z')) or ((c >= 'A') and (c <= 'Z'));
}

fn is_alnum(c: i32) bool {
    return is_alpha(c) or ((c >= '0') and (c <= '9'));
}

fn lower(c: i32) u8 {
    if (c >= 'A') and (c <= 'Z') {
        return @truncate(u8, c + 32);
    }
    return @truncate(u8, c);
}

// -----------------------------------------------------------------------------
// Input
// -----------------------------------------------------------------------------

/// The input stream: s with CR and CRLF made LF and invalid UTF-8 replaced
/// by U+FFFD. Returns s itself when it needs no change, else a copy (also
/// stored in owned) that the caller frees.
fn preprocess(s: []u8, owned: *mut []mut u8) ![]u8 {
    let mut clean = true;
    let mut i: usize = 0;
    while i < s.len {
        let c = s[i];
        if c == '\r' {
            clean = false;
            break;
        }
        if c < 0x80 {
            i += 1;
            continue;
        }
        let r = strs.decode_rune(s[i..]);
        if (r.value == strs.REPLACEMENT) and (r.len == 1) {
            clean = false;
            break;
        }
        i += r.len;
    }
    if clean {
        return s;
    }
    let mut out = strs.String.init();
    normalize(s, i, &mut out) catch |e| {
        out.deinit();
        return e;
    };
    let b = out.take()!;
    owned.* = b;
    return b;
}

fn normalize(s: []u8, i0: usize, out: *mut strs.String) !void {
    let mut i = i0;
    out.reserve(s.len + 16)!;
    out.push_str(s[..i])!;
    while i < s.len {
        let c = s[i];
        if c == '\r' {
            out.push('\n')!;
            i += 1;
            if (i < s.len) and (s[i] == '\n') {
                i += 1;
            }
            continue;
        }
        if c < 0x80 {
            out.push(c)!;
            i += 1;
            continue;
        }
        let r = strs.decode_rune(s[i..]);
        if (r.value == strs.REPLACEMENT) and (r.len == 1) {
            out.push_str("\xef\xbf\xbd")!;
        } else {
            out.push_str(s[i..i + r.len])!;
        }
        i += r.len;
    }
}

// -----------------------------------------------------------------------------
// Tokenizer
// -----------------------------------------------------------------------------

// Token kinds
pub const TK_NONE: u8 = 0;
pub const TK_DOCTYPE: u8 = 1;
pub const TK_START: u8 = 2;
pub const TK_END: u8 = 3;
pub const TK_COMMENT: u8 = 4;
pub const TK_CHARS: u8 = 5;
pub const TK_EOF: u8 = 6;

// Tokenizer states (those the tree builder or a caller may set are public)
pub const S_DATA: u8 = 0;
pub const S_RCDATA: u8 = 1;
pub const S_RAWTEXT: u8 = 2;
pub const S_SCRIPT: u8 = 3;
pub const S_PLAINTEXT: u8 = 4;
pub const S_CDATA: u8 = 5;
const S_TAG_OPEN: u8 = 6;
const S_END_TAG_OPEN: u8 = 7;
const S_TAG_NAME: u8 = 8;
const S_RCDATA_LT: u8 = 9;
const S_RCDATA_END_OPEN: u8 = 10;
const S_RCDATA_END_NAME: u8 = 11;
const S_RAWTEXT_LT: u8 = 12;
const S_RAWTEXT_END_OPEN: u8 = 13;
const S_RAWTEXT_END_NAME: u8 = 14;
const S_SCRIPT_LT: u8 = 15;
const S_SCRIPT_END_OPEN: u8 = 16;
const S_SCRIPT_END_NAME: u8 = 17;
const S_SCRIPT_ESC_START: u8 = 18;
const S_SCRIPT_ESC_START_DASH: u8 = 19;
const S_SCRIPT_ESC: u8 = 20;
const S_SCRIPT_ESC_DASH: u8 = 21;
const S_SCRIPT_ESC_DASH_DASH: u8 = 22;
const S_SCRIPT_ESC_LT: u8 = 23;
const S_SCRIPT_ESC_END_OPEN: u8 = 24;
const S_SCRIPT_ESC_END_NAME: u8 = 25;
const S_SCRIPT_DESC_START: u8 = 26;
const S_SCRIPT_DESC: u8 = 27;
const S_SCRIPT_DESC_DASH: u8 = 28;
const S_SCRIPT_DESC_DASH_DASH: u8 = 29;
const S_SCRIPT_DESC_LT: u8 = 30;
const S_SCRIPT_DESC_END: u8 = 31;
const S_BEFORE_ATTR_NAME: u8 = 32;
const S_ATTR_NAME: u8 = 33;
const S_AFTER_ATTR_NAME: u8 = 34;
const S_BEFORE_ATTR_VALUE: u8 = 35;
const S_ATTR_VALUE_DQ: u8 = 36;
const S_ATTR_VALUE_SQ: u8 = 37;
const S_ATTR_VALUE_UNQ: u8 = 38;
const S_AFTER_ATTR_VALUE: u8 = 39;
const S_SELF_CLOSING: u8 = 40;
const S_BOGUS_COMMENT: u8 = 41;
const S_MARKUP_DECL: u8 = 42;
const S_COMMENT_START: u8 = 43;
const S_COMMENT_START_DASH: u8 = 44;
const S_COMMENT: u8 = 45;
const S_COMMENT_LT: u8 = 46;
const S_COMMENT_LT_BANG: u8 = 47;
const S_COMMENT_LT_BANG_DASH: u8 = 48;
const S_COMMENT_LT_BANG_DASH_DASH: u8 = 49;
const S_COMMENT_END_DASH: u8 = 50;
const S_COMMENT_END: u8 = 51;
const S_COMMENT_END_BANG: u8 = 52;
const S_DOCTYPE: u8 = 53;
const S_BEFORE_DOCTYPE_NAME: u8 = 54;
const S_DOCTYPE_NAME: u8 = 55;
const S_AFTER_DOCTYPE_NAME: u8 = 56;
const S_AFTER_DOCTYPE_PUBLIC_KW: u8 = 57;
const S_BEFORE_DOCTYPE_PUBLIC_ID: u8 = 58;
const S_DOCTYPE_PUBLIC_ID_DQ: u8 = 59;
const S_DOCTYPE_PUBLIC_ID_SQ: u8 = 60;
const S_AFTER_DOCTYPE_PUBLIC_ID: u8 = 61;
const S_BETWEEN_DOCTYPE_IDS: u8 = 62;
const S_AFTER_DOCTYPE_SYSTEM_KW: u8 = 63;
const S_BEFORE_DOCTYPE_SYSTEM_ID: u8 = 64;
const S_DOCTYPE_SYSTEM_ID_DQ: u8 = 65;
const S_DOCTYPE_SYSTEM_ID_SQ: u8 = 66;
const S_AFTER_DOCTYPE_SYSTEM_ID: u8 = 67;
const S_BOGUS_DOCTYPE: u8 = 68;
const S_CDATA_BRACKET: u8 = 69;
const S_CDATA_END: u8 = 70;

pub struct Attr {
    ns: u8,        // A_NONE, or A_XLINK, A_XML or A_XMLNS on foreign elements
    prefix: []u8,  // "xlink" etc. with a namespace, else ""
    name: []u8,    // the local name
    value: []u8,
}

struct Span {
    ns: usize,  // name and value, as offsets into the attribute buffer (the
    ne: usize,  // value runs to the next attribute's name)
    vs: usize,
    dup: bool,  // repeats an earlier name: dropped
}

/// A token: valid until the next call of Tokenizer.next.
pub struct Token {
    kind: u8,
    name: []u8,  // tag name (lowercase) or DOCTYPE name
    tag: u16,    // the tag name's T_ constant, or 0
    attrs: []Attr,
    self_closing: bool,
    data: []u8,  // TK_CHARS and TK_COMMENT
    public_id: []u8,
    system_id: []u8,
    has_name: bool,  // DOCTYPE: whether each part was present
    has_public: bool,
    has_system: bool,
    force_quirks: bool,
}

/// The HTML tokenizer. Characters come as runs (TK_CHARS); a caller building
/// a tree changes `state` after start tags as the tree builder does.
pub struct Tokenizer {
    s: []u8,
    owned: []mut u8,  // s, when preprocessing had to copy it
    i: usize,
    state: u8,
    last_start: strs.String,  // the last start tag's name (for appropriate end tags)
    cdata_ok: bool,           // <![CDATA[ opens a section (set in foreign content)
    kind: u8,                 // the tag being built: TK_START or TK_END
    self_closing: bool,
    name: strs.String,
    tmp: strs.String,
    comment: strs.String,
    chars: strs.String,
    abuf: strs.String,  // the tag's attribute names and values
    spans: vec.Vec.[Span],
    seen: vec.Vec.[u32],  // a hash index of spans (for tags with many attributes)
    attrs: vec.Vec.[Attr],
    public_id: strs.String,
    system_id: strs.String,
    has_name: bool,
    has_public: bool,
    has_system: bool,
    force_quirks: bool,
    pending: u8,  // a finished token waiting behind characters
    done: bool,
    errors: u32,  // parse errors seen (they are not fatal)

    /// A tokenizer over text (which must outlive it).
    pub fn init(text: []u8) !Tokenizer {
        let mut owned: []mut u8 = @slice_from_ptr(@ptr_from_int(*mut u8, 1), 0);
        let s = preprocess(text, &mut owned)!;
        return Tokenizer.{ s: s, owned: owned, i: 0, state: S_DATA, last_start: strs.String.init(), cdata_ok: false,
                           kind: TK_START, self_closing: false, name: strs.String.init(), tmp: strs.String.init(),
                           comment: strs.String.init(), chars: strs.String.init(), abuf: strs.String.init(),
                           spans: vec.Vec.[Span].init(), seen: vec.Vec.[u32].init(), attrs: vec.Vec.[Attr].init(),
                           public_id: strs.String.init(), system_id: strs.String.init(), has_name: false,
                           has_public: false, has_system: false, force_quirks: false, pending: TK_NONE, done: false,
                           errors: 0 };
    }

    pub fn deinit(self: *mut Tokenizer) void {
        if self.owned.len > 0 {
            mem.free(self.owned);
        }
        self.last_start.deinit();
        self.name.deinit();
        self.tmp.deinit();
        self.comment.deinit();
        self.chars.deinit();
        self.abuf.deinit();
        self.spans.deinit();
        self.seen.deinit();
        self.attrs.deinit();
        self.public_id.deinit();
        self.system_id.deinit();
    }

    /// Sets the name of the last start tag (as a tree builder's context
    /// element would), which decides which end tags close RCDATA and the like.
    pub fn set_last_start_tag(self: *mut Tokenizer, name: []u8) !void {
        self.last_start.clear();
        self.last_start.push_str(name)!;
    }

    /// The next token. After TK_EOF, TK_EOF again.
    pub fn next(self: *mut Tokenizer) !Token {
        if self.pending != TK_NONE {
            let k = self.pending;
            self.pending = TK_NONE;
            return self.make(k);
        }
        self.chars.clear();
        if self.done {
            return self.make(TK_EOF);
        }
        let k = self.run()!;
        if k == TK_EOF {
            self.done = true;
        } else if self.i >= self.s.len {
            self.state = S_DATA;  // (a comment or DOCTYPE cut off by the end: EOF next)
        }
        if self.chars.len > 0 {
            self.pending = k;
            return self.make(TK_CHARS);
        }
        return self.make(k);
    }

    fn make(self: *mut Tokenizer, k: u8) !Token {
        let mut t = Token.{ kind: k, name: "", tag: 0, attrs: self.attrs.items()[..0], self_closing: false, data: "",
                            public_id: "", system_id: "", has_name: false, has_public: false, has_system: false,
                            force_quirks: false };
        if k == TK_CHARS {
            t.data = self.chars.bytes();
        } else if k == TK_COMMENT {
            t.data = self.comment.bytes();
        } else if (k == TK_START) or (k == TK_END) {
            t.name = self.name.bytes();
            t.tag = tag_of(t.name);
            t.self_closing = self.self_closing;
            self.attrs.clear();
            let b = self.abuf.bytes();
            let sp = self.spans.items();
            for k2 in @as(usize, 0)..sp.len {
                let mut ve = b.len;
                if (k2 + 1) < sp.len {
                    ve = sp[k2 + 1].ns;
                }
                if sp[k2].dup {
                    continue;
                }
                self.attrs.push(Attr.{ ns: A_NONE, prefix: "", name: b[sp[k2].ns..sp[k2].ne],
                                       value: b[sp[k2].vs..ve] })!;
            }
            t.attrs = self.attrs.items();
            if k == TK_START {
                self.last_start.clear();
                self.last_start.push_str(t.name)!;
            }
        } else if k == TK_DOCTYPE {
            t.name = self.name.bytes();
            t.public_id = self.public_id.bytes();
            t.system_id = self.system_id.bytes();
            t.has_name = self.has_name;
            t.has_public = self.has_public;
            t.has_system = self.has_system;
            t.force_quirks = self.force_quirks;
        }
        return t;
    }

    fn at(self: *Tokenizer) i32 {
        if self.i < self.s.len {
            return @as(i32, self.s[self.i]);
        }
        return -1;
    }

    fn err(self: *mut Tokenizer) void {
        self.errors +%= 1;
    }

    fn new_tag(self: *mut Tokenizer, k: u8) void {
        self.kind = k;
        self.self_closing = false;
        self.name.clear();
        self.abuf.clear();
        self.spans.clear();
    }

    fn new_doctype(self: *mut Tokenizer) void {
        self.name.clear();
        self.public_id.clear();
        self.system_id.clear();
        self.has_name = false;
        self.has_public = false;
        self.has_system = false;
        self.force_quirks = false;
    }

    // Starts an attribute; its name follows in abuf.
    fn new_attr(self: *mut Tokenizer) !void {
        self.spans.push(Span.{ ns: self.abuf.len, ne: self.abuf.len, vs: self.abuf.len, dup: false })!;
    }

    // The attribute name is complete: mark it if it repeats one.
    fn end_attr_name(self: *mut Tokenizer) !void {
        let n = self.spans.len;
        let b = self.abuf.bytes();
        let sp = self.spans.get_ptr(n - 1);
        sp.ne = b.len;
        sp.vs = b.len;
        let nm = b[sp.ns..sp.ne];
        if n <= 16 {
            for k in @as(usize, 0)..n - 1 {
                let o = self.spans.get(k);
                if not o.dup and strs.eq(b[o.ns..o.ne], nm) {
                    self.err();
                    sp.dup = true;
                    return;
                }
            }
            return;
        }
        // many attributes: an open-addressed table of span indexes
        if (n == 17) or ((n * 2) > self.seen.len) {
            let mut size: usize = 64;
            while size < (n * 4) {
                size *= 2;
            }
            self.seen.clear();
            for _k in @as(usize, 0)..size {
                self.seen.push(NONE)!;
            }
            for k in @as(usize, 0)..n - 1 {
                let o = self.spans.get(k);
                if not o.dup {
                    _ = self.seen_insert(b, @truncate(u32, k));
                }
            }
        }
        if not self.seen_insert(b, @truncate(u32, n - 1)) {
            self.err();
            sp.dup = true;
        }
    }

    // Adds span k to the index: false if its name is there already.
    fn seen_insert(self: *mut Tokenizer, b: []u8, k: u32) bool {
        let sp = self.spans.get(@as(usize, k));
        let nm = b[sp.ns..sp.ne];
        let mut h: u32 = 2166136261;
        for c in nm {
            h = (h ^ @as(u32, c)) *% 16777619;
        }
        let mask = self.seen.len - 1;
        let mut at = @as(usize, h) & mask;
        loop {
            let e = self.seen.get(at);
            if e == NONE {
                self.seen.set(at, k);
                return true;
            }
            let o = self.spans.get(@as(usize, e));
            if strs.eq(b[o.ns..o.ne], nm) {
                return false;
            }
            at = (at + 1) & mask;
        }
    }

    // Finishes the current tag: the kind to return.
    fn emit_tag(self: *mut Tokenizer) u8 {
        if self.kind == TK_END {
            if (self.spans.len > 0) or self.self_closing {
                self.err();
            }
        }
        return self.kind;
    }

    // Whether the end tag being read matches the last start tag.
    fn appropriate(self: *Tokenizer) bool {
        return (self.last_start.len > 0) and strs.eq(self.name.bytes(), self.last_start.bytes());
    }

    // "</" and the temporary buffer, as characters.
    fn flush_end_tag(self: *mut Tokenizer) !void {
        self.chars.push_str("</")!;
        self.chars.push_str(self.tmp.bytes())!;
    }

    fn lookahead_ci(self: *Tokenizer, word: []u8) bool {
        if (self.s.len - self.i) < word.len {
            return false;
        }
        return strs.eq_ignore_case(self.s[self.i..self.i + word.len], word);
    }

    fn lookahead(self: *Tokenizer, word: []u8) bool {
        return strs.starts_with(self.s[self.i..], word);
    }

    // Appends a character reference's text to the attribute value or the
    // characters (after the "&" has been consumed).
    fn char_ref(self: *mut Tokenizer, in_attr: bool) !void {
        let mut out = &mut self.chars;
        if in_attr {
            out = &mut self.abuf;
        }
        let s = self.s;
        let i = self.i;
        if (i < s.len) and is_alnum(@as(i32, s[i])) {
            let mut k: usize = 0;
            while ((i + k) < s.len) and is_alnum(@as(i32, s[i + k])) and (k < 40) {
                k += 1;
            }
            // the longest match: with a semicolon (only possible after all k
            // characters), or a legacy name without one
            let mut len: usize = 0;
            let mut idx: usize = 0;
            let mut semi = false;
            if ((i + k) < s.len) and (s[i + k] == ';') {
                let e = entity_index(s[i..i + k]) catch NONE_USIZE;
                if e != NONE_USIZE {
                    len = k + 1;
                    idx = e;
                    semi = true;
                }
            }
            if len == 0 {
                let mut l = @min(k, 8);  // (legacy names are short)
                while l > 0 {
                    let e = legacy_index(s[i..i + l]) catch NONE_USIZE;
                    if e != NONE_USIZE {
                        len = l;
                        idx = e;
                        break;
                    }
                    l -= 1;
                }
            }
            if len == 0 {
                out.push('&')!;  // (the ambiguous ampersand: what follows is text)
                return;
            }
            if in_attr and not semi and ((i + len) < s.len) and
               ((s[i + len] == '=') or is_alnum(@as(i32, s[i + len]))) {
                out.push('&')!;
                out.push_str(s[i..i + len])!;
                self.i = i + len;
                return;
            }
            if not semi {
                self.err();
            }
            out.push_rune(ent.FIRST[idx])!;
            if ent.SECOND[idx] != 0 {
                out.push_rune(ent.SECOND[idx])!;
            }
            self.i = i + len;
            return;
        }
        if (i < s.len) and (s[i] == '#') {
            let mut j = i + 1;
            let mut hex = false;
            if (j < s.len) and ((s[j] == 'x') or (s[j] == 'X')) {
                hex = true;
                j += 1;
            }
            let st = j;
            let mut v: u32 = 0;
            while j < s.len {
                let c = s[j];
                let mut d: u32 = 16;
                if (c >= '0') and (c <= '9') {
                    d = @as(u32, c - '0');
                } else if hex and (c >= 'a') and (c <= 'f') {
                    d = @as(u32, c - 'a') + 10;
                } else if hex and (c >= 'A') and (c <= 'F') {
                    d = @as(u32, c - 'A') + 10;
                }
                if d == 16 {
                    break;
                }
                if hex {
                    v = (v * 16) + d;
                } else {
                    v = (v * 10) + d;
                }
                if v > 0x10ffff {
                    v = 0x110000;
                }
                j += 1;
            }
            if j == st {
                self.err();
                out.push_str(s[i - 1..j])!;  // "&#" or "&#x", as text
                self.i = j;
                return;
            }
            if (j < s.len) and (s[j] == ';') {
                j += 1;
            } else {
                self.err();
            }
            self.i = j;
            if (v == 0) or (v > 0x10ffff) or ((v >= 0xd800) and (v <= 0xdfff)) {
                self.err();
                v = 0xfffd;
            } else if (v >= 0x80) and (v <= 0x9f) {
                self.err();
                let w = C1[@as(usize, v - 0x80)];
                if w != 0 {
                    v = w;
                }
            } else if ((v < 0x20) and (v != '\t') and (v != '\n') and (v != 12)) or (v == 0x7f) {
                self.err();
            }
            out.push_rune(v)!;
            return;
        }
        out.push('&')!;
    }

    // Runs the state machine until a token other than characters is complete
    // (characters collect in self.chars): returns its kind.
    fn run(self: *mut Tokenizer) !u8 {
        let s = self.s;
        let n = s.len;
        loop {
            let st = self.state;
            // --- the text states, a run at a time
            if st == S_DATA {
                let b = self.i;
                let mut i = b;
                while (i < n) and (s[i] != '<') and (s[i] != '&') and (s[i] != 0) {
                    i += 1;
                }
                self.chars.push_str(s[b..i])!;
                self.i = i;
                if i >= n {
                    return TK_EOF;
                }
                self.i = i + 1;
                let c = s[i];
                if c == '&' {
                    self.char_ref(false)!;
                } else if c == '<' {
                    self.state = S_TAG_OPEN;
                } else {
                    self.err();
                    self.chars.push(0)!;  // (the tree builder drops it)
                }
                continue;
            }
            if (st == S_RCDATA) or (st == S_RAWTEXT) or (st == S_SCRIPT) or (st == S_PLAINTEXT) {
                let b = self.i;
                let mut i = b;
                while (i < n) and (s[i] != '<') and (s[i] != 0) and ((s[i] != '&') or (st != S_RCDATA)) {
                    i += 1;
                }
                self.chars.push_str(s[b..i])!;
                self.i = i;
                if i >= n {
                    return TK_EOF;
                }
                self.i = i + 1;
                let c = s[i];
                if c == 0 {
                    self.err();
                    self.chars.push_str("\xef\xbf\xbd")!;
                } else if c == '&' {
                    self.char_ref(false)!;
                } else if st == S_PLAINTEXT {
                    self.chars.push('<')!;
                } else if st == S_RCDATA {
                    self.state = S_RCDATA_LT;
                } else if st == S_RAWTEXT {
                    self.state = S_RAWTEXT_LT;
                } else {
                    self.state = S_SCRIPT_LT;
                }
                continue;
            }
            let c = self.at();
            if st == S_TAG_NAME {
                if c < 0 {
                    self.err();
                    return TK_EOF;
                }
                self.i += 1;
                let b = @truncate(u8, c);
                if is_ws(b) {
                    self.state = S_BEFORE_ATTR_NAME;
                } else if b == '/' {
                    self.state = S_SELF_CLOSING;
                } else if b == '>' {
                    self.state = S_DATA;
                    return self.emit_tag();
                } else if b == 0 {
                    self.err();
                    self.name.push_str("\xef\xbf\xbd")!;
                } else {
                    self.name.push(lower(c))!;
                }
                continue;
            }
            if st == S_BEFORE_ATTR_NAME {
                if (c == '\t') or (c == '\n') or (c == 12) or (c == ' ') {
                    self.i += 1;
                } else if (c == '/') or (c == '>') or (c < 0) {
                    self.state = S_AFTER_ATTR_NAME;
                } else if c == '=' {
                    self.err();
                    self.new_attr()!;
                    self.abuf.push('=')!;
                    self.i += 1;
                    self.state = S_ATTR_NAME;
                } else {
                    self.new_attr()!;
                    self.state = S_ATTR_NAME;
                }
                continue;
            }
            if st == S_ATTR_NAME {
                if (c == '\t') or (c == '\n') or (c == 12) or (c == ' ') or (c == '/') or (c == '>') or (c < 0) {
                    self.end_attr_name()!;
                    self.state = S_AFTER_ATTR_NAME;
                } else if c == '=' {
                    self.end_attr_name()!;
                    self.i += 1;
                    self.state = S_BEFORE_ATTR_VALUE;
                } else {
                    self.i += 1;
                    if c == 0 {
                        self.err();
                        self.abuf.push_str("\xef\xbf\xbd")!;
                    } else {
                        if (c == '"') or (c == '\'') or (c == '<') {
                            self.err();
                        }
                        self.abuf.push(lower(c))!;
                    }
                }
                continue;
            }
            if st == S_AFTER_ATTR_NAME {
                if (c == '\t') or (c == '\n') or (c == 12) or (c == ' ') {
                    self.i += 1;
                } else if c == '/' {
                    self.i += 1;
                    self.state = S_SELF_CLOSING;
                } else if c == '=' {
                    self.i += 1;
                    self.state = S_BEFORE_ATTR_VALUE;
                } else if c == '>' {
                    self.i += 1;
                    self.state = S_DATA;
                    return self.emit_tag();
                } else if c < 0 {
                    self.err();
                    return TK_EOF;
                } else {
                    self.new_attr()!;
                    self.state = S_ATTR_NAME;
                }
                continue;
            }
            if st == S_BEFORE_ATTR_VALUE {
                if (c == '\t') or (c == '\n') or (c == 12) or (c == ' ') {
                    self.i += 1;
                } else if c == '"' {
                    self.i += 1;
                    self.state = S_ATTR_VALUE_DQ;
                } else if c == '\'' {
                    self.i += 1;
                    self.state = S_ATTR_VALUE_SQ;
                } else if c == '>' {
                    self.err();
                    self.i += 1;
                    self.state = S_DATA;
                    return self.emit_tag();
                } else {
                    self.state = S_ATTR_VALUE_UNQ;
                }
                continue;
            }
            if (st == S_ATTR_VALUE_DQ) or (st == S_ATTR_VALUE_SQ) {
                let mut qq: u8 = '"';
                if st == S_ATTR_VALUE_SQ {
                    qq = '\'';
                }
                let b = self.i;
                let mut i = b;
                while (i < n) and (s[i] != qq) and (s[i] != '&') and (s[i] != 0) {
                    i += 1;
                }
                self.abuf.push_str(s[b..i])!;
                self.i = i;
                if i >= n {
                    self.err();
                    return TK_EOF;
                }
                self.i = i + 1;
                if s[i] == qq {
                    self.state = S_AFTER_ATTR_VALUE;
                } else if s[i] == '&' {
                    self.char_ref(true)!;
                } else {
                    self.err();
                    self.abuf.push_str("\xef\xbf\xbd")!;
                }
                continue;
            }
            if st == S_ATTR_VALUE_UNQ {
                if c < 0 {
                    self.err();
                    return TK_EOF;
                }
                self.i += 1;
                let b = @truncate(u8, c);
                if (b == '\t') or (b == '\n') or (b == 12) or (b == ' ') {
                    self.state = S_BEFORE_ATTR_NAME;
                } else if b == '&' {
                    self.char_ref(true)!;
                } else if b == '>' {
                    self.state = S_DATA;
                    return self.emit_tag();
                } else if b == 0 {
                    self.err();
                    self.abuf.push_str("\xef\xbf\xbd")!;
                } else {
                    if (b == '"') or (b == '\'') or (b == '<') or (b == '=') or (b == '`') {
                        self.err();
                    }
                    self.abuf.push(b)!;
                }
                continue;
            }
            if st == S_AFTER_ATTR_VALUE {
                if (c == '\t') or (c == '\n') or (c == 12) or (c == ' ') {
                    self.i += 1;
                    self.state = S_BEFORE_ATTR_NAME;
                } else if c == '/' {
                    self.i += 1;
                    self.state = S_SELF_CLOSING;
                } else if c == '>' {
                    self.i += 1;
                    self.state = S_DATA;
                    return self.emit_tag();
                } else if c < 0 {
                    self.err();
                    return TK_EOF;
                } else {
                    self.err();
                    self.state = S_BEFORE_ATTR_NAME;
                }
                continue;
            }
            if st == S_SELF_CLOSING {
                if c == '>' {
                    self.i += 1;
                    self.self_closing = true;
                    self.state = S_DATA;
                    return self.emit_tag();
                } else if c < 0 {
                    self.err();
                    return TK_EOF;
                } else {
                    self.err();
                    self.state = S_BEFORE_ATTR_NAME;
                }
                continue;
            }
            if st == S_TAG_OPEN {
                if c == '!' {
                    self.i += 1;
                    self.state = S_MARKUP_DECL;
                } else if c == '/' {
                    self.i += 1;
                    self.state = S_END_TAG_OPEN;
                } else if is_alpha(c) {
                    self.new_tag(TK_START);
                    self.state = S_TAG_NAME;
                } else if c == '?' {
                    self.err();
                    self.comment.clear();
                    self.state = S_BOGUS_COMMENT;
                } else if c < 0 {
                    self.err();
                    self.chars.push('<')!;
                    return TK_EOF;
                } else {
                    self.err();
                    self.chars.push('<')!;
                    self.state = S_DATA;
                }
                continue;
            }
            if st == S_END_TAG_OPEN {
                if is_alpha(c) {
                    self.new_tag(TK_END);
                    self.state = S_TAG_NAME;
                } else if c == '>' {
                    self.err();
                    self.i += 1;
                    self.state = S_DATA;
                } else if c < 0 {
                    self.err();
                    self.chars.push_str("</")!;
                    return TK_EOF;
                } else {
                    self.err();
                    self.comment.clear();
                    self.state = S_BOGUS_COMMENT;
                }
                continue;
            }
            // --- RCDATA, RAWTEXT and script data end tags
            if (st == S_RCDATA_LT) or (st == S_RAWTEXT_LT) {
                let mut back = S_RCDATA;
                if st == S_RAWTEXT_LT {
                    back = S_RAWTEXT;
                }
                if c == '/' {
                    self.i += 1;
                    self.tmp.clear();
                    if st == S_RCDATA_LT {
                        self.state = S_RCDATA_END_OPEN;
                    } else {
                        self.state = S_RAWTEXT_END_OPEN;
                    }
                } else {
                    self.chars.push('<')!;
                    self.state = back;
                }
                continue;
            }
            if (st == S_RCDATA_END_OPEN) or (st == S_RAWTEXT_END_OPEN) or (st == S_SCRIPT_END_OPEN) or
               (st == S_SCRIPT_ESC_END_OPEN) {
                if is_alpha(c) {
                    self.new_tag(TK_END);
                    self.state = st + 1;
                } else {
                    self.chars.push_str("</")!;
                    self.state = end_tag_back(st);
                }
                continue;
            }
            if (st == S_RCDATA_END_NAME) or (st == S_RAWTEXT_END_NAME) or (st == S_SCRIPT_END_NAME) or
               (st == S_SCRIPT_ESC_END_NAME) {
                if ((c == '\t') or (c == '\n') or (c == 12) or (c == ' ')) and self.appropriate() {
                    self.i += 1;
                    self.state = S_BEFORE_ATTR_NAME;
                } else if (c == '/') and self.appropriate() {
                    self.i += 1;
                    self.state = S_SELF_CLOSING;
                } else if (c == '>') and self.appropriate() {
                    self.i += 1;
                    self.state = S_DATA;
                    return self.emit_tag();
                } else if is_alpha(c) {
                    self.i += 1;
                    self.name.push(lower(c))!;
                    self.tmp.push(@truncate(u8, c))!;
                } else {
                    self.flush_end_tag()!;
                    self.state = end_tag_back(st - 1);
                }
                continue;
            }
            if st == S_SCRIPT_LT {
                if c == '/' {
                    self.i += 1;
                    self.tmp.clear();
                    self.state = S_SCRIPT_END_OPEN;
                } else if c == '!' {
                    self.i += 1;
                    self.chars.push_str("<!")!;
                    self.state = S_SCRIPT_ESC_START;
                } else {
                    self.chars.push('<')!;
                    self.state = S_SCRIPT;
                }
                continue;
            }
            if (st == S_SCRIPT_ESC_START) or (st == S_SCRIPT_ESC_START_DASH) {
                if c == '-' {
                    self.i += 1;
                    self.chars.push('-')!;
                    if st == S_SCRIPT_ESC_START {
                        self.state = S_SCRIPT_ESC_START_DASH;
                    } else {
                        self.state = S_SCRIPT_ESC_DASH_DASH;
                    }
                } else {
                    self.state = S_SCRIPT;
                }
                continue;
            }
            if (st == S_SCRIPT_ESC) or (st == S_SCRIPT_ESC_DASH) or (st == S_SCRIPT_ESC_DASH_DASH) {
                if c < 0 {
                    self.err();
                    return TK_EOF;
                }
                self.i += 1;
                if c == '-' {
                    self.chars.push('-')!;
                    if st == S_SCRIPT_ESC {
                        self.state = S_SCRIPT_ESC_DASH;
                    } else {
                        self.state = S_SCRIPT_ESC_DASH_DASH;
                    }
                } else if c == '<' {
                    self.state = S_SCRIPT_ESC_LT;
                } else if (c == '>') and (st == S_SCRIPT_ESC_DASH_DASH) {
                    self.chars.push('>')!;
                    self.state = S_SCRIPT;
                } else if c == 0 {
                    self.err();
                    self.chars.push_str("\xef\xbf\xbd")!;
                    self.state = S_SCRIPT_ESC;
                } else {
                    self.chars.push(@truncate(u8, c))!;
                    self.state = S_SCRIPT_ESC;
                }
                continue;
            }
            if st == S_SCRIPT_ESC_LT {
                if c == '/' {
                    self.i += 1;
                    self.tmp.clear();
                    self.state = S_SCRIPT_ESC_END_OPEN;
                } else if is_alpha(c) {
                    self.tmp.clear();
                    self.chars.push('<')!;
                    self.state = S_SCRIPT_DESC_START;
                } else {
                    self.chars.push('<')!;
                    self.state = S_SCRIPT_ESC;
                }
                continue;
            }
            if (st == S_SCRIPT_DESC_START) or (st == S_SCRIPT_DESC_END) {
                if (c == '\t') or (c == '\n') or (c == 12) or (c == ' ') or (c == '/') or (c == '>') {
                    self.i += 1;
                    self.chars.push(@truncate(u8, c))!;
                    let is_script = strs.eq(self.tmp.bytes(), "script");
                    if st == S_SCRIPT_DESC_START {
                        if is_script {
                            self.state = S_SCRIPT_DESC;
                        } else {
                            self.state = S_SCRIPT_ESC;
                        }
                    } else {
                        if is_script {
                            self.state = S_SCRIPT_ESC;
                        } else {
                            self.state = S_SCRIPT_DESC;
                        }
                    }
                } else if is_alpha(c) {
                    self.i += 1;
                    self.tmp.push(lower(c))!;
                    self.chars.push(@truncate(u8, c))!;
                } else {
                    if st == S_SCRIPT_DESC_START {
                        self.state = S_SCRIPT_ESC;
                    } else {
                        self.state = S_SCRIPT_DESC;
                    }
                }
                continue;
            }
            if (st == S_SCRIPT_DESC) or (st == S_SCRIPT_DESC_DASH) or (st == S_SCRIPT_DESC_DASH_DASH) {
                if c < 0 {
                    self.err();
                    return TK_EOF;
                }
                self.i += 1;
                if c == '-' {
                    self.chars.push('-')!;
                    if st == S_SCRIPT_DESC {
                        self.state = S_SCRIPT_DESC_DASH;
                    } else {
                        self.state = S_SCRIPT_DESC_DASH_DASH;
                    }
                } else if c == '<' {
                    self.chars.push('<')!;
                    self.state = S_SCRIPT_DESC_LT;
                } else if (c == '>') and (st == S_SCRIPT_DESC_DASH_DASH) {
                    self.chars.push('>')!;
                    self.state = S_SCRIPT;
                } else if c == 0 {
                    self.err();
                    self.chars.push_str("\xef\xbf\xbd")!;
                    self.state = S_SCRIPT_DESC;
                } else {
                    self.chars.push(@truncate(u8, c))!;
                    self.state = S_SCRIPT_DESC;
                }
                continue;
            }
            if st == S_SCRIPT_DESC_LT {
                if c == '/' {
                    self.i += 1;
                    self.tmp.clear();
                    self.chars.push('/')!;
                    self.state = S_SCRIPT_DESC_END;
                } else {
                    self.state = S_SCRIPT_DESC;
                }
                continue;
            }
            // --- comments
            if st == S_MARKUP_DECL {
                if self.lookahead("--") {
                    self.i += 2;
                    self.comment.clear();
                    self.state = S_COMMENT_START;
                } else if self.lookahead_ci("doctype") {
                    self.i += 7;
                    self.state = S_DOCTYPE;
                } else if self.lookahead("[CDATA[") {
                    self.i += 7;
                    if self.cdata_ok {
                        self.state = S_CDATA;
                    } else {
                        self.err();
                        self.comment.clear();
                        self.comment.push_str("[CDATA[")!;
                        self.state = S_BOGUS_COMMENT;
                    }
                } else {
                    self.err();
                    self.comment.clear();
                    self.state = S_BOGUS_COMMENT;
                }
                continue;
            }
            if st == S_BOGUS_COMMENT {
                let b = self.i;
                let mut i = b;
                while (i < n) and (s[i] != '>') and (s[i] != 0) {
                    i += 1;
                }
                self.comment.push_str(s[b..i])!;
                self.i = i;
                if i >= n {
                    return TK_COMMENT;  // (then EOF)
                }
                self.i = i + 1;
                if s[i] == '>' {
                    self.state = S_DATA;
                    return TK_COMMENT;
                }
                self.err();
                self.comment.push_str("\xef\xbf\xbd")!;
                continue;
            }
            if (st == S_COMMENT_START) or (st == S_COMMENT_START_DASH) {
                if c == '-' {
                    self.i += 1;
                    if st == S_COMMENT_START {
                        self.state = S_COMMENT_START_DASH;
                    } else {
                        self.state = S_COMMENT_END;
                    }
                } else if c == '>' {
                    self.err();
                    self.i += 1;
                    self.state = S_DATA;
                    return TK_COMMENT;
                } else if (c < 0) and (st == S_COMMENT_START_DASH) {
                    self.err();
                    return TK_COMMENT;
                } else {
                    if st == S_COMMENT_START_DASH {
                        self.comment.push('-')!;
                    }
                    self.state = S_COMMENT;
                }
                continue;
            }
            if st == S_COMMENT {
                let b = self.i;
                let mut i = b;
                while (i < n) and (s[i] != '<') and (s[i] != '-') and (s[i] != 0) {
                    i += 1;
                }
                self.comment.push_str(s[b..i])!;
                self.i = i;
                if i >= n {
                    self.err();
                    return TK_COMMENT;
                }
                self.i = i + 1;
                if s[i] == '<' {
                    self.comment.push('<')!;
                    self.state = S_COMMENT_LT;
                } else if s[i] == '-' {
                    self.state = S_COMMENT_END_DASH;
                } else {
                    self.err();
                    self.comment.push_str("\xef\xbf\xbd")!;
                }
                continue;
            }
            if st == S_COMMENT_LT {
                if c == '!' {
                    self.i += 1;
                    self.comment.push('!')!;
                    self.state = S_COMMENT_LT_BANG;
                } else if c == '<' {
                    self.i += 1;
                    self.comment.push('<')!;
                } else {
                    self.state = S_COMMENT;
                }
                continue;
            }
            if st == S_COMMENT_LT_BANG {
                if c == '-' {
                    self.i += 1;
                    self.state = S_COMMENT_LT_BANG_DASH;
                } else {
                    self.state = S_COMMENT;
                }
                continue;
            }
            if st == S_COMMENT_LT_BANG_DASH {
                if c == '-' {
                    self.i += 1;
                    self.state = S_COMMENT_LT_BANG_DASH_DASH;
                } else {
                    self.state = S_COMMENT_END_DASH;
                }
                continue;
            }
            if st == S_COMMENT_LT_BANG_DASH_DASH {
                if (c != '>') and (c >= 0) {
                    self.err();
                }
                self.state = S_COMMENT_END;
                continue;
            }
            if st == S_COMMENT_END_DASH {
                if c == '-' {
                    self.i += 1;
                    self.state = S_COMMENT_END;
                } else if c < 0 {
                    self.err();
                    return TK_COMMENT;
                } else {
                    self.comment.push('-')!;
                    self.state = S_COMMENT;
                }
                continue;
            }
            if st == S_COMMENT_END {
                if c == '>' {
                    self.i += 1;
                    self.state = S_DATA;
                    return TK_COMMENT;
                } else if c == '!' {
                    self.i += 1;
                    self.state = S_COMMENT_END_BANG;
                } else if c == '-' {
                    self.i += 1;
                    self.comment.push('-')!;
                } else if c < 0 {
                    self.err();
                    return TK_COMMENT;
                } else {
                    self.comment.push_str("--")!;
                    self.state = S_COMMENT;
                }
                continue;
            }
            if st == S_COMMENT_END_BANG {
                if c == '-' {
                    self.i += 1;
                    self.comment.push_str("--!")!;
                    self.state = S_COMMENT_END_DASH;
                } else if c == '>' {
                    self.err();
                    self.i += 1;
                    self.state = S_DATA;
                    return TK_COMMENT;
                } else if c < 0 {
                    self.err();
                    return TK_COMMENT;
                } else {
                    self.comment.push_str("--!")!;
                    self.state = S_COMMENT;
                }
                continue;
            }
            // --- CDATA sections (in foreign content)
            if st == S_CDATA {
                let b = self.i;
                let mut i = b;
                while (i < n) and (s[i] != ']') {
                    i += 1;
                }
                self.chars.push_str(s[b..i])!;
                self.i = i;
                if i >= n {
                    self.err();
                    return TK_EOF;
                }
                self.i = i + 1;
                self.state = S_CDATA_BRACKET;
                continue;
            }
            if st == S_CDATA_BRACKET {
                if c == ']' {
                    self.i += 1;
                    self.state = S_CDATA_END;
                } else {
                    self.chars.push(']')!;
                    self.state = S_CDATA;
                }
                continue;
            }
            if st == S_CDATA_END {
                if c == ']' {
                    self.i += 1;
                    self.chars.push(']')!;
                } else if c == '>' {
                    self.i += 1;
                    self.state = S_DATA;
                } else {
                    self.chars.push_str("]]")!;
                    self.state = S_CDATA;
                }
                continue;
            }
            // --- DOCTYPE
            if st == S_DOCTYPE {
                self.new_doctype();
                if (c == '\t') or (c == '\n') or (c == 12) or (c == ' ') {
                    self.i += 1;
                } else if c < 0 {
                    self.err();
                    self.force_quirks = true;
                    return TK_DOCTYPE;
                } else if c != '>' {
                    self.err();
                }
                self.state = S_BEFORE_DOCTYPE_NAME;
                continue;
            }
            if st == S_BEFORE_DOCTYPE_NAME {
                if (c == '\t') or (c == '\n') or (c == 12) or (c == ' ') {
                    self.i += 1;
                } else if c == '>' {
                    self.err();
                    self.i += 1;
                    self.force_quirks = true;
                    self.state = S_DATA;
                    return TK_DOCTYPE;
                } else if c < 0 {
                    self.err();
                    self.force_quirks = true;
                    return TK_DOCTYPE;
                } else {
                    self.has_name = true;
                    self.state = S_DOCTYPE_NAME;
                }
                continue;
            }
            if st == S_DOCTYPE_NAME {
                if (c == '\t') or (c == '\n') or (c == 12) or (c == ' ') {
                    self.i += 1;
                    self.state = S_AFTER_DOCTYPE_NAME;
                } else if c == '>' {
                    self.i += 1;
                    self.state = S_DATA;
                    return TK_DOCTYPE;
                } else if c < 0 {
                    self.err();
                    self.force_quirks = true;
                    return TK_DOCTYPE;
                } else {
                    self.i += 1;
                    if c == 0 {
                        self.err();
                        self.name.push_str("\xef\xbf\xbd")!;
                    } else {
                        self.name.push(lower(c))!;
                    }
                }
                continue;
            }
            if st == S_AFTER_DOCTYPE_NAME {
                if (c == '\t') or (c == '\n') or (c == 12) or (c == ' ') {
                    self.i += 1;
                } else if c == '>' {
                    self.i += 1;
                    self.state = S_DATA;
                    return TK_DOCTYPE;
                } else if c < 0 {
                    self.err();
                    self.force_quirks = true;
                    return TK_DOCTYPE;
                } else if self.lookahead_ci("public") {
                    self.i += 6;
                    self.state = S_AFTER_DOCTYPE_PUBLIC_KW;
                } else if self.lookahead_ci("system") {
                    self.i += 6;
                    self.state = S_AFTER_DOCTYPE_SYSTEM_KW;
                } else {
                    self.err();
                    self.force_quirks = true;
                    self.state = S_BOGUS_DOCTYPE;
                }
                continue;
            }
            if (st == S_AFTER_DOCTYPE_PUBLIC_KW) or (st == S_BEFORE_DOCTYPE_PUBLIC_ID) or
               (st == S_AFTER_DOCTYPE_SYSTEM_KW) or (st == S_BEFORE_DOCTYPE_SYSTEM_ID) {
                let public = (st == S_AFTER_DOCTYPE_PUBLIC_KW) or (st == S_BEFORE_DOCTYPE_PUBLIC_ID);
                let kw = (st == S_AFTER_DOCTYPE_PUBLIC_KW) or (st == S_AFTER_DOCTYPE_SYSTEM_KW);
                if (c == '\t') or (c == '\n') or (c == 12) or (c == ' ') {
                    self.i += 1;
                    if kw {
                        self.state = st + 1;
                    }
                } else if (c == '"') or (c == '\'') {
                    if kw {
                        self.err();
                    }
                    self.i += 1;
                    if public {
                        self.has_public = true;
                        self.state = S_DOCTYPE_PUBLIC_ID_DQ;
                    } else {
                        self.has_system = true;
                        self.state = S_DOCTYPE_SYSTEM_ID_DQ;
                    }
                    if c == '\'' {
                        self.state += 1;
                    }
                } else if c == '>' {
                    self.err();
                    self.i += 1;
                    self.force_quirks = true;
                    self.state = S_DATA;
                    return TK_DOCTYPE;
                } else if c < 0 {
                    self.err();
                    self.force_quirks = true;
                    return TK_DOCTYPE;
                } else {
                    self.err();
                    self.force_quirks = true;
                    self.state = S_BOGUS_DOCTYPE;
                }
                continue;
            }
            if (st == S_DOCTYPE_PUBLIC_ID_DQ) or (st == S_DOCTYPE_PUBLIC_ID_SQ) or (st == S_DOCTYPE_SYSTEM_ID_DQ) or
               (st == S_DOCTYPE_SYSTEM_ID_SQ) {
                let mut q: i32 = '"';
                if (st == S_DOCTYPE_PUBLIC_ID_SQ) or (st == S_DOCTYPE_SYSTEM_ID_SQ) {
                    q = '\'';
                }
                let public = (st == S_DOCTYPE_PUBLIC_ID_DQ) or (st == S_DOCTYPE_PUBLIC_ID_SQ);
                if c == q {
                    self.i += 1;
                    if public {
                        self.state = S_AFTER_DOCTYPE_PUBLIC_ID;
                    } else {
                        self.state = S_AFTER_DOCTYPE_SYSTEM_ID;
                    }
                } else if c == '>' {
                    self.err();
                    self.i += 1;
                    self.force_quirks = true;
                    self.state = S_DATA;
                    return TK_DOCTYPE;
                } else if c < 0 {
                    self.err();
                    self.force_quirks = true;
                    return TK_DOCTYPE;
                } else {
                    self.i += 1;
                    let mut out = &mut self.system_id;
                    if public {
                        out = &mut self.public_id;
                    }
                    if c == 0 {
                        self.err();
                        out.push_str("\xef\xbf\xbd")!;
                    } else {
                        out.push(@truncate(u8, c))!;
                    }
                }
                continue;
            }
            if (st == S_AFTER_DOCTYPE_PUBLIC_ID) or (st == S_BETWEEN_DOCTYPE_IDS) {
                if (c == '\t') or (c == '\n') or (c == 12) or (c == ' ') {
                    self.i += 1;
                    self.state = S_BETWEEN_DOCTYPE_IDS;
                } else if c == '>' {
                    self.i += 1;
                    self.state = S_DATA;
                    return TK_DOCTYPE;
                } else if (c == '"') or (c == '\'') {
                    if st == S_AFTER_DOCTYPE_PUBLIC_ID {
                        self.err();
                    }
                    self.i += 1;
                    self.has_system = true;
                    self.state = S_DOCTYPE_SYSTEM_ID_DQ;
                    if c == '\'' {
                        self.state = S_DOCTYPE_SYSTEM_ID_SQ;
                    }
                } else if c < 0 {
                    self.err();
                    self.force_quirks = true;
                    return TK_DOCTYPE;
                } else {
                    self.err();
                    self.force_quirks = true;
                    self.state = S_BOGUS_DOCTYPE;
                }
                continue;
            }
            if st == S_AFTER_DOCTYPE_SYSTEM_ID {
                if (c == '\t') or (c == '\n') or (c == 12) or (c == ' ') {
                    self.i += 1;
                } else if c == '>' {
                    self.i += 1;
                    self.state = S_DATA;
                    return TK_DOCTYPE;
                } else if c < 0 {
                    self.err();
                    self.force_quirks = true;
                    return TK_DOCTYPE;
                } else {
                    self.err();
                    self.state = S_BOGUS_DOCTYPE;
                }
                continue;
            }
            if st == S_BOGUS_DOCTYPE {
                if c < 0 {
                    return TK_DOCTYPE;
                }
                self.i += 1;
                if c == '>' {
                    self.state = S_DATA;
                    return TK_DOCTYPE;
                }
                continue;
            }
            @trap();
        }
    }
}

const NONE_USIZE: usize = 0xffffffff;

// Where an end tag that turns out not to be one goes back to.
fn end_tag_back(open_state: u8) u8 {
    if open_state == S_RCDATA_END_OPEN {
        return S_RCDATA;
    }
    if open_state == S_RAWTEXT_END_OPEN {
        return S_RAWTEXT;
    }
    if open_state == S_SCRIPT_END_OPEN {
        return S_SCRIPT;
    }
    return S_SCRIPT_ESC;
}

// Replacements for numeric references to C1 controls (0x80-0x9f), or 0.
let C1: [32]u32 = [
    0x20ac,
    0,
    0x201a,
    0x0192,
    0x201e,
    0x2026,
    0x2020,
    0x2021,
    0x02c6,
    0x2030,
    0x0160,
    0x2039,
    0x0152,
    0,
    0x017d,
    0,
    0,
    0x2018,
    0x2019,
    0x201c,
    0x201d,
    0x2022,
    0x2013,
    0x2014,
    0x02dc,
    0x2122,
    0x0161,
    0x203a,
    0x0153,
    0,
    0x017e,
    0x0178,
];

fn entity_name(i: usize) []u8 {
    return ent.NAMES[@as(usize, ent.OFFSET[i])..@as(usize, ent.OFFSET[i + 1])];
}

fn entity_index(name: []u8) !usize {
    let mut lo: usize = 0;
    let mut hi: usize = ent.COUNT;
    while lo < hi {
        let mid = (lo + hi) / 2;
        let c = strs.compare(entity_name(mid), name);
        if c == 0 {
            return mid;
        }
        if c < 0 {
            lo = mid + 1;
        } else {
            hi = mid;
        }
    }
    return error.NotFound;
}

// The index of a name that may appear without a semicolon.
fn legacy_index(name: []u8) !usize {
    let mut lo: usize = 0;
    let mut hi: usize = ent.LEGACY.len;
    while lo < hi {
        let mid = (lo + hi) / 2;
        let k = @as(usize, ent.LEGACY[mid]);
        let c = strs.compare(entity_name(k), name);
        if c == 0 {
            return k;
        }
        if c < 0 {
            lo = mid + 1;
        } else {
            hi = mid;
        }
    }
    return error.NotFound;
}

// -----------------------------------------------------------------------------
// The tree
// -----------------------------------------------------------------------------

const FL_OPEN: u8 = 1;  // on the stack of open elements
const FL_AFE: u8 = 2;   // in the list of active formatting elements
const FL_HIP: u8 = 4;   // an HTML integration point (MathML annotation-xml)

pub struct Node {
    kind: u8,
    ns: u8,      // NS_HTML, NS_SVG or NS_MATHML (elements)
    tag: u16,    // (the parser's number for the name)
    name: []u8,  // element local name; DOCTYPE name
    attrs: []Attr,
    text: []u8,       // TEXT and COMMENT data; DOCTYPE public identifier
    system_id: []u8,  // DOCTYPE system identifier
    parent: u32,
    first: u32,
    last: u32,
    prev: u32,
    next: u32,
    contents: u32,  // a template's contents (a FRAGMENT); a FRAGMENT's template
    flags: u8,
    tx: u32,    // while parsing, a TEXT node's data is tbuf[toff..toff + tlen],
    toff: u32,  // or texts[tx] when tx is not NONE
    tlen: u32,
}

/// A parsed document (or fragment): it owns every node and string.
pub struct Document {
    nodes: vec.Vec.[Node],
    arena: arena.Arena,
    mode: u8,  // NO_QUIRKS, QUIRKS or LIMITED_QUIRKS
    tbuf: strs.String,
    texts: vec.Vec.[strs.String],

    fn init() Document {
        return Document.{ nodes: vec.Vec.[Node].init(), arena: arena.Arena.init(), mode: NO_QUIRKS,
                          tbuf: strs.String.init(), texts: vec.Vec.[strs.String].init() };
    }

    pub fn deinit(self: *mut Document) void {
        self.nodes.deinit();
        self.arena.deinit();
        self.tbuf.deinit();
        for k in @as(usize, 0)..self.texts.len {
            self.texts.get_ptr(k).deinit();
        }
        self.texts.deinit();
    }

    pub fn node(self: *Document, id: u32) *Node {
        return &self.nodes.items()[@as(usize, id)];
    }

    fn nm(self: *mut Document, id: u32) *mut Node {
        return self.nodes.get_ptr(@as(usize, id));
    }

    fn new_node(self: *mut Document, kind: u8) !u32 {
        let id = @truncate(u32, self.nodes.len);
        self.nodes.push(Node.{ kind: kind, ns: NS_HTML, tag: 0, name: "", attrs: no_attrs(), text: "", system_id: "",
                               parent: NONE, first: NONE, last: NONE, prev: NONE, next: NONE, contents: NONE, flags: 0,
                               tx: NONE, toff: 0, tlen: 0 })!;
        return id;
    }

    /// Inserts child (which must have no parent) into parent before ref
    /// (NONE: at the end).
    pub fn insert_before(self: *mut Document, parent: u32, child: u32, ref: u32) void {
        let c = self.nm(child);
        c.parent = parent;
        let p = self.nm(parent);
        if ref == NONE {
            c.prev = p.last;
            c.next = NONE;
            if p.last != NONE {
                self.nm(p.last).next = child;
            } else {
                p.first = child;
            }
            p.last = child;
        } else {
            let r = self.nm(ref);
            c.prev = r.prev;
            c.next = ref;
            if r.prev != NONE {
                self.nm(r.prev).next = child;
            } else {
                p.first = child;
            }
            r.prev = child;
        }
    }

    pub fn append_child(self: *mut Document, parent: u32, child: u32) void {
        self.insert_before(parent, child, NONE);
    }

    /// Takes id out of its parent (it and its subtree stay, detached).
    pub fn remove(self: *mut Document, id: u32) void {
        let c = self.nm(id);
        let p = c.parent;
        if p == NONE {
            return;
        }
        if c.prev != NONE {
            self.nm(c.prev).next = c.next;
        } else {
            self.nm(p).first = c.next;
        }
        if c.next != NONE {
            self.nm(c.next).prev = c.prev;
        } else {
            self.nm(p).last = c.prev;
        }
        c.parent = NONE;
        c.prev = NONE;
        c.next = NONE;
    }

    // The data of a text node while parsing.
    fn tdata(self: *Document, id: u32) []u8 {
        let n = self.node(id);
        if n.tx != NONE {
            return self.texts.items()[@as(usize, n.tx)].bytes();
        }
        return self.tbuf.bytes()[@as(usize, n.toff)..@as(usize, n.toff + n.tlen)];
    }

    fn new_text(self: *mut Document, data: []u8) !u32 {
        let id = self.new_node(TEXT)!;
        let n = self.nm(id);
        n.toff = @truncate(u32, self.tbuf.len);
        n.tlen = @truncate(u32, data.len);
        self.tbuf.push_str(data)!;
        return id;
    }

    fn append_text(self: *mut Document, id: u32, data: []u8) !void {
        let n = self.nm(id);
        if n.tx == NONE {
            if (@as(usize, n.toff + n.tlen) == self.tbuf.len) {
                self.tbuf.push_str(data)!;
                n.tlen += @truncate(u32, data.len);
                return;
            }
            // not at the end of the shared buffer: give it its own
            let mut s = strs.String.from(self.tdata(id))!;
            s.push_str(data) catch |e| {
                s.deinit();
                return e;
            };
            self.texts.push(s) catch |e| {
                s.deinit();
                return e;
            };
            self.nm(id).tx = @truncate(u32, self.texts.len - 1);
            return;
        }
        self.texts.get_ptr(@as(usize, n.tx)).push_str(data)!;
    }

    // Points text nodes' `text` at their data (after parsing).
    fn finish_text(self: *mut Document) void {
        for k in @as(usize, 0)..self.nodes.len {
            let id = @truncate(u32, k);
            let n = self.nm(id);
            if (n.kind == TEXT) and (n.text.len == 0) {
                n.text = self.tdata(id);
            }
        }
    }

    // --- reading

    /// The document element (<html>), or NONE.
    pub fn root(self: *Document) u32 {
        let mut c = self.node(0).first;
        while c != NONE {
            if self.node(c).kind == ELEMENT {
                return c;
            }
            c = self.node(c).next;
        }
        return NONE;
    }

    /// The first child element of the root with this name, or NONE.
    fn root_child(self: *Document, name: []u8) u32 {
        let r = self.root();
        if r == NONE {
            return NONE;
        }
        let mut c = self.node(r).first;
        while c != NONE {
            let n = self.node(c);
            if (n.kind == ELEMENT) and (n.ns == NS_HTML) and strs.eq(n.name, name) {
                return c;
            }
            c = n.next;
        }
        return NONE;
    }

    pub fn head(self: *Document) u32 {
        return self.root_child("head");
    }

    /// The <body> (or <frameset>), or NONE.
    pub fn body(self: *Document) u32 {
        let b = self.root_child("body");
        if b != NONE {
            return b;
        }
        return self.root_child("frameset");
    }

    /// The text of the first <title>, with whitespace collapsed ("" if
    /// none); the caller frees it.
    pub fn title(self: *Document) ![]mut u8 {
        let id = self.find(0, "title");
        let mut s = strs.String.init();
        if id == NONE {
            return s.take();
        }
        let t = self.text_content(id)!;
        defer mem.free(t);
        let mut it = strs.fields(t);
        while true {
            let w = it.next() catch break;
            if s.len > 0 {
                s.push(' ') catch |e| {
                    s.deinit();
                    return e;
                };
            }
            s.push_str(w) catch |e| {
                s.deinit();
                return e;
            };
        }
        return s.take();
    }

    pub fn walk(self: *Document, id: u32) Walker {
        return Walker.{ doc: self, root: id, id: id, entering: true, started: false, templates: true };
    }

    pub fn children(self: *Document, id: u32) Children {
        return Children.{ doc: self, id: self.first_kid(id, true) };
    }

    // The first child, or for a template the first node of its contents.
    fn first_kid(self: *Document, id: u32, templates: bool) u32 {
        let n = self.node(id);
        if templates and (n.kind == ELEMENT) and (n.contents != NONE) {
            return self.node(n.contents).first;
        }
        return n.first;
    }

    // The parent, or for template contents the template.
    fn up(self: *Document, id: u32) u32 {
        let p = self.node(id).parent;
        if (p != NONE) and (self.node(p).kind == FRAGMENT) and (self.node(p).contents != NONE) {
            return self.node(p).contents;
        }
        return p;
    }

    /// The parent element (NONE for the root and detached nodes).
    pub fn parent_element(self: *Document, id: u32) u32 {
        let p = self.up(id);
        if (p != NONE) and (self.node(p).kind == ELEMENT) {
            return p;
        }
        return NONE;
    }

    /// An attribute's value (attributes with a namespace are matched by
    /// "prefix:name").
    pub fn attr(self: *Document, id: u32, name: []u8) ![]u8 {
        for a in self.node(id).attrs {
            if attr_is(a, name) {
                return a.value;
            }
        }
        return error.NotFound;
    }

    pub fn has_attr(self: *Document, id: u32, name: []u8) bool {
        _ = self.attr(id, name) catch return false;
        return true;
    }

    /// The text of the node and its descendants, in order (as the DOM's
    /// textContent); the caller frees it.
    pub fn text_content(self: *Document, id: u32) ![]mut u8 {
        let mut s = strs.String.init();
        let mut w = self.walk(id);
        w.templates = false;
        while w.next() {
            let n = self.node(w.id);
            if w.entering and (n.kind == TEXT) {
                s.push_str(n.text) catch |e| {
                    s.deinit();
                    return e;
                };
            }
        }
        return s.take();
    }

    /// The first element under id (in document order, id included) with
    /// the local name, or NONE.
    pub fn find(self: *Document, id: u32, name: []u8) u32 {
        let mut w = self.walk(id);
        while w.next() {
            let n = self.node(w.id);
            if w.entering and (n.kind == ELEMENT) and name_is(n, name) {
                return w.id;
            }
        }
        return NONE;
    }

    // --- changing

    /// A new, detached HTML element.
    pub fn create_element(self: *mut Document, name: []u8) !u32 {
        let id = self.new_node(ELEMENT)!;
        let lname = self.arena.bytes(name.len)!;
        for k in @as(usize, 0)..name.len {
            lname[k] = strs.to_lower_byte(name[k]);
        }
        let n = self.nm(id);
        n.name = lname;
        n.tag = tag_of(lname);
        if n.tag == T_TEMPLATE {
            let f = self.new_node(FRAGMENT)!;
            self.nm(f).contents = id;
            self.nm(id).contents = f;
        }
        return id;
    }

    pub fn create_text(self: *mut Document, data: []u8) !u32 {
        let id = self.new_node(TEXT)!;
        self.nm(id).text = self.arena.dup(data)!;
        return id;
    }

    pub fn create_comment(self: *mut Document, data: []u8) !u32 {
        let id = self.new_node(COMMENT)!;
        self.nm(id).text = self.arena.dup(data)!;
        return id;
    }

    /// Sets (or adds) an attribute without a namespace.
    pub fn set_attr(self: *mut Document, id: u32, name: []u8, value: []u8) !void {
        let v = self.arena.dup(value)!;
        let old = self.nm(id).attrs;
        let mut all = vec.Vec.[Attr].init();
        defer all.deinit();
        let mut found = false;
        for a in old {
            let mut b = a;
            if (a.ns == A_NONE) and strs.eq(a.name, name) {
                b.value = v;
                found = true;
            }
            all.push(b)!;
        }
        if not found {
            all.push(Attr.{ ns: A_NONE, prefix: "", name: self.arena.dup(name)!, value: v })!;
        }
        self.nm(id).attrs = self.arena.copy.[Attr](all.items())!;
    }

    pub fn remove_attr(self: *mut Document, id: u32, name: []u8) !void {
        let old = self.nm(id).attrs;
        let mut all = vec.Vec.[Attr].init();
        defer all.deinit();
        for a in old {
            if not attr_is(a, name) {
                all.push(a)!;
            }
        }
        if all.len != old.len {
            self.nm(id).attrs = self.arena.copy.[Attr](all.items())!;
        }
    }

    /// A deep copy of a node (detached).
    pub fn clone_tree(self: *mut Document, id: u32) !u32 {
        let top = self.copy_node(id)!;
        let mut work = vec.Vec.[u32].init();  // pairs: source, copy
        defer work.deinit();
        work.push(id)!;
        work.push(top)!;
        while work.len > 0 {
            let dst = work.pop()!;
            let src = work.pop()!;
            let mut from = src;
            let mut to = dst;
            if (self.nm(src).kind == ELEMENT) and (self.nm(src).contents != NONE) {
                from = self.nm(src).contents;
                to = self.nm(dst).contents;
            }
            let mut c = self.nm(from).first;
            while c != NONE {
                let k = self.copy_node(c)!;
                self.append_child(to, k);
                work.push(c)!;
                work.push(k)!;
                c = self.nm(c).next;
            }
        }
        return top;
    }

    fn copy_node(self: *mut Document, id: u32) !u32 {
        let k = self.new_node(self.nm(id).kind)!;
        let o = self.nm(id);
        let n = self.nm(k);
        n.ns = o.ns;
        n.tag = o.tag;
        n.name = o.name;
        n.attrs = o.attrs;
        n.text = o.text;
        n.system_id = o.system_id;
        if (o.kind == ELEMENT) and (o.contents != NONE) {
            let f = self.new_node(FRAGMENT)!;
            self.nm(f).contents = k;
            self.nm(k).contents = f;
        }
        return k;
    }

    // --- serialization

    /// The HTML for a node and its descendants (outerHTML); for node 0,
    /// the whole document. The caller frees it.
    pub fn to_html(self: *Document, id: u32) ![]mut u8 {
        let mut s = strs.String.init();
        self.serialize(s.writer(), id, true) catch |e| {
            s.deinit();
            return e;
        };
        return s.take();
    }

    /// The HTML for a node's children (innerHTML).
    pub fn inner_html(self: *Document, id: u32) ![]mut u8 {
        let mut s = strs.String.init();
        self.serialize(s.writer(), id, false) catch |e| {
            s.deinit();
            return e;
        };
        return s.take();
    }

    /// Writes the HTML serialization of a node (outer: the node itself too).
    pub fn serialize(self: *Document, out: io.Writer, id: u32, outer: bool) !void {
        let mut w = self.walk(id);
        while w.next() {
            let n = self.node(w.id);
            let is_top = w.id == id;
            if is_top and not outer {
                continue;
            }
            if not w.entering {
                if (n.kind == ELEMENT) and not ((n.ns == NS_HTML) and ((TAG_FLAGS[@as(usize, n.tag)] & F_VOID) != 0)) {
                    out.write_all("</")!;
                    out.write_all(n.name)!;
                    out.write_all(">")!;
                }
                continue;
            }
            if n.kind == ELEMENT {
                out.write_all("<")!;
                out.write_all(n.name)!;
                for a in n.attrs {
                    out.write_all(" ")!;
                    if a.ns == A_XML {
                        out.write_all("xml:")!;
                    } else if a.ns == A_XLINK {
                        out.write_all("xlink:")!;
                    } else if (a.ns == A_XMLNS) and not strs.eq(a.name, "xmlns") {
                        out.write_all("xmlns:")!;
                    }
                    out.write_all(a.name)!;
                    out.write_all("=\"")!;
                    escape(out, a.value, true)!;
                    out.write_all("\"")!;
                }
                out.write_all(">")!;
                if (n.ns == NS_HTML) and ((TAG_FLAGS[@as(usize, n.tag)] & F_VOID) != 0) {
                    w.skip();
                }
            } else if n.kind == TEXT {
                let p = self.parent_element(w.id);
                if (p != NONE) and (self.node(p).ns == NS_HTML) and
                   ((TAG_FLAGS[@as(usize, self.node(p).tag)] & F_RAW) != 0) {
                    out.write_all(n.text)!;
                } else {
                    escape(out, n.text, false)!;
                }
            } else if n.kind == COMMENT {
                out.write_all("<!--")!;
                out.write_all(n.text)!;
                out.write_all("-->")!;
            } else if n.kind == DOCTYPE {
                out.write_all("<!DOCTYPE ")!;
                out.write_all(n.name)!;
                out.write_all(">")!;
            }
        }
    }

    // --- selectors

    /// The first element under id (id excluded) that matches a CSS
    /// selector, or NONE.
    pub fn query_from(self: *Document, id: u32, selector: []u8) !u32 {
        let mut sel = compile(selector)!;
        defer sel.deinit();
        let mut w = self.walk(id);
        _ = w.next();
        while w.next() {
            if w.entering and (self.node(w.id).kind == ELEMENT) and sel.matches(self, w.id) {
                return w.id;
            }
        }
        return NONE;
    }

    /// The first element in the document matching a CSS selector, or NONE.
    pub fn query(self: *Document, selector: []u8) !u32 {
        return self.query_from(0, selector);
    }

    /// Every element under id (id excluded) that matches, in document
    /// order; the caller frees the list.
    pub fn query_all_from(self: *Document, id: u32, selector: []u8) ![]mut u32 {
        let mut sel = compile(selector)!;
        defer sel.deinit();
        let mut out = vec.Vec.[u32].init();
        let mut w = self.walk(id);
        _ = w.next();
        while w.next() {
            if w.entering and (self.node(w.id).kind == ELEMENT) and sel.matches(self, w.id) {
                out.push(w.id) catch |e| {
                    out.deinit();
                    return e;
                };
            }
        }
        return take_vec(&mut out);
    }

    pub fn query_all(self: *Document, selector: []u8) ![]mut u32 {
        return self.query_all_from(0, selector);
    }

    /// Whether an element matches a CSS selector.
    pub fn matches(self: *Document, id: u32, selector: []u8) !bool {
        let mut sel = compile(selector)!;
        defer sel.deinit();
        return sel.matches(self, id);
    }
}

fn no_attrs() []Attr {
    return @slice_from_ptr(@ptr_from_int(*Attr, 8), 0);
}

// Insertion modes
const M_INITIAL: u8 = 0;
const M_BEFORE_HTML: u8 = 1;
const M_BEFORE_HEAD: u8 = 2;
const M_IN_HEAD: u8 = 3;
const M_IN_HEAD_NOSCRIPT: u8 = 4;
const M_AFTER_HEAD: u8 = 5;
const M_IN_BODY: u8 = 6;
const M_TEXT: u8 = 7;
const M_IN_TABLE: u8 = 8;
const M_IN_TABLE_TEXT: u8 = 9;
const M_IN_CAPTION: u8 = 10;
const M_IN_COLUMN_GROUP: u8 = 11;
const M_IN_TABLE_BODY: u8 = 12;
const M_IN_ROW: u8 = 13;
const M_IN_CELL: u8 = 14;
const M_IN_TEMPLATE: u8 = 15;
const M_AFTER_BODY: u8 = 16;
const M_IN_FRAMESET: u8 = 17;
const M_AFTER_FRAMESET: u8 = 18;
const M_AFTER_AFTER_BODY: u8 = 19;
const M_AFTER_AFTER_FRAMESET: u8 = 20;

// Kinds of scope
const SC_DEFAULT: u8 = 0;
const SC_LIST: u8 = 1;
const SC_BUTTON: u8 = 2;
const SC_TABLE: u8 = 3;

const MARKER: u32 = 0xfffffffe;

/// The most formatting elements that stay active (after the last marker);
/// the spec's limit is three of each kind, which leaves quadratic growth
/// open to hostile input.
const MAX_ACTIVE_FORMATTING: usize = 32;

pub struct Options {
    scripting: bool,  // parse as if scripts ran: <noscript> holds raw text

    pub fn default() Options {
        return Options.{ scripting: false };
    }
}

struct Loc {
    parent: u32,
    before: u32,
}

struct Parser {
    doc: *mut Document,
    tz: *mut Tokenizer,
    stack: vec.Vec.[u32],
    open: [124]u32,  // how many HTML elements of each T_ are on the stack
    afe: vec.Vec.[u32],
    tmodes: vec.Vec.[u8],
    mode: u8,
    orig: u8,
    head: u32,
    form: u32,
    context: u32,
    frameset_ok: bool,
    foster: bool,
    scripting: bool,
    skip_lf: bool,
    text_elem: u32,        // the element "text" mode reads into, if opened
    pend: strs.String,     // pending table character tokens
    pend_text: bool,       // ... some are not whitespace
    sbuf: strs.String,     // scratch for characters
    abuf: vec.Vec.[Attr],  // scratch for attributes

    fn deinit(self: *mut Parser) void {
        self.stack.deinit();
        self.afe.deinit();
        self.tmodes.deinit();
        self.pend.deinit();
        self.sbuf.deinit();
        self.abuf.deinit();
    }

    fn nd(self: *Parser, id: u32) *mut Node {
        return self.doc.nm(id);
    }

    fn cur(self: *Parser) u32 {
        if self.stack.len == 0 {
            return NONE;
        }
        return self.stack.items()[self.stack.len - 1];
    }

    fn adjusted_current(self: *Parser) u32 {
        if (self.context != NONE) and (self.stack.len == 1) {
            return self.context;
        }
        return self.cur();
    }

    // Whether id is an HTML element with tag t.
    fn is(self: *Parser, id: u32, t: u16) bool {
        if id == NONE {
            return false;
        }
        let n = self.nd(id);
        return (n.ns == NS_HTML) and (n.tag == t) and (n.kind == ELEMENT);
    }

    fn cur_is(self: *Parser, t: u16) bool {
        return self.is(self.cur(), t);
    }

    fn special(self: *Parser, id: u32) bool {
        let n = self.nd(id);
        if n.ns == NS_HTML {
            return (TAG_FLAGS[@as(usize, n.tag)] & F_SPECIAL) != 0;
        }
        return scope_foreign(n);
    }

    fn mathml_text_ip(self: *Parser, id: u32) bool {
        let n = self.nd(id);
        return (n.ns == NS_MATHML) and
               ((n.tag == T_MI) or (n.tag == T_MO) or (n.tag == T_MN) or (n.tag == T_MS) or (n.tag == T_MTEXT));
    }

    fn html_ip(self: *Parser, id: u32) bool {
        let n = self.nd(id);
        if n.ns == NS_MATHML {
            return (n.flags & FL_HIP) != 0;
        }
        return (n.ns == NS_SVG) and ((n.tag == T_FOREIGNOBJECT) or (n.tag == T_DESC) or (n.tag == T_TITLE));
    }

    fn has_template(self: *Parser) bool {
        return self.open[@as(usize, T_TEMPLATE)] > 0;
    }

    fn in_template_contents(self: *Parser) bool {
        return self.has_template() or self.is(self.context, T_TEMPLATE);
    }

    // --- the stack of open elements

    fn push(self: *mut Parser, id: u32) !bool {
        if self.stack.len >= MAX_DEPTH {
            return false;
        }
        self.stack.push(id)!;
        self.opened_at(id);
        return true;
    }

    // Bookkeeping for an element put on the stack, or taken off it.
    fn opened_at(self: *mut Parser, id: u32) void {
        let n = self.nd(id);
        n.flags |= FL_OPEN;
        if n.ns == NS_HTML {
            self.open[@as(usize, n.tag)] += 1;
        }
    }

    fn closed_at(self: *mut Parser, id: u32) void {
        let n = self.nd(id);
        n.flags &= ~FL_OPEN;
        if n.ns == NS_HTML {
            self.open[@as(usize, n.tag)] -= 1;
        }
    }

    // Pops the current node (never the root element).
    fn pop(self: *mut Parser) void {
        if self.stack.len <= 1 {
            return;
        }
        let id = self.stack.items()[self.stack.len - 1];
        self.stack.truncate(self.stack.len - 1);
        self.closed_at(id);
    }

    // Pops up to (not including) index i.
    fn pop_to(self: *mut Parser, i: usize) void {
        let k = @max(i, 1);
        while self.stack.len > k {
            self.pop();
        }
    }

    fn pop_until(self: *mut Parser, t: u16) void {
        while self.stack.len > 1 {
            let id = self.cur();
            self.pop();
            if self.is(id, t) {
                return;
            }
        }
    }

    fn pop_until_node(self: *mut Parser, id: u32) void {
        while self.stack.len > 1 {
            let c = self.cur();
            self.pop();
            if c == id {
                return;
            }
        }
    }

    fn stack_index(self: *Parser, id: u32) usize {
        let items = self.stack.items();
        let mut i = items.len;
        while i > 0 {
            i -= 1;
            if items[i] == id {
                return i;
            }
        }
        return NONE_USIZE;
    }

    fn stack_remove(self: *mut Parser, id: u32) void {
        let i = self.stack_index(id);
        if (i != NONE_USIZE) and (i > 0) {
            _ = self.stack.remove(i);
            self.closed_at(id);
        }
    }

    fn boundary(self: *Parser, id: u32, sc: u8) bool {
        let n = self.nd(id);
        if sc == SC_TABLE {
            return (n.ns == NS_HTML) and ((n.tag == T_HTML) or (n.tag == T_TABLE) or (n.tag == T_TEMPLATE));
        }
        if n.ns == NS_HTML {
            if (TAG_FLAGS[@as(usize, n.tag)] & F_SCOPE) != 0 {
                return true;
            }
            if sc == SC_LIST {
                return (n.tag == T_OL) or (n.tag == T_UL);
            }
            if sc == SC_BUTTON {
                return n.tag == T_BUTTON;
            }
            return false;
        }
        return scope_foreign(n);
    }

    fn in_scope(self: *Parser, t: u16, sc: u8) bool {
        if self.open[@as(usize, t)] == 0 {
            return false;
        }
        let items = self.stack.items();
        let mut i = items.len;
        while i > 0 {
            i -= 1;
            if self.is(items[i], t) {
                return true;
            }
            if self.boundary(items[i], sc) {
                return false;
            }
        }
        return false;
    }

    fn node_in_scope(self: *Parser, id: u32) bool {
        let items = self.stack.items();
        let mut i = items.len;
        while i > 0 {
            i -= 1;
            if items[i] == id {
                return true;
            }
            if self.boundary(items[i], SC_DEFAULT) {
                return false;
            }
        }
        return false;
    }

    fn heading_in_scope(self: *Parser) bool {
        let items = self.stack.items();
        let mut i = items.len;
        while i > 0 {
            i -= 1;
            let n = self.nd(items[i]);
            if (n.ns == NS_HTML) and (n.tag >= T_H1) and (n.tag <= T_H6) {
                return true;
            }
            if self.boundary(items[i], SC_DEFAULT) {
                return false;
            }
        }
        return false;
    }

    fn gen_implied(self: *mut Parser, except: u16) void {
        loop {
            let c = self.cur();
            let n = self.nd(c);
            if (n.ns != NS_HTML) or (n.tag == except) or ((TAG_FLAGS[@as(usize, n.tag)] & F_IMPLIED) == 0) {
                return;
            }
            self.pop();
        }
    }

    fn gen_thorough(self: *mut Parser) void {
        loop {
            let n = self.nd(self.cur());
            if (n.ns != NS_HTML) or ((TAG_FLAGS[@as(usize, n.tag)] & F_THOROUGH) == 0) {
                return;
            }
            self.pop();
        }
    }

    fn close_p(self: *mut Parser) void {
        self.gen_implied(T_P);
        self.pop_until(T_P);
    }

    fn close_p_in_button_scope(self: *mut Parser) void {
        if self.in_scope(T_P, SC_BUTTON) {
            self.close_p();
        }
    }

    // Pops until the current node is one of the tags, or html (or template).
    fn clear_to(self: *mut Parser, a: u16, b: u16, c: u16) void {
        loop {
            let id = self.cur();
            if self.is(id, a) or self.is(id, b) or self.is(id, c) or self.is(id, T_TEMPLATE) or self.is(id, T_HTML) or
               (self.stack.len <= 1) {
                return;
            }
            self.pop();
        }
    }

    fn reset_mode(self: *mut Parser) void {
        let items = self.stack.items();
        let mut i = items.len;
        while i > 0 {
            i -= 1;
            let mut id = items[i];
            let last = i == 0;
            if last and (self.context != NONE) {
                id = self.context;
            }
            let n = self.nd(id);
            if n.ns == NS_HTML {
                let t = n.tag;
                if ((t == T_TD) or (t == T_TH)) and not last {
                    self.mode = M_IN_CELL;
                    return;
                }
                if t == T_TR {
                    self.mode = M_IN_ROW;
                    return;
                }
                if (t == T_TBODY) or (t == T_THEAD) or (t == T_TFOOT) {
                    self.mode = M_IN_TABLE_BODY;
                    return;
                }
                if t == T_CAPTION {
                    self.mode = M_IN_CAPTION;
                    return;
                }
                if t == T_COLGROUP {
                    self.mode = M_IN_COLUMN_GROUP;
                    return;
                }
                if t == T_TABLE {
                    self.mode = M_IN_TABLE;
                    return;
                }
                if t == T_TEMPLATE {
                    self.mode = self.tmodes.last() catch M_IN_BODY;
                    return;
                }
                if (t == T_HEAD) and not last {
                    self.mode = M_IN_HEAD;
                    return;
                }
                if t == T_BODY {
                    self.mode = M_IN_BODY;
                    return;
                }
                if t == T_FRAMESET {
                    self.mode = M_IN_FRAMESET;
                    return;
                }
                if t == T_HTML {
                    if self.head == NONE {
                        self.mode = M_BEFORE_HEAD;
                    } else {
                        self.mode = M_AFTER_HEAD;
                    }
                    return;
                }
            }
            if last {
                self.mode = M_IN_BODY;
                return;
            }
        }
        self.mode = M_IN_BODY;
    }

    // --- the list of active formatting elements

    fn afe_index(self: *Parser, id: u32) usize {
        let items = self.afe.items();
        let mut i = items.len;
        while i > 0 {
            i -= 1;
            if items[i] == id {
                return i;
            }
        }
        return NONE_USIZE;
    }

    fn afe_remove_at(self: *mut Parser, i: usize) void {
        let id = self.afe.remove(i);
        if id != MARKER {
            self.nd(id).flags &= ~FL_AFE;
        }
    }

    fn afe_remove(self: *mut Parser, id: u32) void {
        let i = self.afe_index(id);
        if i != NONE_USIZE {
            self.afe_remove_at(i);
        }
    }

    fn afe_push(self: *mut Parser, id: u32) !void {
        if (self.nd(id).flags & FL_OPEN) == 0 {
            return;  // (not opened: past MAX_DEPTH)
        }
        // Noah's Ark: at most three alike after the last marker
        let items = self.afe.items();
        let mut same: usize = 0;
        let mut earliest = NONE_USIZE;
        let mut count: usize = 0;
        let mut first = NONE_USIZE;
        let mut i = items.len;
        while i > 0 {
            i -= 1;
            let e = items[i];
            if e == MARKER {
                break;
            }
            count += 1;
            first = i;
            if self.alike(e, id) {
                same += 1;
                earliest = i;
            }
        }
        if same >= 3 {
            self.afe_remove_at(earliest);
        } else if count >= MAX_ACTIVE_FORMATTING {
            self.afe_remove_at(first);
        }
        self.afe.push(id)!;
        self.nd(id).flags |= FL_AFE;
    }

    fn alike(self: *Parser, a: u32, b: u32) bool {
        let x = self.nd(a);
        let y = self.nd(b);
        if (x.tag != y.tag) or (x.ns != y.ns) or not strs.eq(x.name, y.name) or (x.attrs.len != y.attrs.len) {
            return false;
        }
        for p in x.attrs {
            let mut found = false;
            for q in y.attrs {
                if (p.ns == q.ns) and strs.eq(p.name, q.name) and strs.eq(p.value, q.value) {
                    found = true;
                    break;
                }
            }
            if not found {
                return false;
            }
        }
        return true;
    }

    fn afe_clear_to_marker(self: *mut Parser) void {
        while self.afe.len > 0 {
            let e = self.afe.items()[self.afe.len - 1];
            self.afe_remove_at(self.afe.len - 1);
            if e == MARKER {
                return;
            }
        }
    }

    fn reconstruct(self: *mut Parser) !void {
        let n = self.afe.len;
        if n == 0 {
            return;
        }
        let items = self.afe.items();
        let last = items[n - 1];
        if (last == MARKER) or ((self.nd(last).flags & FL_OPEN) != 0) {
            return;
        }
        let mut i = n - 1;
        while i > 0 {
            let e = self.afe.items()[i - 1];
            if (e == MARKER) or ((self.nd(e).flags & FL_OPEN) != 0) {
                break;
            }
            i -= 1;
        }
        while i < self.afe.len {
            if self.stack.len >= MAX_DEPTH {
                return;
            }
            let old = self.afe.items()[i];
            let e = self.clone_element(old)!;
            self.insert_node(self.place(NONE), e);
            _ = self.push(e)!;
            self.afe.items()[i] = e;
            self.nd(old).flags &= ~FL_AFE;
            self.nd(e).flags |= FL_AFE;
            i += 1;
        }
    }

    // --- creating and inserting nodes

    fn place(self: *Parser, override: u32) Loc {
        let mut target = override;
        if target == NONE {
            target = self.cur();
        }
        let mut before = NONE;
        if self.foster and
           (self.is(target, T_TABLE) or self.is(target, T_TBODY) or self.is(target, T_TFOOT) or
            self.is(target, T_THEAD) or self.is(target, T_TR)) {
            let items = self.stack.items();
            let mut i = items.len;
            let mut found = false;
            while i > 0 {
                i -= 1;
                if self.is(items[i], T_TEMPLATE) or self.is(items[i], T_TABLE) {
                    found = true;
                    break;
                }
            }
            if not found {
                return Loc.{ parent: items[0], before: NONE };
            }
            let lt = items[i];
            if self.is(lt, T_TEMPLATE) {
                target = lt;
            } else if self.nd(lt).parent != NONE {
                return Loc.{ parent: self.nd(lt).parent, before: lt };
            } else {
                target = items[i - 1];
            }
        }
        if self.is(target, T_TEMPLATE) and (self.nd(target).contents != NONE) {
            return Loc.{ parent: self.nd(target).contents, before: NONE };
        }
        return Loc.{ parent: target, before: before };
    }

    fn insert_node(self: *mut Parser, at: Loc, id: u32) void {
        self.doc.insert_before(at.parent, id, at.before);
    }

    fn insert_text(self: *mut Parser, data: []u8) !void {
        if data.len == 0 {
            return;
        }
        let at = self.place(NONE);
        let p = self.nd(at.parent);
        if p.kind == DOCUMENT {
            return;
        }
        let mut prev = p.last;
        if at.before != NONE {
            prev = self.nd(at.before).prev;
        }
        if (prev != NONE) and (self.nd(prev).kind == TEXT) {
            self.doc.append_text(prev, data)!;
            return;
        }
        let id = self.doc.new_text(data)!;
        self.insert_node(at, id);
    }

    fn insert_comment(self: *mut Parser, t: *Token, at: Loc) !void {
        let id = self.doc.new_node(COMMENT)!;
        self.nd(id).text = self.doc.arena.dup(t.data)!;
        self.insert_node(at, id);
    }

    fn create_element(self: *mut Parser, t: *Token, ns: u8) !u32 {
        let id = self.doc.new_node(ELEMENT)!;
        let mut name = t.name;
        if (ns == NS_HTML) and (t.tag != 0) {
            name = TAG_NAMES[@as(usize, t.tag) - 1];
        } else if ns == NS_SVG {
            name = svg_tag_name(t.name);
        }
        if name.ptr == t.name.ptr {
            name = self.doc.arena.dup(name)!;
        }
        let attrs = self.copy_attrs(t.attrs, ns)!;
        let n = self.nd(id);
        n.ns = ns;
        n.tag = t.tag;
        n.name = name;
        n.attrs = attrs;
        if (ns == NS_MATHML) and (t.tag == T_ANNOTATION_XML) {
            for a in attrs {
                if strs.eq(a.name, "encoding") and
                   (strs.eq_ignore_case(a.value, "text/html") or
                    strs.eq_ignore_case(a.value, "application/xhtml+xml")) {
                    n.flags |= FL_HIP;
                }
            }
        }
        if (ns == NS_HTML) and (t.tag == T_TEMPLATE) {
            let f = self.doc.new_node(FRAGMENT)!;
            self.nd(f).contents = id;
            self.nd(id).contents = f;
        }
        return id;
    }

    // The token's attributes in the arena, adjusted for SVG and MathML.
    fn copy_attrs(self: *mut Parser, attrs: []Attr, ns: u8) ![]Attr {
        if attrs.len == 0 {
            return no_attrs();
        }
        let mut total: usize = 0;
        for a in attrs {
            total += a.name.len + a.value.len;
        }
        let buf = self.doc.arena.bytes(total)!;
        let mut at: usize = 0;
        self.abuf.clear();
        for a in attrs {
            @mem_copy(buf[at..at + a.name.len], a.name);
            let nm = buf[at..at + a.name.len];
            at += a.name.len;
            @mem_copy(buf[at..at + a.value.len], a.value);
            let v = buf[at..at + a.value.len];
            at += a.value.len;
            let mut b = Attr.{ ns: A_NONE, prefix: "", name: nm, value: v };
            if ns == NS_MATHML {
                if strs.eq(nm, "definitionurl") {
                    b.name = "definitionURL";
                }
                foreign_attr(&mut b);
            } else if ns == NS_SVG {
                b.name = svg_attr_name(nm);
                foreign_attr(&mut b);
            }
            self.abuf.push(b)!;
        }
        return self.doc.arena.copy.[Attr](self.abuf.items());
    }

    fn clone_element(self: *mut Parser, id: u32) !u32 {
        let e = self.doc.new_node(ELEMENT)!;
        let o = self.nd(id);
        let n = self.nd(e);
        n.ns = o.ns;
        n.tag = o.tag;
        n.name = o.name;
        n.attrs = o.attrs;
        n.flags = o.flags & FL_HIP;
        return e;
    }

    // Inserts an element for the token and opens it (unless the stack is
    // full).
    fn insert(self: *mut Parser, t: *Token, ns: u8) !u32 {
        let at = self.place(NONE);
        let id = self.create_element(t, ns)!;
        self.insert_node(at, id);
        _ = self.push(id)!;
        return id;
    }

    // Inserts an element that is closed at once.
    fn insert_void(self: *mut Parser, t: *Token) !u32 {
        let at = self.place(NONE);
        let id = self.create_element(t, NS_HTML)!;
        self.insert_node(at, id);
        return id;
    }

    fn insert_fake(self: *mut Parser, tag: u16) !u32 {
        let mut t = fake_token(tag);
        return self.insert(&mut t, NS_HTML);
    }

    fn opened(self: *Parser, id: u32) bool {
        return (self.nd(id).flags & FL_OPEN) != 0;
    }

    fn raw_text(self: *mut Parser, t: *Token, state: u8) !void {
        let id = self.insert(t, NS_HTML)!;
        self.tz.state = state;
        self.orig = self.mode;
        self.mode = M_TEXT;
        self.text_elem = NONE;
        if self.opened(id) {
            self.text_elem = id;
        }
    }

    fn add_missing_attrs(self: *mut Parser, id: u32, t: *Token) !void {
        let n = self.nd(id);
        let mut all = vec.Vec.[Attr].init();
        defer all.deinit();
        all.extend(n.attrs)!;
        let extra = self.copy_attrs(t.attrs, NS_HTML)!;
        for a in extra {
            let mut have = false;
            for b in n.attrs {
                if strs.eq(a.name, b.name) and (b.ns == A_NONE) {
                    have = true;
                }
            }
            if not have {
                all.push(a)!;
            }
        }
        if all.len != n.attrs.len {
            self.nd(id).attrs = self.doc.arena.copy.[Attr](all.items())!;
        }
    }
}

fn fake_token(tag: u16) Token {
    return Token.{ kind: TK_START, name: TAG_NAMES[@as(usize, tag) - 1], tag: tag, attrs: no_attrs(),
                   self_closing: false, data: "", public_id: "", system_id: "", has_name: false, has_public: false,
                   has_system: false, force_quirks: false };
}

fn scope_foreign(n: *Node) bool {
    if n.ns == NS_MATHML {
        return (n.tag == T_MI) or (n.tag == T_MO) or (n.tag == T_MN) or (n.tag == T_MS) or (n.tag == T_MTEXT) or
               (n.tag == T_ANNOTATION_XML);
    }
    if n.ns == NS_SVG {
        return (n.tag == T_FOREIGNOBJECT) or (n.tag == T_DESC) or (n.tag == T_TITLE);
    }
    return false;
}

fn foreign_attr(a: *mut Attr) void {
    let nm = a.name;
    if strs.starts_with(nm, "xlink:") {
        let local = nm[6..];
        for x in ["actuate", "arcrole", "href", "role", "show", "title", "type"]{
            if strs.eq(local, x) {
                a.ns = A_XLINK;
                a.prefix = "xlink";
                a.name = local;
                return;
            }
        }
        return;
    }
    if strs.eq(nm, "xml:lang") or strs.eq(nm, "xml:space") {
        a.ns = A_XML;
        a.prefix = "xml";
        a.name = nm[4..];
        return;
    }
    if strs.eq(nm, "xmlns") {
        a.ns = A_XMLNS;
        return;
    }
    if strs.eq(nm, "xmlns:xlink") {
        a.ns = A_XMLNS;
        a.prefix = "xmlns";
        a.name = nm[6..];
    }
}

let SVG_TAGS: [37][]u8 = [
    "altGlyph",
    "altGlyphDef",
    "altGlyphItem",
    "animateColor",
    "animateMotion",
    "animateTransform",
    "clipPath",
    "feBlend",
    "feColorMatrix",
    "feComponentTransfer",
    "feComposite",
    "feConvolveMatrix",
    "feDiffuseLighting",
    "feDisplacementMap",
    "feDistantLight",
    "feDropShadow",
    "feFlood",
    "feFuncA",
    "feFuncB",
    "feFuncG",
    "feFuncR",
    "feGaussianBlur",
    "feImage",
    "feMerge",
    "feMergeNode",
    "feMorphology",
    "feOffset",
    "fePointLight",
    "feSpecularLighting",
    "feSpotLight",
    "feTile",
    "feTurbulence",
    "foreignObject",
    "glyphRef",
    "linearGradient",
    "radialGradient",
    "textPath",
];

let SVG_ATTRS: [58][]u8 = [
    "attributeName",
    "attributeType",
    "baseFrequency",
    "baseProfile",
    "calcMode",
    "clipPathUnits",
    "diffuseConstant",
    "edgeMode",
    "filterUnits",
    "glyphRef",
    "gradientTransform",
    "gradientUnits",
    "kernelMatrix",
    "kernelUnitLength",
    "keyPoints",
    "keySplines",
    "keyTimes",
    "lengthAdjust",
    "limitingConeAngle",
    "markerHeight",
    "markerUnits",
    "markerWidth",
    "maskContentUnits",
    "maskUnits",
    "numOctaves",
    "pathLength",
    "patternContentUnits",
    "patternTransform",
    "patternUnits",
    "pointsAtX",
    "pointsAtY",
    "pointsAtZ",
    "preserveAlpha",
    "preserveAspectRatio",
    "primitiveUnits",
    "refX",
    "refY",
    "repeatCount",
    "repeatDur",
    "requiredExtensions",
    "requiredFeatures",
    "specularConstant",
    "specularExponent",
    "spreadMethod",
    "startOffset",
    "stdDeviation",
    "stitchTiles",
    "surfaceScale",
    "systemLanguage",
    "tableValues",
    "targetX",
    "targetY",
    "textLength",
    "viewBox",
    "viewTarget",
    "xChannelSelector",
    "yChannelSelector",
    "zoomAndPan",
];

fn svg_tag_name(name: []u8) []u8 {
    for x in SVG_TAGS {
        if strs.eq_ignore_case(x, name) {
            return x;
        }
    }
    return name;
}

fn svg_attr_name(name: []u8) []u8 {
    for x in SVG_ATTRS {
        if strs.eq_ignore_case(x, name) {
            return x;
        }
    }
    return name;
}

fn leading_ws(s: []u8) usize {
    let mut k: usize = 0;
    while (k < s.len) and is_ws(s[k]) {
        k += 1;
    }
    return k;
}

fn all_ws(s: []u8) bool {
    return leading_ws(s) == s.len;
}

// -----------------------------------------------------------------------------
// Tree construction
// -----------------------------------------------------------------------------

fn is_start(t: *Token, tag: u16) bool {
    return (t.kind == TK_START) and (t.tag == tag);
}

fn is_end(t: *Token, tag: u16) bool {
    return (t.kind == TK_END) and (t.tag == tag);
}

fn tag_in(t: *Token, tags: []u16) bool {
    if t.tag == 0 {
        return false;
    }
    for x in tags {
        if x == t.tag {
            return true;
        }
    }
    return false;
}

// Runs the tree construction dispatcher for one token.
fn dispatch(p: *mut Parser, t0: Token) !void {
    let mut t = t0;
    loop {
        let mut again = false;
        if html_content(p, &t) {
            again = step(p, p.mode, &mut t)!;
        } else {
            again = foreign(p, &mut t)!;
        }
        if not again {
            return;
        }
    }
}

fn html_content(p: *mut Parser, t: *Token) bool {
    if p.stack.len == 0 {
        return true;
    }
    let a = p.adjusted_current();
    let n = p.nd(a);
    if (n.ns == NS_HTML) or (t.kind == TK_EOF) {
        return true;
    }
    if p.mathml_text_ip(a) {
        if (t.kind == TK_START) and (t.tag != T_MGLYPH) and (t.tag != T_MALIGNMARK) {
            return true;
        }
        if t.kind == TK_CHARS {
            return true;
        }
    }
    if (n.ns == NS_MATHML) and (n.tag == T_ANNOTATION_XML) and is_start(t, T_SVG) {
        return true;
    }
    if p.html_ip(a) and ((t.kind == TK_START) or (t.kind == TK_CHARS)) {
        return true;
    }
    return false;
}

// Processes the token by the rules of an insertion mode: true to reprocess
// it (in p.mode).
fn step(p: *mut Parser, mode: u8, t: *mut Token) !bool {
    if mode == M_IN_BODY {
        return in_body(p, t);
    }
    if mode == M_TEXT {
        return in_text(p, t);
    }
    if mode == M_IN_TABLE {
        return in_table(p, t);
    }
    if mode == M_IN_CELL {
        return in_cell(p, t);
    }
    if mode == M_IN_ROW {
        return in_row(p, t);
    }
    if mode == M_IN_TABLE_BODY {
        return in_table_body(p, t);
    }
    if mode == M_IN_TABLE_TEXT {
        return in_table_text(p, t);
    }
    if mode == M_IN_HEAD {
        return in_head(p, t);
    }
    if mode == M_INITIAL {
        return initial(p, t);
    }
    if mode == M_BEFORE_HTML {
        return before_html(p, t);
    }
    if mode == M_BEFORE_HEAD {
        return before_head(p, t);
    }
    if mode == M_IN_HEAD_NOSCRIPT {
        return in_head_noscript(p, t);
    }
    if mode == M_AFTER_HEAD {
        return after_head(p, t);
    }
    if mode == M_IN_CAPTION {
        return in_caption(p, t);
    }
    if mode == M_IN_COLUMN_GROUP {
        return in_column_group(p, t);
    }
    if mode == M_IN_TEMPLATE {
        return in_template(p, t);
    }
    if mode == M_AFTER_BODY {
        return after_body(p, t);
    }
    if mode == M_IN_FRAMESET {
        return in_frameset(p, t);
    }
    if mode == M_AFTER_FRAMESET {
        return after_frameset(p, t);
    }
    if mode == M_AFTER_AFTER_BODY {
        return after_after_body(p, t);
    }
    return after_after_frameset(p, t);
}

// Whitespace at the start of a character token is handled by `f`'s mode
// (in_body, or 255 to insert it, or 254 to drop it); true if characters
// remain in t.
fn split_ws(p: *mut Parser, t: *mut Token, how: u8) !bool {
    let k = leading_ws(t.data);
    if k > 0 {
        if how == 255 {
            p.insert_text(t.data[..k])!;
        } else if how == M_IN_BODY {
            let mut w = t.*;
            w.data = t.data[..k];
            _ = in_body(p, &mut w)!;
        }
    }
    t.data = t.data[k..];
    return t.data.len > 0;
}

const WS_INSERT: u8 = 255;
const WS_DROP: u8 = 254;

fn initial(p: *mut Parser, t: *mut Token) !bool {
    if t.kind == TK_CHARS {
        if not split_ws(p, t, WS_DROP)! {
            return false;
        }
    } else if t.kind == TK_COMMENT {
        p.insert_comment(t, Loc.{ parent: 0, before: NONE })!;
        return false;
    } else if t.kind == TK_DOCTYPE {
        let id = p.doc.new_node(DOCTYPE)!;
        let n = p.nd(id);
        n.name = p.doc.arena.dup(t.name)!;
        n.text = p.doc.arena.dup(t.public_id)!;
        n.system_id = p.doc.arena.dup(t.system_id)!;
        p.doc.append_child(0, id);
        p.doc.mode = doctype_mode(t);
        p.mode = M_BEFORE_HTML;
        return false;
    }
    p.doc.mode = QUIRKS;
    p.mode = M_BEFORE_HTML;
    return true;
}

let QUIRKY_PUBLIC: [55][]u8 = [
    "+//silmaril//dtd html pro v0r11 19970101//",
    "-//as//dtd html 3.0 aswedit + extensions//",
    "-//advasoft ltd//dtd html 3.0 aswedit + extensions//",
    "-//ietf//dtd html 2.0 level 1//",
    "-//ietf//dtd html 2.0 level 2//",
    "-//ietf//dtd html 2.0 strict level 1//",
    "-//ietf//dtd html 2.0 strict level 2//",
    "-//ietf//dtd html 2.0 strict//",
    "-//ietf//dtd html 2.0//",
    "-//ietf//dtd html 2.1e//",
    "-//ietf//dtd html 3.0//",
    "-//ietf//dtd html 3.2 final//",
    "-//ietf//dtd html 3.2//",
    "-//ietf//dtd html 3//",
    "-//ietf//dtd html level 0//",
    "-//ietf//dtd html level 1//",
    "-//ietf//dtd html level 2//",
    "-//ietf//dtd html level 3//",
    "-//ietf//dtd html strict level 0//",
    "-//ietf//dtd html strict level 1//",
    "-//ietf//dtd html strict level 2//",
    "-//ietf//dtd html strict level 3//",
    "-//ietf//dtd html strict//",
    "-//ietf//dtd html//",
    "-//metrius//dtd metrius presentational//",
    "-//microsoft//dtd internet explorer 2.0 html strict//",
    "-//microsoft//dtd internet explorer 2.0 html//",
    "-//microsoft//dtd internet explorer 2.0 tables//",
    "-//microsoft//dtd internet explorer 3.0 html strict//",
    "-//microsoft//dtd internet explorer 3.0 html//",
    "-//microsoft//dtd internet explorer 3.0 tables//",
    "-//netscape comm. corp.//dtd html//",
    "-//netscape comm. corp.//dtd strict html//",
    "-//o'reilly and associates//dtd html 2.0//",
    "-//o'reilly and associates//dtd html extended 1.0//",
    "-//o'reilly and associates//dtd html extended relaxed 1.0//",
    "-//sq//dtd html 2.0 hotmetal + extensions//",
    "-//softquad software//dtd hotmetal pro 6.0::19990601::extensions to html 4.0//",
    "-//softquad//dtd hotmetal pro 4.0::19971010::extensions to html 4.0//",
    "-//spyglass//dtd html 2.0 extended//",
    "-//sun microsystems corp.//dtd hotjava html//",
    "-//sun microsystems corp.//dtd hotjava strict html//",
    "-//w3c//dtd html 3 1995-03-24//",
    "-//w3c//dtd html 3.2 draft//",
    "-//w3c//dtd html 3.2 final//",
    "-//w3c//dtd html 3.2//",
    "-//w3c//dtd html 3.2s draft//",
    "-//w3c//dtd html 4.0 frameset//",
    "-//w3c//dtd html 4.0 transitional//",
    "-//w3c//dtd html experimental 19960712//",
    "-//w3c//dtd html experimental 970421//",
    "-//w3c//dtd w3 html//",
    "-//w3o//dtd w3 html 3.0//",
    "-//webtechs//dtd mozilla html 2.0//",
    "-//webtechs//dtd mozilla html//",
];

fn starts_ci(s: []u8, prefix: []u8) bool {
    return (s.len >= prefix.len) and strs.eq_ignore_case(s[..prefix.len], prefix);
}

fn doctype_mode(t: *Token) u8 {
    let pub_id = t.public_id;
    let sys = t.system_id;
    if t.force_quirks or not strs.eq(t.name, "html") {
        return QUIRKS;
    }
    if t.has_public {
        if strs.eq_ignore_case(pub_id, "-//W3O//DTD W3 HTML Strict 3.0//EN//") or
           strs.eq_ignore_case(pub_id, "-/W3C/DTD HTML 4.0 Transitional/EN") or strs.eq_ignore_case(pub_id, "HTML") {
            return QUIRKS;
        }
        for q in QUIRKY_PUBLIC {
            if starts_ci(pub_id, q) {
                return QUIRKS;
            }
        }
    }
    if t.has_system and strs.eq_ignore_case(sys, "http://www.ibm.com/data/dtd/v11/ibmxhtml1-transitional.dtd") {
        return QUIRKS;
    }
    let html401 = starts_ci(pub_id, "-//W3C//DTD HTML 4.01 Frameset//") or
                  starts_ci(pub_id, "-//W3C//DTD HTML 4.01 Transitional//");
    if t.has_public and html401 and not t.has_system {
        return QUIRKS;
    }
    if t.has_public and
       (starts_ci(pub_id, "-//W3C//DTD XHTML 1.0 Frameset//") or
        starts_ci(pub_id, "-//W3C//DTD XHTML 1.0 Transitional//") or (html401 and t.has_system)) {
        return LIMITED_QUIRKS;
    }
    return NO_QUIRKS;
}

// An end tag the "before" modes treat as anything else.
fn end_hbhb(t: *Token) bool {
    return (t.kind == TK_END) and ((t.tag == T_HEAD) or (t.tag == T_BODY) or (t.tag == T_HTML) or (t.tag == T_BR));
}

fn before_html(p: *mut Parser, t: *mut Token) !bool {
    if t.kind == TK_DOCTYPE {
        return false;
    }
    if t.kind == TK_COMMENT {
        p.insert_comment(t, Loc.{ parent: 0, before: NONE })!;
        return false;
    }
    if t.kind == TK_CHARS {
        if not split_ws(p, t, WS_DROP)! {
            return false;
        }
    }
    if is_start(t, T_HTML) {
        let id = p.create_element(t, NS_HTML)!;
        p.doc.append_child(0, id);
        _ = p.push(id)!;
        p.mode = M_BEFORE_HEAD;
        return false;
    }
    if (t.kind == TK_END) and not end_hbhb(t) {
        return false;
    }
    let mut h = fake_token(T_HTML);
    let id = p.create_element(&mut h, NS_HTML)!;
    p.doc.append_child(0, id);
    _ = p.push(id)!;
    p.mode = M_BEFORE_HEAD;
    return true;
}

fn before_head(p: *mut Parser, t: *mut Token) !bool {
    if t.kind == TK_CHARS {
        if not split_ws(p, t, WS_DROP)! {
            return false;
        }
    } else if t.kind == TK_COMMENT {
        p.insert_comment(t, p.place(NONE))!;
        return false;
    } else if t.kind == TK_DOCTYPE {
        return false;
    } else if is_start(t, T_HTML) {
        return in_body(p, t);
    } else if is_start(t, T_HEAD) {
        p.head = p.insert(t, NS_HTML)!;
        p.mode = M_IN_HEAD;
        return false;
    } else if (t.kind == TK_END) and not end_hbhb(t) {
        return false;
    }
    p.head = p.insert_fake(T_HEAD)!;
    p.mode = M_IN_HEAD;
    return true;
}

fn in_head(p: *mut Parser, t: *mut Token) !bool {
    if t.kind == TK_CHARS {
        if not split_ws(p, t, WS_INSERT)! {
            return false;
        }
    } else if t.kind == TK_COMMENT {
        p.insert_comment(t, p.place(NONE))!;
        return false;
    } else if t.kind == TK_DOCTYPE {
        return false;
    } else if t.kind == TK_START {
        let g = t.tag;
        if g == T_HTML {
            return in_body(p, t);
        }
        if (g == T_BASE) or (g == T_BASEFONT) or (g == T_BGSOUND) or (g == T_LINK) or (g == T_META) {
            _ = p.insert_void(t)!;
            return false;
        }
        if g == T_TITLE {
            p.raw_text(t, S_RCDATA)!;
            return false;
        }
        if ((g == T_NOSCRIPT) and p.scripting) or (g == T_NOFRAMES) or (g == T_STYLE) {
            p.raw_text(t, S_RAWTEXT)!;
            return false;
        }
        if g == T_NOSCRIPT {
            _ = p.insert(t, NS_HTML)!;
            p.mode = M_IN_HEAD_NOSCRIPT;
            return false;
        }
        if g == T_SCRIPT {
            p.raw_text(t, S_SCRIPT)!;
            return false;
        }
        if g == T_TEMPLATE {
            let id = p.insert(t, NS_HTML)!;
            if not p.opened(id) {
                return false;
            }
            p.afe.push(MARKER)!;
            p.frameset_ok = false;
            p.mode = M_IN_TEMPLATE;
            p.tmodes.push(M_IN_TEMPLATE)!;
            return false;
        }
        if g == T_HEAD {
            return false;
        }
    } else if t.kind == TK_END {
        let g = t.tag;
        if g == T_HEAD {
            p.pop();
            p.mode = M_AFTER_HEAD;
            return false;
        }
        if g == T_TEMPLATE {
            if not p.has_template() {
                return false;
            }
            p.gen_thorough();
            p.pop_until(T_TEMPLATE);
            p.afe_clear_to_marker();
            _ = p.tmodes.pop() catch 0;
            p.reset_mode();
            return false;
        }
        if (g != T_BODY) and (g != T_HTML) and (g != T_BR) {
            return false;
        }
    }
    p.pop();
    p.mode = M_AFTER_HEAD;
    return true;
}

fn in_head_noscript(p: *mut Parser, t: *mut Token) !bool {
    if t.kind == TK_DOCTYPE {
        return false;
    }
    if is_start(t, T_HTML) {
        return in_body(p, t);
    }
    if is_end(t, T_NOSCRIPT) {
        p.pop();
        p.mode = M_IN_HEAD;
        return false;
    }
    if t.kind == TK_CHARS {
        if not split_ws(p, t, WS_INSERT)! {
            return false;
        }
    } else if t.kind == TK_COMMENT {
        return in_head(p, t);
    } else if t.kind == TK_START {
        let g = t.tag;
        if (g == T_BASEFONT) or (g == T_BGSOUND) or (g == T_LINK) or (g == T_META) or (g == T_NOFRAMES) or
           (g == T_STYLE) {
            return in_head(p, t);
        }
        if (g == T_HEAD) or (g == T_NOSCRIPT) {
            return false;
        }
    } else if (t.kind == TK_END) and (t.tag != T_BR) {
        return false;
    }
    p.pop();
    p.mode = M_IN_HEAD;
    return true;
}

fn after_head(p: *mut Parser, t: *mut Token) !bool {
    if t.kind == TK_CHARS {
        if not split_ws(p, t, WS_INSERT)! {
            return false;
        }
    } else if t.kind == TK_COMMENT {
        p.insert_comment(t, p.place(NONE))!;
        return false;
    } else if t.kind == TK_DOCTYPE {
        return false;
    } else if t.kind == TK_START {
        let g = t.tag;
        if g == T_HTML {
            return in_body(p, t);
        }
        if g == T_BODY {
            _ = p.insert(t, NS_HTML)!;
            p.frameset_ok = false;
            p.mode = M_IN_BODY;
            return false;
        }
        if g == T_FRAMESET {
            _ = p.insert(t, NS_HTML)!;
            p.mode = M_IN_FRAMESET;
            return false;
        }
        if (g == T_BASE) or (g == T_BASEFONT) or (g == T_BGSOUND) or (g == T_LINK) or (g == T_META) or
           (g == T_NOFRAMES) or (g == T_SCRIPT) or (g == T_STYLE) or (g == T_TEMPLATE) or (g == T_TITLE) {
            let h = p.head;
            if (h == NONE) or not p.push(h)! {
                return in_head(p, t);
            }
            let r = in_head(p, t)!;
            p.stack_remove(h);
            return r;
        }
        if g == T_HEAD {
            return false;
        }
    } else if t.kind == TK_END {
        if t.tag == T_TEMPLATE {
            return in_head(p, t);
        }
        if (t.tag != T_BODY) and (t.tag != T_HTML) and (t.tag != T_BR) {
            return false;
        }
    }
    _ = p.insert_fake(T_BODY)!;
    p.frameset_ok = true;
    p.mode = M_IN_BODY;
    return true;
}

let CLOSES_P: [25]u16 = [
    T_ADDRESS,
    T_ARTICLE,
    T_ASIDE,
    T_BLOCKQUOTE,
    T_CENTER,
    T_DETAILS,
    T_DIALOG,
    T_DIR,
    T_DIV,
    T_DL,
    T_FIELDSET,
    T_FIGCAPTION,
    T_FIGURE,
    T_FOOTER,
    T_HEADER,
    T_HGROUP,
    T_MAIN,
    T_MENU,
    T_NAV,
    T_OL,
    T_P,
    T_SEARCH,
    T_SECTION,
    T_SUMMARY,
    T_UL,
];

let BLOCK_ENDS: [28]u16 = [
    T_ADDRESS,
    T_ARTICLE,
    T_ASIDE,
    T_BLOCKQUOTE,
    T_BUTTON,
    T_CENTER,
    T_DETAILS,
    T_DIALOG,
    T_DIR,
    T_DIV,
    T_DL,
    T_FIELDSET,
    T_FIGCAPTION,
    T_FIGURE,
    T_FOOTER,
    T_HEADER,
    T_HGROUP,
    T_LISTING,
    T_MAIN,
    T_MENU,
    T_NAV,
    T_OL,
    T_PRE,
    T_SEARCH,
    T_SECTION,
    T_SELECT,
    T_SUMMARY,
    T_UL,
];

let FORMATTING: [12]u16 = [T_B, T_BIG, T_CODE, T_EM, T_FONT, T_I, T_S, T_SMALL, T_STRIKE, T_STRONG, T_TT, T_U];

let FORMATTING_ENDS: [14]u16 = [
    T_A,
    T_B,
    T_BIG,
    T_CODE,
    T_EM,
    T_FONT,
    T_I,
    T_NOBR,
    T_S,
    T_SMALL,
    T_STRIKE,
    T_STRONG,
    T_TT,
    T_U,
];

let HEAD_TAGS: [10]u16 = [T_BASE, T_BASEFONT, T_BGSOUND, T_LINK, T_META, T_NOFRAMES, T_SCRIPT, T_STYLE, T_TEMPLATE,
                          T_TITLE];

let TABLE_PARTS: [11]u16 = [T_CAPTION, T_COL, T_COLGROUP, T_FRAME, T_HEAD, T_TBODY, T_TD, T_TFOOT, T_TH, T_THEAD, T_TR];

fn in_body_chars(p: *mut Parser, data: []u8) !void {
    let mut s = data;
    if (strs.index_of_byte(data, 0) catch NONE_USIZE) != NONE_USIZE {
        p.sbuf.clear();
        for c in data {
            if c != 0 {
                p.sbuf.push(c)!;
            }
        }
        s = p.sbuf.bytes();
    }
    if s.len == 0 {
        return;
    }
    p.reconstruct()!;
    p.insert_text(s)!;
    if not all_ws(s) {
        p.frameset_ok = false;
    }
}

fn in_body(p: *mut Parser, t: *mut Token) !bool {
    let k = t.kind;
    if k == TK_CHARS {
        in_body_chars(p, t.data)!;
        return false;
    }
    if k == TK_START {
        return in_body_start(p, t);
    }
    if k == TK_END {
        return in_body_end(p, t);
    }
    if k == TK_COMMENT {
        p.insert_comment(t, p.place(NONE))!;
        return false;
    }
    if k == TK_EOF {
        if p.tmodes.len > 0 {
            return in_template(p, t);
        }
    }
    return false;
}

fn in_body_start(p: *mut Parser, t: *mut Token) !bool {
    let g = t.tag;
    if g == 0 {
        p.reconstruct()!;
        _ = p.insert(t, NS_HTML)!;
        return false;
    }
    if tag_in(t, FORMATTING[..]) {
        p.reconstruct()!;
        let id = p.insert(t, NS_HTML)!;
        p.afe_push(id)!;
        return false;
    }
    if tag_in(t, CLOSES_P[..]) {
        p.close_p_in_button_scope();
        _ = p.insert(t, NS_HTML)!;
        return false;
    }
    if g == T_HTML {
        if not p.has_template() {
            p.add_missing_attrs(p.stack.items()[0], t)!;
        }
        return false;
    }
    if tag_in(t, HEAD_TAGS[..]) {
        return in_head(p, t);
    }
    if g == T_BODY {
        if (p.stack.len == 1) or not p.is(p.stack.items()[1], T_BODY) or p.has_template() {
            return false;
        }
        p.frameset_ok = false;
        p.add_missing_attrs(p.stack.items()[1], t)!;
        return false;
    }
    if g == T_FRAMESET {
        if (p.stack.len == 1) or not p.is(p.stack.items()[1], T_BODY) or not p.frameset_ok {
            return false;
        }
        p.doc.remove(p.stack.items()[1]);
        p.pop_to(1);
        _ = p.insert(t, NS_HTML)!;
        p.mode = M_IN_FRAMESET;
        return false;
    }
    if (g >= T_H1) and (g <= T_H6) {
        p.close_p_in_button_scope();
        let c = p.nd(p.cur());
        if (c.ns == NS_HTML) and (c.tag >= T_H1) and (c.tag <= T_H6) {
            p.pop();
        }
        _ = p.insert(t, NS_HTML)!;
        return false;
    }
    if (g == T_PRE) or (g == T_LISTING) {
        p.close_p_in_button_scope();
        _ = p.insert(t, NS_HTML)!;
        p.skip_lf = true;
        p.frameset_ok = false;
        return false;
    }
    if g == T_FORM {
        let tpl = p.has_template();
        if (p.form != NONE) and not tpl {
            return false;
        }
        p.close_p_in_button_scope();
        let id = p.insert(t, NS_HTML)!;
        if not tpl {
            p.form = id;
        }
        return false;
    }
    if (g == T_LI) or (g == T_DD) or (g == T_DT) {
        p.frameset_ok = false;
        let mut i = p.stack.len;
        while i > 0 {
            i -= 1;
            let id = p.stack.items()[i];
            let n = p.nd(id);
            let mut hit: u16 = 0;
            if (g == T_LI) and p.is(id, T_LI) {
                hit = T_LI;
            } else if (g != T_LI) and (p.is(id, T_DD) or p.is(id, T_DT)) {
                hit = n.tag;
            }
            if hit != 0 {
                p.gen_implied(hit);
                p.pop_until(hit);
                break;
            }
            if p.special(id) and not p.is(id, T_ADDRESS) and not p.is(id, T_DIV) and not p.is(id, T_P) {
                break;
            }
        }
        p.close_p_in_button_scope();
        _ = p.insert(t, NS_HTML)!;
        return false;
    }
    if g == T_PLAINTEXT {
        p.close_p_in_button_scope();
        _ = p.insert(t, NS_HTML)!;
        p.tz.state = S_PLAINTEXT;
        return false;
    }
    if g == T_BUTTON {
        if p.in_scope(T_BUTTON, SC_DEFAULT) {
            p.gen_implied(0);
            p.pop_until(T_BUTTON);
        }
        p.reconstruct()!;
        _ = p.insert(t, NS_HTML)!;
        p.frameset_ok = false;
        return false;
    }
    if g == T_A {
        let items = p.afe.items();
        let mut i = items.len;
        let mut old = NONE;
        while i > 0 {
            i -= 1;
            if items[i] == MARKER {
                break;
            }
            if p.is(items[i], T_A) {
                old = items[i];
                break;
            }
        }
        if old != NONE {
            if not adoption(p, t)! {
                any_other_end(p, t);
            }
            p.afe_remove(old);
            p.stack_remove(old);
        }
        p.reconstruct()!;
        let id = p.insert(t, NS_HTML)!;
        p.afe_push(id)!;
        return false;
    }
    if g == T_NOBR {
        p.reconstruct()!;
        if p.in_scope(T_NOBR, SC_DEFAULT) {
            if not adoption(p, t)! {
                any_other_end(p, t);
            }
            p.reconstruct()!;
        }
        let id = p.insert(t, NS_HTML)!;
        p.afe_push(id)!;
        return false;
    }
    if (g == T_APPLET) or (g == T_MARQUEE) or (g == T_OBJECT) {
        p.reconstruct()!;
        let id = p.insert(t, NS_HTML)!;
        if p.opened(id) {
            p.afe.push(MARKER)!;
        }
        p.frameset_ok = false;
        return false;
    }
    if g == T_TABLE {
        if p.doc.mode != QUIRKS {
            p.close_p_in_button_scope();
        }
        _ = p.insert(t, NS_HTML)!;
        p.frameset_ok = false;
        p.mode = M_IN_TABLE;
        return false;
    }
    if (g == T_AREA) or (g == T_BR) or (g == T_EMBED) or (g == T_IMG) or (g == T_KEYGEN) or (g == T_WBR) {
        p.reconstruct()!;
        _ = p.insert_void(t)!;
        p.frameset_ok = false;
        return false;
    }
    if g == T_INPUT {
        if p.is(p.context, T_SELECT) {
            return false;
        }
        if p.in_scope(T_SELECT, SC_DEFAULT) {
            p.pop_until(T_SELECT);
        }
        p.reconstruct()!;
        _ = p.insert_void(t)!;
        if not is_hidden_input(t) {
            p.frameset_ok = false;
        }
        return false;
    }
    if (g == T_PARAM) or (g == T_SOURCE) or (g == T_TRACK) {
        _ = p.insert_void(t)!;
        return false;
    }
    if g == T_HR {
        p.close_p_in_button_scope();
        if p.in_scope(T_SELECT, SC_DEFAULT) {
            p.gen_implied(0);
        }
        _ = p.insert_void(t)!;
        p.frameset_ok = false;
        return false;
    }
    if g == T_IMAGE {
        t.name = "img";
        t.tag = T_IMG;
        return true;
    }
    if g == T_TEXTAREA {
        p.raw_text(t, S_RCDATA)!;
        p.skip_lf = true;
        p.frameset_ok = false;
        return false;
    }
    if g == T_XMP {
        p.close_p_in_button_scope();
        p.reconstruct()!;
        p.frameset_ok = false;
        p.raw_text(t, S_RAWTEXT)!;
        return false;
    }
    if g == T_IFRAME {
        p.frameset_ok = false;
        p.raw_text(t, S_RAWTEXT)!;
        return false;
    }
    if (g == T_NOEMBED) or ((g == T_NOSCRIPT) and p.scripting) {
        p.raw_text(t, S_RAWTEXT)!;
        return false;
    }
    if g == T_SELECT {
        if p.is(p.context, T_SELECT) {
            return false;
        }
        if p.in_scope(T_SELECT, SC_DEFAULT) {
            p.pop_until(T_SELECT);
            return false;
        }
        p.reconstruct()!;
        _ = p.insert(t, NS_HTML)!;
        p.frameset_ok = false;
        return false;
    }
    if g == T_OPTION {
        if p.in_scope(T_SELECT, SC_DEFAULT) {
            p.gen_implied(T_OPTGROUP);
        } else if p.cur_is(T_OPTION) {
            p.pop();
        }
        p.reconstruct()!;
        _ = p.insert(t, NS_HTML)!;
        return false;
    }
    if g == T_OPTGROUP {
        if p.in_scope(T_SELECT, SC_DEFAULT) {
            p.gen_implied(0);
        } else if p.cur_is(T_OPTION) {
            p.pop();
        }
        p.reconstruct()!;
        _ = p.insert(t, NS_HTML)!;
        return false;
    }
    if (g == T_RB) or (g == T_RTC) {
        if p.in_scope(T_RUBY, SC_DEFAULT) {
            p.gen_implied(0);
        }
        _ = p.insert(t, NS_HTML)!;
        return false;
    }
    if (g == T_RP) or (g == T_RT) {
        if p.in_scope(T_RUBY, SC_DEFAULT) {
            p.gen_implied(T_RTC);
        }
        _ = p.insert(t, NS_HTML)!;
        return false;
    }
    if (g == T_MATH) or (g == T_SVG) {
        p.reconstruct()!;
        let mut ns = NS_MATHML;
        if g == T_SVG {
            ns = NS_SVG;
        }
        let id = p.insert(t, ns)!;
        if t.self_closing and p.opened(id) {
            p.pop();
        }
        return false;
    }
    if tag_in(t, TABLE_PARTS[..]) {
        return false;
    }
    p.reconstruct()!;
    _ = p.insert(t, NS_HTML)!;
    return false;
}

fn is_hidden_input(t: *Token) bool {
    for a in t.attrs {
        if strs.eq(a.name, "type") {
            return strs.eq_ignore_case(a.value, "hidden");
        }
    }
    return false;
}

fn in_body_end(p: *mut Parser, t: *mut Token) !bool {
    let g = t.tag;
    if g == 0 {
        any_other_end(p, t);
        return false;
    }
    if tag_in(t, FORMATTING_ENDS[..]) {
        if not adoption(p, t)! {
            any_other_end(p, t);
        }
        return false;
    }
    if tag_in(t, BLOCK_ENDS[..]) {
        if not p.in_scope(g, SC_DEFAULT) {
            return false;
        }
        p.gen_implied(0);
        p.pop_until(g);
        return false;
    }
    if g == T_TEMPLATE {
        return in_head(p, t);
    }
    if (g == T_BODY) or (g == T_HTML) {
        if not p.in_scope(T_BODY, SC_DEFAULT) {
            return false;
        }
        p.mode = M_AFTER_BODY;
        return g == T_HTML;
    }
    if g == T_FORM {
        if not p.has_template() {
            let node = p.form;
            p.form = NONE;
            if (node == NONE) or not p.node_in_scope(node) {
                return false;
            }
            p.gen_implied(0);
            p.stack_remove(node);
        } else {
            if not p.in_scope(T_FORM, SC_DEFAULT) {
                return false;
            }
            p.gen_implied(0);
            p.pop_until(T_FORM);
        }
        return false;
    }
    if g == T_P {
        if not p.in_scope(T_P, SC_BUTTON) {
            _ = p.insert_fake(T_P)!;
        }
        p.close_p();
        return false;
    }
    if g == T_LI {
        if not p.in_scope(T_LI, SC_LIST) {
            return false;
        }
        p.gen_implied(T_LI);
        p.pop_until(T_LI);
        return false;
    }
    if (g == T_DD) or (g == T_DT) {
        if not p.in_scope(g, SC_DEFAULT) {
            return false;
        }
        p.gen_implied(g);
        p.pop_until(g);
        return false;
    }
    if (g >= T_H1) and (g <= T_H6) {
        if not p.heading_in_scope() {
            return false;
        }
        p.gen_implied(0);
        while p.stack.len > 1 {
            let n = p.nd(p.cur());
            let h = (n.ns == NS_HTML) and (n.tag >= T_H1) and (n.tag <= T_H6);
            p.pop();
            if h {
                break;
            }
        }
        return false;
    }
    if (g == T_APPLET) or (g == T_MARQUEE) or (g == T_OBJECT) {
        if not p.in_scope(g, SC_DEFAULT) {
            return false;
        }
        p.gen_implied(0);
        p.pop_until(g);
        p.afe_clear_to_marker();
        return false;
    }
    if g == T_BR {
        let mut b = fake_token(T_BR);
        p.reconstruct()!;
        _ = p.insert_void(&mut b)!;
        p.frameset_ok = false;
        return false;
    }
    any_other_end(p, t);
    return false;
}

fn same_name(n: *Node, t: *Token) bool {
    return (n.kind == ELEMENT) and (n.ns == NS_HTML) and (n.tag == t.tag) and ((t.tag != 0) or strs.eq(n.name, t.name));
}

fn any_other_end(p: *mut Parser, t: *Token) void {
    let mut i = p.stack.len;
    while i > 1 {
        i -= 1;
        let id = p.stack.items()[i];
        if same_name(p.nd(id), t) {
            p.gen_implied(t.tag);
            p.pop_until_node(id);
            return;
        }
        if p.special(id) {
            return;
        }
    }
}

// The adoption agency algorithm: false when the caller should act as for
// "any other end tag".
fn adoption(p: *mut Parser, t: *Token) !bool {
    let subject = t.tag;
    let c = p.cur();
    if p.is(c, subject) and ((p.nd(c).flags & FL_AFE) == 0) {
        p.pop();
        return true;
    }
    let mut outer: u32 = 0;
    while outer < 8 {
        outer += 1;
        // the formatting element
        let mut fi = p.afe.len;
        let mut fe = NONE;
        while fi > 0 {
            fi -= 1;
            let e = p.afe.items()[fi];
            if e == MARKER {
                break;
            }
            if p.is(e, subject) {
                fe = e;
                break;
            }
        }
        if fe == NONE {
            return false;
        }
        if not p.opened(fe) {
            p.afe_remove_at(fi);
            return true;
        }
        if not p.node_in_scope(fe) {
            return true;
        }
        // the furthest block
        let si = p.stack_index(fe);
        let mut fb = NONE;
        let mut fbi = si + 1;
        while fbi < p.stack.len {
            if p.special(p.stack.items()[fbi]) {
                fb = p.stack.items()[fbi];
                break;
            }
            fbi += 1;
        }
        if fb == NONE {
            p.pop_to(si);
            p.afe_remove(fe);
            return true;
        }
        let ca = p.stack.items()[si - 1];
        let mut bookmark = fi;
        let mut ni = fbi;
        let mut last = fb;
        let mut inner: u32 = 0;
        loop {
            inner += 1;
            ni -= 1;
            let node = p.stack.items()[ni];
            if node == fe {
                break;
            }
            let mut ai = p.afe_index(node);
            if (inner > 3) and (ai != NONE_USIZE) {
                p.afe_remove_at(ai);
                if ai < bookmark {
                    bookmark -= 1;
                }
                ai = NONE_USIZE;
            }
            if ai == NONE_USIZE {
                _ = p.stack.remove(ni);
                p.closed_at(node);
                continue;
            }
            let e = p.clone_element(node)!;
            p.afe.items()[ai] = e;
            p.stack.items()[ni] = e;
            p.closed_at(node);
            p.nd(node).flags &= ~FL_AFE;
            p.opened_at(e);
            p.nd(e).flags |= FL_AFE;
            if last == fb {
                bookmark = ai + 1;
            }
            p.doc.remove(last);
            p.doc.append_child(e, last);
            last = e;
        }
        p.doc.remove(last);
        p.insert_node(p.place(ca), last);
        let e = p.clone_element(fe)!;
        // the furthest block's children move into the new element
        loop {
            let ch = p.nd(fb).first;
            if ch == NONE {
                break;
            }
            p.doc.remove(ch);
            p.doc.append_child(e, ch);
        }
        p.doc.append_child(fb, e);
        let fi2 = p.afe_index(fe);
        p.afe_remove_at(fi2);
        if fi2 < bookmark {
            bookmark -= 1;
        }
        p.afe.insert(@min(bookmark, p.afe.len), e)!;
        p.nd(e).flags |= FL_AFE;
        p.stack_remove(fe);
        let at = p.stack_index(fb);
        p.stack.insert(at + 1, e)!;
        p.opened_at(e);
    }
    return true;
}

fn in_text(p: *mut Parser, t: *mut Token) !bool {
    if t.kind == TK_CHARS {
        p.insert_text(t.data)!;
        return false;
    }
    if (t.kind == TK_EOF) or (t.kind == TK_END) {
        if (p.text_elem != NONE) and (p.cur() == p.text_elem) {
            p.pop();
        }
        p.text_elem = NONE;
        p.mode = p.orig;
        return t.kind == TK_EOF;
    }
    return false;
}

fn cur_is_table_ish(p: *mut Parser) bool {
    let c = p.cur();
    return p.is(c, T_TABLE) or p.is(c, T_TBODY) or p.is(c, T_TEMPLATE) or p.is(c, T_TFOOT) or p.is(c, T_THEAD) or
           p.is(c, T_TR);
}

fn in_table(p: *mut Parser, t: *mut Token) !bool {
    let k = t.kind;
    if (k == TK_CHARS) and cur_is_table_ish(p) {
        p.pend.clear();
        p.pend_text = false;
        p.orig = p.mode;
        p.mode = M_IN_TABLE_TEXT;
        return true;
    }
    if k == TK_COMMENT {
        p.insert_comment(t, p.place(NONE))!;
        return false;
    }
    if k == TK_DOCTYPE {
        return false;
    }
    if k == TK_START {
        let g = t.tag;
        if g == T_CAPTION {
            p.clear_to(T_TABLE, T_TABLE, T_TABLE);
            p.afe.push(MARKER)!;
            _ = p.insert(t, NS_HTML)!;
            p.mode = M_IN_CAPTION;
            return false;
        }
        if g == T_COLGROUP {
            p.clear_to(T_TABLE, T_TABLE, T_TABLE);
            _ = p.insert(t, NS_HTML)!;
            p.mode = M_IN_COLUMN_GROUP;
            return false;
        }
        if g == T_COL {
            p.clear_to(T_TABLE, T_TABLE, T_TABLE);
            _ = p.insert_fake(T_COLGROUP)!;
            p.mode = M_IN_COLUMN_GROUP;
            return true;
        }
        if (g == T_TBODY) or (g == T_TFOOT) or (g == T_THEAD) {
            p.clear_to(T_TABLE, T_TABLE, T_TABLE);
            _ = p.insert(t, NS_HTML)!;
            p.mode = M_IN_TABLE_BODY;
            return false;
        }
        if (g == T_TD) or (g == T_TH) or (g == T_TR) {
            p.clear_to(T_TABLE, T_TABLE, T_TABLE);
            _ = p.insert_fake(T_TBODY)!;
            p.mode = M_IN_TABLE_BODY;
            return true;
        }
        if g == T_TABLE {
            if not p.in_scope(T_TABLE, SC_TABLE) {
                return false;
            }
            p.pop_until(T_TABLE);
            p.reset_mode();
            return true;
        }
        if (g == T_STYLE) or (g == T_SCRIPT) or (g == T_TEMPLATE) {
            return in_head(p, t);
        }
        if (g == T_INPUT) and is_hidden_input(t) {
            _ = p.insert_void(t)!;
            return false;
        }
        if g == T_FORM {
            if (p.form != NONE) and not p.in_template_contents() {
                return false;
            }
            let id = p.insert_void(t)!;
            if not p.in_template_contents() {
                p.form = id;
            }
            return false;
        }
    } else if k == TK_END {
        let g = t.tag;
        if g == T_TABLE {
            if not p.in_scope(T_TABLE, SC_TABLE) {
                return false;
            }
            p.pop_until(T_TABLE);
            p.reset_mode();
            return false;
        }
        if (g == T_BODY) or (g == T_CAPTION) or (g == T_COL) or (g == T_COLGROUP) or (g == T_HTML) or (g == T_TBODY) or
           (g == T_TD) or (g == T_TFOOT) or (g == T_TH) or (g == T_THEAD) or (g == T_TR) {
            return false;
        }
        if g == T_TEMPLATE {
            return in_head(p, t);
        }
    } else if k == TK_EOF {
        return in_body(p, t);
    }
    p.foster = true;
    let r = in_body(p, t);
    p.foster = false;
    return r;
}

fn in_table_text(p: *mut Parser, t: *mut Token) !bool {
    if t.kind == TK_CHARS {
        for c in t.data {
            if c == 0 {
                continue;
            }
            p.pend.push(c)!;
            if not is_ws(c) {
                p.pend_text = true;
            }
        }
        return false;
    }
    if p.pend_text {
        p.foster = true;
        let r = in_body_chars(p, p.pend.bytes());
        p.foster = false;
        r!;
    } else {
        p.insert_text(p.pend.bytes())!;
    }
    p.pend.clear();
    p.mode = p.orig;
    return true;
}

fn close_caption(p: *mut Parser) bool {
    if not p.in_scope(T_CAPTION, SC_TABLE) {
        return false;
    }
    p.gen_implied(0);
    p.pop_until(T_CAPTION);
    p.afe_clear_to_marker();
    p.mode = M_IN_TABLE;
    return true;
}

fn in_caption(p: *mut Parser, t: *mut Token) !bool {
    let g = t.tag;
    if is_end(t, T_CAPTION) {
        _ = close_caption(p);
        return false;
    }
    if ((t.kind == TK_START) and
        ((g == T_CAPTION) or (g == T_COL) or (g == T_COLGROUP) or (g == T_TBODY) or (g == T_TD) or (g == T_TFOOT) or
         (g == T_TH) or (g == T_THEAD) or (g == T_TR))) or is_end(t, T_TABLE) {
        return close_caption(p);
    }
    if (t.kind == TK_END) and
       ((g == T_BODY) or (g == T_COL) or (g == T_COLGROUP) or (g == T_HTML) or (g == T_TBODY) or (g == T_TD) or
        (g == T_TFOOT) or (g == T_TH) or (g == T_THEAD) or (g == T_TR)) {
        return false;
    }
    return in_body(p, t);
}

fn in_column_group(p: *mut Parser, t: *mut Token) !bool {
    let k = t.kind;
    if k == TK_CHARS {
        if not split_ws(p, t, WS_INSERT)! {
            return false;
        }
    } else if k == TK_COMMENT {
        p.insert_comment(t, p.place(NONE))!;
        return false;
    } else if k == TK_DOCTYPE {
        return false;
    } else if is_start(t, T_HTML) {
        return in_body(p, t);
    } else if is_start(t, T_COL) {
        _ = p.insert_void(t)!;
        return false;
    } else if is_end(t, T_COLGROUP) {
        if p.cur_is(T_COLGROUP) {
            p.pop();
            p.mode = M_IN_TABLE;
        }
        return false;
    } else if is_end(t, T_COL) {
        return false;
    } else if is_start(t, T_TEMPLATE) or is_end(t, T_TEMPLATE) {
        return in_head(p, t);
    } else if k == TK_EOF {
        return in_body(p, t);
    }
    if not p.cur_is(T_COLGROUP) {
        return false;
    }
    p.pop();
    p.mode = M_IN_TABLE;
    return true;
}

fn in_table_body(p: *mut Parser, t: *mut Token) !bool {
    let g = t.tag;
    if t.kind == TK_START {
        if g == T_TR {
            p.clear_to(T_TBODY, T_TFOOT, T_THEAD);
            _ = p.insert(t, NS_HTML)!;
            p.mode = M_IN_ROW;
            return false;
        }
        if (g == T_TH) or (g == T_TD) {
            p.clear_to(T_TBODY, T_TFOOT, T_THEAD);
            _ = p.insert_fake(T_TR)!;
            p.mode = M_IN_ROW;
            return true;
        }
        if (g == T_CAPTION) or (g == T_COL) or (g == T_COLGROUP) or (g == T_TBODY) or (g == T_TFOOT) or (g == T_THEAD) {
            return leave_table_body(p);
        }
    } else if t.kind == TK_END {
        if (g == T_TBODY) or (g == T_TFOOT) or (g == T_THEAD) {
            if not p.in_scope(g, SC_TABLE) {
                return false;
            }
            p.clear_to(T_TBODY, T_TFOOT, T_THEAD);
            p.pop();
            p.mode = M_IN_TABLE;
            return false;
        }
        if g == T_TABLE {
            return leave_table_body(p);
        }
        if (g == T_BODY) or (g == T_CAPTION) or (g == T_COL) or (g == T_COLGROUP) or (g == T_HTML) or (g == T_TD) or
           (g == T_TH) or (g == T_TR) {
            return false;
        }
    }
    return in_table(p, t);
}

fn leave_table_body(p: *mut Parser) bool {
    if not (p.in_scope(T_TBODY, SC_TABLE) or p.in_scope(T_THEAD, SC_TABLE) or p.in_scope(T_TFOOT, SC_TABLE)) {
        return false;
    }
    p.clear_to(T_TBODY, T_TFOOT, T_THEAD);
    p.pop();
    p.mode = M_IN_TABLE;
    return true;
}

fn leave_row(p: *mut Parser) bool {
    if not p.in_scope(T_TR, SC_TABLE) {
        return false;
    }
    p.clear_to(T_TR, T_TR, T_TR);
    p.pop();
    p.mode = M_IN_TABLE_BODY;
    return true;
}

fn in_row(p: *mut Parser, t: *mut Token) !bool {
    let g = t.tag;
    if t.kind == TK_START {
        if (g == T_TH) or (g == T_TD) {
            p.clear_to(T_TR, T_TR, T_TR);
            let id = p.insert(t, NS_HTML)!;
            p.mode = M_IN_CELL;
            if p.opened(id) {
                p.afe.push(MARKER)!;
            }
            return false;
        }
        if (g == T_CAPTION) or (g == T_COL) or (g == T_COLGROUP) or (g == T_TBODY) or (g == T_TFOOT) or
           (g == T_THEAD) or (g == T_TR) {
            return leave_row(p);
        }
    } else if t.kind == TK_END {
        if g == T_TR {
            _ = leave_row(p);
            return false;
        }
        if g == T_TABLE {
            return leave_row(p);
        }
        if (g == T_TBODY) or (g == T_TFOOT) or (g == T_THEAD) {
            if not p.in_scope(g, SC_TABLE) {
                return false;
            }
            return leave_row(p);
        }
        if (g == T_BODY) or (g == T_CAPTION) or (g == T_COL) or (g == T_COLGROUP) or (g == T_HTML) or (g == T_TD) or
           (g == T_TH) {
            return false;
        }
    }
    return in_table(p, t);
}

fn close_cell(p: *mut Parser) void {
    p.gen_implied(0);
    while p.stack.len > 1 {
        let c = p.cur();
        p.pop();
        if p.is(c, T_TD) or p.is(c, T_TH) {
            break;
        }
    }
    p.afe_clear_to_marker();
    p.mode = M_IN_ROW;
}

fn in_cell(p: *mut Parser, t: *mut Token) !bool {
    let g = t.tag;
    if t.kind == TK_END {
        if (g == T_TD) or (g == T_TH) {
            if not p.in_scope(g, SC_TABLE) {
                return false;
            }
            p.gen_implied(0);
            p.pop_until(g);
            p.afe_clear_to_marker();
            p.mode = M_IN_ROW;
            return false;
        }
        if (g == T_BODY) or (g == T_CAPTION) or (g == T_COL) or (g == T_COLGROUP) or (g == T_HTML) {
            return false;
        }
        if (g == T_TABLE) or (g == T_TBODY) or (g == T_TFOOT) or (g == T_THEAD) or (g == T_TR) {
            if not p.in_scope(g, SC_TABLE) {
                return false;
            }
            close_cell(p);
            return true;
        }
    } else if t.kind == TK_START {
        if (g == T_CAPTION) or (g == T_COL) or (g == T_COLGROUP) or (g == T_TBODY) or (g == T_TD) or (g == T_TFOOT) or
           (g == T_TH) or (g == T_THEAD) or (g == T_TR) {
            if not (p.in_scope(T_TD, SC_TABLE) or p.in_scope(T_TH, SC_TABLE)) {
                return false;
            }
            close_cell(p);
            return true;
        }
    }
    return in_body(p, t);
}

fn switch_template(p: *mut Parser, mode: u8) !bool {
    _ = p.tmodes.pop() catch 0;
    p.tmodes.push(mode)!;
    p.mode = mode;
    return true;
}

fn in_template(p: *mut Parser, t: *mut Token) !bool {
    let k = t.kind;
    let g = t.tag;
    if (k == TK_CHARS) or (k == TK_COMMENT) or (k == TK_DOCTYPE) {
        return in_body(p, t);
    }
    if k == TK_START {
        if tag_in(t, HEAD_TAGS[..]) {
            return in_head(p, t);
        }
        if (g == T_CAPTION) or (g == T_COLGROUP) or (g == T_TBODY) or (g == T_TFOOT) or (g == T_THEAD) {
            return switch_template(p, M_IN_TABLE);
        }
        if g == T_COL {
            return switch_template(p, M_IN_COLUMN_GROUP);
        }
        if g == T_TR {
            return switch_template(p, M_IN_TABLE_BODY);
        }
        if (g == T_TD) or (g == T_TH) {
            return switch_template(p, M_IN_ROW);
        }
        return switch_template(p, M_IN_BODY);
    }
    if k == TK_END {
        if g == T_TEMPLATE {
            return in_head(p, t);
        }
        return false;
    }
    if k == TK_EOF {
        if not p.has_template() {
            return false;
        }
        p.pop_until(T_TEMPLATE);
        p.afe_clear_to_marker();
        _ = p.tmodes.pop() catch 0;
        p.reset_mode();
        return true;
    }
    return false;
}

fn after_body(p: *mut Parser, t: *mut Token) !bool {
    let k = t.kind;
    if k == TK_CHARS {
        if not split_ws(p, t, M_IN_BODY)! {
            return false;
        }
    } else if k == TK_COMMENT {
        p.insert_comment(t, Loc.{ parent: p.stack.items()[0], before: NONE })!;
        return false;
    } else if k == TK_DOCTYPE {
        return false;
    } else if is_start(t, T_HTML) {
        return in_body(p, t);
    } else if is_end(t, T_HTML) {
        if p.context == NONE {
            p.mode = M_AFTER_AFTER_BODY;
        }
        return false;
    } else if k == TK_EOF {
        return false;
    }
    p.mode = M_IN_BODY;
    return true;
}

// Inserts the whitespace characters of t (the rest are dropped).
fn insert_ws_only(p: *mut Parser, t: *Token) !void {
    p.sbuf.clear();
    for c in t.data {
        if is_ws(c) {
            p.sbuf.push(c)!;
        }
    }
    p.insert_text(p.sbuf.bytes())!;
}

fn in_frameset(p: *mut Parser, t: *mut Token) !bool {
    let k = t.kind;
    if k == TK_CHARS {
        insert_ws_only(p, t)!;
    } else if k == TK_COMMENT {
        p.insert_comment(t, p.place(NONE))!;
    } else if is_start(t, T_HTML) {
        return in_body(p, t);
    } else if is_start(t, T_FRAMESET) {
        _ = p.insert(t, NS_HTML)!;
    } else if is_end(t, T_FRAMESET) {
        if p.stack.len > 1 {
            p.pop();
            if (p.context == NONE) and not p.cur_is(T_FRAMESET) {
                p.mode = M_AFTER_FRAMESET;
            }
        }
    } else if is_start(t, T_FRAME) {
        _ = p.insert_void(t)!;
    } else if is_start(t, T_NOFRAMES) {
        return in_head(p, t);
    }
    return false;
}

fn after_frameset(p: *mut Parser, t: *mut Token) !bool {
    let k = t.kind;
    if k == TK_CHARS {
        insert_ws_only(p, t)!;
    } else if k == TK_COMMENT {
        p.insert_comment(t, p.place(NONE))!;
    } else if is_start(t, T_HTML) {
        return in_body(p, t);
    } else if is_end(t, T_HTML) {
        p.mode = M_AFTER_AFTER_FRAMESET;
    } else if is_start(t, T_NOFRAMES) {
        return in_head(p, t);
    }
    return false;
}

fn after_after_body(p: *mut Parser, t: *mut Token) !bool {
    let k = t.kind;
    if k == TK_COMMENT {
        p.insert_comment(t, Loc.{ parent: 0, before: NONE })!;
        return false;
    }
    if (k == TK_DOCTYPE) or is_start(t, T_HTML) {
        return in_body(p, t);
    }
    if k == TK_CHARS {
        if not split_ws(p, t, M_IN_BODY)! {
            return false;
        }
    }
    if k == TK_EOF {
        return false;
    }
    p.mode = M_IN_BODY;
    return true;
}

fn after_after_frameset(p: *mut Parser, t: *mut Token) !bool {
    let k = t.kind;
    if k == TK_COMMENT {
        p.insert_comment(t, Loc.{ parent: 0, before: NONE })!;
        return false;
    }
    if (k == TK_DOCTYPE) or is_start(t, T_HTML) {
        return in_body(p, t);
    }
    if k == TK_CHARS {
        p.sbuf.clear();
        for c in t.data {
            if is_ws(c) {
                p.sbuf.push(c)!;
            }
        }
        let mut w = t.*;
        w.data = p.sbuf.bytes();
        if w.data.len > 0 {
            return in_body(p, &mut w);
        }
        return false;
    }
    if is_start(t, T_NOFRAMES) {
        return in_head(p, t);
    }
    return false;
}

let BREAKOUT: [44]u16 = [
    T_B,
    T_BIG,
    T_BLOCKQUOTE,
    T_BODY,
    T_BR,
    T_CENTER,
    T_CODE,
    T_DD,
    T_DIV,
    T_DL,
    T_DT,
    T_EM,
    T_EMBED,
    T_H1,
    T_H2,
    T_H3,
    T_H4,
    T_H5,
    T_H6,
    T_HEAD,
    T_HR,
    T_I,
    T_IMG,
    T_LI,
    T_LISTING,
    T_MENU,
    T_META,
    T_NOBR,
    T_OL,
    T_P,
    T_PRE,
    T_RUBY,
    T_S,
    T_SMALL,
    T_SPAN,
    T_STRONG,
    T_STRIKE,
    T_SUB,
    T_SUP,
    T_TABLE,
    T_TT,
    T_U,
    T_UL,
    T_VAR,
];

fn foreign(p: *mut Parser, t: *mut Token) !bool {
    let k = t.kind;
    if k == TK_CHARS {
        p.sbuf.clear();
        let mut text = false;
        for c in t.data {
            if c == 0 {
                p.sbuf.push_str("\xef\xbf\xbd")!;
            } else {
                p.sbuf.push(c)!;
                if not is_ws(c) {
                    text = true;
                }
            }
        }
        p.insert_text(p.sbuf.bytes())!;
        if text {
            p.frameset_ok = false;
        }
        return false;
    }
    if k == TK_COMMENT {
        p.insert_comment(t, p.place(NONE))!;
        return false;
    }
    if k == TK_DOCTYPE {
        return false;
    }
    let mut breakout = (k == TK_START) and tag_in(t, BREAKOUT[..]);
    if is_start(t, T_FONT) {
        for a in t.attrs {
            if strs.eq(a.name, "color") or strs.eq(a.name, "face") or strs.eq(a.name, "size") {
                breakout = true;
            }
        }
    }
    if is_end(t, T_BR) or is_end(t, T_P) {
        breakout = true;
    }
    if breakout {
        while p.stack.len > 1 {
            let c = p.cur();
            if p.mathml_text_ip(c) or p.html_ip(c) or (p.nd(c).ns == NS_HTML) {
                break;
            }
            p.pop();
        }
        return step(p, p.mode, t);
    }
    if k == TK_START {
        let ns = p.nd(p.adjusted_current()).ns;
        let id = p.insert(t, ns)!;
        if t.self_closing and p.opened(id) {
            p.pop();
        }
        return false;
    }
    if k == TK_END {
        let mut i = p.stack.len - 1;
        loop {
            if i == 0 {
                return false;
            }
            let id = p.stack.items()[i];
            if strs.eq_ignore_case(p.nd(id).name, t.name) {
                p.pop_until_node(id);
                return false;
            }
            i -= 1;
            if p.nd(p.stack.items()[i]).ns == NS_HTML {
                return step(p, p.mode, t);
            }
        }
    }
    return false;
}

// -----------------------------------------------------------------------------
// Parsing
// -----------------------------------------------------------------------------

fn new_parser(doc: *mut Document, tz: *mut Tokenizer, opts: Options) Parser {
    return Parser.{ doc: doc, tz: tz, stack: vec.Vec.[u32].init(), open: [0; 124], afe: vec.Vec.[u32].init(),
                    tmodes: vec.Vec.[u8].init(), mode: M_INITIAL, orig: M_INITIAL, head: NONE, form: NONE,
                    context: NONE, frameset_ok: true, foster: false, scripting: opts.scripting, skip_lf: false,
                    text_elem: NONE, pend: strs.String.init(), pend_text: false, sbuf: strs.String.init(),
                    abuf: vec.Vec.[Attr].init() };
}

fn run_parser(p: *mut Parser) !void {
    loop {
        let a = p.adjusted_current();
        p.tz.cdata_ok = (a != NONE) and (p.nd(a).ns != NS_HTML);
        let mut t = p.tz.next()!;
        if p.skip_lf {
            p.skip_lf = false;
            if (t.kind == TK_CHARS) and (t.data[0] == '\n') {
                t.data = t.data[1..];
                if t.data.len == 0 {
                    continue;
                }
            }
        }
        dispatch(p, t)!;
        if t.kind == TK_EOF {
            return;
        }
    }
}

/// Parses an HTML document (UTF-8).
pub fn parse(text: []u8) !Document {
    return parse_with(text, Options.default());
}

pub fn parse_with(text: []u8, opts: Options) !Document {
    let mut doc = Document.init();
    build(&mut doc, text, opts, "", NS_HTML) catch |e| {
        doc.deinit();
        return e;
    };
    return doc;
}

/// Parses a fragment as the children of an element named context (for
/// example "div" or "tr"; "svg path" and "math mi" name SVG and MathML
/// elements), as innerHTML does. Node 0 of the result is a FRAGMENT that
/// holds the parsed nodes.
pub fn parse_fragment(text: []u8, context: []u8, opts: Options) !Document {
    let mut doc = Document.init();
    let mut ns = NS_HTML;
    let mut name = context;
    if strs.starts_with(context, "svg ") {
        ns = NS_SVG;
        name = context[4..];
    } else if strs.starts_with(context, "math ") {
        ns = NS_MATHML;
        name = context[5..];
    }
    build(&mut doc, text, opts, name, ns) catch |e| {
        doc.deinit();
        return e;
    };
    return doc;
}

/// Parses bytes in an unknown encoding: a byte-order mark (UTF-8, UTF-16LE
/// or UTF-16BE) decides, else a <meta charset> (or http-equiv
/// Content-Type) in the first 1024 bytes naming windows-1252 or one of its
/// aliases (ISO-8859-1, ASCII, ...), else UTF-8 if the bytes are valid
/// UTF-8, else windows-1252.
pub fn parse_bytes(bytes: []u8, opts: Options) !Document {
    let mut s = strs.String.init();
    defer s.deinit();
    let text = decode(bytes, &mut s)!;
    return parse_with(text, opts);
}

/// Decodes bytes as parse_bytes does, into out if they are not already
/// UTF-8: returns the UTF-8 text.
pub fn decode(b: []u8, out: *mut strs.String) ![]u8 {
    if (b.len >= 2) and (((b[0] == 0xff) and (b[1] == 0xfe)) or ((b[0] == 0xfe) and (b[1] == 0xff))) {
        let le = b[0] == 0xff;
        let mut i: usize = 2;
        while (i + 1) < b.len {
            let mut u = (@as(u32, b[i + 1]) << 8) | @as(u32, b[i]);
            if not le {
                u = (@as(u32, b[i]) << 8) | @as(u32, b[i + 1]);
            }
            i += 2;
            if (u >= 0xd800) and (u < 0xdc00) and ((i + 1) < b.len) {
                let mut v = (@as(u32, b[i + 1]) << 8) | @as(u32, b[i]);
                if not le {
                    v = (@as(u32, b[i]) << 8) | @as(u32, b[i + 1]);
                }
                if (v >= 0xdc00) and (v < 0xe000) {
                    u = 0x10000 + ((u - 0xd800) << 10) + (v - 0xdc00);
                    i += 2;
                }
            }
            out.push_rune(u)!;  // (lone surrogates become U+FFFD)
        }
        if i < b.len {
            out.push_rune(0xfffd)!;
        }
        return out.bytes();
    }
    if (b.len >= 3) and (b[0] == 0xef) and (b[1] == 0xbb) and (b[2] == 0xbf) {
        return b;
    }
    let cs = prescan_charset(b[..@min(b.len, 1024)]);
    let mut latin = is_windows_1252(cs);
    if (cs.len == 0) and not strs.valid_utf8(b) {
        latin = true;
    }
    if not latin {
        return b;
    }
    out.reserve(b.len + (b.len / 4))!;
    for c in b {
        if c < 0x80 {
            out.push(c)!;
        } else if (c < 0xa0) and (C1[@as(usize, c - 0x80)] != 0) {
            out.push_rune(C1[@as(usize, c - 0x80)])!;
        } else {
            out.push_rune(@as(u32, c))!;
        }
    }
    return out.bytes();
}

fn is_windows_1252(cs: []u8) bool {
    for x in ["windows-1252", "cp1252", "x-cp1252", "iso-8859-1", "iso8859-1", "iso_8859-1", "latin1", "l1", "us-ascii",
              "ascii", "iso-ir-100", "csisolatin1", "cp819", "ibm819", "ansi_x3.4-1968"]{
        if strs.eq_ignore_case(cs, x) {
            return true;
        }
    }
    return false;
}

// The charset a <meta> near the start declares, or "".
fn prescan_charset(b: []u8) []u8 {
    let mut i: usize = 0;
    while i < b.len {
        let lt = strs.index_of_from(b, "<", i) catch return "";
        i = lt + 1;
        if (i + 4) > b.len {
            return "";
        }
        if strs.starts_with(b[i..], "!--") {
            i = (strs.index_of_from(b, "-->", i) catch return "") + 3;
            continue;
        }
        if not (strs.eq_ignore_case(b[i..i + 4], "meta") and ((i + 4) < b.len) and
                (is_ws(b[i + 4]) or (b[i + 4] == '/'))) {
            continue;
        }
        let end = strs.index_of_from(b, ">", i) catch b.len;
        let tag = b[i + 4..end];
        let cs = charset_in(tag);
        if cs.len > 0 {
            return cs;
        }
        i = end;
    }
    return "";
}

// charset=... in a meta tag's text (as an attribute or inside content=).
fn charset_in(tag: []u8) []u8 {
    let mut i: usize = 0;
    while (i + 8) <= tag.len {
        if strs.eq_ignore_case(tag[i..i + 7], "charset") {
            let mut j = i + 7;
            while (j < tag.len) and is_ws(tag[j]) {
                j += 1;
            }
            if (j < tag.len) and (tag[j] == '=') {
                j += 1;
                while (j < tag.len) and (is_ws(tag[j]) or (tag[j] == '"') or (tag[j] == '\'')) {
                    j += 1;
                }
                let st = j;
                while (j < tag.len) and not is_ws(tag[j]) and (tag[j] != '"') and (tag[j] != '\'') and
                      (tag[j] != ';') and (tag[j] != '/') {
                    j += 1;
                }
                return tag[st..j];
            }
        }
        i += 1;
    }
    return "";
}

fn build(doc: *mut Document, text0: []u8, opts: Options, context: []u8, cns: u8) !void {
    _ = doc.new_node(DOCUMENT)!;
    let mut text = text0;
    if (text.len >= 3) and (text[0] == 0xef) and (text[1] == 0xbb) and (text[2] == 0xbf) {
        text = text[3..];  // a byte-order mark
    }
    let mut tz = Tokenizer.init(text)!;
    defer tz.deinit();
    let mut p = new_parser(doc, &mut tz, opts);
    defer p.deinit();
    let mut root = NONE;
    if context.len > 0 {
        let lname = strs.to_lower(context)!;
        defer mem.free(lname);
        let mut ct = fake_token(T_HTML);
        ct.name = lname;
        ct.tag = tag_of(lname);
        p.context = p.create_element(&mut ct, cns)!;
        if cns == NS_HTML {
            let g = ct.tag;
            if (g == T_TITLE) or (g == T_TEXTAREA) {
                tz.state = S_RCDATA;
            } else if (g == T_STYLE) or (g == T_XMP) or (g == T_IFRAME) or (g == T_NOEMBED) or (g == T_NOFRAMES) or
                      ((g == T_NOSCRIPT) and opts.scripting) {
                tz.state = S_RAWTEXT;
            } else if g == T_SCRIPT {
                tz.state = S_SCRIPT;
            } else if g == T_PLAINTEXT {
                tz.state = S_PLAINTEXT;
            }
        }
        let mut h = fake_token(T_HTML);
        root = p.create_element(&mut h, NS_HTML)!;
        doc.append_child(0, root);
        _ = p.push(root)!;
        if p.is(p.context, T_TEMPLATE) {
            p.tmodes.push(M_IN_TEMPLATE)!;
        }
        p.reset_mode();
        if p.is(p.context, T_FORM) {
            p.form = p.context;
        }
    }
    run_parser(&mut p)!;
    if root != NONE {
        // the fragment: the root element's children, under node 0
        doc.remove(root);
        doc.nm(0).kind = FRAGMENT;
        loop {
            let c = doc.node(root).first;
            if c == NONE {
                break;
            }
            doc.remove(c);
            doc.append_child(0, c);
        }
    }
    doc.finish_text();
    for id in p.stack.items() {
        doc.nm(id).flags &= ~FL_OPEN;
    }
    for id in p.afe.items() {
        if id != MARKER {
            doc.nm(id).flags &= ~FL_AFE;
        }
    }
    fill_selectedcontent(doc)!;
}

// A <select>'s <selectedcontent> shows a copy of the selected <option>'s
// content (the last option marked selected, else the first).
fn fill_selectedcontent(doc: *mut Document) !void {
    let n = @truncate(u32, doc.nodes.len);
    for sel in @as(u32, 0)..n {
        let s = doc.node(sel);
        if (s.kind != ELEMENT) or (s.ns != NS_HTML) or (s.tag != T_SELECT) or has_attr_named(s, "multiple") {
            continue;
        }
        let mut sc = NONE;
        let mut first = NONE;
        let mut chosen = NONE;
        let mut w = doc.walk(sel);
        _ = w.next();
        while w.next() {
            if not w.entering {
                continue;
            }
            let e = doc.node(w.id);
            if (e.kind != ELEMENT) or (e.ns != NS_HTML) {
                continue;
            }
            if (e.tag == T_SELECTEDCONTENT) and (sc == NONE) {
                sc = w.id;
            } else if e.tag == T_OPTION {
                if first == NONE {
                    first = w.id;
                }
                if has_attr_named(e, "selected") {
                    chosen = w.id;
                }
            }
        }
        if chosen == NONE {
            chosen = first;
        }
        if (sc == NONE) or (chosen == NONE) or (doc.node(sc).first != NONE) {
            continue;
        }
        let mut c = doc.node(chosen).first;
        while c != NONE {
            let k = doc.clone_tree(c)!;
            doc.append_child(sc, k);
            c = doc.node(c).next;
        }
    }
}

fn has_attr_named(n: *Node, name: []u8) bool {
    for a in n.attrs {
        if (a.ns == A_NONE) and strs.eq(a.name, name) {
            return true;
        }
    }
    return false;
}

// -----------------------------------------------------------------------------
// Walking, reading and changing the tree
// -----------------------------------------------------------------------------

/// A depth-first walk over a subtree: each node is visited entering (before
/// its children) and leaving (after them). Template contents are walked as
/// the template's children.
pub struct Walker {
    doc: *Document,
    root: u32,
    id: u32,
    entering: bool,
    started: bool,
    templates: bool,

    /// Moves on; false when the walk is over.
    pub fn next(self: *mut Walker) bool {
        if not self.started {
            self.started = true;
            self.id = self.root;
            self.entering = true;
            return true;
        }
        if self.entering {
            let f = self.doc.first_kid(self.id, self.templates);
            if f != NONE {
                self.id = f;
                return true;
            }
            self.entering = false;
            return true;
        }
        if self.id == self.root {
            return false;
        }
        let nx = self.doc.node(self.id).next;
        if nx != NONE {
            self.id = nx;
            self.entering = true;
            return true;
        }
        self.id = self.doc.up(self.id);
        return true;
    }

    /// Skips the children of the node just entered.
    pub fn skip(self: *mut Walker) void {
        self.entering = false;
    }
}

/// The children of a node, in order.
pub struct Children {
    doc: *Document,
    id: u32,

    pub fn next(self: *mut Children) !u32 {
        if self.id == NONE {
            return error.NotFound;
        }
        let r = self.id;
        self.id = self.doc.node(r).next;
        return r;
    }
}

fn take_vec(v: *mut vec.Vec.[u32]) ![]mut u32 {
    let out = mem.alloc.[u32](v.len)!;
    @mem_copy(out, v.items());
    v.deinit();
    return out;
}

fn attr_is(a: Attr, name: []u8) bool {
    if a.prefix.len == 0 {
        return strs.eq(a.name, name);
    }
    return (name.len == a.prefix.len + 1 + a.name.len) and strs.starts_with(name, a.prefix) and
           (name[a.prefix.len] == ':') and strs.ends_with(name, a.name);
}

// Whether an element has a local name (ASCII case-insensitively for HTML).
fn name_is(n: *Node, name: []u8) bool {
    if n.ns == NS_HTML {
        return strs.eq_ignore_case(n.name, name);
    }
    return strs.eq(n.name, name);
}

/// Writes text escaped for HTML: & < > and no-break space, and " too in
/// attribute values.
pub fn escape(out: io.Writer, s: []u8, attr: bool) !void {
    let mut st: usize = 0;
    let mut i: usize = 0;
    while i < s.len {
        let c = s[i];
        let mut rep: []u8 = "";
        let mut len: usize = 1;
        if c == '&' {
            rep = "&amp;";
        } else if c == '<' {
            rep = "&lt;";
        } else if c == '>' {
            rep = "&gt;";
        } else if (c == '"') and attr {
            rep = "&quot;";
        } else if (c == 0xc2) and ((i + 1) < s.len) and (s[i + 1] == 0xa0) {
            rep = "&nbsp;";
            len = 2;
        }
        if rep.len > 0 {
            out.write_all(s[st..i])!;
            out.write_all(rep)!;
            i += len;
            st = i;
        } else {
            i += 1;
        }
    }
    out.write_all(s[st..])!;
}

// -----------------------------------------------------------------------------
// CSS selectors
// -----------------------------------------------------------------------------
//
// Supported: type and universal selectors, #id, .class, attribute selectors
// ([a] [a=v] [a~=v] [a|=v] [a^=v] [a$=v] [a*=v], with an i flag), the
// combinators (descendant, >, +, ~), selector lists, and the pseudo-classes
// :root :empty :first-child :last-child :only-child :first-of-type
// :last-of-type :only-of-type :nth-child() :nth-last-child() :nth-of-type()
// :nth-last-of-type() :not() :is() :where() :link :checked :disabled
// :enabled.

// Simple selector kinds
const SS_TYPE: u8 = 0;
const SS_UNIVERSAL: u8 = 1;
const SS_ID: u8 = 2;
const SS_CLASS: u8 = 3;
const SS_ATTR: u8 = 4;
const SS_ROOT: u8 = 5;
const SS_EMPTY: u8 = 6;
const SS_NTH: u8 = 7;  // op: bit 0 from the end, bit 1 of type
const SS_NOT: u8 = 8;  // sub: a list
const SS_IS: u8 = 9;
const SS_LINK: u8 = 10;
const SS_CHECKED: u8 = 11;
const SS_DISABLED: u8 = 12;
const SS_ENABLED: u8 = 13;

// Attribute operators
const AO_EXISTS: u8 = 0;
const AO_EQ: u8 = 1;
const AO_WORD: u8 = 2;    // ~=
const AO_DASH: u8 = 3;    // |=
const AO_PREFIX: u8 = 4;  // ^=
const AO_SUFFIX: u8 = 5;  // $=
const AO_SUB: u8 = 6;     // *=

// Combinators (to the compound on the left)
const C_NONE: u8 = 0;
const C_DESCENDANT: u8 = 1;
const C_CHILD: u8 = 2;
const C_NEXT: u8 = 3;
const C_LATER: u8 = 4;

struct Simple {
    kind: u8,
    op: u8,
    ci: bool,
    name: []u8,
    value: []u8,
    a: i64,
    b: i64,
    sub: u32,
}

struct Compound {
    first: u32,  // simples[first..first + count]
    count: u32,
    comb: u8,  // how it relates to the compound before it
}

struct Complex {
    first: u32,  // compounds[first..first + count], left to right
    count: u32,
}

struct Range {
    first: u32,
    count: u32,
}

// Match results (as in Servo, which avoids exponential backtracking)
const R_MATCHED: u8 = 0;
const R_LATER_SIBLING: u8 = 1;  // not matched: restart from the closest later-sibling combinator
const R_DESCENDANT: u8 = 2;     // ... from the closest descendant combinator
const R_GLOBALLY: u8 = 3;       // not matched, and no other placement can match

/// A compiled selector list.
pub struct Selector {
    simples: vec.Vec.[Simple],
    compounds: vec.Vec.[Compound],
    complexes: vec.Vec.[Complex],
    lists: vec.Vec.[Range],  // ranges of complexes; the last is the whole selector
    top: u32,
    arena: arena.Arena,

    pub fn deinit(self: *mut Selector) void {
        self.simples.deinit();
        self.compounds.deinit();
        self.complexes.deinit();
        self.lists.deinit();
        self.arena.deinit();
    }

    /// Whether an element matches.
    pub fn matches(self: *Selector, doc: *Document, id: u32) bool {
        if doc.node(id).kind != ELEMENT {
            return false;
        }
        return self.list_matches(doc, self.top, id, 0);
    }

    fn list_matches(self: *Selector, doc: *Document, list: u32, id: u32, depth: u32) bool {
        let r = self.lists.get(@as(usize, list));
        for k in r.first..r.first + r.count {
            let c = self.complexes.get(@as(usize, k));
            if self.match_from(doc, c, c.count - 1, id, depth) == R_MATCHED {
                return true;
            }
        }
        return false;
    }

    fn match_from(self: *Selector, doc: *Document, cx: Complex, i: u32, id: u32, depth: u32) u8 {
        let comp = self.compounds.get(@as(usize, cx.first + i));
        if not self.compound_matches(doc, comp, id, depth) {
            return R_LATER_SIBLING;
        }
        if i == 0 {
            return R_MATCHED;
        }
        let comb = comp.comb;
        let mut not_found = R_GLOBALLY;
        if (comb == C_NEXT) or (comb == C_LATER) {
            not_found = R_DESCENDANT;
        }
        let mut next = id;
        loop {
            if (comb == C_CHILD) or (comb == C_DESCENDANT) {
                next = doc.parent_element(next);
            } else {
                next = prev_element(doc, next);
            }
            if next == NONE {
                return not_found;
            }
            let r = self.match_from(doc, cx, i - 1, next, depth);
            if (r == R_MATCHED) or (r == R_GLOBALLY) or (comb == C_NEXT) {
                return r;
            }
            if comb == C_CHILD {
                return R_DESCENDANT;
            }
            if (r == R_DESCENDANT) and (comb == C_LATER) {
                return r;
            }
        }
    }

    fn compound_matches(self: *Selector, doc: *Document, comp: Compound, id: u32, depth: u32) bool {
        for k in comp.first..comp.first + comp.count {
            if not self.simple_matches(doc, self.simples.get(@as(usize, k)), id, depth) {
                return false;
            }
        }
        return true;
    }

    fn simple_matches(self: *Selector, doc: *Document, s: Simple, id: u32, depth: u32) bool {
        let n = doc.node(id);
        let k = s.kind;
        if k == SS_TYPE {
            return name_is(n, s.name);
        }
        if k == SS_UNIVERSAL {
            return true;
        }
        if k == SS_ID {
            let v = doc.attr(id, "id") catch return false;
            return strs.eq(v, s.name);
        }
        if k == SS_CLASS {
            let v = doc.attr(id, "class") catch return false;
            return has_word(v, s.name, false);
        }
        if k == SS_ATTR {
            for a in n.attrs {
                let mut hit = false;
                if n.ns == NS_HTML {
                    hit = (a.prefix.len == 0) and strs.eq_ignore_case(a.name, s.name);
                } else {
                    hit = attr_is(a, s.name);
                }
                if hit {
                    return attr_value_matches(a.value, s);
                }
            }
            return false;
        }
        if k == SS_ROOT {
            return (n.parent != NONE) and (doc.node(n.parent).kind == DOCUMENT);
        }
        if k == SS_EMPTY {
            let mut c = n.first;
            while c != NONE {
                let ck = doc.node(c).kind;
                if (ck == ELEMENT) or ((ck == TEXT) and (doc.node(c).text.len > 0)) {
                    return false;
                }
                c = doc.node(c).next;
            }
            return true;
        }
        if k == SS_NTH {
            let from_end = (s.op & 1) != 0;
            let of_type = (s.op & 2) != 0;
            let mut pos: i64 = 1;
            let mut c = id;
            loop {
                if from_end {
                    c = next_element(doc, c);
                } else {
                    c = prev_element(doc, c);
                }
                if c == NONE {
                    break;
                }
                if not of_type or ((doc.node(c).ns == n.ns) and strs.eq(doc.node(c).name, n.name)) {
                    pos += 1;
                }
            }
            if s.a == 0 {
                return pos == s.b;
            }
            let d = pos - s.b;
            return ((d % s.a) == 0) and ((d / s.a) >= 0);
        }
        if (k == SS_NOT) or (k == SS_IS) {
            if depth > 32 {
                return false;
            }
            let m = self.list_matches(doc, s.sub, id, depth + 1);
            return m == (k == SS_IS);
        }
        if (n.ns != NS_HTML) {
            return false;
        }
        if k == SS_LINK {
            return ((n.tag == T_A) or (n.tag == T_AREA)) and doc.has_attr(id, "href");
        }
        if k == SS_CHECKED {
            return (((n.tag == T_INPUT) and doc.has_attr(id, "checked")) or
                    ((n.tag == T_OPTION) and doc.has_attr(id, "selected")));
        }
        let form = (n.tag == T_BUTTON) or (n.tag == T_INPUT) or (n.tag == T_SELECT) or (n.tag == T_TEXTAREA) or
                   (n.tag == T_OPTGROUP) or (n.tag == T_OPTION) or (n.tag == T_FIELDSET);
        if k == SS_DISABLED {
            return form and doc.has_attr(id, "disabled");
        }
        return form and not doc.has_attr(id, "disabled");
    }
}

fn prev_element(doc: *Document, id: u32) u32 {
    let mut c = doc.node(id).prev;
    while (c != NONE) and (doc.node(c).kind != ELEMENT) {
        c = doc.node(c).prev;
    }
    return c;
}

fn next_element(doc: *Document, id: u32) u32 {
    let mut c = doc.node(id).next;
    while (c != NONE) and (doc.node(c).kind != ELEMENT) {
        c = doc.node(c).next;
    }
    return c;
}

fn has_word(list: []u8, word: []u8, ci: bool) bool {
    if word.len == 0 {
        return false;
    }
    let mut it = strs.fields(list);
    while true {
        let w = it.next() catch break;
        if strs.eq(w, word) or (ci and strs.eq_ignore_case(w, word)) {
            return true;
        }
    }
    return false;
}

fn attr_value_matches(v: []u8, s: Simple) bool {
    let want = s.value;
    let op = s.op;
    if op == AO_EXISTS {
        return true;
    }
    if op == AO_WORD {
        return has_word(v, want, s.ci) and not strs.contains(want, " ");
    }
    if (op != AO_EQ) and (want.len == 0) {
        return false;
    }
    let mut part = v;
    if op == AO_PREFIX {
        if v.len < want.len {
            return false;
        }
        part = v[..want.len];
    } else if op == AO_SUFFIX {
        if v.len < want.len {
            return false;
        }
        part = v[v.len - want.len..];
    } else if op == AO_SUB {
        if s.ci {
            let mut i: usize = 0;
            while (i + want.len) <= v.len {
                if strs.eq_ignore_case(v[i..i + want.len], want) {
                    return true;
                }
                i += 1;
            }
            return false;
        }
        return strs.contains(v, want);
    } else if op == AO_DASH {
        if (v.len > want.len) and (v[want.len] == '-') {
            part = v[..want.len];
        }
    }
    if s.ci {
        return strs.eq_ignore_case(part, want);
    }
    return strs.eq(part, want);
}

/// Compiles a selector list (for matching many elements).
pub fn compile(text: []u8) !Selector {
    let mut sel = Selector.{ simples: vec.Vec.[Simple].init(), compounds: vec.Vec.[Compound].init(),
                             complexes: vec.Vec.[Complex].init(), lists: vec.Vec.[Range].init(), top: 0,
                             arena: arena.Arena.init() };
    let mut ps = SelParser.{ s: text, i: 0, sel: &mut sel };
    let top = ps.list(0) catch |e| {
        sel.deinit();
        return e;
    };
    ps.ws();
    if ps.i < text.len {
        sel.deinit();
        return error.SyntaxError;
    }
    sel.top = top;
    return sel;
}

struct SelParser {
    s: []u8,
    i: usize,
    sel: *mut Selector,

    fn peek(self: *SelParser) u8 {
        if self.i < self.s.len {
            return self.s[self.i];
        }
        return 0;
    }

    fn ws(self: *mut SelParser) void {
        while (self.i < self.s.len) and is_ws(self.s[self.i]) {
            self.i += 1;
        }
    }

    // A selector list, up to ")" or the end: the index of its Range.
    fn list(self: *mut SelParser, depth: u32) !u32 {
        if depth > 32 {
            return error.SyntaxError;
        }
        let mut items = vec.Vec.[Complex].init();
        defer items.deinit();
        loop {
            self.ws();
            items.push(self.complex(depth)!)!;
            self.ws();
            if self.peek() != ',' {
                break;
            }
            self.i += 1;
        }
        let first = @truncate(u32, self.sel.complexes.len);
        self.sel.complexes.extend(items.items())!;
        self.sel.lists.push(Range.{ first: first, count: @truncate(u32, items.len) })!;
        return @truncate(u32, self.sel.lists.len - 1);
    }

    fn complex(self: *mut SelParser, depth: u32) !Complex {
        let mut comps = vec.Vec.[Compound].init();
        defer comps.deinit();
        let mut comb = C_NONE;
        loop {
            let mut c = self.compound(depth)!;
            c.comb = comb;
            comps.push(c)!;
            let had_ws = (self.i < self.s.len) and is_ws(self.s[self.i]);
            self.ws();
            let ch = self.peek();
            if ch == '>' {
                comb = C_CHILD;
            } else if ch == '+' {
                comb = C_NEXT;
            } else if ch == '~' {
                comb = C_LATER;
            } else if had_ws and (ch != 0) and (ch != ',') and (ch != ')') {
                comb = C_DESCENDANT;
                continue;
            } else {
                break;
            }
            self.i += 1;
            self.ws();
        }
        let first = @truncate(u32, self.sel.compounds.len);
        self.sel.compounds.extend(comps.items())!;
        return Complex.{ first: first, count: @truncate(u32, comps.len) };
    }

    fn ident(self: *mut SelParser) ![]u8 {
        let st = self.i;
        while self.i < self.s.len {
            let c = self.s[self.i];
            if is_alnum(@as(i32, c)) or (c == '-') or (c == '_') or (c >= 0x80) {
                self.i += 1;
            } else if (c == '\\') and ((self.i + 1) < self.s.len) {
                self.i += 2;
            } else {
                break;
            }
        }
        if self.i == st {
            return error.SyntaxError;
        }
        return unescape_css(self.s[st..self.i], &mut self.sel.arena);
    }

    fn compound(self: *mut SelParser, depth: u32) !Compound {
        // (collected apart: :not() and :is() add their own simples as they go)
        let mut out = vec.Vec.[Simple].init();
        defer out.deinit();
        let c = self.peek();
        if c == '*' {
            self.i += 1;
            add(&mut out, SS_UNIVERSAL, "")!;
        } else if is_alnum(@as(i32, c)) or (c == '-') or (c == '_') or (c == '\\') or (c >= 0x80) {
            let nm = self.ident()!;
            add(&mut out, SS_TYPE, nm)!;
        }
        loop {
            let ch = self.peek();
            if ch == '#' {
                self.i += 1;
                let nm = self.ident()!;
                add(&mut out, SS_ID, nm)!;
            } else if ch == '.' {
                self.i += 1;
                let nm = self.ident()!;
                add(&mut out, SS_CLASS, nm)!;
            } else if ch == '[' {
                self.i += 1;
                self.attribute(&mut out)!;
            } else if ch == ':' {
                self.i += 1;
                if self.peek() == ':' {
                    return error.SyntaxError;  // (pseudo-elements never match)
                }
                self.pseudo(&mut out, depth)!;
            } else {
                break;
            }
        }
        if out.len == 0 {
            return error.SyntaxError;
        }
        let first = @truncate(u32, self.sel.simples.len);
        self.sel.simples.extend(out.items())!;
        return Compound.{ first: first, count: @truncate(u32, out.len), comb: C_NONE };
    }

    fn attribute(self: *mut SelParser, out: *mut vec.Vec.[Simple]) !void {
        self.ws();
        let nm = self.ident()!;
        self.ws();
        let mut s = Simple.{ kind: SS_ATTR, op: AO_EXISTS, ci: false, name: nm, value: "", a: 0, b: 0, sub: 0 };
        let c = self.peek();
        if c != ']' {
            if c == '=' {
                s.op = AO_EQ;
                self.i += 1;
            } else {
                if (self.i + 1) >= self.s.len or (self.s[self.i + 1] != '=') {
                    return error.SyntaxError;
                }
                if c == '~' {
                    s.op = AO_WORD;
                } else if c == '|' {
                    s.op = AO_DASH;
                } else if c == '^' {
                    s.op = AO_PREFIX;
                } else if c == '$' {
                    s.op = AO_SUFFIX;
                } else if c == '*' {
                    s.op = AO_SUB;
                } else {
                    return error.SyntaxError;
                }
                self.i += 2;
            }
            self.ws();
            let q = self.peek();
            if (q == '"') or (q == '\'') {
                self.i += 1;
                let st = self.i;
                while (self.i < self.s.len) and (self.s[self.i] != q) {
                    if self.s[self.i] == '\\' {
                        self.i += 1;
                    }
                    self.i += 1;
                }
                if self.i >= self.s.len {
                    return error.SyntaxError;
                }
                s.value = unescape_css(self.s[st..self.i], &mut self.sel.arena)!;
                self.i += 1;
            } else {
                s.value = self.ident()!;
            }
            self.ws();
            if (self.peek() == 'i') or (self.peek() == 'I') {
                s.ci = true;
                self.i += 1;
                self.ws();
            } else if (self.peek() == 's') or (self.peek() == 'S') {
                self.i += 1;
                self.ws();
            }
        }
        if self.peek() != ']' {
            return error.SyntaxError;
        }
        self.i += 1;
        out.push(s)!;
    }

    fn pseudo(self: *mut SelParser, out: *mut vec.Vec.[Simple], depth: u32) !void {
        let nm = self.ident()!;
        if strs.eq_ignore_case(nm, "root") {
            return add(out, SS_ROOT, "");
        }
        if strs.eq_ignore_case(nm, "empty") {
            return add(out, SS_EMPTY, "");
        }
        if strs.eq_ignore_case(nm, "link") or strs.eq_ignore_case(nm, "any-link") {
            return add(out, SS_LINK, "");
        }
        if strs.eq_ignore_case(nm, "checked") {
            return add(out, SS_CHECKED, "");
        }
        if strs.eq_ignore_case(nm, "disabled") {
            return add(out, SS_DISABLED, "");
        }
        if strs.eq_ignore_case(nm, "enabled") {
            return add(out, SS_ENABLED, "");
        }
        // positions: (from the end, of type) and an+b
        let mut op: u8 = 0;
        let mut a: i64 = 0;
        let mut b: i64 = 1;
        let mut both = false;  // only-*: first and last
        if strs.eq_ignore_case(nm, "first-child") {
            op = 0;
        } else if strs.eq_ignore_case(nm, "last-child") {
            op = 1;
        } else if strs.eq_ignore_case(nm, "only-child") {
            both = true;
        } else if strs.eq_ignore_case(nm, "first-of-type") {
            op = 2;
        } else if strs.eq_ignore_case(nm, "last-of-type") {
            op = 3;
        } else if strs.eq_ignore_case(nm, "only-of-type") {
            op = 2;
            both = true;
        } else {
            if self.peek() != '(' {
                return error.SyntaxError;
            }
            self.i += 1;
            self.ws();
            if strs.eq_ignore_case(nm, "not") or strs.eq_ignore_case(nm, "is") or strs.eq_ignore_case(nm, "where") or
               strs.eq_ignore_case(nm, "matches") {
                let sub = self.list(depth + 1)!;
                self.ws();
                if self.peek() != ')' {
                    return error.SyntaxError;
                }
                self.i += 1;
                let mut kind = SS_IS;
                if strs.eq_ignore_case(nm, "not") {
                    kind = SS_NOT;
                }
                out.push(Simple.{ kind: kind, op: 0, ci: false, name: "", value: "", a: 0, b: 0, sub: sub })!;
                return;
            }
            if strs.eq_ignore_case(nm, "nth-child") {
                op = 0;
            } else if strs.eq_ignore_case(nm, "nth-last-child") {
                op = 1;
            } else if strs.eq_ignore_case(nm, "nth-of-type") {
                op = 2;
            } else if strs.eq_ignore_case(nm, "nth-last-of-type") {
                op = 3;
            } else {
                return error.SyntaxError;
            }
            let st = self.i;
            while (self.i < self.s.len) and (self.s[self.i] != ')') {
                self.i += 1;
            }
            if self.i >= self.s.len {
                return error.SyntaxError;
            }
            parse_nth(strs.trim(self.s[st..self.i]), &mut a, &mut b)!;
            self.i += 1;
        }
        out.push(Simple.{ kind: SS_NTH, op: op, ci: false, name: "", value: "", a: a, b: b, sub: 0 })!;
        if both {
            out.push(Simple.{ kind: SS_NTH, op: op | 1, ci: false, name: "", value: "", a: 0, b: 1, sub: 0 })!;
        }
    }
}

fn add(out: *mut vec.Vec.[Simple], kind: u8, name: []u8) !void {
    out.push(Simple.{ kind: kind, op: 0, ci: false, name: name, value: "", a: 0, b: 0, sub: 0 })!;
}

// an+b (or odd, even, or an integer).
fn parse_nth(s: []u8, a: *mut i64, b: *mut i64) !void {
    if strs.eq_ignore_case(s, "odd") {
        a.* = 2;
        b.* = 1;
        return;
    }
    if strs.eq_ignore_case(s, "even") {
        a.* = 2;
        b.* = 0;
        return;
    }
    let mut compact = strs.String.init();
    defer compact.deinit();
    for c in s {
        if not is_ws(c) {
            compact.push(strs.to_lower_byte(c))!;
        }
    }
    let t = compact.bytes();
    let n_at = strs.index_of_byte(t, 'n') catch NONE_USIZE;
    if n_at == NONE_USIZE {
        a.* = 0;
        b.* = fmt.parse_int.[i64](t) catch return error.SyntaxError;
        return;
    }
    let head = t[..n_at];
    if (head.len == 0) or strs.eq(head, "+") {
        a.* = 1;
    } else if strs.eq(head, "-") {
        a.* = -1;
    } else {
        a.* = fmt.parse_int.[i64](head) catch return error.SyntaxError;
    }
    let tail = t[n_at + 1..];
    if tail.len == 0 {
        b.* = 0;
        return;
    }
    if (tail[0] != '+') and (tail[0] != '-') {
        return error.SyntaxError;
    }
    let v = fmt.parse_int.[i64](tail[1..]) catch return error.SyntaxError;
    if tail[0] == '-' {
        b.* = -v;
    } else {
        b.* = v;
    }
}

// Removes CSS backslash escapes (\x and hex escapes).
fn unescape_css(s: []u8, a: *mut arena.Arena) ![]u8 {
    if (strs.index_of_byte(s, '\\') catch NONE_USIZE) == NONE_USIZE {
        return a.dup(s);
    }
    let mut out = strs.String.init();
    defer out.deinit();
    let mut i: usize = 0;
    while i < s.len {
        if (s[i] == '\\') and ((i + 1) < s.len) {
            i += 1;
            let mut v: u32 = 0;
            let mut k: usize = 0;
            while (k < 6) and ((i + k) < s.len) and strs.is_hex_digit(s[i + k]) {
                v = (v * 16) + @as(u32, hexv(s[i + k]));
                k += 1;
            }
            if k > 0 {
                out.push_rune(v)!;
                i += k;
                if (i < s.len) and is_ws(s[i]) {
                    i += 1;
                }
                continue;
            }
        }
        out.push(s[i])!;
        i += 1;
    }
    return a.dup(out.bytes());
}

fn hexv(c: u8) u8 {
    if c <= '9' {
        return c - '0';
    }
    if c >= 'a' {
        return c - 'a' + 10;
    }
    return c - 'A' + 10;
}

// -----------------------------------------------------------------------------
// Tests
// -----------------------------------------------------------------------------

// The html5lib-tests tree format.
fn dump(doc: *Document, from: u32, out: *mut strs.String) !void {
    let mut work = vec.Vec.[u32].init();  // pairs: node, depth
    defer work.deinit();
    push_children(doc, &mut work, doc.node(from).first, 0)!;
    while work.len > 0 {
        let depth = work.pop()!;
        let id = work.pop()!;
        let n = doc.node(id);
        out.push_str("| ")!;
        for _k in @as(u32, 0)..depth {
            out.push_str("  ")!;
        }
        if n.kind == FRAGMENT {
            out.push_str("content\n")!;
            push_children(doc, &mut work, n.first, depth + 1)!;
            continue;
        }
        if n.kind == ELEMENT {
            out.push('<')!;
            if n.ns == NS_SVG {
                out.push_str("svg ")!;
            } else if n.ns == NS_MATHML {
                out.push_str("math ")!;
            }
            out.push_str(n.name)!;
            out.push_str(">\n")!;
            // attributes, sorted by their displayed names
            let mut attrs = vec.Vec.[Attr].init();
            defer attrs.deinit();
            attrs.extend(n.attrs)!;
            attrs.sort(display_less);
            for a in attrs.items() {
                out.push_str("| ")!;
                for _k in @as(u32, 0)..depth + 1 {
                    out.push_str("  ")!;
                }
                if a.prefix.len > 0 {
                    out.push_str(a.prefix)!;
                    out.push(' ')!;
                }
                out.push_str(a.name)!;
                out.push_str("=\"")!;
                out.push_str(a.value)!;
                out.push_str("\"\n")!;
            }
            if n.contents != NONE {
                work.push(n.contents)!;
                work.push(depth + 1)!;
            }
            push_children(doc, &mut work, n.first, depth + 1)!;
        } else if n.kind == TEXT {
            out.push('"')!;
            out.push_str(n.text)!;
            out.push_str("\"\n")!;
        } else if n.kind == COMMENT {
            out.push_str("<!-- ")!;
            out.push_str(n.text)!;
            out.push_str(" -->\n")!;
        } else if n.kind == DOCTYPE {
            out.push_str("<!DOCTYPE ")!;
            out.push_str(n.name)!;
            if (n.text.len > 0) or (n.system_id.len > 0) {
                out.push_str(" \"")!;
                out.push_str(n.text)!;
                out.push_str("\" \"")!;
                out.push_str(n.system_id)!;
                out.push('"')!;
            }
            out.push_str(">\n")!;
        }
    }
}

// The byte at i of an attribute's displayed name ("prefix name"), or -1.
fn display_at(a: Attr, i: usize) i32 {
    let mut k = i;
    if a.prefix.len > 0 {
        if k < a.prefix.len {
            return @as(i32, a.prefix[k]);
        }
        if k == a.prefix.len {
            return ' ';
        }
        k -= a.prefix.len + 1;
    }
    if k < a.name.len {
        return @as(i32, a.name[k]);
    }
    return -1;
}

fn display_less(a: Attr, b: Attr) bool {
    let mut i: usize = 0;
    loop {
        let x = display_at(a, i);
        let y = display_at(b, i);
        if x != y {
            return x < y;
        }
        if x < 0 {
            return false;
        }
        i += 1;
    }
}

// Pushes the siblings from `first` on, last first (so they pop in order).
fn push_children(doc: *Document, work: *mut vec.Vec.[u32], first: u32, depth: u32) !void {
    let st = work.len;
    let mut c = first;
    while c != NONE {
        work.push(c)!;
        work.push(depth)!;
        c = doc.node(c).next;
    }
    // reverse the pairs
    let items = work.items();
    let cnt = (work.len - st) / 2;
    for k in @as(usize, 0)..cnt / 2 {
        let i = st + (2 * k);
        let j = st + (2 * (cnt - 1 - k));
        let a0 = items[i];
        let a1 = items[i + 1];
        items[i] = items[j];
        items[i + 1] = items[j + 1];
        items[j] = a0;
        items[j + 1] = a1;
    }
}

fn tree_of(input: []u8, context: []u8, scripting: bool) ![]mut u8 {
    let mut o = Options.default();
    o.scripting = scripting;
    let mut doc = Document.init();
    if context.len > 0 {
        doc = parse_fragment(input, context, o)!;
    } else {
        doc = parse_with(input, o)!;
    }
    defer doc.deinit();
    let mut s = strs.String.init();
    dump(&doc, 0, &mut s) catch |e| {
        s.deinit();
        return e;
    };
    return s.take();
}

test "html5lib tree construction" {
    let data = fs.read_file("std/text/html/testdata/tree.dat") catch return;
    defer mem.free(data);
    let mut parts = strs.split(data, "\n#data\n");
    _ = parts.next() catch return;
    let mut n: usize = 0;
    let mut failed: usize = 0;
    while true {
        let t = parts.next() catch break;
        let mut input: []u8 = "";
        if not strs.starts_with(t, "#errors\n") {
            let e = strs.index_of(t, "\n#errors\n") catch {
                @expect(false);
                break;
            };
            input = t[..e];
        }
        let mut context: []u8 = "";
        let fr = strs.index_of(t, "\n#document-fragment\n") catch NONE_USIZE;
        if fr != NONE_USIZE {
            let st = fr + 20;
            let nl = strs.index_of_from(t, "\n", st) catch t.len;
            context = t[st..nl];
        }
        let mut modes: u8 = 3;  // bit 0: scripting off, bit 1: on
        if strs.contains(t, "\n#script-on\n") {
            modes = 2;
        } else if strs.contains(t, "\n#script-off\n") {
            modes = 1;
        }
        let d = strs.index_of(t, "\n#document\n") catch {
            @expect(false);
            break;
        };
        let mut want = t[d + 11..];
        while (want.len > 0) and (want[want.len - 1] == '\n') {
            want = want[..want.len - 1];
        }
        for m in [@as(u8, 1), 2]{
            if (modes & m) == 0 {
                continue;
            }
            n += 1;
            let got = tree_of(input, context, m == 2)!;
            defer mem.free(got);
            let mut g = got[..];
            while (g.len > 0) and (g[g.len - 1] == '\n') {
                g = g[..g.len - 1];
            }
            if not strs.eq(g, want) {
                failed += 1;
                if failed <= 30 {
                    fmt.print("--- #data\n{}\n--- context {} scripting {}\n--- want\n{}\n--- got\n{}\n",
                              [fmt.str(input), fmt.str(context), fmt.int(m - 1), fmt.str(want), fmt.str(g)]) catch {};
                }
            }
        }
    }
    if failed > 0 {
        fmt.print("tree construction: {} of {} failed\n", [fmt.int(failed), fmt.int(n)]) catch {};
    }
    @expect(n > 3000);
    @expect_eq(failed, 0);
}

fn esc_field(out: *mut strs.String, s: []u8) !void {
    let hex = "0123456789abcdef";
    for c in s {
        if (c < 0x20) or (c == '\\') or (c == '|') {
            out.push_str("\\x")!;
            out.push(hex[@as(usize, c >> 4)])!;
            out.push(hex[@as(usize, c & 15)])!;
        } else {
            out.push(c)!;
        }
    }
}

fn attr_less(a: Attr, b: Attr) bool {
    return strs.less(a.name, b.name);
}

fn tokens_of(input: []u8, state: u8, last: []u8, out: *mut strs.String) !void {
    let mut tz = Tokenizer.init(input)!;
    defer tz.deinit();
    tz.state = state;
    if not strs.eq(last, "-") {
        tz.set_last_start_tag(last)!;
    }
    let mut chars = false;
    loop {
        let t = tz.next()!;
        let k = t.kind;
        if k == TK_EOF {
            return;
        }
        if k == TK_CHARS {
            if not chars {
                if out.len > 0 {
                    out.push('\n')!;
                }
                out.push_str("T|")!;
            }
            esc_field(out, t.data)!;
            chars = true;
            continue;
        }
        chars = false;
        if out.len > 0 {
            out.push('\n')!;
        }
        if k == TK_COMMENT {
            out.push_str("C|")!;
            esc_field(out, t.data)!;
        } else if k == TK_END {
            out.push_str("E|")!;
            esc_field(out, t.name)!;
        } else if k == TK_START {
            out.push_str("S|")!;
            esc_field(out, t.name)!;
            if t.self_closing {
                out.push_str("|1")!;
            } else {
                out.push_str("|0")!;
            }
            let mut attrs = vec.Vec.[Attr].init();
            defer attrs.deinit();
            attrs.extend(t.attrs)!;
            attrs.sort(attr_less);
            for a in attrs.items() {
                out.push('|')!;
                esc_field(out, a.name)!;
                out.push('|')!;
                esc_field(out, a.value)!;
            }
        } else if k == TK_DOCTYPE {
            out.push_str("D|")!;
            let parts: [3][]u8 = [t.name, t.public_id, t.system_id];
            let has: [3]bool = [t.has_name, t.has_public, t.has_system];
            for i in @as(usize, 0)..3 {
                if has[i] {
                    out.push('+')!;
                    esc_field(out, parts[i])!;
                } else {
                    out.push('-')!;
                }
                out.push('|')!;
            }
            if t.force_quirks {
                out.push('0')!;
            } else {
                out.push('1')!;
            }
        }
    }
}

test "html5lib tokenizer" {
    let data = fs.read_file("std/text/html/testdata/tokenizer.txt") catch return;
    defer mem.free(data);
    let mut at = (strs.index_of(data, "\n") catch 0) + 1;
    let mut n: usize = 0;
    let mut failed: usize = 0;
    while at < data.len {
        let nl = strs.index_of_from(data, "\n", at) catch data.len;
        let mut f = strs.fields(data[at..nl]);
        at = nl + 1;
        _ = f.next() catch break;
        let st = f.next() catch break;
        let last = f.next() catch break;
        let ilen = fmt.parse_int.[usize](f.next() catch break) catch break;
        let elen = fmt.parse_int.[usize](f.next() catch break) catch break;
        let input = data[at..at + ilen];
        at += ilen + 1;
        let want = data[at..at + elen];
        at += elen + 1;
        let mut state = S_DATA;
        if strs.eq(st, "plaintext") {
            state = S_PLAINTEXT;
        } else if strs.eq(st, "rcdata") {
            state = S_RCDATA;
        } else if strs.eq(st, "rawtext") {
            state = S_RAWTEXT;
        } else if strs.eq(st, "script") {
            state = S_SCRIPT;
        } else if strs.eq(st, "cdata") {
            state = S_CDATA;
        }
        let mut got = strs.String.init();
        defer got.deinit();
        tokens_of(input, state, last, &mut got)!;
        n += 1;
        if not strs.eq(got.bytes(), want) {
            failed += 1;
            if failed <= 30 {
                fmt.print(
                    "--- {} {}: {}\n--- want\n{}\n--- got\n{}\n", [fmt.str(st), fmt.str(last), fmt.str(input),
                                                                   fmt.str(want), fmt.str(got.bytes())]) catch {};
            }
        }
    }
    if failed > 0 {
        fmt.print("tokenizer: {} of {} failed\n", [fmt.int(failed), fmt.int(n)]) catch {};
    }
    @expect(n > 6000);
    @expect_eq(failed, 0);
}

test "reading the tree" {
    let text =
        "<!doctype html><title> A  <b>title</b>\n</title><body class='x y'><ul id=l><li>one<li data-n=2>two &amp; &lt;three&gt;</ul><p>caf&eacute;&notit; &#x1F600;</p><template><i>t</i></template>";
    let mut doc = parse(text)!;
    defer doc.deinit();
    @expect_eq(doc.mode, NO_QUIRKS);
    let t = doc.title()!;
    defer mem.free(t);
    @expect_eq(t, "A <b>title</b>");
    let body = doc.body();
    @expect_eq(doc.node(body).name, "body");
    @expect_eq(doc.attr(body, "class")!, "x y");
    @expect(not doc.has_attr(body, "id"));
    let ul = doc.find(0, "UL");
    @expect_eq(doc.attr(ul, "id")!, "l");
    let mut names = strs.String.init();
    defer names.deinit();
    let mut it = doc.children(ul);
    while true {
        let c = it.next() catch break;
        names.push_str(doc.node(c).name)!;
    }
    @expect_eq(names.bytes(), "lili");
    let tx = doc.text_content(ul)!;
    defer mem.free(tx);
    @expect_eq(tx, "onetwo & <three>");
    let p = doc.find(0, "p");
    let pt = doc.text_content(p)!;
    defer mem.free(pt);
    @expect_eq(pt, "caf\xc3\xa9\xc2\xacit; \xf0\x9f\x98\x80");
    // template contents are walked as children, but are not text content
    let tpl = doc.find(0, "template");
    @expect_eq(doc.node(tpl).first, NONE);
    @expect(doc.find(tpl, "i") != NONE);
    let tt = doc.text_content(tpl)!;
    defer mem.free(tt);
    @expect_eq(tt, "");
    @expect_eq(doc.parent_element(doc.find(tpl, "i")), tpl);
    // quirks
    let mut q = parse("<p>x")!;
    defer q.deinit();
    @expect_eq(q.mode, QUIRKS);
}

fn round(text: []u8) ![]mut u8 {
    let mut doc = parse(text)!;
    defer doc.deinit();
    return doc.inner_html(doc.body());
}

test "serialization" {
    let cases: [8][2][]u8 = [
        ["<p class=\"a&quot;b\" title='<&>'>x &amp; y &lt; z</p>",
         "<p class=\"a&quot;b\" title=\"&lt;&amp;&gt;\">x &amp; y &lt; z</p>"],
        ["<br><img src=a.png><input disabled>", "<br><img src=\"a.png\"><input disabled=\"\">"],
        ["x<script>if (a < b && c) {}</script><style>p > a {}</style>",
         "x<script>if (a < b && c) {}</script><style>p > a {}</style>"],
        ["<textarea>a&lt;b</textarea>", "<textarea>a&lt;b</textarea>"],
        ["<svg viewbox=\"0 0 1 1\"><use xlink:href=#a /></svg>",
         "<svg viewBox=\"0 0 1 1\"><use xlink:href=\"#a\"></use></svg>"],
        ["x<template><td>x</td></template>", "x<template><td>x</td></template>"],
        ["a\xc2\xa0b<!--c-->", "a&nbsp;b<!--c-->"],
        ["<table><tr><td>1</table>", "<table><tbody><tr><td>1</td></tr></tbody></table>"],
    ];
    for c in cases {
        let got = round(c[0])!;
        defer mem.free(got);
        @expect_eq(got, c[1]);
    }
    let mut doc = parse("<!DOCTYPE html><html><head></head><body>x</body></html>")!;
    defer doc.deinit();
    let all = doc.to_html(0)!;
    defer mem.free(all);
    @expect_eq(all, "<!DOCTYPE html><html><head></head><body>x</body></html>");
}

test "selectors" {
    let text =
        "<div id=main class='a b'><p class=a>1</p><p lang=en-GB>2</p><span>3</span><p title='x y'>4</p></div><ul><li>a<li class=on>b<li>c<li>d</ul><a href=/x>l</a><a>m</a><input type=checkbox checked><svg><clipPath id=c /></svg>";
    let mut doc = parse(text)!;
    defer doc.deinit();
    let cases: [22][2][]u8 = [
        ["p", "1 2 4"],
        ["#main > p.a", "1"],
        ["div p", "1 2 4"],
        ["p + span", "3"],
        ["p ~ p", "2 4"],
        ["[lang|=en]", "2"],
        ["[title~=y]", "4"],
        ["[title^='x ']", "4"],
        ["li:nth-child(2n+1)", "a c"],
        ["li:nth-child(odd)", "a c"],
        ["li:nth-last-child(1)", "d"],
        ["li:not(.on):not(:first-child)", "c d"],
        ["li:first-child, li:last-child", "a d"],
        ["ul > :is(.on, :last-child)", "b d"],
        ["a:link", "l"],
        ["p:nth-of-type(2)", "2"],
        ["div > :only-of-type", "3"],
        ["span:first-child", ""],
        ["DIV#main.b", "(div)"],
        ["[href$=x]", "l"],
        ["clipPath", "(clipPath)"],
        [":checked", "(input)"],
    ];
    for c in cases {
        let ids = doc.query_all(c[0])!;
        defer mem.free(ids);
        let mut got = strs.String.init();
        defer got.deinit();
        for id in ids {
            if got.len > 0 {
                got.push(' ')!;
            }
            let t = doc.text_content(id)!;
            defer mem.free(t);
            if t.len > 0 {
                got.push_str(t)!;
            } else {
                got.push('(')!;
                got.push_str(doc.node(id).name)!;
                got.push(')')!;
            }
        }
        let want = c[1];
        if strs.eq(want, "(div)") {
            @expect_eq(ids.len, 1);
            @expect_eq(doc.node(ids[0]).name, "div");
            continue;
        }
        if not strs.eq(got.bytes(), want) {
            fmt.print("{}: got {} want {}\n", [fmt.str(c[0]), fmt.str(got.bytes()), fmt.str(want)]) catch {};
        }
        @expect_eq(got.bytes(), want);
    }
    @expect(doc.matches(doc.find(0, "span"), "div > span:nth-child(3)")!);
    @expect_eq(doc.query("nav")!, NONE);
    let bad: [6][]u8 = ["", "p >", "[x", ":nth-child(x)", "p::before", "a,,b"];
    for b in bad {
        _ = compile(b) catch continue;
        fmt.print("accepted: {}\n", [fmt.str(b)]) catch {};
        @expect(false);
    }
}

test "changing the tree" {
    let mut doc = parse("<ul><li>a</li></ul>")!;
    defer doc.deinit();
    let ul = doc.find(0, "ul");
    let li = doc.create_element("LI")!;
    doc.set_attr(li, "class", "new")!;
    doc.append_child(li, doc.create_text("b & c")!);
    doc.append_child(ul, li);
    let copy = doc.clone_tree(li)!;
    doc.set_attr(copy, "class", "copy")!;
    doc.set_attr(copy, "id", "z")!;
    doc.insert_before(ul, copy, doc.node(ul).first);
    doc.remove(doc.find(ul, "li"));  // the copy
    doc.remove_attr(li, "class")!;
    doc.append_child(ul, doc.create_comment("end")!);
    let s = doc.to_html(ul)!;
    defer mem.free(s);
    @expect_eq(s, "<ul><li>a</li><li>b &amp; c</li><!--end--></ul>");
}

test "fragments" {
    let mut f = parse_fragment("<td>x<td>y", "tr", Options.default())!;
    defer f.deinit();
    @expect_eq(f.node(0).kind, FRAGMENT);
    let s = f.inner_html(0)!;
    defer mem.free(s);
    @expect_eq(s, "<td>x</td><td>y</td>");
    let mut g = parse_fragment("<path d=M0/>x</title>y", "svg title", Options.default())!;
    defer g.deinit();
    let t = g.inner_html(0)!;
    defer mem.free(t);
    // (svg title is an HTML integration point, so that path is HTML, and the
    // unquoted value takes the slash)
    @expect_eq(t, "<path d=\"M0/\">xy</path>");
    let mut h = parse_fragment("<path d='M0'/>x<clippath>", "svg g", Options.default())!;
    defer h.deinit();
    let u = h.inner_html(0)!;
    defer mem.free(u);
    @expect_eq(u, "<path d=\"M0\"></path>x<clipPath></clipPath>");
}

test "encodings" {
    // UTF-16LE with a byte-order mark
    let u16le: [22]u8 = [0xff, 0xfe, '<', 0, 'p', 0, '>', 0, 0xe9, 0, 0x3d, 0xd8, 0x00, 0xde, '<', 0, '/', 0, 'p', 0,
                         '>', 0];
    let mut a = parse_bytes(u16le[..], Options.default())!;
    defer a.deinit();
    let ta = a.text_content(a.find(0, "p"))!;
    defer mem.free(ta);
    @expect_eq(ta, "\xc3\xa9\xf0\x9f\x98\x80");
    let mut b = parse_bytes("<meta charset=\"windows-1252\"><p>\x80\xe9</p>", Options.default())!;
    defer b.deinit();
    let tb = b.text_content(b.find(0, "p"))!;
    defer mem.free(tb);
    @expect_eq(tb, "\xe2\x82\xac\xc3\xa9");
    let mut c = parse_bytes("<p>caf\xc3\xa9", Options.default())!;
    defer c.deinit();
    let tc = c.text_content(c.find(0, "p"))!;
    defer mem.free(tc);
    @expect_eq(tc, "caf\xc3\xa9");
    let mut d = parse_bytes("<p>caf\xe9", Options.default())!;  // not UTF-8: windows-1252
    defer d.deinit();
    let td = d.text_content(d.find(0, "p"))!;
    defer mem.free(td);
    @expect_eq(td, "caf\xc3\xa9");
    // invalid UTF-8 given to parse
    let mut e = parse("<p>a\xffb\r\nc")!;
    defer e.deinit();
    let te = e.text_content(e.find(0, "p"))!;
    defer mem.free(te);
    @expect_eq(te, "a\xef\xbf\xbdb\nc");
}

test "hostile input" {
    let mut s = strs.String.init();
    defer s.deinit();
    // deep nesting: the stack stops at MAX_DEPTH, and nothing recurses
    for _k in @as(usize, 0)..100000 {
        s.push_str("<div>")!;
    }
    for _k in @as(usize, 0)..20000 {
        s.push_str("</p>")!;
    }
    let mut d1 = parse(s.bytes())!;
    let h1 = d1.to_html(0)!;
    mem.free(h1);
    d1.deinit();
    // misnested formatting elements with distinct attributes
    s.clear();
    for k in @as(usize, 0)..20000 {
        fmt.write(s.writer(), "<p><b id={}><i>x</p>", [fmt.int(k)])!;
    }
    let mut d2 = parse(s.bytes())!;
    @expect(d2.nodes.len < 3000000);
    d2.deinit();
    // many attributes, most repeated
    s.clear();
    s.push_str("<p")!;
    for k in @as(usize, 0)..50000 {
        fmt.write(s.writer(), " a{}=1", [fmt.int(k % 1000)])!;
    }
    s.push_str(">")!;
    let mut d3 = parse(s.bytes())!;
    @expect_eq(d3.node(d3.find(0, "p")).attrs.len, 1000);
    d3.deinit();
    // long runs of things that look like references, tables and foster parenting
    s.clear();
    for _k in @as(usize, 0)..20000 {
        s.push_str("&aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa<table>x<tr>y<td>z</table>&#99999999999;")!;
    }
    let mut d4 = parse(s.bytes())!;
    d4.deinit();
}
