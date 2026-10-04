# 0065: C compiler: "Invalid size request on a scalable vector" from LLVM at -O2

- Type: bug
- Status: open
- Area: compiler (C)

Found while writing `tests/run/simd_masks.ax` (0058, 0062). This program, built by the C
compiler for x86-64 at `-O2` (`compiler/build/axiom build f.ax --target=native -O2`),
aborts inside LLVM:

```
LLVM ERROR: Invalid size request on a scalable vector.
```

It builds at `-O0`/`-O1`, for wasm32, and with the self-hosted compiler (which prints 3529).
Both shuffles in `widen` are needed: without the first one it builds. Not investigated
further; x86-64 has no scalable vectors, so the IR the C compiler emits probably leads an
LLVM pass astray (or it emits a type it should not).

```
import "../lib/io.ax" as io;

let mut F: [64]f32 = [0.0; 64];
let mut U: [64]u32 = [0; 64];
let mut B: [64]u8 = [0; 64];

fn fill() void {
    for i in @as(usize, 0)..64 {
        let k = @as(i32, @int_cast(i32, i));
        F[i] = (@float_from_int(f32, k - 30) * 0.75) + 0.125;
        U[i] = @bit_cast(u32, (k * 37) ^ 0x55) & 127;
        B[i] = @truncate(u8, (i * 7) % 5);
    }
    F[5] = -0.0;
    F[6] = 0.0;
    F[9] = @bit_cast(f32, @as(u32, 0x7fc00000));  // NaN
}

fn widen(p: []u8) u64 {
    let z: u8x64 = @splat(0);
    let bytes = @load(u8x64, p[0..64]);
    let n = @bit_cast(u16x32, @shuffle(bytes, z, [0, 64, 1, 64, 2, 64, 3, 64, 4, 64, 5, 64, 6, 64, 7, 64, 8, 64, 9, 64,
                                                  10, 64, 11, 64, 12, 64, 13, 64, 14, 64, 15, 64, 16, 64, 17, 64, 18,
                                                  64, 19, 64, 20, 64, 21, 64, 22, 64, 23, 64, 24, 64, 25, 64, 26, 64,
                                                  27, 64, 28, 64, 29, 64, 30, 64, 31, 64]));
    let w = @bit_cast(u32x8,
                      @shuffle(
                          @load(u16x16, @slice_from_ptr(@ptr_cast(*u16, p.ptr), 16)), @as(u16x16, @splat(0)),
                          [0, 16, 1, 16, 2, 16, 3, 16, 4, 16, 5, 16, 6, 16, 7, 16]));
    return @as(u64, @reduce_add(n *% n)) +% @as(u64, @reduce_add(w));
}

export fn run() void {
    fill();
    io.uint(widen(B[0..]));
}
```

(as `tests/run/<name>.ax`; `simd_masks.ax` reads the `u16` lanes from the widened vector
instead, which builds.)
