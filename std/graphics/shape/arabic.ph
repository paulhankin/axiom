// =============================================================================
// std/graphics/shape/arabic.ph - the Arabic shaper (Arabic, Syriac): each letter gets
// its isolated, initial, medial or final form from how it joins its
// neighbours (HarfBuzz's hb-ot-shaper-arabic).
// =============================================================================

import "../../collections/vec.ph" as vec;
import "buffer.ph" as buf;
import "layout.ph" as layout;
import "plan.ph" as plan;
import "tables.ph" as t;

// Joining types
const JT_U: u32 = 0;
const JT_R: u32 = 2;
const JT_T: u32 = 6;

// Actions (the feature each glyph gets)
pub const ISOL: u8 = 0;
pub const FINA: u8 = 1;
pub const FIN2: u8 = 2;
pub const FIN3: u8 = 3;
pub const MEDI: u8 = 4;
pub const MED2: u8 = 5;
pub const INIT: u8 = 6;
pub const NONE: u8 = 7;
pub const STCH_FIXED: u8 = 8;
pub const STCH_REPEATING: u8 = 9;

pub let FEATURES: [7][]u8 = ["isol", "fina", "fin2", "fin3", "medi", "med2", "init"];

fn joining_type(u: u32, gc: u32) u32 {
    let cps = t.JOINING_CP[..];
    let mut lo: usize = 0;
    let mut hi = cps.len;
    while lo < hi {
        let mid = (lo + hi) / 2;
        if cps[mid] < u {
            lo = mid + 1;
        } else if cps[mid] > u {
            hi = mid;
        } else {
            return @as(u32, t.JOINING_TYPE[mid]);
        }
    }
    if (gc == plan.GC_MN) or (gc == plan.GC_ME) or (gc == plan.GC_CF) {
        return JT_T;
    }
    return JT_U;
}

// The state machine: [state][joining type] = (previous action, current
// action, next state)
let STATE: [7][6][3]u8 = [
    [[NONE, NONE, 0], [NONE, ISOL, 2], [NONE, ISOL, 1], [NONE, ISOL, 2], [NONE, ISOL, 1], [NONE, ISOL, 6]],
    [[NONE, NONE, 0], [NONE, ISOL, 2], [NONE, ISOL, 1], [NONE, ISOL, 2], [NONE, FIN2, 5], [NONE, ISOL, 6]],
    [[NONE, NONE, 0], [NONE, ISOL, 2], [INIT, FINA, 1], [INIT, FINA, 3], [INIT, FINA, 4], [INIT, FINA, 6]],
    [[NONE, NONE, 0], [NONE, ISOL, 2], [MEDI, FINA, 1], [MEDI, FINA, 3], [MEDI, FINA, 4], [MEDI, FINA, 6]],
    [[NONE, NONE, 0], [NONE, ISOL, 2], [MED2, ISOL, 1], [MED2, ISOL, 2], [MED2, FIN2, 5], [MED2, ISOL, 6]],
    [[NONE, NONE, 0], [NONE, ISOL, 2], [ISOL, ISOL, 1], [ISOL, ISOL, 2], [ISOL, FIN2, 5], [ISOL, ISOL, 6]],
    [[NONE, NONE, 0], [NONE, ISOL, 2], [NONE, ISOL, 1], [NONE, ISOL, 2], [NONE, FIN3, 5], [NONE, ISOL, 6]],
];

fn joining(b: *mut buf.Buffer) void {
    let mut prev: usize = 0;
    let mut has_prev = false;
    let mut state: usize = 0;
    for i in @as(usize, 0)..b.len {
        let jt = joining_type(b.info[i].cp, layout.gen_cat(&b.info[i]));
        if jt == JT_T {
            b.info[i].cat = NONE;
            continue;
        }
        let e = STATE[state][@as(usize, jt)];
        if (e[0] != NONE) and has_prev {
            b.info[prev].cat = e[0];
        }
        b.info[i].cat = e[1];
        prev = i;
        has_prev = true;
        state = @as(usize, e[2]);
    }
}

pub fn setup_masks(p: *plan.Plan, b: *mut buf.Buffer) void {
    joining(b);
    if p.script == plan.tag("Mong") {
        for i in @as(usize, 1)..b.len {
            let u = b.info[i].cp;
            if ((u >= 0x180b) and (u <= 0x180d)) or (u == 0x180f) {
                b.info[i].cat = b.info[i - 1].cat;
            }
        }
    }
    for i in @as(usize, 0)..b.len {
        b.info[i].mask |= p.arabic_mask[@as(usize, b.info[i].cat)];
    }
}

/// The pause after the stch feature: marks the pieces it made.
pub fn record_stch(p: *plan.Plan, b: *mut buf.Buffer) void {
    if not p.arabic_has_stch {
        return;
    }
    for i in @as(usize, 0)..b.len {
        if layout.multiplied(&b.info[i]) {
            let comp = layout.lig_comp(&b.info[i]);
            if (comp % 2) != 0 {
                b.info[i].cat = STCH_REPEATING;
            } else {
                b.info[i].cat = STCH_FIXED;
            }
            b.scratch |= buf.SF_ARABIC_HAS_STCH;
        }
    }
}

fn is_word_gc(gc: u32) bool {
    return (gc == plan.GC_CN) or (gc == plan.GC_CO) or (gc == plan.GC_LM) or (gc == plan.GC_LO) or (gc == plan.GC_MC) or
           (gc == plan.GC_ME) or (gc == plan.GC_MN) or (gc == plan.GC_ND) or (gc == plan.GC_NL) or (gc == plan.GC_NO) or
           (gc == plan.GC_SC) or (gc == plan.GC_SK) or (gc == plan.GC_SM) or (gc == plan.GC_SO);
}

fn is_stch(a: u8) bool {
    return (a == STCH_FIXED) or (a == STCH_REPEATING);
}

/// Stretches stch sequences (Syriac abbreviation marks) across the word
/// before them, repeating the repeatable pieces (HarfBuzz's apply_stch).
pub fn apply_stch(p: *plan.Plan, b: *mut buf.Buffer) void {
    if (b.scratch & buf.SF_ARABIC_HAS_STCH) == 0 {
        return;
    }
    let rtl = b.rtl;
    if not rtl {
        b.reverse();
    }
    let mut extra: usize = 0;
    for step in @as(u32, 0)..2 {
        let count = b.len;
        let new_len = count + extra;
        let mut j = new_len;
        let mut i = count;
        while i > 0 {
            if not is_stch(b.info[i - 1].cat) {
                if step == 1 {
                    j -= 1;
                    b.info[j] = b.info[i - 1];
                    b.pos[j] = b.pos[i - 1];
                }
                i -= 1;
                continue;
            }
            let mut w_total: i32 = 0;
            let mut w_fixed: i32 = 0;
            let mut w_repeating: i32 = 0;
            let mut n_fixed: i64 = 0;
            let mut n_repeating: i64 = 0;
            let end = i;
            while (i > 0) and is_stch(b.info[i - 1].cat) {
                i -= 1;
                let width = p.f.advance(b.info[i].cp);
                if b.info[i].cat == STCH_FIXED {
                    w_fixed = buf.sat_add(w_fixed, width);
                    n_fixed += 1;
                } else {
                    w_repeating = buf.sat_add(w_repeating, width);
                    n_repeating += 1;
                }
            }
            let start = i;
            let mut context = i;
            while (context > 0) and not is_stch(b.info[context - 1].cat) and
                  (layout.is_default_ignorable(&b.info[context - 1]) or
                   is_word_gc(layout.gen_cat(&b.info[context - 1]))) {
                context -= 1;
                w_total = buf.sat_add(w_total, b.pos[context].xa);
            }
            i += 1;  // don't touch i again (the loop below steps it back)
            let mut n_copies: i64 = 0;
            let w_remaining_signed = @as(i64, w_total) - @as(i64, w_fixed);
            let w_repeating_signed = @as(i64, w_repeating);
            let mut w_remaining = buf.sat_sub(w_total, w_fixed);
            if (w_remaining_signed > w_repeating_signed) and (w_repeating_signed > 0) {
                n_copies = (w_remaining_signed / w_repeating_signed) - 1;
            }
            let mut extra_repeat_overlap: i32 = 0;
            let shortfall = w_remaining_signed - (w_repeating_signed * (n_copies + 1));
            if (shortfall > 0) and (n_repeating > 0) {
                n_copies += 1;
                let excess = ((n_copies + 1) * w_repeating_signed) - w_remaining_signed;
                if excess > 0 {
                    extra_repeat_overlap = @truncate(i32, excess / (n_copies * n_repeating));
                    w_remaining = 0;
                }
            }
            let mut max_copies: i64 = 0;
            if n_repeating > 0 {
                let base_glyphs = n_fixed + n_repeating;
                if base_glyphs < 256 {
                    max_copies = (256 - base_glyphs) / n_repeating;
                }
            }
            n_copies = @min(n_copies, max_copies);
            if step == 0 {
                extra += @truncate(usize, @bit_cast(u64, n_copies * n_repeating));
            } else {
                let mut x_offset = w_remaining / 2;
                let mut k = end;
                while k > start {
                    let width = p.f.advance(b.info[k - 1].cp);
                    let mut repeat: i64 = 1;
                    if b.info[k - 1].cat == STCH_REPEATING {
                        repeat += n_copies;
                    }
                    b.pos[k - 1].xa = 0;
                    for n in @as(i64, 0)..repeat {
                        if rtl {
                            x_offset = buf.sat_sub(x_offset, width);
                            if n > 0 {
                                x_offset = buf.sat_add(x_offset, extra_repeat_overlap);
                            }
                        }
                        b.pos[k - 1].xo = x_offset;
                        j -= 1;
                        b.info[j] = b.info[k - 1];
                        b.pos[j] = b.pos[k - 1];
                        if not rtl {
                            x_offset = buf.sat_add(x_offset, width);
                            if n > 0 {
                                x_offset = buf.sat_sub(x_offset, extra_repeat_overlap);
                            }
                        }
                    }
                    k -= 1;
                }
            }
            i -= 1;
        }
        if step == 0 {
            if not b.ensure(count + extra) {
                break;
            }
        } else {
            b.len = new_len;
        }
    }
    if not rtl {
        b.reverse();
    }
}

// -----------------------------------------------------------------------------
// Fallback shaping for fonts without Arabic features: lookups synthesized
// from Unicode's presentation forms (HarfBuzz's arabic_fallback_plan)
// -----------------------------------------------------------------------------

fn put16(v: *mut vec.Vec.[u8], x: usize) !void {
    v.push(@truncate(u8, x >> 8))!;
    v.push(@truncate(u8, x))!;
}

fn set16(v: *mut vec.Vec.[u8], at: usize, x: usize) void {
    let bs = v.items();
    bs[at] = @truncate(u8, x >> 8);
    bs[at + 1] = @truncate(u8, x);
}

/// Stable insertion sort by first glyph.
fn sort_pairs(xs: []mut [2]u32) void {
    if xs.len < 2 {
        return;
    }
    for i in @as(usize, 1)..xs.len {
        let x = xs[i];
        let mut j = i;
        while (j > 0) and (xs[j - 1][0] > x[0]) {
            xs[j] = xs[j - 1];
            j -= 1;
        }
        xs[j] = x;
    }
}

/// A single-substitution lookup from base forms to form k (0 initial,
/// 1 medial, 2 final, 3 isolated); false if the font has none of them.
fn synth_single(p: *plan.Plan, k: usize, out: *mut vec.Vec.[u8]) !bool {
    let mut pairs = vec.Vec.[[2]u32].init();
    defer pairs.deinit();
    let mut u = t.ARABIC_FORMS_FIRST;
    while u <= t.ARABIC_FORMS_LAST {
        let s = @as(u32, t.ARABIC_FORMS[(4 * @as(usize, u - t.ARABIC_FORMS_FIRST)) + k]);
        u += 1;
        if s == 0 {
            continue;
        }
        let ug = p.glyph(u - 1);
        let sg = p.glyph(s);
        if (ug == 0) or (sg == 0) or (ug == sg) or (ug > 0xffff) or (sg > 0xffff) {
            continue;
        }
        pairs.push([ug, sg])!;
    }
    let n = pairs.len;
    if n == 0 {
        return false;
    }
    sort_pairs(pairs.items());
    // Lookup: type 1, IgnoreMarks, one subtable at 8
    put16(out, 1)!;
    put16(out, 8)!;
    put16(out, 1)!;
    put16(out, 8)!;
    // SingleSubstFormat2 (at 8): coverage after the substitutes
    put16(out, 2)!;
    put16(out, 6 + (2 * n))!;
    put16(out, n)!;
    for pr in pairs.items() {
        put16(out, @as(usize, pr[1]))!;
    }
    put16(out, 1)!;
    put16(out, n)!;
    for pr in pairs.items() {
        put16(out, @as(usize, pr[0]))!;
    }
    return true;
}

/// A ligature lookup from rows of n components then the ligature.
fn synth_ligature(p: *plan.Plan, rows: []u16, n: usize, flags: usize, out: *mut vec.Vec.[u8]) !bool {
    let w = n + 1;
    let nrows = rows.len / w;
    // first glyphs (one per group of rows with the same first character),
    // with the index of the group's first row
    let mut firsts = vec.Vec.[[2]u32].init();
    defer firsts.deinit();
    let mut r: usize = 0;
    while r < nrows {
        let first = @as(u32, rows[r * w]);
        let g = p.glyph(first);
        if g != 0 {
            firsts.push([g, @truncate(u32, r)])!;
        }
        let mut e = r + 1;
        while (e < nrows) and (@as(u32, rows[e * w]) == first) {
            e += 1;
        }
        r = e;
    }
    sort_pairs(firsts.items());
    // the ligatures of each first glyph: [ligature glyph, component glyphs...]
    let mut ligs = vec.Vec.[u32].init();
    defer ligs.deinit();
    let mut counts = vec.Vec.[usize].init();
    defer counts.deinit();
    let mut total: usize = 0;
    for fg in firsts.items() {
        let start = @as(usize, fg[1]);
        let first = rows[start * w];
        let mut count: usize = 0;
        let mut k = start;
        while (k < nrows) and (rows[k * w] == first) {
            let lg = p.glyph(@as(u32, rows[(k * w) + n]));
            let mut ok = lg != 0;
            let mut comps: [2]u32 = [0, 0];
            for j in @as(usize, 1)..n {
                let cu = @as(u32, rows[(k * w) + j]);
                let cg = p.glyph(cu);
                if (cu == 0) or (cg == 0) {
                    ok = false;
                }
                comps[j - 1] = cg;
            }
            if ok {
                ligs.push(lg)!;
                for j in @as(usize, 0)..(n - 1) {
                    ligs.push(comps[j])!;
                }
                count += 1;
            }
            k += 1;
        }
        counts.push(count)!;
        total += count;
    }
    if total == 0 {
        return false;
    }
    let nf = firsts.len;
    let base = out.len;
    put16(out, 4)!;
    put16(out, flags)!;
    put16(out, 1)!;
    put16(out, 8)!;
    // LigatureSubstFormat1 at 8: format, coverage, count, set offsets
    let st = base + 8;
    put16(out, 1)!;
    put16(out, 0)!;  // coverage offset, set below
    put16(out, nf)!;
    let sets_at = out.len;
    for _i in @as(usize, 0)..nf {
        put16(out, 0)!;
    }
    let mut li: usize = 0;
    let cs = counts.items();
    let ls = ligs.items();
    for i in @as(usize, 0)..nf {
        let set = out.len;
        set16(out, sets_at + (2 * i), set - st);
        put16(out, cs[i])!;
        let offs_at = out.len;
        for _j in @as(usize, 0)..cs[i] {
            put16(out, 0)!;
        }
        for j in @as(usize, 0)..cs[i] {
            set16(out, offs_at + (2 * j), out.len - set);
            put16(out, @as(usize, ls[li * n]))!;
            put16(out, n)!;
            for c in @as(usize, 1)..n {
                put16(out, @as(usize, ls[(li * n) + c]))!;
            }
            li += 1;
        }
    }
    set16(out, st + 2, out.len - st);
    put16(out, 1)!;
    put16(out, nf)!;
    for fg in firsts.items() {
        put16(out, @as(usize, fg[0]))!;
    }
    return true;
}

let FALLBACK_FEATURES: [7][]u8 = ["init", "medi", "fina", "isol", "rlig", "rlig", "rlig"];

fn synth(p: *plan.Plan, k: usize, out: *mut vec.Vec.[u8]) !bool {
    if k < 4 {
        return synth_single(p, k, out);
    }
    if k == 4 {
        return synth_ligature(p, t.ARABIC_LIGA3[..], 3, 8, out);
    }
    if k == 5 {
        return synth_ligature(p, t.ARABIC_LIGA2[..], 2, 8, out);
    }
    return synth_ligature(p, t.ARABIC_LIGA_MARK[..], 2, 0, out);
}

/// The pause after rlig: applies the synthesized lookups, if the font has
/// no Arabic features.
pub fn fallback_shape(p: *plan.Plan, b: *mut buf.Buffer) void {
    if not p.arabic_do_fallback {
        return;
    }
    fallback_shape_impl(p, b) catch {
        b.oom = true;
        b.successful = false;
    };
}

fn fallback_shape_impl(p: *plan.Plan, b: *mut buf.Buffer) !void {
    let mut lookups: [7]vec.Vec.[u8] = [vec.Vec.[u8].init(); 7];
    defer {
        for i in @as(usize, 0)..7 {
            lookups[i].deinit();
        }
    }
    let mut masks: [7]u32 = [0; 7];
    let mut n: usize = 0;
    for k in @as(usize, 0)..7 {
        let mask = p.map.get_1_mask(plan.tag(FALLBACK_FEATURES[k]));
        if mask == 0 {
            continue;
        }
        if synth(p, k, &mut lookups[n])! {
            masks[n] = mask;
            n += 1;
        } else {
            lookups[n].clear();
        }
    }
    if n == 0 {
        return;
    }
    // the table: a pad, then the LookupList (at 2), then the lookups
    let mut data = vec.Vec.[u8].init();
    defer data.deinit();
    put16(&mut data, 0)!;
    put16(&mut data, n)!;
    let mut at = 2 + (2 * n);
    for i in @as(usize, 0)..n {
        put16(&mut data, at)!;
        at += lookups[i].len;
    }
    for i in @as(usize, 0)..n {
        data.extend(lookups[i].items())!;
    }
    let face = layout.synthetic_face(&p.face, data.items());
    let mut c = layout.ctx_init(&face, b, 0);
    let all = layout.digest_full();
    for i in @as(usize, 0)..n {
        layout.set_lookup(&mut c, masks[i], true, true, false, false);
        layout.apply_string(&mut c, @truncate(u32, i), &all);
    }
}
