// =============================================================================
// std/c.ph - helpers for the C FFI (spec §14.5).
//
// C type          Phalange type
// -------------   ---------------------------------------------
// int8_t..int64_t i8..i64          uint8_t..uint64_t  u8..u64
// int / unsigned  i32 / u32        (every supported target)
// long            isize            unsigned long      usize   (LP64 and ILP32)
// size_t          usize            ptrdiff_t/intptr_t isize
// float / double  f32 / f64        bool (_Bool)       bool
// char            u8 (or i8)       T* / const T*      *mut T / *T
// void*           *mut void        function pointer   fn(...) R
// struct S*       *S               (layouts match; structs by value: not yet)
// =============================================================================

import "mem.ph" as mem;

error { OutOfMemory }

/// A null pointer, for passing NULL to C. Never dereference it.
pub fn null_ptr[T]() *T {
    return @ptr_from_int(*T, 0);
}

pub fn null_mut[T]() *mut T {
    return @ptr_from_int(*mut T, 0);
}

/// C functions may return NULL; Phalange pointers are never checked for it
/// implicitly.
pub fn is_null[T](p: *T) bool {
    return @int_from_ptr(p) == 0;
}

/// is_null for C's void* (generic parameters cannot be void).
pub fn is_null_void(p: *void) bool {
    return @int_from_ptr(p) == 0;
}

/// Length of a NUL-terminated C string.
pub fn cstr_len(p: *u8) usize {
    let base = @int_from_ptr(p);
    let mut n: usize = 0;
    while @ptr_from_int(*u8, base + n).* != 0 {
        n += 1;
    }
    return n;
}

/// Views a NUL-terminated C string as a slice (without the NUL). The bytes
/// are not copied.
pub fn from_cstr(p: *u8) []u8 {
    return @slice_from_ptr(p, cstr_len(p));
}

/// Copies `s` into memory from `alloc`, adding a terminating NUL, for passing
/// to C. Free it with free_cstr.
pub fn to_cstr(s: []u8) !*mut u8 {
    let buf = mem.alloc.[u8](s.len + 1)!;
    @mem_copy(buf[..s.len], s);
    buf[s.len] = 0;
    return buf.ptr;
}

pub fn free_cstr(p: *mut u8) void {
    mem.free(@slice_from_ptr(p, cstr_len(p) + 1));
}

// -----------------------------------------------------------------------------
// Tests (phalange test std/c.ph)
// -----------------------------------------------------------------------------

test "C strings" {
    let p = to_cstr("hello")!;
    @expect_eq(cstr_len(p), 5);
    @expect_eq(from_cstr(p), "hello");
    @expect_eq(cstr_len("literal".ptr), 7);  // literals are NUL-terminated
    free_cstr(p);
    @expect(is_null(null_ptr.[u8]()));
}
