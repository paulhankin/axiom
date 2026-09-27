// =============================================================================
// std/mem.ph - allocators (spec §16).
// =============================================================================

error { OutOfMemory }

/// The allocator interface: a context pointer plus two hooks.
pub struct Allocator {
    ctx: *mut void,
    alloc_fn: fn(*mut void, usize, usize) !*mut u8,  // (ctx, size, align)
    free_fn: fn(*mut void, *mut u8, usize) void,     // (ctx, ptr, size)

    /// Allocates `n` values of type T. Contents are unspecified.
    pub fn alloc[T](self: Allocator, n: usize) ![]mut T {
        let bytes = @mul_checked(n, @size_of(T)) catch return error.OutOfMemory;
        let p = self.alloc_fn(self.ctx, bytes, @align_of(T))!;
        return @slice_from_ptr(@ptr_cast(*mut T, p), n);
    }

    pub fn free[T](self: Allocator, s: []mut T) void {
        self.free_fn(self.ctx, @ptr_cast(*mut u8, s.ptr), s.len * @size_of(T));
    }
}

// -----------------------------------------------------------------------------
// page_allocator: bump allocation from pages obtained with memory.grow.
// WebAssembly memory cannot shrink, so free is a no-op. Thread-safe.
// -----------------------------------------------------------------------------
let mut page_next: usize = 0;
let mut page_end: usize = 0;
let mut page_lock: u32 = 0;  // page_allocator may be used from several threads

fn page_alloc(ctx: *mut void, size: usize, align: usize) !*mut u8 {
    while @atomic_xchg(&mut page_lock, 1) != 0 {
        _ = @atomic_wait(&page_lock, 1, -1);
    }
    let r = page_alloc_locked(size, align);
    @atomic_store(&mut page_lock, 0);
    _ = @atomic_notify(&page_lock, 1);
    return r;
}

fn page_alloc_locked(size: usize, align: usize) !*mut u8 {
    let mask = align - 1;
    let mut start = (page_next + mask) & ~mask;
    if (page_next == 0) or ((start + size) > page_end) {
        let pages = (size + align + 65535) / 65536;
        let prev = @memory_grow(pages)!;
        let base = prev * 65536;
        if base != page_end {
            page_next = base;
        }
        page_end = (prev + pages) * 65536;
        start = (page_next + mask) & ~mask;
    }
    page_next = start + size;
    return @ptr_from_int(*mut u8, start);
}

fn page_free(ctx: *mut void, p: *mut u8, size: usize) void {}

pub let page_allocator: Allocator = Allocator.{
    ctx: @ptr_from_int(*mut void, 0),
    alloc_fn: page_alloc,
    free_fn: page_free,
};

// -----------------------------------------------------------------------------
// Arena: grows in chunks taken from a backing allocator; everything is
// released at once by deinit().
// -----------------------------------------------------------------------------
struct ChunkHeader {
    prev: usize,  // address of the previous chunk, 0 if none
    size: usize,
}

pub struct Arena {
    backing: Allocator,
    head: usize,  // address of the newest chunk, 0 if none
    cur: usize,
    end: usize,

    pub fn init(backing: Allocator) Arena {
        return Arena.{ backing: backing, head: 0, cur: 0, end: 0 };
    }

    /// The returned allocator points at this Arena: do not move the Arena
    /// while allocators obtained from it are in use.
    pub fn allocator(self: *mut Arena) Allocator {
        return Allocator.{ ctx: @ptr_cast(*mut void, self), alloc_fn: arena_alloc, free_fn: arena_free };
    }

    pub fn alloc_bytes(self: *mut Arena, size: usize, align: usize) !*mut u8 {
        let mask = align - 1;
        let mut start = (self.cur + mask) & ~mask;
        if (self.cur == 0) or ((start + size) > self.end) {
            let hdr = @size_of(ChunkHeader);
            let mut chunk_size: usize = 65536;
            if chunk_size < (size + align + hdr) {
                chunk_size = size + align + hdr;
            }
            let raw = self.backing.alloc.[u8](chunk_size)!;
            let base = @int_from_ptr(raw.ptr);
            let h = @ptr_from_int(*mut ChunkHeader, base);
            h.prev = self.head;
            h.size = chunk_size;
            self.head = base;
            self.cur = base + hdr;
            self.end = base + chunk_size;
            start = (self.cur + mask) & ~mask;
        }
        self.cur = start + size;
        return @ptr_from_int(*mut u8, start);
    }

    /// Frees every chunk except the newest, which is kept for reuse.
    pub fn reset(self: *mut Arena) void {
        if self.head == 0 {
            return;
        }
        let newest = @ptr_from_int(*mut ChunkHeader, self.head);
        let mut c = newest.prev;
        while c != 0 {
            let h = @ptr_from_int(*mut ChunkHeader, c);
            let next = h.prev;
            self.backing.free(@slice_from_ptr(@ptr_from_int(*mut u8, c), h.size));
            c = next;
        }
        newest.prev = 0;
        self.cur = self.head + @size_of(ChunkHeader);
    }

    /// Returns all chunks to the backing allocator.
    pub fn deinit(self: *mut Arena) void {
        let mut c = self.head;
        while c != 0 {
            let h = @ptr_from_int(*mut ChunkHeader, c);
            let next = h.prev;
            self.backing.free(@slice_from_ptr(@ptr_from_int(*mut u8, c), h.size));
            c = next;
        }
        self.head = 0;
        self.cur = 0;
        self.end = 0;
    }
}

fn arena_alloc(ctx: *mut void, size: usize, align: usize) !*mut u8 {
    let a = @ptr_cast(*mut Arena, ctx);
    return a.alloc_bytes(size, align);
}

fn arena_free(ctx: *mut void, p: *mut u8, size: usize) void {}

// -----------------------------------------------------------------------------
// FixedBuffer: bump allocator over a caller-provided buffer. Never grows.
// -----------------------------------------------------------------------------
pub struct FixedBuffer {
    buf: []mut u8,
    used: usize,

    pub fn init(buf: []mut u8) FixedBuffer {
        return FixedBuffer.{ buf: buf, used: 0 };
    }

    pub fn allocator(self: *mut FixedBuffer) Allocator {
        return Allocator.{ ctx: @ptr_cast(*mut void, self), alloc_fn: fixed_alloc, free_fn: fixed_free };
    }

    pub fn reset(self: *mut FixedBuffer) void {
        self.used = 0;
    }
}

fn fixed_alloc(ctx: *mut void, size: usize, align: usize) !*mut u8 {
    let f = @ptr_cast(*mut FixedBuffer, ctx);
    let base = @int_from_ptr(f.buf.ptr);
    let mask = align - 1;
    let start = ((base + f.used + mask) & ~mask) - base;
    if (start + size) > f.buf.len {
        return error.OutOfMemory;
    }
    f.used = start + size;
    return @ptr_from_int(*mut u8, base + start);
}

fn fixed_free(ctx: *mut void, p: *mut u8, size: usize) void {}
