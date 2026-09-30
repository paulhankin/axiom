// =============================================================================
// std/graphics/shape/plan.ph - what shaping a run needs to know, worked out once: the
// feature map, which shaper handles the script and its settings, and the
// Unicode properties shaping keeps per character (HarfBuzz's
// hb_ot_shape_plan_t and hb-ot-layout.hh helpers).
// =============================================================================

import "../../unicode.ph" as uni;
import "../font.ph" as font;
import "buffer.ph" as buf;
import "layout.ph" as layout;
import "tables.ph" as t;

// Shapers
pub const SHAPER_DEFAULT: u8 = 0;
pub const SHAPER_ARABIC: u8 = 1;
pub const SHAPER_HEBREW: u8 = 2;
pub const SHAPER_INDIC: u8 = 3;

// When mark advances are zeroed
pub const ZWM_NONE: u8 = 0;
pub const ZWM_EARLY: u8 = 1;
pub const ZWM_LATE: u8 = 2;

// Normalization modes
pub const NM_DECOMPOSED: u8 = 1;
pub const NM_COMPOSED: u8 = 2;
pub const NM_COMPOSED_NO_SHORT_CIRCUIT: u8 = 3;

// Pauses between GSUB stages
pub const PAUSE_NONE: u8 = 0;
pub const PAUSE_ARABIC_STCH: u8 = 1;
pub const PAUSE_ARABIC_FALLBACK: u8 = 2;
pub const PAUSE_INDIC_SYLLABLES: u8 = 3;
pub const PAUSE_INDIC_INITIAL: u8 = 4;
pub const PAUSE_INDIC_FINAL: u8 = 5;

// General categories (std/unicode's numbering)
pub const GC_CN: u32 = 0;
pub const GC_LU: u32 = 1;
pub const GC_LL: u32 = 2;
pub const GC_LT: u32 = 3;
pub const GC_LM: u32 = 4;
pub const GC_LO: u32 = 5;
pub const GC_MN: u32 = 6;
pub const GC_MC: u32 = 7;
pub const GC_ME: u32 = 8;
pub const GC_ND: u32 = 9;
pub const GC_NL: u32 = 10;
pub const GC_NO: u32 = 11;
pub const GC_SM: u32 = 19;
pub const GC_SC: u32 = 20;
pub const GC_SK: u32 = 21;
pub const GC_SO: u32 = 22;
pub const GC_ZS: u32 = 23;
pub const GC_CC: u32 = 26;
pub const GC_CF: u32 = 27;
pub const GC_CS: u32 = 28;
pub const GC_CO: u32 = 29;

pub fn tag(s: []u8) u32 {
    return layout.tag_of(s);
}

// Indic settings (HarfBuzz's indic_config_t)
pub const REPH_AFTER_MAIN: u8 = 5;
pub const REPH_BEFORE_SUB: u8 = 7;
pub const REPH_AFTER_SUB: u8 = 9;
pub const REPH_BEFORE_POST: u8 = 10;
pub const REPH_AFTER_POST: u8 = 12;
pub const REPH_IMPLICIT: u8 = 0;
pub const REPH_EXPLICIT: u8 = 1;
pub const REPH_LOG_REPHA: u8 = 2;
pub const BLWF_PRE_AND_POST: u8 = 0;
pub const BLWF_POST_ONLY: u8 = 1;

pub struct IndicConfig {
    has_old_spec: bool,
    virama: u32,
    reph_pos: u8,
    reph_mode: u8,
    blwf_mode: u8,
}

pub fn indic_config(script: u32) IndicConfig {
    if script == tag("Deva") {
        return IndicConfig.{ has_old_spec: true, virama: 0x094d, reph_pos: REPH_BEFORE_POST, reph_mode: REPH_IMPLICIT,
                             blwf_mode: BLWF_PRE_AND_POST };
    }
    if script == tag("Beng") {
        return IndicConfig.{ has_old_spec: true, virama: 0x09cd, reph_pos: REPH_AFTER_SUB, reph_mode: REPH_IMPLICIT,
                             blwf_mode: BLWF_PRE_AND_POST };
    }
    if script == tag("Guru") {
        return IndicConfig.{ has_old_spec: true, virama: 0x0a4d, reph_pos: REPH_BEFORE_SUB, reph_mode: REPH_IMPLICIT,
                             blwf_mode: BLWF_PRE_AND_POST };
    }
    if script == tag("Gujr") {
        return IndicConfig.{ has_old_spec: true, virama: 0x0acd, reph_pos: REPH_BEFORE_POST, reph_mode: REPH_IMPLICIT,
                             blwf_mode: BLWF_PRE_AND_POST };
    }
    if script == tag("Orya") {
        return IndicConfig.{ has_old_spec: true, virama: 0x0b4d, reph_pos: REPH_AFTER_MAIN, reph_mode: REPH_IMPLICIT,
                             blwf_mode: BLWF_PRE_AND_POST };
    }
    if script == tag("Taml") {
        return IndicConfig.{ has_old_spec: true, virama: 0x0bcd, reph_pos: REPH_AFTER_POST, reph_mode: REPH_IMPLICIT,
                             blwf_mode: BLWF_PRE_AND_POST };
    }
    if script == tag("Telu") {
        return IndicConfig.{ has_old_spec: true, virama: 0x0c4d, reph_pos: REPH_AFTER_POST, reph_mode: REPH_EXPLICIT,
                             blwf_mode: BLWF_POST_ONLY };
    }
    if script == tag("Knda") {
        return IndicConfig.{ has_old_spec: true, virama: 0x0ccd, reph_pos: REPH_AFTER_POST, reph_mode: REPH_IMPLICIT,
                             blwf_mode: BLWF_POST_ONLY };
    }
    if script == tag("Mlym") {
        return IndicConfig.{ has_old_spec: true, virama: 0x0d4d, reph_pos: REPH_AFTER_MAIN, reph_mode: REPH_LOG_REPHA,
                             blwf_mode: BLWF_PRE_AND_POST };
    }
    return IndicConfig.{ has_old_spec: false, virama: 0, reph_pos: REPH_BEFORE_POST, reph_mode: REPH_IMPLICIT,
                         blwf_mode: BLWF_PRE_AND_POST };
}

// The Indic features, in order (masks are kept for the non-global ones)
pub const INDIC_NUM_FEATURES: usize = 17;
pub const INDIC_RPHF: usize = 2;
pub const INDIC_PREF: usize = 4;
pub const INDIC_BLWF: usize = 5;
pub const INDIC_ABVF: usize = 6;
pub const INDIC_HALF: usize = 7;
pub const INDIC_PSTF: usize = 8;
pub const INDIC_INIT: usize = 11;
pub const INDIC_BASIC_FEATURES: usize = 11;

// Lookup ranges for would-substitute checks
pub const WOULD_RPHF: usize = 0;
pub const WOULD_PREF: usize = 1;
pub const WOULD_BLWF: usize = 2;
pub const WOULD_PSTF: usize = 3;
pub const WOULD_VATU: usize = 4;

pub struct Plan {
    f: *font.Font,
    face: layout.Face,
    map: layout.Map,
    script: u32,  // ISO 15924 code (0: none)
    rtl: bool,
    shaper: u8,
    zwm: u8,
    norm: u8,
    frac_mask: u32,
    numr_mask: u32,
    dnom_mask: u32,
    has_frac: bool,
    rtlm_mask: u32,
    kern_mask: u32,
    apply_gpos: bool,
    apply_kern: bool,
    zero_marks: bool,
    has_gpos_mark: bool,
    adjust_mark_positioning_when_zeroing: bool,
    fallback_mark_positioning: bool,
    fallback_glyph_classes: bool,
    kern: usize,  // the kern table (0: none)
    // Arabic
    arabic_mask: [8]u32,
    arabic_do_fallback: bool,
    arabic_has_stch: bool,
    // Indic
    indic: IndicConfig,
    indic_old_spec: bool,
    indic_mask: [17]u32,
    indic_zero_context: bool,
    virama_glyph: i64,  // -1: not looked up yet
    would: [5][2]usize,

    pub fn deinit(self: *mut Plan) void {
        self.map.deinit();
    }

    /// The font's glyph for a character (0 if none).
    pub fn glyph(self: *Plan, cp: u32) u32 {
        return self.f.glyph_index(cp);
    }

    /// Whether any lookup of a would-substitute set substitutes gs.
    pub fn would_substitute(self: *Plan, which: usize, gs: []u32) bool {
        let r = self.would[which];
        let ls = self.map.lookups[0].items();
        for i in r[0]..r[1] {
            if layout.would_substitute(&self.face, ls[i].index, gs, self.indic_zero_context) {
                return true;
            }
        }
        return false;
    }
}

// -----------------------------------------------------------------------------
// Scripts
// -----------------------------------------------------------------------------

/// The ISO 15924 script code of a character ("Zyyy" common, "Zinh"
/// inherited, "Zzzz" unknown).
pub fn script_of(cp: u32) u32 {
    let firsts = t.SCRIPT_FIRST[..];
    let mut lo: usize = 0;
    let mut hi = firsts.len;
    while lo < hi {
        let mid = (lo + hi) / 2;
        if firsts[mid] <= cp {
            lo = mid + 1;
        } else {
            hi = mid;
        }
    }
    if (lo > 0) and (cp <= t.SCRIPT_LAST[lo - 1]) {
        return t.SCRIPT_TAG[lo - 1];
    }
    return tag("Zzzz");
}

/// Scripts written right to left.
pub fn script_is_rtl(s: u32) bool {
    let rtl: [40][]u8 = [
        "Arab",
        "Hebr",
        "Syrc",
        "Thaa",
        "Cprt",
        "Khar",
        "Phnx",
        "Nkoo",
        "Lydi",
        "Avst",
        "Armi",
        "Phli",
        "Prti",
        "Sarb",
        "Orkh",
        "Samr",
        "Mand",
        "Merc",
        "Mero",
        "Mani",
        "Mend",
        "Nbat",
        "Narb",
        "Palm",
        "Phlp",
        "Hatr",
        "Adlm",
        "Rohg",
        "Sogo",
        "Sogd",
        "Elym",
        "Chrs",
        "Yezi",
        "Ougr",
        "Gara",
        "Sidt",
        "Hung",
        "Ital",
        "Runr",
        "Tfng",
    ];
    for r in rtl {
        if tag(r) == s {
            // Old Hungarian, Old Italic, Runic and Tifinagh may go either way;
            // HarfBuzz treats them as invalid (no preferred direction)
            if (s == tag("Hung")) or (s == tag("Ital")) or (s == tag("Runr")) or (s == tag("Tfng")) {
                return false;
            }
            return true;
        }
    }
    return false;
}

/// The OpenType script tags for a script, most preferred first.
pub fn script_tags(s: u32, out: *mut [3]u32) usize {
    if s == 0 {
        return 0;
    }
    let mut n: usize = 0;
    let news: [9][]u8 = ["Beng", "Deva", "Gujr", "Guru", "Knda", "Mlym", "Orya", "Taml", "Telu"];
    let newt: [9][]u8 = ["bng2", "dev2", "gjr2", "gur2", "knd2", "mlm2", "ory2", "tml2", "tel2"];
    for i in @as(usize, 0)..9 {
        if s == tag(news[i]) {
            let nt = tag(newt[i]);
            out[0] = (nt & 0xffffff00) | 0x33;  // "dev3" and so on
            out[1] = nt;
            n = 2;
        }
    }
    if s == tag("Mymr") {
        out[0] = tag("mym2");
        n = 1;
    }
    let mut old = s | 0x20000000;  // "Arab" -> "arab"
    if (s == tag("Hira")) or (s == tag("Hrkt")) {
        old = tag("kana");
    } else if s == tag("Laoo") {
        old = tag("lao ");
    } else if s == tag("Yiii") {
        old = tag("yi  ");
    } else if s == tag("Nkoo") {
        old = tag("nko ");
    } else if s == tag("Vaii") {
        old = tag("vai ");
    } else if s == tag("Zmth") {
        old = tag("math");
    }
    out[n] = old;
    return n + 1;
}

/// The shaper for a script, given the GSUB script tag the font chose.
pub fn shaper_for(s: u32, gsub_script: u32) u8 {
    if (s == tag("Arab")) or (s == tag("Syrc")) {
        if (gsub_script != tag("DFLT")) or (s == tag("Arab")) {
            return SHAPER_ARABIC;
        }
        return SHAPER_DEFAULT;
    }
    if s == tag("Hebr") {
        return SHAPER_HEBREW;
    }
    let indic: [9][]u8 = ["Beng", "Deva", "Gujr", "Guru", "Knda", "Mlym", "Orya", "Taml", "Telu"];
    for i in indic {
        if s == tag(i) {
            if (gsub_script == tag("DFLT")) or (gsub_script == tag("latn")) {
                return SHAPER_DEFAULT;
            }
            if (gsub_script & 0xff) == 0x33 {
                // new-style ("dev3") fonts use the Universal Shaping Engine,
                // which std/graphics/shape does not have
                return SHAPER_DEFAULT;
            }
            return SHAPER_INDIC;
        }
    }
    return SHAPER_DEFAULT;
}

// -----------------------------------------------------------------------------
// Unicode properties
// -----------------------------------------------------------------------------

/// HarfBuzz's reordering of combining classes (Hebrew, Arabic, Syriac,
/// Telugu, Thai, Lao and Tibetan marks sort as the fonts expect).
fn modified_class(cc: u32) u32 {
    if (cc >= 10) and (cc <= 26) {
        let heb: [17]u8 = [22, 15, 16, 17, 23, 18, 19, 20, 21, 14, 24, 12, 25, 13, 10, 11, 26];
        return @as(u32, heb[@as(usize, cc - 10)]);
    }
    if (cc >= 27) and (cc <= 35) {
        let arab: [9]u8 = [28, 29, 30, 31, 32, 33, 27, 34, 35];
        return @as(u32, arab[@as(usize, cc - 27)]);
    }
    if cc == 84 {
        return 4;
    }
    if cc == 91 {
        return 5;
    }
    if cc == 103 {
        return 3;
    }
    if cc == 130 {
        return 132;
    }
    if cc == 132 {
        return 131;
    }
    return cc;
}

pub fn modified_combining_class(u: u32) u32 {
    if u == 0x1a60 {
        return 254;
    }
    if u == 0x0fc6 {
        return 254;
    }
    if u == 0x0f39 {
        return 127;
    }
    return modified_class(uni.combining_class(u));
}

pub fn is_default_ignorable(ch: u32) bool {
    let plane = ch >> 16;
    if plane == 0 {
        let page = ch >> 8;
        if page == 0x00 {
            return ch == 0x00ad;
        }
        if page == 0x03 {
            return ch == 0x034f;
        }
        if page == 0x06 {
            return ch == 0x061c;
        }
        if page == 0x17 {
            return (ch >= 0x17b4) and (ch <= 0x17b5);
        }
        if page == 0x18 {
            return (ch >= 0x180b) and (ch <= 0x180f);
        }
        if page == 0x20 {
            return ((ch >= 0x200b) and (ch <= 0x200f)) or ((ch >= 0x202a) and (ch <= 0x202e)) or
                   ((ch >= 0x2060) and (ch <= 0x206f));
        }
        if page == 0xfe {
            return ((ch >= 0xfe00) and (ch <= 0xfe0f)) or (ch == 0xfeff);
        }
        if page == 0xff {
            return (ch >= 0xfff0) and (ch <= 0xfff8);
        }
        return false;
    }
    if plane == 0x01 {
        return (ch >= 0x1d173) and (ch <= 0x1d17a);
    }
    if plane == 0x0e {
        return (ch >= 0xe0000) and (ch <= 0xe0fff);
    }
    return false;
}

pub fn is_mark_gc(gc: u32) bool {
    return (gc == GC_MN) or (gc == GC_MC) or (gc == GC_ME);
}

/// Sets a character's Unicode properties (HarfBuzz's
/// _hb_glyph_info_set_unicode_props).
pub fn set_unicode_props(b: *mut buf.Buffer, i: *mut buf.Info) void {
    let u = i.cp;
    let gc = uni.category_code(u);
    let mut props = @truncate(u16, gc);
    if u >= 0x80 {
        if is_default_ignorable(u) {
            b.scratch |= buf.SF_HAS_DEFAULT_IGNORABLES;
            props |= buf.UP_IGNORABLE;
            if u == 0x200c {
                props |= buf.UP_ZWNJ;
            } else if u == 0x200d {
                props |= buf.UP_ZWJ;
            } else if ((u >= 0x180b) and (u <= 0x180d)) or (u == 0x180f) {
                props |= buf.UP_HIDDEN;
            } else if (u >= 0xe0020) and (u <= 0xe007f) {
                props |= buf.UP_HIDDEN;
            } else if u == 0x034f {
                b.scratch |= buf.SF_HAS_CGJ;
                props |= buf.UP_HIDDEN;
            }
        }
        if is_mark_gc(gc) {
            b.scratch |= buf.SF_HAS_CONTINUATIONS;
            props |= buf.UP_CONTINUATION;
            props |= @truncate(u16, modified_combining_class(u) << 8);
        }
    }
    i.uprops = props;
}

pub fn set_continuation(b: *mut buf.Buffer, i: *mut buf.Info) void {
    b.scratch |= buf.SF_HAS_CONTINUATIONS;
    i.uprops |= buf.UP_CONTINUATION;
}

/// The space type for the fallback widths of Unicode spaces the font lacks.
pub const SPACE_NOT: u32 = 0;
pub const SPACE_EM: u32 = 1;
pub const SPACE_EM_2: u32 = 2;
pub const SPACE_EM_3: u32 = 3;
pub const SPACE_EM_4: u32 = 4;
pub const SPACE_EM_5: u32 = 5;
pub const SPACE_EM_6: u32 = 6;
pub const SPACE_EM_16: u32 = 16;
pub const SPACE_4_EM_18: u32 = 17;
pub const SPACE: u32 = 18;
pub const SPACE_FIGURE: u32 = 19;
pub const SPACE_PUNCTUATION: u32 = 20;
pub const SPACE_NARROW: u32 = 21;

pub fn space_fallback_type(u: u32) u32 {
    if (u == 0x20) or (u == 0xa0) {
        return SPACE;
    }
    if (u == 0x2000) or (u == 0x2002) {
        return SPACE_EM_2;
    }
    if (u == 0x2001) or (u == 0x2003) or (u == 0x3000) {
        return SPACE_EM;
    }
    if u == 0x2004 {
        return SPACE_EM_3;
    }
    if u == 0x2005 {
        return SPACE_EM_4;
    }
    if u == 0x2006 {
        return SPACE_EM_6;
    }
    if u == 0x2007 {
        return SPACE_FIGURE;
    }
    if u == 0x2008 {
        return SPACE_PUNCTUATION;
    }
    if u == 0x2009 {
        return SPACE_EM_5;
    }
    if u == 0x200a {
        return SPACE_EM_16;
    }
    if u == 0x202f {
        return SPACE_NARROW;
    }
    if u == 0x205f {
        return SPACE_4_EM_18;
    }
    return SPACE_NOT;
}
