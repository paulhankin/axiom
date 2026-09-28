// =============================================================================
// File: dot_product.ph
// Build: phalange build dot_product.ph --target=wasm32-freestanding -O3 -o engine.wasm
// =============================================================================

import "std/mem" as mem;

import host "env" {
    fn console_log(ptr: *u8, len: usize) void;
}

error { DimensionMismatch }

// -----------------------------------------------------------------------------
// SIMD dot product
// -----------------------------------------------------------------------------
pub fn dot_product_simd(a: []f32, b: []f32) !f32 {
    if a.len != b.len {
        return error.DimensionMismatch;
    }

    let n = a.len;
    let chunks = n / 4;
    let mut acc: f32x4 = @splat(0.0);

    let mut i: usize = 0;
    while i < chunks * 4 {
        let va = @load(f32x4, a[i..i + 4]);
        let vb = @load(f32x4, b[i..i + 4]);
        acc = acc + va * vb;
        i += 4;
    }

    // Pairwise horizontal sum: (l0 + l1) + (l2 + l3)
    let mut total = @reduce_add(acc);

    // Scalar tail
    while i < n {
        total += a[i] * b[i];
        i += 1;
    }

    return total;
}

// -----------------------------------------------------------------------------
// Exports
// -----------------------------------------------------------------------------
export fn compute(length: usize) f32 {
    let vec_a = mem.alloc.[f32](length) catch return -1.0;
    defer mem.free(vec_a);
    let vec_b = mem.alloc.[f32](length) catch return -1.0;
    defer mem.free(vec_b);

    for i in 0..length {
        vec_a[i] = 1.5;
        vec_b[i] = 2.0;
    }

    let result = dot_product_simd(vec_a, vec_b) catch return -2.0;

    let msg = "dot product computed";
    console_log(msg.ptr, msg.len);

    return result;
}
