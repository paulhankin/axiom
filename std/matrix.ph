// =============================================================================
// std/matrix.ph - dense matrices and small SIMD linear algebra.
//
//   Matrix[T: Num]   heap-allocated, row-major, any numeric element type
//   Mat4             4x4 f32 matrix stored as four f32x4 columns
//   dot, axpy        SIMD kernels on []f32
// =============================================================================

import "mem.ph" as mem;

error { DimensionMismatch, OutOfMemory }

pub struct Matrix[T: Num] {
    alloc: mem.Allocator,
    rows: usize,
    cols: usize,
    data: []mut T,  // row-major: element (r, c) is data[r * cols + c]

    /// A rows x cols matrix of zeros.
    pub fn init(alloc: mem.Allocator, rows: usize, cols: usize) !Self {
        let n = @mul_checked(rows, cols) catch return error.OutOfMemory;
        let data = alloc.alloc.[T](n)!;
        @mem_set(data, 0);
        return Self.{ alloc: alloc, rows: rows, cols: cols, data: data };
    }

    pub fn identity(alloc: mem.Allocator, n: usize) !Self {
        let mut m = Self.init(alloc, n, n)!;
        for i in 0..n {
            m.data[(i * n) + i] = 1;
        }
        return m;
    }

    /// Copies `xs` (row-major, rows * cols elements) into a new matrix.
    pub fn from_slice(alloc: mem.Allocator, rows: usize, cols: usize, xs: []T) !Self {
        let n = @mul_checked(rows, cols) catch return error.DimensionMismatch;
        if n != xs.len {
            return error.DimensionMismatch;
        }
        let m = Self.init(alloc, rows, cols)!;
        @mem_copy(m.data, xs);
        return m;
    }

    pub fn deinit(self: *mut Self) void {
        if self.data.len > 0 {
            self.alloc.free(self.data);
        }
        self.rows = 0;
        self.cols = 0;
    }

    /// Element (r, c); traps if out of range.
    pub fn get(self: *Self, r: usize, c: usize) T {
        if (r >= self.rows) or (c >= self.cols) {
            @trap();
        }
        return self.data[(r * self.cols) + c];
    }

    pub fn set(self: *mut Self, r: usize, c: usize, v: T) void {
        if (r >= self.rows) or (c >= self.cols) {
            @trap();
        }
        self.data[(r * self.cols) + c] = v;
    }

    /// Row r as a slice.
    pub fn row(self: *Self, r: usize) []mut T {
        if r >= self.rows {
            @trap();
        }
        return self.data[r * self.cols..(r + 1) * self.cols];
    }

    pub fn clone(self: *Self) !Self {
        return Self.from_slice(self.alloc, self.rows, self.cols, self.data);
    }

    /// self + other, as a new matrix.
    pub fn add(self: *Self, other: *Self) !Self {
        if (self.rows != other.rows) or (self.cols != other.cols) {
            return error.DimensionMismatch;
        }
        let m = Self.init(self.alloc, self.rows, self.cols)!;
        for i in 0..self.data.len {
            m.data[i] = self.data[i] + other.data[i];
        }
        return m;
    }

    /// self - other, as a new matrix.
    pub fn sub(self: *Self, other: *Self) !Self {
        if (self.rows != other.rows) or (self.cols != other.cols) {
            return error.DimensionMismatch;
        }
        let m = Self.init(self.alloc, self.rows, self.cols)!;
        for i in 0..self.data.len {
            m.data[i] = self.data[i] - other.data[i];
        }
        return m;
    }

    /// Multiplies every element by k, in place.
    pub fn scale(self: *mut Self, k: T) void {
        for i in 0..self.data.len {
            self.data[i] = self.data[i] * k;
        }
    }

    /// The matrix product self * other, as a new matrix.
    pub fn mul(self: *Self, other: *Self) !Self {
        if self.cols != other.rows {
            return error.DimensionMismatch;
        }
        let m = Self.init(self.alloc, self.rows, other.cols)!;
        let n = other.cols;
        for r in 0..self.rows {
            for k in 0..self.cols {
                let a = self.data[(r * self.cols) + k];
                let brow = other.data[k * n..(k + 1) * n];
                let orow = m.data[r * n..(r + 1) * n];
                for c in 0..n {
                    orow[c] = orow[c] + (a * brow[c]);
                }
            }
        }
        return m;
    }

    pub fn transpose(self: *Self) !Self {
        let m = Self.init(self.alloc, self.cols, self.rows)!;
        for r in 0..self.rows {
            for c in 0..self.cols {
                m.data[(c * self.rows) + r] = self.data[(r * self.cols) + c];
            }
        }
        return m;
    }

    pub fn equals(self: *Self, other: *Self) bool {
        if (self.rows != other.rows) or (self.cols != other.cols) {
            return false;
        }
        for i in 0..self.data.len {
            if self.data[i] != other.data[i] {
                return false;
            }
        }
        return true;
    }
}

// -----------------------------------------------------------------------------
// SIMD kernels
// -----------------------------------------------------------------------------

/// Dot product of two equal-length f32 slices.
pub fn dot(a: []f32, b: []f32) !f32 {
    if a.len != b.len {
        return error.DimensionMismatch;
    }
    let mut acc: f32x4 = @splat(0.0);
    let mut i: usize = 0;
    while (i + 4) <= a.len {
        acc = acc + @load(f32x4, a[i..i + 4]) * @load(f32x4, b[i..i + 4]);
        i += 4;
    }
    let mut total = @reduce_add(acc);
    while i < a.len {
        total += a[i] * b[i];
        i += 1;
    }
    return total;
}

/// y = a * x + y
pub fn axpy(a: f32, x: []f32, y: []mut f32) !void {
    if x.len != y.len {
        return error.DimensionMismatch;
    }
    let va: f32x4 = @splat(a);
    let mut i: usize = 0;
    while (i + 4) <= x.len {
        @store(y[i..i + 4], (va * @load(f32x4, x[i..i + 4])) + @load(f32x4, y[i..i + 4]));
        i += 4;
    }
    while i < x.len {
        y[i] = (a * x[i]) + y[i];
        i += 1;
    }
}

// -----------------------------------------------------------------------------
// Mat4: 4x4 f32, column-major, one f32x4 per column.
// -----------------------------------------------------------------------------

pub struct Mat4 {
    cols: [4]f32x4,

    pub fn identity() Mat4 {
        return Mat4.{ cols: [@vec(f32x4, 1.0, 0.0, 0.0, 0.0), @vec(f32x4, 0.0, 1.0, 0.0, 0.0),
                             @vec(f32x4, 0.0, 0.0, 1.0, 0.0), @vec(f32x4, 0.0, 0.0, 0.0, 1.0)] };
    }

    pub fn translation(x: f32, y: f32, z: f32) Mat4 {
        let mut m = Mat4.identity();
        m.cols[3] = @vec(f32x4, x, y, z, 1.0);
        return m;
    }

    pub fn scaling(x: f32, y: f32, z: f32) Mat4 {
        return Mat4.{ cols: [@vec(f32x4, x, 0.0, 0.0, 0.0), @vec(f32x4, 0.0, y, 0.0, 0.0),
                             @vec(f32x4, 0.0, 0.0, z, 0.0), @vec(f32x4, 0.0, 0.0, 0.0, 1.0)] };
    }

    /// Element at row r, column c (both < 4).
    pub fn get(self: Mat4, r: usize, c: usize) f32 {
        let mut tmp: [4]f32 = [0.0; 4];
        @store(tmp[..], self.cols[c]);
        return tmp[r];
    }

    /// self * v
    pub fn transform(self: Mat4, v: f32x4) f32x4 {
        let x: f32x4 = @splat(@extract_lane(v, 0));
        let y: f32x4 = @splat(@extract_lane(v, 1));
        let z: f32x4 = @splat(@extract_lane(v, 2));
        let w: f32x4 = @splat(@extract_lane(v, 3));
        return ((self.cols[0] * x) + (self.cols[1] * y)) + ((self.cols[2] * z) + (self.cols[3] * w));
    }

    /// self * other
    pub fn mul(self: Mat4, other: Mat4) Mat4 {
        let mut out = Mat4.identity();
        for j in @as(usize, 0)..4 {
            out.cols[j] = self.transform(other.cols[j]);
        }
        return out;
    }

    pub fn transpose(self: Mat4) Mat4 {
        let a = self.cols[0];
        let b = self.cols[1];
        let c = self.cols[2];
        let d = self.cols[3];
        let ab_lo = @shuffle(a, b, [0, 4, 1, 5]);
        let ab_hi = @shuffle(a, b, [2, 6, 3, 7]);
        let cd_lo = @shuffle(c, d, [0, 4, 1, 5]);
        let cd_hi = @shuffle(c, d, [2, 6, 3, 7]);
        return Mat4.{ cols: [@shuffle(ab_lo, cd_lo, [0, 1, 4, 5]), @shuffle(ab_lo, cd_lo, [2, 3, 6, 7]),
                             @shuffle(ab_hi, cd_hi, [0, 1, 4, 5]), @shuffle(ab_hi, cd_hi, [2, 3, 6, 7])] };
    }
}
