// =============================================================================
// File: shapes.ph
// Tagged unions, match, methods and error handling.
// Build: phalange build shapes.ph --target=wasm32-freestanding -o shapes.wasm
// =============================================================================

error { InvalidInput }

pub union Shape {
    Circle: struct { radius: f32 },
    Rect: struct { width: f32, height: f32 },
    Empty,

    pub fn area(self: Shape) f32 {
        match self {
            .Circle(c) => { return 3.14159265 * c.radius * c.radius; }
            .Rect(r)   => { return r.width * r.height; }
            .Empty     => { return 0.0; }
        }
    }
}

fn make_shape(kind: u32, a: f32, b: f32) !Shape {
    if (a < 0.0) or (b < 0.0) {
        return error.InvalidInput;
    }
    match kind {
        0    => { return Shape.Circle(.{ radius: a }); }
        1    => { return Shape.Rect(.{ width: a, height: b }); }
        else => { return Shape.Empty; }
    }
}

fn count_leading_zeros(val: u32) u32 {
    return @clz(val);
}

export fn shape_area(kind: u32, a: f32, b: f32) f32 {
    let s = make_shape(kind, a, b) catch return -1.0;
    return s.area();
}

export fn clz(x: u32) u32 {
    return count_leading_zeros(x);
}
