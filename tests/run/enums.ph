// Enums: members, representation, methods, match, comparison and conversion.
import "../lib/io.ph" as io;

enum Align: u8 {
    START,
    END,
    LEFT = 5,
    RIGHT,

    pub fn name(self: Align) []u8 {
        match self {
            .START => { return "start"; }
            .END   => { return "end"; }
            .LEFT  => { return "left"; }
            .RIGHT => { return "right"; }
        }
    }

    pub fn is_side(self: *Align) bool {
        return (self.* == Align.LEFT) or (self.* == Align.RIGHT);
    }
}

// Without a representation: the smallest type holding every value.
enum Sign {
    NEG = -1,
    ZERO,
    POS,
}

enum Wide {
    A,
    B = 70000,
}

struct Cell {
    align: Align,
    width: u16,
}

const DEFAULT: Align = Align.LEFT;
let CELLS: [3]Cell = [
    Cell.{ align: Align.END, width: 1 },
    Cell.{ align: DEFAULT, width: 2 },
    Cell.{ align: Align.RIGHT, width: 3 },
];
let mut current: Align = Align.START;

struct Pair[T] {
    a: T,
    b: T,

    pub fn init(a: T, b: T) Pair.[T] {
        return Pair.[T].{ a: a, b: b };
    }
}

// An enum inside a generic instance, built by a generic function.
struct Held {
    pair: Pair.[Align],
    n: u32,
}

fn same[T: Eq](a: T, b: T) bool {
    return a == b;
}

fn sign(x: i32) Sign {
    if x < 0 {
        return Sign.NEG;
    }
    if x == 0 {
        return Sign.ZERO;
    }
    return Sign.POS;
}

export fn run() void {
    for c in CELLS {
        io.str(c.align.name());
    }
    // expect: end
    // expect: left
    // expect: right
    io.boolean(CELLS[1].align.is_side());
    io.boolean(CELLS[0].align.is_side());
    // expect: true
    // expect: false
    io.uint(@as(u64, @int_from_enum(Align.RIGHT)));
    io.int(@as(i64, @int_from_enum(sign(-5))));
    io.int(@as(i64, @int_from_enum(sign(7))));
    // expect: 6
    // expect: -1
    // expect: 1
    io.uint(@as(u64, @size_of(Align)));
    io.uint(@as(u64, @size_of(Sign)));
    io.uint(@as(u64, @size_of(Wide)));
    io.uint(@as(u64, @size_of(Cell)));
    // expect: 1
    // expect: 1
    // expect: 4
    // expect: 4
    let mut i: u32 = 0;
    while i < 8 {
        if (i == 0) or (i == 1) or (i == 5) or (i == 6) {
            let a = @enum_from_int(Align, i);
            io.str(a.name());
        }
        i += 1;
    }
    // expect: start
    // expect: end
    // expect: left
    // expect: right
    io.boolean(same(Align.END, CELLS[0].align));
    io.boolean(Align.END != DEFAULT);
    // expect: true
    // expect: true
    current = Align.RIGHT;
    let mut arr = [Align.START; 4];
    arr[2] = current;
    let p = &mut arr[1];
    p.* = Align.END;
    for a in arr {
        match a {
            .START => { io.str("s"); }
            else   => { io.str(a.name()); }
        }
    }
    // expect: s
    // expect: end
    // expect: right
    // expect: s
    match sign(0) {
        .NEG  => { io.str("negative"); }
        .ZERO => { io.str("zero"); }
        .POS  => { io.str("positive"); }
    }
    // expect: zero
    let h = Held.{ pair: Pair.[Align].init(Align.LEFT, Align.END), n: 1 };
    io.str(h.pair.a.name());
    io.str(h.pair.b.name());
    // expect: left
    // expect: end
}
