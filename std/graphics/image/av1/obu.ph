// =============================================================================
// std/graphics/image/av1/obu.ph - AV1 bitstream headers: OBUs, the sequence header and
// the frame header of key and intra-only frames (as dav1d reads them).
// =============================================================================

import "../../../mem.ph" as mem;

error { InvalidImage, Unsupported, OutOfMemory }

/// A value of T with every byte zero.
pub fn zeroed[T]() !T {
    let p = mem.alloc_zeroed.[T](1)!;
    let v = p[0];
    mem.free(p);
    return v;
}

pub const OBU_SEQ_HDR: u32 = 1;
pub const OBU_TD: u32 = 2;
pub const OBU_FRAME_HDR: u32 = 3;
pub const OBU_TILE_GRP: u32 = 4;
pub const OBU_METADATA: u32 = 5;
pub const OBU_FRAME: u32 = 6;
pub const OBU_REDUNDANT_FRAME_HDR: u32 = 7;
pub const OBU_PADDING: u32 = 15;

/// Pixel layouts.
pub const I400: u32 = 0;
pub const I420: u32 = 1;
pub const I422: u32 = 2;
pub const I444: u32 = 3;

/// Frame types.
pub const KEY: u32 = 0;
pub const INTER: u32 = 1;
pub const INTRA: u32 = 2;
pub const SWITCH: u32 = 3;

/// Restoration types (as signalled, remapped).
pub const RT_NONE: u32 = 0;
pub const RT_SWITCHABLE: u32 = 1;
pub const RT_WIENER: u32 = 2;
pub const RT_SGRPROJ: u32 = 3;

/// Transform modes.
pub const TX_ONLY4X4: u32 = 0;
pub const TX_LARGEST: u32 = 1;
pub const TX_SWITCHABLE: u32 = 2;

pub const MAX_TILE_COLS: usize = 64;
pub const MAX_TILE_ROWS: usize = 64;

// -----------------------------------------------------------------------------
// Bits, most significant first
// -----------------------------------------------------------------------------

pub struct Bits {
    data: []u8,
    pos: usize,  // in bits
    err: bool,

    pub fn init(data: []u8) Bits {
        return Bits.{ data: data, pos: 0, err: false };
    }

    pub fn bit(self: *mut Bits) u32 {
        let i = self.pos >> 3;
        if i >= self.data.len {
            self.err = true;
            self.pos += 1;
            return 0;
        }
        let b = (@as(u32, self.data[i]) >> @truncate(u32, 7 - (self.pos & 7))) & 1;
        self.pos += 1;
        return b;
    }

    pub fn bits(self: *mut Bits, n: u32) u32 {
        let mut v: u32 = 0;
        for _q in @as(u32, 0)..n {
            v = (v << 1) | self.bit();
        }
        return v;
    }

    pub fn flag(self: *mut Bits) bool {
        return self.bit() == 1;
    }

    /// n bits, two's complement
    pub fn sbits(self: *mut Bits, n: u32) i32 {
        let v = self.bits(n);
        let sign = @as(u32, 1) << (n - 1);
        return @bit_cast(i32, (v ^ sign) -% sign);
    }

    pub fn uniform(self: *mut Bits, max: u32) u32 {
        let l = ulog2(max) + 1;
        let m = (@as(u32, 1) << l) - max;
        let v = self.bits(l - 1);
        if v < m {
            return v;
        }
        return ((v << 1) - m) + self.bit();
    }

    pub fn vlc(self: *mut Bits) u32 {
        if self.flag() {
            return 0;
        }
        let mut n: u32 = 0;
        loop {
            n += 1;
            if n == 32 {
                return 0xffffffff;
            }
            if self.flag() {
                break;
            }
        }
        return ((@as(u32, 1) << n) - 1) + self.bits(n);
    }

    pub fn leb128(self: *mut Bits) usize {
        let mut v: usize = 0;
        let mut shift: usize = 0;
        loop {
            let b = self.bits(8);
            v |= @as(usize, b & 0x7f) << shift;
            shift += 7;
            if ((b & 0x80) == 0) or (shift >= 56) {
                break;
            }
        }
        return v;
    }

    pub fn byte_align(self: *mut Bits) void {
        self.pos = (self.pos + 7) & ~@as(usize, 7);
    }

    pub fn byte_pos(self: *Bits) usize {
        return self.pos >> 3;
    }
}

pub fn ulog2(v: u32) u32 {
    let mut n: u32 = 0;
    while (v >> (n + 1)) != 0 {
        n += 1;
    }
    return n;
}

// -----------------------------------------------------------------------------
// The sequence header
// -----------------------------------------------------------------------------

pub struct SeqHdr {
    profile: u32,
    still_picture: bool,
    reduced: bool,  // reduced still picture header
    width_bits: u32,
    height_bits: u32,
    max_width: u32,
    max_height: u32,
    frame_id_numbers: bool,
    frame_id_bits: u32,
    sb128: bool,
    filter_intra: bool,
    intra_edge_filter: bool,
    screen_content_tools: u32,  // 0 off, 1 on, 2 per frame
    force_integer_mv: u32,
    order_hint: bool,
    order_hint_bits: u32,
    super_res: bool,
    cdef: bool,
    restoration: bool,
    bitdepth: u32,
    mono: bool,
    layout: u32,
    ss_hor: u32,
    ss_ver: u32,
    color_description: bool,
    primaries: u32,
    transfer: u32,
    matrix: u32,
    full_range: bool,
    separate_uv_delta_q: bool,
    film_grain: bool,
    decoder_model_info: bool,
    equal_picture_interval: bool,
    frame_presentation_delay_length: u32,
    buffer_removal_delay_length: u32,
    num_operating_points: u32,
    op_idc: [32]u32,
    op_decoder_model: [32]bool,
}

pub fn parse_seq_hdr(b: *mut Bits) !SeqHdr {
    let mut s = zeroed.[SeqHdr]()!;
    s.profile = b.bits(3);
    if s.profile > 2 {
        return error.InvalidImage;
    }
    s.still_picture = b.flag();
    s.reduced = b.flag();
    if s.reduced and not s.still_picture {
        return error.InvalidImage;
    }
    let mut delay_length: u32 = 0;
    if s.reduced {
        s.num_operating_points = 1;
        _ = b.bits(5);  // level
    } else {
        if b.flag() {
            // timing info
            _ = b.bits(32);
            _ = b.bits(32);
            s.equal_picture_interval = b.flag();
            if s.equal_picture_interval {
                _ = b.vlc();
            }
            s.decoder_model_info = b.flag();
            if s.decoder_model_info {
                delay_length = b.bits(5) + 1;
                _ = b.bits(32);
                s.buffer_removal_delay_length = b.bits(5) + 1;
                s.frame_presentation_delay_length = b.bits(5) + 1;
            }
        }
        let display_model = b.flag();
        s.num_operating_points = b.bits(5) + 1;
        for i in @as(usize, 0)..@as(usize, s.num_operating_points) {
            s.op_idc[i] = b.bits(12);
            let major = 2 + b.bits(3);
            _ = b.bits(2);
            if major > 3 {
                _ = b.bit();  // tier
            }
            if s.decoder_model_info {
                s.op_decoder_model[i] = b.flag();
                if s.op_decoder_model[i] {
                    _ = b.bits(delay_length);
                    _ = b.bits(delay_length);
                    _ = b.bit();
                }
            }
            if display_model {
                if b.flag() {
                    _ = b.bits(4);
                }
            }
        }
    }
    s.width_bits = b.bits(4) + 1;
    s.height_bits = b.bits(4) + 1;
    s.max_width = b.bits(s.width_bits) + 1;
    s.max_height = b.bits(s.height_bits) + 1;
    if not s.reduced {
        s.frame_id_numbers = b.flag();
        if s.frame_id_numbers {
            let delta = b.bits(4) + 2;
            s.frame_id_bits = b.bits(3) + delta + 1;
        }
    }
    s.sb128 = b.flag();
    s.filter_intra = b.flag();
    s.intra_edge_filter = b.flag();
    if s.reduced {
        s.screen_content_tools = 2;
        s.force_integer_mv = 2;
    } else {
        _ = b.bits(4);  // inter-intra, masked compound, warped motion, dual filter
        s.order_hint = b.flag();
        if s.order_hint {
            _ = b.bits(2);  // jnt_comp, ref_frame_mvs
        }
        if b.flag() {
            s.screen_content_tools = 2;
        } else {
            s.screen_content_tools = b.bit();
        }
        s.force_integer_mv = 2;
        if s.screen_content_tools != 0 {
            if b.flag() {
                s.force_integer_mv = 2;
            } else {
                s.force_integer_mv = b.bit();
            }
        }
        if s.order_hint {
            s.order_hint_bits = b.bits(3) + 1;
        }
    }
    s.super_res = b.flag();
    s.cdef = b.flag();
    s.restoration = b.flag();
    let mut hbd = b.bit();
    if (s.profile == 2) and (hbd != 0) {
        hbd += b.bit();
    }
    s.bitdepth = 8 + (2 * hbd);
    if s.profile != 1 {
        s.mono = b.flag();
    }
    s.color_description = b.flag();
    s.primaries = 2;
    s.transfer = 2;
    s.matrix = 2;
    if s.color_description {
        s.primaries = b.bits(8);
        s.transfer = b.bits(8);
        s.matrix = b.bits(8);
    }
    if s.mono {
        s.full_range = b.flag();
        s.layout = I400;
        s.ss_hor = 1;
        s.ss_ver = 1;
    } else if (s.primaries == 1) and (s.transfer == 13) and (s.matrix == 0) {
        s.layout = I444;
        s.full_range = true;
    } else {
        s.full_range = b.flag();
        if s.profile == 0 {
            s.layout = I420;
            s.ss_hor = 1;
            s.ss_ver = 1;
        } else if s.profile == 1 {
            s.layout = I444;
        } else {
            if hbd == 2 {
                s.ss_hor = b.bit();
                if s.ss_hor != 0 {
                    s.ss_ver = b.bit();
                }
            } else {
                s.ss_hor = 1;
            }
            s.layout = I444;
            if s.ss_hor != 0 {
                s.layout = I422;
                if s.ss_ver != 0 {
                    s.layout = I420;
                }
            }
        }
        if (s.ss_hor & s.ss_ver) != 0 {
            _ = b.bits(2);  // chroma sample position
        }
    }
    if not s.mono {
        s.separate_uv_delta_q = b.flag();
    }
    s.film_grain = b.flag();
    if b.err {
        return error.InvalidImage;
    }
    return s;
}

// -----------------------------------------------------------------------------
// The frame header
// -----------------------------------------------------------------------------

pub struct FrameHdr {
    frame_type: u32,
    show_frame: bool,
    showable_frame: bool,
    error_resilient: bool,
    disable_cdf_update: bool,
    allow_screen_content_tools: bool,
    force_integer_mv: bool,
    width: u32,  // coded (before super-resolution)
    upscaled_width: u32,
    height: u32,
    super_res: bool,
    allow_intrabc: bool,
    refresh_context: bool,
    // tiling
    tile_cols: u32,
    tile_rows: u32,
    log2_cols: u32,
    log2_rows: u32,
    col_start_sb: [65]u32,
    row_start_sb: [65]u32,
    tile_n_bytes: u32,
    context_update_tile: u32,
    // quantization
    yac: u32,
    ydc_delta: i32,
    udc_delta: i32,
    uac_delta: i32,
    vdc_delta: i32,
    vac_delta: i32,
    qm: bool,
    qm_y: u32,
    qm_u: u32,
    qm_v: u32,
    // segmentation
    seg_enabled: bool,
    seg_update_map: bool,
    seg_temporal: bool,
    seg_delta_q: [8]i32,
    seg_delta_lf: [32]i32,  // [segment][y vertical, y horizontal, u, v]
    seg_ref: [8]i32,
    seg_skip: [8]bool,
    seg_globalmv: [8]bool,
    seg_preskip: bool,
    seg_last_active: i32,
    seg_qidx: [8]u32,
    lossless: [8]bool,
    all_lossless: bool,
    // deltas
    delta_q_present: bool,
    delta_q_res_log2: u32,
    delta_lf_present: bool,
    delta_lf_res_log2: u32,
    delta_lf_multi: bool,
    // loop filter
    lf_level_y: [2]u32,
    lf_level_u: u32,
    lf_level_v: u32,
    lf_sharpness: u32,
    lf_mode_ref_delta_enabled: bool,
    lf_ref_delta: [8]i32,
    lf_mode_delta: [2]i32,
    // cdef
    cdef_damping: u32,
    cdef_bits: u32,
    cdef_y: [8]u32,
    cdef_uv: [8]u32,
    // loop restoration
    lr_type: [3]u32,
    lr_unit_size: [2]u32,  // log2
    txfm_mode: u32,
    reduced_txtp_set: bool,
    film_grain: bool,
}

fn tile_log2(sz: u32, tgt: u32) u32 {
    let mut k: u32 = 0;
    while (sz << k) < tgt {
        k += 1;
    }
    return k;
}

fn delta_q(b: *mut Bits) i32 {
    if b.flag() {
        return b.sbits(7);
    }
    return 0;
}

/// A key or intra-only frame's header.
pub fn parse_frame_hdr(s: *SeqHdr, b: *mut Bits) !FrameHdr {
    let mut h = zeroed.[FrameHdr]()!;
    if not s.reduced {
        if b.flag() {
            return error.Unsupported;  // show an existing frame
        }
        h.frame_type = b.bits(2);
        h.show_frame = b.flag();
    } else {
        h.frame_type = KEY;
        h.show_frame = true;
    }
    if (h.frame_type != KEY) and (h.frame_type != INTRA) {
        return error.Unsupported;  // (inter frames: not for still images)
    }
    if h.show_frame {
        if s.decoder_model_info and not s.equal_picture_interval {
            _ = b.bits(s.frame_presentation_delay_length);
        }
    } else {
        h.showable_frame = b.flag();
    }
    h.error_resilient = s.reduced or ((h.frame_type == KEY) and h.show_frame);
    if not h.error_resilient {
        h.error_resilient = b.flag();
    }
    h.disable_cdf_update = b.flag();
    if s.screen_content_tools == 2 {
        h.allow_screen_content_tools = b.flag();
    } else {
        h.allow_screen_content_tools = s.screen_content_tools == 1;
    }
    if h.allow_screen_content_tools and (s.force_integer_mv == 2) {
        _ = b.bit();
    }
    h.force_integer_mv = true;
    if s.frame_id_numbers {
        _ = b.bits(s.frame_id_bits);
    }
    let mut size_override = false;
    if not s.reduced {
        size_override = b.flag();
    }
    if s.order_hint {
        _ = b.bits(s.order_hint_bits);
    }
    if s.decoder_model_info {
        if b.flag() {
            for i in @as(usize, 0)..@as(usize, s.num_operating_points) {
                if s.op_decoder_model[i] {
                    let idc = s.op_idc[i];
                    if (idc == 0) or (((idc & 1) != 0) and (((idc >> 8) & 1) != 0)) {
                        _ = b.bits(s.buffer_removal_delay_length);
                    }
                }
            }
        }
    }
    let mut refresh: u32 = 0xff;
    if not ((h.frame_type == KEY) and h.show_frame) {
        refresh = b.bits(8);
    }
    if (refresh != 0xff) and h.error_resilient and s.order_hint {
        for _q in @as(u32, 0)..8 {
            _ = b.bits(s.order_hint_bits);
        }
    }
    // frame size
    h.upscaled_width = s.max_width;
    h.height = s.max_height;
    if size_override {
        h.upscaled_width = b.bits(s.width_bits) + 1;
        h.height = b.bits(s.height_bits) + 1;
    }
    h.width = h.upscaled_width;
    if s.super_res and b.flag() {
        return error.Unsupported;  // super-resolution
    }
    if b.flag() {
        _ = b.bits(32);  // render size
    }
    h.allow_intrabc = h.allow_screen_content_tools and b.flag();
    h.refresh_context = (not s.reduced) and (not h.disable_cdf_update) and not b.flag();
    parse_tiling(s, &mut h, b)!;
    // quantization
    h.yac = b.bits(8);
    h.ydc_delta = delta_q(b);
    if not s.mono {
        let mut diff_uv = false;
        if s.separate_uv_delta_q {
            diff_uv = b.flag();
        }
        h.udc_delta = delta_q(b);
        h.uac_delta = delta_q(b);
        h.vdc_delta = h.udc_delta;
        h.vac_delta = h.uac_delta;
        if diff_uv {
            h.vdc_delta = delta_q(b);
            h.vac_delta = delta_q(b);
        }
    }
    h.qm = b.flag();
    if h.qm {
        h.qm_y = b.bits(4);
        h.qm_u = b.bits(4);
        h.qm_v = h.qm_u;
        if s.separate_uv_delta_q {
            h.qm_v = b.bits(4);
        }
    }
    // segmentation (no reference frame, so everything is sent)
    for i in @as(usize, 0)..8 {
        h.seg_ref[i] = -1;
    }
    h.seg_last_active = -1;
    h.seg_enabled = b.flag();
    if h.seg_enabled {
        h.seg_update_map = true;
        for i in @as(usize, 0)..8 {
            if b.flag() {
                h.seg_delta_q[i] = b.sbits(9);
                h.seg_last_active = @as(i32, @truncate(u8, i));
            }
            for k in @as(usize, 0)..4 {
                if b.flag() {
                    h.seg_delta_lf[(i * 4) + k] = b.sbits(7);
                    h.seg_last_active = @as(i32, @truncate(u8, i));
                }
            }
            if b.flag() {
                h.seg_ref[i] = @bit_cast(i32, b.bits(3));
                h.seg_last_active = @as(i32, @truncate(u8, i));
                h.seg_preskip = true;
            }
            if b.flag() {
                h.seg_skip[i] = true;
                h.seg_last_active = @as(i32, @truncate(u8, i));
                h.seg_preskip = true;
            }
            if b.flag() {
                h.seg_globalmv[i] = true;
                h.seg_last_active = @as(i32, @truncate(u8, i));
                h.seg_preskip = true;
            }
        }
    }
    let delta_lossless = (h.ydc_delta == 0) and (h.udc_delta == 0) and (h.uac_delta == 0) and (h.vdc_delta == 0) and
                         (h.vac_delta == 0);
    h.all_lossless = true;
    for i in @as(usize, 0)..8 {
        let mut q = @bit_cast(i32, h.yac);
        if h.seg_enabled {
            q = @max(0, @min(255, q + h.seg_delta_q[i]));
        }
        h.seg_qidx[i] = @bit_cast(u32, q);
        h.lossless[i] = (q == 0) and delta_lossless;
        if not h.lossless[i] {
            h.all_lossless = false;
        }
    }
    // delta q and delta lf
    if h.yac != 0 {
        h.delta_q_present = b.flag();
    }
    if h.delta_q_present {
        h.delta_q_res_log2 = b.bits(2);
        if not h.allow_intrabc {
            h.delta_lf_present = b.flag();
        }
        if h.delta_lf_present {
            h.delta_lf_res_log2 = b.bits(2);
            h.delta_lf_multi = b.flag();
        }
    }
    // loop filter
    let defaults: [8]i32 = [1, 0, 0, 0, -1, 0, -1, -1];
    h.lf_ref_delta = defaults;
    if h.all_lossless or h.allow_intrabc {
        h.lf_mode_ref_delta_enabled = true;
    } else {
        h.lf_level_y[0] = b.bits(6);
        h.lf_level_y[1] = b.bits(6);
        if (not s.mono) and ((h.lf_level_y[0] != 0) or (h.lf_level_y[1] != 0)) {
            h.lf_level_u = b.bits(6);
            h.lf_level_v = b.bits(6);
        }
        h.lf_sharpness = b.bits(3);
        h.lf_mode_ref_delta_enabled = b.flag();
        if h.lf_mode_ref_delta_enabled and b.flag() {
            for i in @as(usize, 0)..8 {
                if b.flag() {
                    h.lf_ref_delta[i] = b.sbits(7);
                }
            }
            for i in @as(usize, 0)..2 {
                if b.flag() {
                    h.lf_mode_delta[i] = b.sbits(7);
                }
            }
        }
    }
    // cdef
    if (not h.all_lossless) and s.cdef and not h.allow_intrabc {
        h.cdef_damping = b.bits(2) + 3;
        h.cdef_bits = b.bits(2);
        for i in @as(usize, 0)..(@as(usize, 1) << @as(usize, h.cdef_bits)) {
            h.cdef_y[i] = b.bits(6);
            if not s.mono {
                h.cdef_uv[i] = b.bits(6);
            }
        }
    }
    // loop restoration
    if (not h.all_lossless) and s.restoration and not h.allow_intrabc {
        h.lr_type[0] = b.bits(2);
        if not s.mono {
            h.lr_type[1] = b.bits(2);
            h.lr_type[2] = b.bits(2);
        }
        if (h.lr_type[0] != RT_NONE) or (h.lr_type[1] != RT_NONE) or (h.lr_type[2] != RT_NONE) {
            let mut u: u32 = 6;
            if s.sb128 {
                u = 7;
            }
            if b.flag() {
                u += 1;
                if not s.sb128 {
                    u += b.bit();
                }
            }
            h.lr_unit_size[0] = u;
            h.lr_unit_size[1] = u;
            if ((h.lr_type[1] != RT_NONE) or (h.lr_type[2] != RT_NONE)) and (s.ss_hor == 1) and (s.ss_ver == 1) {
                h.lr_unit_size[1] = u - b.bit();
            }
        } else {
            h.lr_unit_size[0] = 8;
        }
    }
    if h.all_lossless {
        h.txfm_mode = TX_ONLY4X4;
    } else if b.flag() {
        h.txfm_mode = TX_SWITCHABLE;
    } else {
        h.txfm_mode = TX_LARGEST;
    }
    h.reduced_txtp_set = b.flag();
    if s.film_grain and (h.show_frame or h.showable_frame) and b.flag() {
        h.film_grain = true;  // (read, not applied)
        skip_film_grain(s, b);
    }
    if b.err {
        return error.InvalidImage;
    }
    return h;
}

fn parse_tiling(s: *SeqHdr, h: *mut FrameHdr, b: *mut Bits) !void {
    let uniform = b.flag();
    let mut sb_log2: u32 = 6;
    if s.sb128 {
        sb_log2 = 7;
    }
    let sbw = (h.width + ((@as(u32, 1) << sb_log2) - 1)) >> sb_log2;
    let sbh = (h.height + ((@as(u32, 1) << sb_log2) - 1)) >> sb_log2;
    let max_tile_width_sb = @as(u32, 4096) >> sb_log2;
    let mut max_tile_area_sb = (@as(u32, 4096) * 2304) >> (2 * sb_log2);
    let min_log2_cols = tile_log2(max_tile_width_sb, sbw);
    let max_log2_cols = tile_log2(1, @min(sbw, 64));
    let max_log2_rows = tile_log2(1, @min(sbh, 64));
    let min_log2_tiles = @max(tile_log2(max_tile_area_sb, sbw * sbh), min_log2_cols);
    let mut cols: u32 = 0;
    let mut rows: u32 = 0;
    if uniform {
        h.log2_cols = min_log2_cols;
        while (h.log2_cols < max_log2_cols) and b.flag() {
            h.log2_cols += 1;
        }
        let tile_w = 1 + ((sbw - 1) >> h.log2_cols);
        let mut x: u32 = 0;
        while x < sbw {
            h.col_start_sb[@as(usize, cols)] = x;
            x += tile_w;
            cols += 1;
        }
        let mut min_log2_rows: u32 = 0;
        if min_log2_tiles > h.log2_cols {
            min_log2_rows = min_log2_tiles - h.log2_cols;
        }
        h.log2_rows = min_log2_rows;
        while (h.log2_rows < max_log2_rows) and b.flag() {
            h.log2_rows += 1;
        }
        let tile_h = 1 + ((sbh - 1) >> h.log2_rows);
        let mut y: u32 = 0;
        while y < sbh {
            h.row_start_sb[@as(usize, rows)] = y;
            y += tile_h;
            rows += 1;
        }
    } else {
        let mut widest: u32 = 0;
        let mut x: u32 = 0;
        while (x < sbw) and (cols < 64) {
            let tw_sb = @min(sbw - x, max_tile_width_sb);
            let mut tw: u32 = 1;
            if tw_sb > 1 {
                tw = 1 + b.uniform(tw_sb);
            }
            h.col_start_sb[@as(usize, cols)] = x;
            x += tw;
            widest = @max(widest, tw);
            cols += 1;
        }
        h.log2_cols = tile_log2(1, cols);
        if min_log2_tiles != 0 {
            max_tile_area_sb >>= min_log2_tiles + 1;
        } else {
            max_tile_area_sb = sbw * sbh;
        }
        let max_tile_height_sb = @max(max_tile_area_sb / widest, 1);
        let mut y: u32 = 0;
        while (y < sbh) and (rows < 64) {
            let th_sb = @min(sbh - y, max_tile_height_sb);
            let mut th: u32 = 1;
            if th_sb > 1 {
                th = 1 + b.uniform(th_sb);
            }
            h.row_start_sb[@as(usize, rows)] = y;
            y += th;
            rows += 1;
        }
        h.log2_rows = tile_log2(1, rows);
    }
    h.col_start_sb[@as(usize, cols)] = sbw;
    h.row_start_sb[@as(usize, rows)] = sbh;
    h.tile_cols = cols;
    h.tile_rows = rows;
    if (h.log2_cols != 0) or (h.log2_rows != 0) {
        h.context_update_tile = b.bits(h.log2_cols + h.log2_rows);
        if h.context_update_tile >= (cols * rows) {
            return error.InvalidImage;
        }
        h.tile_n_bytes = b.bits(2) + 1;
    }
}

fn skip_film_grain(s: *SeqHdr, b: *mut Bits) void {
    _ = b.bits(16);  // seed
    // (a key frame always updates)
    let ny = b.bits(4);
    for _q in @as(u32, 0)..ny {
        _ = b.bits(16);
    }
    let mut from_luma = false;
    if not s.mono {
        from_luma = b.flag();
    }
    let mut nuv: [2]u32 = [0, 0];
    if not (s.mono or from_luma or ((s.ss_ver == 1) and (s.ss_hor == 1) and (ny == 0))) {
        for pl in @as(usize, 0)..2 {
            nuv[pl] = b.bits(4);
            for _q in @as(u32, 0)..nuv[pl] {
                _ = b.bits(16);
            }
        }
    }
    _ = b.bits(2);  // scaling shift
    let lag = b.bits(2);
    let npos = 2 * lag * (lag + 1);
    if ny != 0 {
        for _q in @as(u32, 0)..npos {
            _ = b.bits(8);
        }
    }
    for pl in @as(usize, 0)..2 {
        if (nuv[pl] != 0) or from_luma {
            let mut n = npos;
            if ny != 0 {
                n += 1;
            }
            for _q in @as(u32, 0)..n {
                _ = b.bits(8);
            }
        }
    }
    _ = b.bits(4);  // ar shift, grain scale shift
    for pl in @as(usize, 0)..2 {
        if nuv[pl] != 0 {
            _ = b.bits(25);
        }
    }
    _ = b.bits(2);  // overlap, clip
}
