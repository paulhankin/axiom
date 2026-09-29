// =============================================================================
// std/mem.ph - the global allocator (spec §12.4, §15.1).
//
//     let xs = mem.alloc.[u32](1000)!;   // []mut u32, contents unspecified
//     defer mem.free(xs);
//     let p = mem.create(Point.{ x: 1, y: 2 })!;   // *mut Point
//     mem.destroy(p);
//
// Every allocation in a Phalange program comes from here. Memory is taken
// from the system with @memory_grow (linear memory on every target) and is
// managed in two ways:
//
// - Small objects (up to 8 KiB) are rounded up to one of 32 size classes.
//   A span is a 64 KiB-aligned block holding objects of one class, found
//   from an object's address by masking. A span hands out objects from its
//   free list, then from the part never used. Each heap has, per class, the
//   span it allocates from and a list of spans with free objects.
// - Large objects are blocks of whole 4 KiB units with a header in front.
//   Free blocks are kept in size bins (with a bitmap of the non-empty ones)
//   and merged with free neighbours (boundary tags); a block next to the
//   never-used end of memory (the wilderness) goes back into it, and a
//   block there can grow in place (realloc). Past a budget of free memory,
//   big free blocks are given back to the system (@memory_discard).
//
// Until a second thread starts (enable_threads, called by std/thread), there
// is one heap and no locking. Then there are 32 heaps, each with a lock; a
// thread uses the heap its stack address hashes to, and frees go to the heap
// that owns the span. Blocks and spans come from one pool with its own lock.
//
// Sizes are passed to free (a slice knows its length), so objects carry no
// header. Freeing memory that did not come from here traps.
// =============================================================================

error { OutOfMemory }

const PAGE: usize = 65536;  // a span; the unit of @memory_grow
const UNIT: usize = 4096;   // large blocks are multiples of this
const HDR: usize = 128;     // the header at the start of every span and block
const SMALL_MAX: usize = 8192;
const NCLS: usize = 32;
const NHEAPS: usize = 32;
const NBINS: usize = 160;
const GROW_MIN: usize = 16;          // pages taken from the system at a time
const SPAN_CACHE: usize = 32;        // empty spans kept for reuse
const RETAIN: usize = 64 << 20;      // free bytes kept before giving memory back
const DISCARD_MIN: usize = 1 << 20;  // the smallest free block given back

const MAGIC_SPAN: u64 = 0x5350_414E_9E37_79B9;
const MAGIC_USED: u64 = 0x5553_4544_7F4A_7C15;
const MAGIC_FREE: u64 = 0x4652_4545_85EB_CA6B;

// span states
const ACTIVE: u32 = 1;   // a heap allocates from it
const PARTIAL: u32 = 2;  // in its heap's list of spans with free objects
const FULL: u32 = 3;     // no free objects, in no list
const CACHED: u32 = 4;   // empty, in the pool's span cache

let class_size: [32]u32 = [16, 32, 48, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 384, 448, 512, 640, 768, 896,
                           1024, 1280, 1536, 1792, 2048, 2560, 3072, 3584, 4096, 5120, 6144, 7168, 8192];

/// The header of a span (small objects). The first three fields are shared
/// with Block.
struct Span {
    magic: u64,        // MAGIC_SPAN ^ address
    size: usize,       // PAGE
    prev_free: usize,  // 1: the block before is free
    next: usize,       // links in a heap's partial list or the span cache
    prev: usize,
    free: usize,  // freed objects, each holding the address of the next
    bump: usize,  // the first object never handed out
    end: usize,   // the end of the objects
    osize: u32,
    used: u32,  // objects handed out and not freed
    cls: u32,
    heap: u32,
    state: u32,
}

/// The header of a large block, used or free. A free block also has its size
/// in its last word, so the block after it can find its start.
struct Block {
    magic: u64,        // MAGIC_USED or MAGIC_FREE ^ address
    size: usize,       // bytes, header included; a multiple of UNIT
    prev_free: usize,  // 1: the block before is free
    next: usize,       // bin links (free blocks)
    prev: usize,
}

// class_of for sizes up to 1024, by (size + 15) / 16
let small_class: [65]u8 = [0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 12, 12, 13, 13, 13, 13, 14,
                           14, 14, 14, 15, 15, 15, 15, 16, 16, 16, 16, 16, 16, 16, 16, 17, 17, 17, 17, 17, 17, 17, 17,
                           18, 18, 18, 18, 18, 18, 18, 18, 19, 19, 19, 19, 19, 19, 19, 19];

// -----------------------------------------------------------------------------
// State
// -----------------------------------------------------------------------------
let mut multi: bool = false;
let mut heap_lock: [32]u32 = [0; 32];
let mut active: [1024]usize = [0; 1024];  // [heap * NCLS + class]
let mut partial: [1024]usize = [0; 1024];
let mut heap_bytes: [32]usize = [0; 32];  // bytes of small objects in use, per heap

let mut pool_lock: u32 = 0;
let mut wild_lo: usize = 0;  // [wild_lo, wild_hi): memory never used
let mut wild_hi: usize = 0;
// [clean, wild_hi) has never been handed out, so it is still zero (as the
// system gives it); [wild_lo, clean) is memory freed back to the wilderness.
let mut clean: usize = 0;
// where the fresh (zero) part of the last block_alloc starts
let mut fresh_at: usize = 0;
let mut bins: [160]usize = [0; 160];
let mut bin_bits: [3]u64 = [0; 3];
let mut span_cache: usize = 0;
let mut span_cached: usize = 0;
let mut large_bytes: usize = 0;  // bytes of large blocks in use
let mut retained: usize = 0;     // free block bytes not given back

fn span(s: usize) *mut Span {
    return @ptr_from_int(*mut Span, s);
}

fn block(s: usize) *mut Block {
    return @ptr_from_int(*mut Block, s);
}

fn word(a: usize) *mut usize {
    return @ptr_from_int(*mut usize, a);
}

fn magic_at(a: usize) u64 {
    return @ptr_from_int(*u64, a).*;
}

fn tag(m: u64, a: usize) u64 {
    return m ^ @as(u64, a);
}

// -----------------------------------------------------------------------------
// Locks: 0 free, 1 held, 2 held with waiters.
// -----------------------------------------------------------------------------
fn lock(l: *mut u32) void {
    if @atomic_cas(l, 0, 1) == 0 {
        return;
    }
    let mut i: u32 = 0;
    while i < 64 {
        if (@atomic_load(l) == 0) and (@atomic_cas(l, 0, 1) == 0) {
            return;
        }
        i += 1;
    }
    while @atomic_xchg(l, 2) != 0 {
        _ = @atomic_wait(l, 2, -1);
    }
}

fn unlock(l: *mut u32) void {
    if @atomic_xchg(l, 0) == 2 {
        _ = @atomic_notify(l, 1);
    }
}

fn pool_enter() void {
    if multi {
        lock(&mut pool_lock);
    }
}

fn pool_leave() void {
    if multi {
        unlock(&mut pool_lock);
    }
}

/// Called before the program starts its second thread (std/thread does).
/// From then on the allocator locks, and threads use separate heaps.
pub fn enable_threads() void {
    multi = true;
}

// The heap for the calling thread: threads have separate stacks.
fn heap_index() usize {
    if not multi {
        return 0;
    }
    let marker: u32 = 0;
    let a = @as(u64, @int_from_ptr(&marker));
    return @truncate(usize, ((a >> 20) *% 0x9E37_79B9_7F4A_7C15) >> 59);
}

// -----------------------------------------------------------------------------
// Size classes
// -----------------------------------------------------------------------------
fn log2(x: usize) usize {
    return 63 - @truncate(usize, @clz(@as(u64, x)));
}

// The class of 1 <= size <= SMALL_MAX: 16-byte steps to 128, then four
// classes per doubling.
fn class_of(size: usize) usize {
    if size <= 1024 {
        return @as(usize, small_class[(size + 15) >> 4]);
    }
    let x = size - 1;
    let b = 63 - @truncate(usize, @clz(@as(u64, x)));
    return (8 + ((b - 7) * 4) + (x >> (b - 2))) - 4;
}

// -----------------------------------------------------------------------------
// The pool: memory from the system, large blocks, empty spans
// -----------------------------------------------------------------------------
fn bin_of(units: usize) usize {
    if units < 64 {
        return units;
    }
    let b = log2(units);
    return 64 + ((b - 6) * 4) + ((units >> (b - 2)) & 3);
}

fn bin_insert(s: usize, size: usize) void {
    let bi = bin_of(size / UNIT);
    let b = block(s);
    b.magic = tag(MAGIC_FREE, s);
    b.size = size;
    b.prev = 0;
    b.next = bins[bi];
    if b.next != 0 {
        block(b.next).prev = s;
    }
    bins[bi] = s;
    bin_bits[bi >> 6] |= @as(u64, 1) << @truncate(u32, bi & 63);
    word((s + size) - @size_of(usize)).* = size;
    set_prev_free(s + size, 1);
}

fn bin_remove(s: usize) void {
    let b = block(s);
    let bi = bin_of(b.size / UNIT);
    if b.prev != 0 {
        block(b.prev).next = b.next;
    } else {
        bins[bi] = b.next;
        if b.next == 0 {
            bin_bits[bi >> 6] &= ~(@as(u64, 1) << @truncate(u32, bi & 63));
        }
    }
    if b.next != 0 {
        block(b.next).prev = b.prev;
    }
    set_prev_free(s + b.size, 0);
}

// Marks whether the block before the one at `a` is free (if a block starts
// there: a used block or a span; the wilderness and memory from elsewhere
// have no header).
fn set_prev_free(a: usize, v: usize) void {
    if a == wild_lo {
        return;
    }
    let m = magic_at(a);
    if (m == tag(MAGIC_USED, a)) or (m == tag(MAGIC_SPAN, a)) {
        block(a).prev_free = v;
    }
}

// A free block of at least `size` bytes, taken out of its bin (0: none).
fn take_free(size: usize) usize {
    let units = size / UNIT;
    let first = bin_of(units);
    // the first bin can hold smaller blocks (above 63 units)
    let mut s = bins[first];
    while s != 0 {
        if block(s).size >= size {
            bin_remove(s);
            return s;
        }
        s = block(s).next;
    }
    let mut bi = first + 1;
    while bi < NBINS {
        let w = bin_bits[bi >> 6] >> @truncate(u32, bi & 63);
        if w == 0 {
            bi = (bi | 63) + 1;
            continue;
        }
        bi += @truncate(usize, @ctz(w));
        let t = bins[bi];
        bin_remove(t);
        return t;
    }
    return 0;
}

// Makes [s, s + size) a free block: merged with free neighbours, returned to
// the wilderness if it borders it, else put in a bin.
fn release_block(s0: usize, size0: usize) void {
    block(s0).magic = 0;  // no longer a used block (a second free traps)
    let mut s = s0;
    let mut size = size0;
    let n = s + size;
    if (n != wild_lo) and (magic_at(n) == tag(MAGIC_FREE, n)) {
        let ns = block(n).size;
        bin_remove(n);
        took(ns);
        size += ns;
    }
    if block(s).prev_free != 0 {
        let ps = word(s - @size_of(usize)).*;
        bin_remove(s - ps);
        took(ps);
        s -= ps;
        size += ps;
    }
    if (s + size) == wild_lo {
        wild_lo = s;
        return;
    }
    block(s).prev_free = 0;
    retained += size;
    if (size >= DISCARD_MIN) and (retained > RETAIN) {
        @memory_discard(s + UNIT, size - (2 * UNIT));
        retained -= size;
    }
    bin_insert(s, size);
}

fn took(size: usize) void {
    if retained >= size {
        retained -= size;
    } else {
        retained = 0;
    }
}

// Takes `pages` more pages from the system; false if there are none.
fn grow(pages: usize) bool {
    let n = @max(pages, GROW_MIN);
    let prev = @memory_grow(n) catch return false;
    let base = prev * PAGE;
    if base != wild_hi {
        // not next to the wilderness (memory was grown elsewhere): what is
        // left of it becomes a free block
        if wild_hi > wild_lo {
            let rest_at = wild_lo;
            let rest = wild_hi - wild_lo;
            wild_lo = base;
            block(rest_at).prev_free = 0;
            bin_insert(rest_at, rest);
            retained += rest;
        }
        wild_lo = base;
        clean = base;
    }
    wild_hi = base + (n * PAGE);
    return true;
}

// A used block of at least `size` bytes (a multiple of UNIT), or 0.
fn block_alloc(size: usize) usize {
    let mut s = take_free(size);
    if s != 0 {
        fresh_at = s + size;  // (none of it is known to be zero)
        took(block(s).size);
        let rest = block(s).size - size;
        if rest >= UNIT {
            let t = s + size;
            block(t).prev_free = 0;
            block(s).size = size;
            bin_insert(t, rest);
        }
    } else {
        if (wild_hi - wild_lo) < size {
            let need = (size - (wild_hi - wild_lo)) + PAGE - 1;
            if not grow(need / PAGE) {
                return 0;
            }
            if (wild_hi - wild_lo) < size {
                if not grow((size + PAGE - 1) / PAGE) {
                    return 0;
                }
            }
        }
        s = wild_lo;
        wild_lo += size;
        fresh_at = @max(s, clean);
        clean = @max(clean, wild_lo);
        let b = block(s);
        b.size = size;
        b.prev_free = 0;
    }
    block(s).magic = tag(MAGIC_USED, s);
    set_prev_free(s + block(s).size, 0);
    return s;
}

// An empty 64 KiB-aligned span, or 0. Its prev_free is kept up to date.
fn span_take() usize {
    if span_cache != 0 {
        let s = span_cache;
        span_cache = span(s).next;
        span_cached -= 1;
        return s;
    }
    let mut a = (wild_lo + PAGE - 1) & ~(PAGE - 1);
    if (a + PAGE) > wild_hi {
        if not grow(2) {
            return 0;
        }
        a = (wild_lo + PAGE - 1) & ~(PAGE - 1);
    }
    let gap = a - wild_lo;
    if gap > 0 {
        block(wild_lo).prev_free = 0;
        let g = wild_lo;
        wild_lo = a;
        bin_insert(g, gap);
        retained += gap;
    }
    wild_lo = a + PAGE;
    clean = @max(clean, wild_lo);
    let sp = span(a);
    sp.magic = tag(MAGIC_SPAN, a);
    sp.size = PAGE;
    sp.prev_free = 0;
    if gap > 0 {
        sp.prev_free = 1;
    }
    return a;
}

fn span_give(s: usize) void {
    if span_cached < SPAN_CACHE {
        let sp = span(s);
        sp.state = CACHED;
        sp.next = span_cache;
        span_cache = s;
        span_cached += 1;
        return;
    }
    block(s).magic = tag(MAGIC_USED, s);  // (a block now, about to be freed)
    release_block(s, PAGE);
}

// -----------------------------------------------------------------------------
// Small objects
// -----------------------------------------------------------------------------
fn partial_remove(i: usize, s: usize) void {
    let sp = span(s);
    if sp.prev != 0 {
        span(sp.prev).next = sp.next;
    } else {
        partial[i] = sp.next;
    }
    if sp.next != 0 {
        span(sp.next).prev = sp.prev;
    }
}

// The heap's current span for class c has no free object: finds one that
// has, and allocates from it. 0 if memory is exhausted.
fn refill(h: usize, c: usize) usize {
    let i = (h * NCLS) + c;
    let old = active[i];
    if old != 0 {
        span(old).state = FULL;
        active[i] = 0;
    }
    let mut s = partial[i];
    if s != 0 {
        partial_remove(i, s);
    } else {
        pool_enter();
        s = span_take();
        pool_leave();
        if s == 0 {
            return 0;
        }
        let sp = span(s);
        let osize = @as(usize, class_size[c]);
        sp.free = 0;
        sp.bump = s + HDR;
        sp.end = s + HDR + (((PAGE - HDR) / osize) * osize);
        sp.osize = class_size[c];
        sp.used = 0;
        sp.cls = @truncate(u32, c);
        sp.heap = @truncate(u32, h);
    }
    let sp = span(s);
    sp.state = ACTIVE;
    sp.next = 0;
    sp.prev = 0;
    active[i] = s;
    return span_alloc(sp, h);
}

fn span_alloc(sp: *mut Span, h: usize) usize {
    let mut p = sp.free;
    if p != 0 {
        sp.free = word(p).*;
    } else {
        p = sp.bump;
        sp.bump += @as(usize, sp.osize);
    }
    sp.used += 1;
    heap_bytes[h] += @as(usize, sp.osize);
    return p;
}

fn small_alloc(c: usize) usize {
    let h = heap_index();
    let m = multi;
    if m {
        lock(&mut heap_lock[h]);
    }
    let s = active[(h * NCLS) + c];
    let mut r: usize = 0;
    if s != 0 {
        let sp = span(s);
        if (sp.free != 0) or (sp.bump < sp.end) {
            r = span_alloc(sp, h);
        }
    }
    if r == 0 {
        r = refill(h, c);
    }
    if m {
        unlock(&mut heap_lock[h]);
    }
    return r;
}

fn small_free(p: usize, c: usize) void {
    let s = p & ~(PAGE - 1);
    let sp = span(s);
    if (sp.magic != tag(MAGIC_SPAN, s)) or (sp.cls != @truncate(u32, c)) or (sp.state == CACHED) {
        @trap();  // not an object of this size from the allocator
    }
    let h = @as(usize, sp.heap);
    let m = multi;
    if m {
        lock(&mut heap_lock[h]);
    }
    word(p).* = sp.free;
    sp.free = p;
    sp.used -= 1;
    heap_bytes[h] -= @as(usize, sp.osize);
    if sp.state == FULL {
        let i = (h * NCLS) + c;
        sp.state = PARTIAL;
        sp.prev = 0;
        sp.next = partial[i];
        if sp.next != 0 {
            span(sp.next).prev = s;
        }
        partial[i] = s;
    } else if (sp.state == PARTIAL) and (sp.used == 0) {
        partial_remove((h * NCLS) + c, s);
        pool_enter();
        span_give(s);
        pool_leave();
    }
    if m {
        unlock(&mut heap_lock[h]);
    }
}

// -----------------------------------------------------------------------------
// Large objects
// -----------------------------------------------------------------------------
/// Requests beyond what any machine has fail (with OutOfMemory) before
/// their sizes are rounded, binned or added to (which could overflow).
fn too_big(size: usize) bool {
    return (size > (@max_value(usize) / 2)) or (@as(u64, size) > (@as(u64, 1) << 40));
}

fn block_size_for(size: usize) usize {
    return ((size + HDR) + UNIT - 1) & ~(UNIT - 1);
}

fn large_alloc(size: usize) usize {
    if too_big(size) {
        return 0;
    }
    let bs = block_size_for(size);
    pool_enter();
    let s = block_alloc(bs);
    if s != 0 {
        large_bytes += block(s).size;
    }
    pool_leave();
    if s == 0 {
        return 0;
    }
    return s + HDR;
}

/// A large object whose bytes are all zero: only the part that is not
/// fresh from the system needs clearing (so a huge zeroed buffer costs
/// nothing until it is used).
fn large_alloc_zeroed(size: usize) usize {
    if too_big(size) {
        return 0;
    }
    let bs = block_size_for(size);
    pool_enter();
    let s = block_alloc(bs);
    let f = fresh_at;
    if s != 0 {
        large_bytes += block(s).size;
    }
    pool_leave();
    if s == 0 {
        return 0;
    }
    let p = s + HDR;
    if f > p {
        let n = @min(f - p, size);
        @mem_set(@slice_from_ptr(@ptr_from_int(*mut u8, p), n), 0);
    }
    return p;
}

fn large_check(s: usize) void {
    if ((s & (UNIT - 1)) != 0) or (magic_at(s) != tag(MAGIC_USED, s)) {
        @trap();  // not a large object from the allocator (or freed twice)
    }
}

fn large_free(p: usize) void {
    let s = p - HDR;
    pool_enter();
    large_check(s);
    large_bytes -= block(s).size;
    release_block(s, block(s).size);
    pool_leave();
}

// Resizes the large object at p in place if it can; true if it did.
fn large_resize(p: usize, size: usize) bool {
    if too_big(size) {
        return false;
    }
    let s = p - HDR;
    let need = block_size_for(size);
    pool_enter();
    large_check(s);
    let b = block(s);
    let have = b.size;
    let mut ok = false;
    if need <= have {
        if (have - need) >= UNIT {
            b.size = need;
            large_bytes -= have - need;
            block(s + need).magic = tag(MAGIC_USED, s + need);
            block(s + need).prev_free = 0;
            release_block(s + need, have - need);
        }
        ok = true;
    } else {
        let n = s + have;
        let extra = need - have;
        if n == wild_lo {
            if (wild_hi - wild_lo) < extra {
                _ = grow(((extra - (wild_hi - wild_lo)) + PAGE - 1) / PAGE);
            }
            if (wild_lo == n) and ((wild_hi - wild_lo) >= extra) {
                wild_lo += extra;
                b.size = need;
                large_bytes += extra;
                ok = true;
            }
        } else if (magic_at(n) == tag(MAGIC_FREE, n)) and ((have + block(n).size) >= need) {
            let ns = block(n).size;
            bin_remove(n);
            took(ns);
            let rest = (have + ns) - need;
            b.size = need;
            large_bytes += need - have;
            if rest >= UNIT {
                block(s + need).prev_free = 0;
                bin_insert(s + need, rest);
            } else {
                b.size = have + ns;
                large_bytes += rest;
            }
            ok = true;
        }
    }
    pool_leave();
    return ok;
}

// -----------------------------------------------------------------------------
// The address sanitizer (phalange build --sanitize=address, spec §16.8)
//
// When @sanitize_address() is true, every allocation gets a 64-byte header
// in front (its size, links in a list of live allocations, where it was
// made) and a redzone of at least 32 bytes behind it, and shadow memory
// records which heap bytes may be used: one byte per 8 bytes of heap, 0 if
// all 8 may, k (1-7) if the first k may, or a poison kind. The compiler
// checks every access through a pointer or slice against it (asan_check).
// Freed memory stays poisoned in a quarantine (64 MiB) before it is really
// freed, so that late uses of it are caught; frees are checked; and at exit
// (asan_exit) the allocations never freed are reported as leaks.
// -----------------------------------------------------------------------------

const AS_HDR: usize = 64;
const AS_RZ: usize = 32;
const AS_QUARANTINE: usize = 64 << 20;
const AS_SITES: usize = 3;
const SH_LEFT: u8 = 0xfa;   // a header (the left redzone)
const SH_RIGHT: u8 = 0xfb;  // the right redzone
const SH_FREED: u8 = 0xfd;  // freed memory
const MAGIC_ALIVE: u64 = 0x4153_414E_A11C_E5ED;
const MAGIC_DEAD: u64 = 0x4153_414E_DEAD_F4EE;

let mut as_lock_word: u32 = 0;
let mut as_ready: bool = false;
let mut as_base: usize = 0;              // the first 64 KiB page tracked
let mut as_l1: [4096]usize = [0; 4096];  // page tables: 4096 x 4096 pages (1 TiB)
let mut as_meta: usize = 0;              // [as_meta, as_meta_end): zeroed memory for tables
let mut as_meta_end: usize = 0;
let mut as_live: usize = 0;     // the newest live allocation's header
let mut as_q_first: usize = 0;  // the quarantine, oldest first
let mut as_q_last: usize = 0;
let mut as_q_bytes: usize = 0;
let mut as_ignoring: u32 = 0;      // leaks_ignored depth
let mut as_live_bytes: usize = 0;  // requested bytes of live allocations

// The header's fields (u64 on every target).
fn hfield(raw: usize, i: usize) *mut u64 {
    return @ptr_from_int(*mut u64, raw + (8 * i));
}

const H_MAGIC: usize = 0;
const H_SIZE: usize = 1;
const H_PREV: usize = 2;
const H_NEXT: usize = 3;
const H_SITE: usize = 4;   // 4, 5, 6: innermost call sites outside this module
const H_FLAGS: usize = 7;  // 1: not a leak

fn as_enter() void {
    if multi {
        lock(&mut as_lock_word);
    }
}

fn as_leave() void {
    if multi {
        unlock(&mut as_lock_word);
    }
}

/// Zeroed memory for shadow tables, taken straight from the system.
fn as_meta_alloc(bytes: usize) usize {
    if (as_meta + bytes) > as_meta_end {
        let prev = @memory_grow(1) catch return 0;
        as_meta = prev * PAGE;
        as_meta_end = as_meta + PAGE;
    }
    let r = as_meta;
    as_meta += bytes;
    return r;
}

/// The shadow byte of addr, or 0 if its page is not tracked (and make is
/// false, or memory ran out).
fn as_shadow(addr: usize, make: bool) usize {
    let rel = (addr >> 16) -% as_base;
    if rel >= (1 << 24) {
        return 0;
    }
    let mut l2 = as_l1[rel >> 12];
    if l2 == 0 {
        if not make {
            return 0;
        }
        l2 = as_meta_alloc(4096 * 8);
        if l2 == 0 {
            return 0;
        }
        as_l1[rel >> 12] = l2;
    }
    let slot = hfield(l2, rel & 4095);
    let mut sp = @truncate(usize, slot.*);
    if sp == 0 {
        if not make {
            return 0;
        }
        sp = as_meta_alloc(PAGE / 8);
        if sp == 0 {
            return 0;
        }
        slot.* = @as(u64, sp);
    }
    return sp + ((addr & (PAGE - 1)) >> 3);
}

fn sbyte(s: usize) *mut u8 {
    return @ptr_from_int(*mut u8, s);
}

/// Sets the shadow of [addr, addr + len), both multiples of 8, to v.
fn as_set(addr: usize, len: usize, v: u8) void {
    let mut a = addr;
    while a < (addr + len) {
        let s = as_shadow(a, true);
        if s != 0 {
            sbyte(s).* = v;
        }
        a += 8;
    }
}

/// Marks size bytes at addr (a multiple of 8) usable.
fn as_unpoison(addr: usize, size: usize) void {
    as_set(addr, size & ~@as(usize, 7), 0);
    if (size & 7) != 0 {
        let s = as_shadow(addr + (size & ~@as(usize, 7)), true);
        if s != 0 {
            sbyte(s).* = @truncate(u8, size & 7);
        }
    }
}

/// Whether size bytes at addr may be used; if not, reports why (the
/// compiler then traps with T0012 at the access).
pub fn asan_check(addr: usize, size: usize) bool {
    if (size == 0) or not as_ready {
        return true;
    }
    let s = as_shadow(addr, false);
    if s != 0 {
        if (sbyte(s).* == 0) and (((addr & 7) + size) <= 8) {
            return true;
        }
    } else if ((addr & (PAGE - 1)) + size) <= PAGE {
        return true;  // not heap memory
    }
    return as_check_slow(addr, size);
}

fn as_check_slow(addr: usize, size: usize) bool {
    let end = addr +% size;
    if end < addr {
        as_report_access(addr, size, addr, SH_RIGHT);
        return false;
    }
    let mut a = addr & ~@as(usize, 7);
    while a < end {
        let s = as_shadow(a, false);
        if s == 0 {
            a = (a | (PAGE - 1)) +% 1;  // the next page
            if a == 0 {
                return true;
            }
            continue;
        }
        let v = sbyte(s).*;
        if v != 0 {
            // the bytes of this granule the access uses: [lo, hi)
            let lo = @max(a, addr) - a;
            let hi = @min(a + 8, end) - a;
            if (v >= 8) or (hi > @as(usize, v)) {
                let mut bad = a + @max(lo, @as(usize, v));
                if v >= 8 {
                    bad = a + lo;
                }
                as_report_access(addr, size, bad, v);
                return false;
            }
        }
        a += 8;
    }
    return true;
}

/// The header of the allocation whose redzones or body contain addr, or 0.
fn as_find(addr: usize) usize {
    let mut a = addr & ~@as(usize, 7);
    let limit = a -% (16 << 20);
    // back over the body (and right redzone) to the header
    loop {
        let s = as_shadow(a, false);
        if (s == 0) or (a == limit) {
            return 0;
        }
        if sbyte(s).* == SH_LEFT {
            break;
        }
        a -= 8;
    }
    // back to the header's start
    loop {
        let s = as_shadow(a - 8, false);
        if (s == 0) or (sbyte(s).* != SH_LEFT) {
            break;
        }
        a -= 8;
    }
    // a hit in a left redzone may be after the header's start: headers are
    // exactly AS_HDR bytes, aligned to 16, so step back to one
    let m = hfield(a, H_MAGIC).*;
    if (m == tag(MAGIC_ALIVE, a)) or (m == tag(MAGIC_DEAD, a)) {
        return a;
    }
    return 0;
}

// --- reports ------------------------------------------------------------------

struct AsSite {
    code: u32,
    file_id: u32,
    start: u32,
    end: u32,
    line: u32,
    col: u32,
    file: *u8,
    text: *u8,
    fn_: *u8,
    callee: *u8,
}

struct AsStack {
    depth: u32,
    errors: u32,
    calls: [256]*AsSite,
}

extern "C" {
    fn phalange_shadow_stack() *AsStack;
}

import host "phalange" {
    fn ph_write(fd: i32, p: *u8, n: usize) isize;
}

/// Messages to stderr, built in a fixed buffer.
struct AsOut {
    buf: [1024]u8,
    n: usize,

    fn s(self: *mut AsOut, t: []u8) void {
        for c in t {
            if self.n == self.buf.len {
                self.flush();
            }
            self.buf[self.n] = c;
            self.n += 1;
        }
    }

    fn dec(self: *mut AsOut, v: u64) void {
        let mut d: [20]u8 = [0; 20];
        let mut i: usize = 20;
        let mut x = v;
        loop {
            i -= 1;
            d[i] = '0' + @truncate(u8, x % 10);
            x = x / 10;
            if x == 0 {
                break;
            }
        }
        self.s(d[i..]);
    }

    fn hex(self: *mut AsOut, v: u64) void {
        let digits = "0123456789abcdef";
        self.s("0x");
        let mut started = false;
        let mut k: u32 = 64;
        while k > 0 {
            k -= 4;
            let dgt = @truncate(usize, (v >> k) & 15);
            if (dgt != 0) or started or (k == 0) {
                self.buf[self.n] = digits[dgt];
                self.n += 1;
                if self.n == self.buf.len {
                    self.flush();
                }
                started = true;
            }
        }
    }

    fn cs(self: *mut AsOut, p: *u8) void {
        let mut a = @int_from_ptr(p);
        if a == 0 {
            return;
        }
        while @ptr_from_int(*u8, a).* != 0 {
            let one: [1]u8 = [@ptr_from_int(*u8, a).*];
            self.s(one[..]);
            a += 1;
        }
    }

    fn flush(self: *mut AsOut) void {
        _ = ph_write(2, &self.buf[0], self.n);
        self.n = 0;
    }
}

fn is_mem_site(site: *AsSite) bool {
    let f = @int_from_ptr(site.file);
    if f == 0 {
        return false;
    }
    let want = "std/mem.ph";
    for i in 0..want.len {
        if @ptr_from_int(*u8, f + i).* != want[i] {
            return false;
        }
    }
    return @ptr_from_int(*u8, f + want.len).* == 0;
}

/// Records where an allocation is made: the innermost call sites outside
/// this module.
fn as_record_sites(raw: usize) void {
    as_record_sites_at(raw + (8 * H_SITE));
}

/// AS_SITES call sites (u64 each) at `at`.
fn as_record_sites_at(at: usize) void {
    let ss = phalange_shadow_stack();
    let mut k: usize = 0;
    if @int_from_ptr(ss) != 0 {
        let mut i = @as(usize, @min(ss.depth, 256));
        while (i > 0) and (k < AS_SITES) {
            i -= 1;
            let site = ss.calls[i];
            if (@int_from_ptr(site) != 0) and not is_mem_site(site) {
                hfield(at, k).* = @as(u64, @int_from_ptr(site));
                k += 1;
            }
        }
    }
    while k < AS_SITES {
        hfield(at, k).* = 0;
        k += 1;
    }
}

fn as_print_sites(o: *mut AsOut, raw: usize, what: []u8) void {
    as_print_sites_at(o, raw + (8 * H_SITE), what);
}

/// The sites recorded where a freed allocation's right redzone starts.
fn as_free_sites(raw: usize) usize {
    let n = @truncate(usize, hfield(raw, H_SIZE).*);
    return (raw + AS_HDR) + ((n + 7) & ~@as(usize, 7));
}

fn as_print_sites_at(o: *mut AsOut, at: usize, what: []u8) void {
    if hfield(at, 0).* == 0 {
        return;
    }
    o.s(what);
    o.s(":\n");
    for k in 0..AS_SITES {
        let a = @truncate(usize, hfield(at, k).*);
        if a == 0 {
            break;
        }
        let site = @ptr_from_int(*AsSite, a);
        o.s("  at ");
        o.cs(site.fn_);
        o.s(" (");
        o.cs(site.file);
        o.s(":");
        o.dec(@as(u64, site.line));
        o.s(":");
        o.dec(@as(u64, site.col));
        o.s(")\n");
    }
}

fn as_report_access(addr: usize, size: usize, bad: usize, v: u8) void {
    let mut o = AsOut.{ buf: [0; 1024], n: 0 };
    o.s("phalange: AddressSanitizer: ");
    if v == SH_FREED {
        o.s("heap-use-after-free");
    } else {
        o.s("heap-buffer-overflow");
    }
    o.s(": ");
    o.dec(@as(u64, size));
    o.s("-byte access at ");
    o.hex(@as(u64, addr));
    let raw = as_find(bad);
    if raw != 0 {
        let p = raw + AS_HDR;
        let n = @truncate(usize, hfield(raw, H_SIZE).*);
        o.s(": ");
        if bad < p {
            o.dec(@as(u64, p - bad));
            o.s(" bytes before");
        } else if bad >= (p + n) {
            o.dec(@as(u64, bad - (p + n)));
            o.s(" bytes after the end of");
        } else {
            o.dec(@as(u64, bad - p));
            o.s(" bytes inside");
        }
        o.s(" a ");
        o.dec(@as(u64, n));
        o.s("-byte allocation at ");
        o.hex(@as(u64, p));
        let dead = hfield(raw, H_MAGIC).* == tag(MAGIC_DEAD, raw);
        if dead {
            o.s(", freed");
        }
        o.s("\n");
        if dead {
            as_print_sites_at(&mut o, as_free_sites(raw), "freed");
        }
        as_print_sites(&mut o, raw, "allocated");
    } else {
        o.s("\n");
    }
    o.flush();
}

fn as_report_free(p: usize, size: usize, what: []u8, raw: usize) void {
    let mut o = AsOut.{ buf: [0; 1024], n: 0 };
    o.s("phalange: AddressSanitizer: ");
    o.s(what);
    o.s(" of ");
    o.dec(@as(u64, size));
    o.s(" bytes at ");
    o.hex(@as(u64, p));
    if raw != 0 {
        o.s(" (a ");
        o.dec(hfield(raw, H_SIZE).*);
        o.s("-byte allocation)\n");
        if hfield(raw, H_MAGIC).* == tag(MAGIC_DEAD, raw) {
            as_print_sites_at(&mut o, as_free_sites(raw), "first freed");
        }
        as_print_sites(&mut o, raw, "allocated");
    } else {
        o.s("\n");
    }
    o.flush();
}

// --- allocation -------------------------------------------------------------

fn as_total(size: usize) usize {
    return (AS_HDR + ((size + 7) & ~@as(usize, 7))) + AS_RZ;
}

fn asan_alloc(size: usize) usize {
    if (size == 0) or too_big(size) {
        return 0;
    }
    as_enter();
    if not as_ready {
        as_base = @memory_size();
        as_ready = true;
    }
    let total = as_total(size);
    let raw = alloc_slow(total);
    if raw == 0 {
        as_leave();
        return 0;
    }
    let p = raw + AS_HDR;
    let body = (size + 7) & ~@as(usize, 7);
    as_set(raw, AS_HDR, SH_LEFT);
    as_unpoison(p, size);
    as_set(p + body, total - (AS_HDR + body), SH_RIGHT);
    hfield(raw, H_MAGIC).* = tag(MAGIC_ALIVE, raw);
    hfield(raw, H_SIZE).* = @as(u64, size);
    hfield(raw, H_PREV).* = 0;
    hfield(raw, H_NEXT).* = @as(u64, as_live);
    if as_live != 0 {
        hfield(as_live, H_PREV).* = @as(u64, raw);
    }
    as_live = raw;
    as_live_bytes += size;
    hfield(raw, H_FLAGS).* = 0;
    if as_ignoring > 0 {
        hfield(raw, H_FLAGS).* = 1;
    }
    as_record_sites(raw);
    as_leave();
    return p;
}

/// Checks that p and size are an allocation that may be freed; reports and
/// traps (T0012) if not.
fn as_check_free(p: usize, size: usize) void {
    let raw = p -% AS_HDR;
    let s = as_shadow(raw, false);
    let ok_place = ((p & 15) == 0) and (s != 0) and (sbyte(s).* == SH_LEFT) and (as_shadow(p - 8, false) != 0);
    if ok_place {
        let m = hfield(raw, H_MAGIC).*;
        if m == tag(MAGIC_DEAD, raw) {
            as_report_free(p, size, "double free", raw);
            @memory_error();
        }
        if m == tag(MAGIC_ALIVE, raw) {
            if hfield(raw, H_SIZE).* != @as(u64, size) {
                as_report_free(p, size, "free with the wrong size", raw);
                @memory_error();
            }
            return;
        }
    }
    as_report_free(p, size, "free of memory that was not allocated", 0);
    @memory_error();
}

fn asan_free(p: usize, size: usize) void {
    if size == 0 {
        return;
    }
    as_enter();
    as_check_free(p, size);
    let raw = p - AS_HDR;
    // out of the live list
    let prev = @truncate(usize, hfield(raw, H_PREV).*);
    let next = @truncate(usize, hfield(raw, H_NEXT).*);
    if prev != 0 {
        hfield(prev, H_NEXT).* = @as(u64, next);
    } else {
        as_live = next;
    }
    if next != 0 {
        hfield(next, H_PREV).* = @as(u64, prev);
    }
    hfield(raw, H_MAGIC).* = tag(MAGIC_DEAD, raw);
    as_live_bytes -= size;
    as_set(p, (size + 7) & ~@as(usize, 7), SH_FREED);
    as_record_sites_at(as_free_sites(raw));
    // into the quarantine; the oldest out of it
    hfield(raw, H_NEXT).* = 0;
    if as_q_last != 0 {
        hfield(as_q_last, H_NEXT).* = @as(u64, raw);
    } else {
        as_q_first = raw;
    }
    as_q_last = raw;
    as_q_bytes += as_total(size);
    while as_q_bytes > AS_QUARANTINE {
        let old = as_q_first;
        let n = as_total(@truncate(usize, hfield(old, H_SIZE).*));
        as_q_first = @truncate(usize, hfield(old, H_NEXT).*);
        if as_q_first == 0 {
            as_q_last = 0;
        }
        as_q_bytes -= n;
        free_slow(old, n);
    }
    as_leave();
}

fn asan_realloc(p: usize, old: usize, size: usize) usize {
    if old == 0 {
        return asan_alloc(size);
    }
    if size == 0 {
        asan_free(p, old);
        return 0;
    }
    as_enter();
    as_check_free(p, old);
    as_leave();
    // always moves, so that stale pointers to the old memory are caught
    let q = asan_alloc(size);
    if q == 0 {
        return 0;
    }
    let n = @min(old, size);
    @mem_copy(@slice_from_ptr(@ptr_from_int(*mut u8, q), n), @slice_from_ptr(@ptr_from_int(*u8, p), n));
    asan_free(p, old);
    return q;
}

/// Leak checking: allocations made while this is on (calls nest) are not
/// reported as leaks at exit, for memory kept on purpose until the program
/// ends (a cache in a global, say). Does nothing without --sanitize=address.
pub fn leaks_ignored(on: bool) void {
    if not @sanitize_address() {
        return;
    }
    as_enter();
    if on {
        as_ignoring += 1;
    } else if as_ignoring > 0 {
        as_ignoring -= 1;
    }
    as_leave();
}

/// Called when main returns (--sanitize=address): reports the allocations
/// never freed and returns the exit status, 23 if there were leaks and main
/// returned 0.
pub fn asan_exit(code: i32) i32 {
    as_enter();
    let mut count: u64 = 0;
    let mut bytes: u64 = 0;
    let mut o = AsOut.{ buf: [0; 1024], n: 0 };
    let mut a = as_live;
    while a != 0 {
        if hfield(a, H_FLAGS).* == 0 {
            count += 1;
            bytes += hfield(a, H_SIZE).*;
            if count <= 10 {
                if count == 1 {
                    o.s("phalange: AddressSanitizer: memory leaks\n");
                }
                o.s("leak of ");
                o.dec(hfield(a, H_SIZE).*);
                o.s(" bytes at ");
                o.hex(@as(u64, a + AS_HDR));
                o.s("\n");
                as_print_sites(&mut o, a, "allocated");
            }
        }
        a = @truncate(usize, hfield(a, H_NEXT).*);
    }
    as_leave();
    if count == 0 {
        return code;
    }
    o.s("phalange: AddressSanitizer: ");
    o.dec(bytes);
    o.s(" bytes leaked in ");
    o.dec(count);
    o.s(" allocations\n");
    o.flush();
    if code == 0 {
        return 23;
    }
    return code;
}

// -----------------------------------------------------------------------------
// The interface
// -----------------------------------------------------------------------------
/// `size` bytes, aligned to 16; 0 if memory is exhausted (or size is 0).
pub fn alloc_bytes(size: usize) usize {
    if @sanitize_address() {
        return asan_alloc(size);
    }
    // the common case, without calls: one thread, and an object in the
    // current span of the class
    if ((size -% 1) < 1024) and not multi {
        let s = active[@as(usize, small_class[(size +% 15) >> 4])];
        if s != 0 {
            let sp = span(s);
            let mut p = sp.free;
            if p != 0 {
                sp.free = word(p).*;
            } else if sp.bump < sp.end {
                p = sp.bump;
                sp.bump +%= @as(usize, sp.osize);
            }
            if p != 0 {
                sp.used +%= 1;
                heap_bytes[0] +%= @as(usize, sp.osize);
                return p;
            }
        }
    }
    return alloc_slow(size);
}

fn alloc_slow(size: usize) usize {
    if size == 0 {
        return 0;
    }
    if size <= SMALL_MAX {
        return small_alloc(class_of(size));
    }
    return large_alloc(size);
}

/// Frees memory from alloc_bytes; `size` must be the size it was allocated
/// with (or resized to).
pub fn free_bytes(p: usize, size: usize) void {
    if @sanitize_address() {
        asan_free(p, size);
        return;
    }
    // the common case, without calls: one thread, and the span stays in the
    // list it is in
    if ((size -% 1) < 1024) and not multi {
        let c = small_class[(size +% 15) >> 4];
        let s = p & ~(PAGE - 1);
        let sp = span(s);
        if (sp.magic == tag(MAGIC_SPAN, s)) and (sp.cls == @as(u32, c)) and
           ((sp.state == ACTIVE) or ((sp.state == PARTIAL) and (sp.used > 1))) {
            word(p).* = sp.free;
            sp.free = p;
            sp.used -%= 1;
            heap_bytes[0] -%= @as(usize, sp.osize);
            return;
        }
    }
    free_slow(p, size);
}

fn free_slow(p: usize, size: usize) void {
    if size == 0 {
        return;
    }
    if size <= SMALL_MAX {
        small_free(p, class_of(size));
        return;
    }
    large_free(p);
}

/// Resizes memory from alloc_bytes, keeping its contents (up to the smaller
/// size); moves it if it must. 0 if memory is exhausted (the old memory is
/// then still valid).
pub fn realloc_bytes(p: usize, old: usize, size: usize) usize {
    if @sanitize_address() {
        return asan_realloc(p, old, size);
    }
    if old == 0 {
        return alloc_bytes(size);
    }
    if size == 0 {
        free_bytes(p, old);
        return 0;
    }
    if (old <= SMALL_MAX) and (size <= SMALL_MAX) {
        if class_of(old) == class_of(size) {
            return p;
        }
    } else if (old > SMALL_MAX) and (size > SMALL_MAX) and large_resize(p, size) {
        return p;
    }
    let q = alloc_bytes(size);
    if q == 0 {
        return 0;
    }
    let n = @min(old, size);
    @mem_copy(@slice_from_ptr(@ptr_from_int(*mut u8, q), n), @slice_from_ptr(@ptr_from_int(*u8, p), n));
    free_bytes(p, old);
    return q;
}

fn empty[T]() []mut T {
    return @slice_from_ptr(@ptr_from_int(*mut T, @align_of(T)), 0);
}

/// `n` values of type T. Contents are unspecified.
pub fn alloc[T](n: usize) ![]mut T {
    let bytes = @mul_checked(n, @size_of(T)) catch return error.OutOfMemory;
    if bytes == 0 {
        return empty.[T]();
    }
    let p = alloc_bytes(bytes);
    if p == 0 {
        return error.OutOfMemory;
    }
    return @slice_from_ptr(@ptr_from_int(*mut T, p), n);
}

/// `n` values of type T, every byte zero.
pub fn alloc_zeroed[T](n: usize) ![]mut T {
    let bytes = @mul_checked(n, @size_of(T)) catch return error.OutOfMemory;
    if (bytes > SMALL_MAX) and not @sanitize_address() {
        let p = large_alloc_zeroed(bytes);
        if p == 0 {
            return error.OutOfMemory;
        }
        return @slice_from_ptr(@ptr_from_int(*mut T, p), n);
    }
    let s = alloc.[T](n)!;
    @mem_set(@slice_from_ptr(@ptr_cast(*mut u8, s.ptr), n * @size_of(T)), 0);
    return s;
}

/// Frees memory from alloc (the whole slice it returned, or a slice that
/// realloc returned).
pub fn free[T](s: []mut T) void {
    free_bytes(@int_from_ptr(s.ptr), s.len * @size_of(T));
}

/// `s` resized to `n` values: the first min(s.len, n) are kept. The result
/// may be at another address; `s` must not be used afterwards (unless this
/// fails, and then it is unchanged).
pub fn realloc[T](s: []mut T, n: usize) ![]mut T {
    let bytes = @mul_checked(n, @size_of(T)) catch return error.OutOfMemory;
    let old = s.len * @size_of(T);
    if bytes == 0 {
        free_bytes(@int_from_ptr(s.ptr), old);
        return empty.[T]();
    }
    let p = realloc_bytes(@int_from_ptr(s.ptr), old, bytes);
    if p == 0 {
        return error.OutOfMemory;
    }
    return @slice_from_ptr(@ptr_from_int(*mut T, p), n);
}

/// A copy of `s` in new memory.
pub fn dup[T](s: []T) ![]mut T {
    let d = alloc.[T](s.len)!;
    @mem_copy(d, s);
    return d;
}

/// One value of type T, initialized to `v`.
pub fn create[T](v: T) !*mut T {
    let s = alloc.[T](1)!;
    s[0] = v;
    return &mut s[0];
}

/// Frees a value from create.
pub fn destroy[T](p: *mut T) void {
    free_bytes(@int_from_ptr(p), @size_of(T));
}

/// Bytes handed out and not yet freed (small objects count their size class;
/// large ones whole 4 KiB units, header included). With --sanitize=address,
/// the bytes of the allocations not yet freed, as requested.
pub fn in_use() usize {
    if @sanitize_address() {
        return as_live_bytes;  // what the program asked for: not headers or the quarantine
    }
    let mut n = large_bytes;
    for h in 0..NHEAPS {
        n += heap_bytes[h];
    }
    return n;
}

// -----------------------------------------------------------------------------
// Tests (phalange test std/mem.ph)
// -----------------------------------------------------------------------------

fn class_by_formula(size: usize) usize {
    if size <= 128 {
        return ((size + 15) >> 4) - 1;
    }
    let x = size - 1;
    let b = log2(x);
    return (8 + ((b - 7) * 4) + (x >> (b - 2))) - 4;
}

test "size classes" {
    for sz in @as(usize, 1)..SMALL_MAX + 1 {
        @expect_eq(class_of(sz), class_by_formula(sz));
    }
    let mut s: usize = 1;
    while s <= SMALL_MAX {
        let c = class_of(s);
        @expect(@as(usize, class_size[c]) >= s);
        if c > 0 {
            @expect(@as(usize, class_size[c - 1]) < s);
        }
        s += 1;
    }
    @expect_eq(class_of(SMALL_MAX), NCLS - 1);
}

test "small objects are distinct, aligned and reused" {
    let base = in_use();
    let ptrs = alloc.[usize](1000)!;
    for i in 0..ptrs.len {
        let x = alloc.[u8](1 + (i % 200))!;
        @expect_eq(@int_from_ptr(x.ptr) % 16, 0);
        @mem_set(x, @truncate(u8, i));
        ptrs[i] = @int_from_ptr(x.ptr);
    }
    for i in 0..ptrs.len {
        let x = @slice_from_ptr(@ptr_from_int(*mut u8, ptrs[i]), 1 + (i % 200));
        @expect_eq(x[x.len - 1], @truncate(u8, i));
        free(x);
    }
    free(ptrs);
    @expect_eq(in_use(), base);
    if @sanitize_address() {
        return;  // freed memory is quarantined, not reused at once
    }
    // freed memory comes back
    let a = alloc.[u64](4)!;
    free(a);
    let b = alloc.[u64](4)!;
    @expect_eq(@int_from_ptr(a.ptr), @int_from_ptr(b.ptr));
    free(b);
}

test "large objects merge and are reused" {
    if @sanitize_address() {
        return;  // freed memory is quarantined, not reused at once
    }
    let base = in_use();
    let a = alloc.[u8](100000)!;
    let b = alloc.[u8](200000)!;
    let c = alloc.[u8](300000)!;
    a[99999] = 1;
    c[0] = 3;
    free(a);
    free(b);
    // a and b merged: room for more than either
    let d = alloc.[u8](290000)!;
    @expect(@int_from_ptr(d.ptr) < @int_from_ptr(c.ptr));
    free(d);
    free(c);
    @expect_eq(in_use(), base);
}

test "realloc keeps contents and grows in place when it can" {
    let mut v = alloc.[u32](10)!;
    for i in 0..v.len {
        v[i] = @truncate(u32, i);
    }
    let mut n: usize = 10;
    while n < 1000000 {
        n *= 3;
        v = realloc(v, n)!;
        @expect_eq(v[9], 9);
    }
    v = realloc(v, 5)!;
    @expect_eq(v[4], 4);
    free(v);
    let e = alloc.[u32](0)!;
    @expect_eq(e.len, 0);
    free(e);
}

test "create and destroy" {
    let p = create.[u64](42)!;
    @expect_eq(p.*, 42);
    destroy(p);
    let z = alloc_zeroed.[u64](3000)!;
    @expect_eq(z[2999], 0);
    free(z);
}

fn next_rand(s: *mut u64) u64 {
    s.* ^= s.* << 13;
    s.* ^= s.* >> 7;
    s.* ^= s.* << 17;
    return s.*;
}

fn fill(x: []mut u8, k: usize) void {
    for i in 0..x.len {
        x[i] = @truncate(u8, (k *% 131) +% i);
    }
}

fn intact(x: []u8, k: usize) bool {
    for i in 0..x.len {
        if x[i] != @truncate(u8, (k *% 131) +% i) {
            return false;
        }
    }
    return true;
}

test "random allocations, frees and reallocs keep every object intact" {
    let base = in_use();
    let n: usize = 300;
    let objs = alloc.[[]mut u8](n)!;
    let keys = alloc.[usize](n)!;
    for i in 0..n {
        objs[i] = alloc.[u8](0)!;
        keys[i] = 0;
    }
    let mut seed: u64 = 0x2545_F491_4F6C_DD1D;
    for step in @as(usize, 0)..20000 {
        let i = @truncate(usize, next_rand(&mut seed) % @as(u64, n));
        @expect(intact(objs[i], keys[i]));
        let r = next_rand(&mut seed);
        // mostly small, some medium, a few large
        let mut size = @truncate(usize, r % 200);
        if (r % 7) == 0 {
            size = @truncate(usize, r % 9000);
        }
        if (r % 97) == 0 {
            size = @truncate(usize, r % 300000);
        }
        if (r % 3) == 0 {
            let old_len = objs[i].len;
            objs[i] = realloc(objs[i], size)!;
            @expect(intact(objs[i][..@min(old_len, size)], keys[i]));
        } else {
            free(objs[i]);
            objs[i] = alloc.[u8](size)!;
        }
        keys[i] = step;
        fill(objs[i], step);
    }
    for i in 0..n {
        @expect(intact(objs[i], keys[i]));
        free(objs[i]);
    }
    free(objs);
    free(keys);
    @expect_eq(in_use(), base);
}

test "freeing a large block twice traps" expect trap {
    let a = alloc.[u8](100000)!;
    free(a);
    free(a);
}

test "freeing what did not come from alloc traps" expect trap {
    let big = alloc.[u8](200000)!;
    free(big[70000..70016]);
}

test "huge requests fail cleanly" {
    @expect(huge_fails((@max_value(usize) / 2) + 1));
    @expect(huge_fails(@max_value(usize) - 10));
    let a = alloc.[u8](100000)!;
    let r = realloc(a, @max_value(usize) - 10) catch {
        free(a);
        return;
    };
    free(r);
    @expect(false);
}

fn huge_fails(n: usize) bool {
    let a = alloc.[u8](n) catch return true;
    free(a);
    return false;
}

test "zeroed memory is zero, fresh or reused" {
    for round in @as(usize, 0)..3 {
        let a = alloc.[u8](200000)!;
        @mem_set(a, 0xab);
        free(a);
        let z = alloc_zeroed.[u8](200000 + round)!;
        for b in z {
            @expect_eq(b, 0);
        }
        @mem_set(z, 0xcd);
        free(z);
        let big = alloc_zeroed.[u64](1 << 20)!;
        @expect_eq(big[0], 0);
        @expect_eq(big[big.len - 1], 0);
        @expect_eq(big[12345], 0);
        free(big);
    }
}
