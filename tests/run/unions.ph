// Tagged unions and match.
import "../lib/io.ph" as io;

union Shape {
    Circle: struct { r: f64 },
    Rect: struct { w: f64, h: f64 },
    Empty,

    pub fn area(self: Shape) f64 {
        match self {
            .Circle(c) => { return 3.0 * c.r * c.r; }
            .Rect(r)   => { return r.w * r.h; }
            .Empty     => { return 0.0; }
        }
    }
}

fn kind(s: Shape) []u8 {
    match s {
        .Circle => { return "circle"; }
        else    => { return "other"; }
    }
}

fn classify(n: i32) []u8 {
    match n {
        0    => { return "zero"; }
        1    => { return "one"; }
        -1   => { return "minus one"; }
        else => { return "many"; }
    }
}

export fn run() void {
    let shapes: [3]Shape = [Shape.Circle(.{ r: 2.0 }), Shape.Rect(.{ w: 2.0, h: 5.0 }), Shape.Empty];
    for s in shapes {
        io.float(s.area());
        io.str(kind(s));
    }
    // expect: 12
    // expect: circle
    // expect: 10
    // expect: other
    // expect: 0
    // expect: other
    io.str(classify(0));   // expect: zero
    io.str(classify(-1));  // expect: minus one
    io.str(classify(42));  // expect: many
    let b = true;
    match b {
        true  => { io.str("yes"); }
        false => { io.str("no"); }
    }
    // expect: yes
    io.uint(@as(u64, @size_of(Shape)));  // expect: 24
}
