// A module laid out badly: `phalange fmt` gives layout.expected.
error{TooBig,Empty}

pub struct Point{x:i32,y:i32 // no comma after the last field
    pub fn sum(self:*Self)i32{return self.x+self.y;}
}

union Shape { Circle: struct { r: f32 }, Square: f32, None }

const LIMIT:i32=100;   // the largest total
const SMALL: i32 = 3; // a small one



fn classify(n:i32)!i32{
    if n>LIMIT{return error.TooBig;}else if n==0{return error.Empty;}
    let mut total:i32=-n;
    total=total*-2+(n%7);
    match n { 1 => { return 10; } -1 => { return 11; } else => { total += 1; total += 2; } }
    return total;
}

fn area(s:Shape)f32{
    match s{
        .Circle(c)=>{return 3.14*c.r*c.r;}
        .Square(w)=>{return w*w;}
        .None=>{return 0.0;}
    }
}

fn first(xs:[]i32,p:*mut Point)i32{
    let table: [3]i32 = [
        1, 2,  // one, two
        3,
        // and no more
    ];
    let v = classify(xs[0]) catch |e| { if e == error.Empty { return 0; } return -1; };
    let w = classify(xs[1]) catch 0;
    for i in 0..xs.len { if xs[i..].len > 0 and table[0] == 1 and v > 0 and w > 0 { p.x += 1 ; } }
    let q: *mut Point = p;
    q.* = Point.{ x: 1, y: 2 };
    let pts: [2]Point = [Point.{x:1,y:2}, Point.{ x: 3, y: 4 }];
    let long_name_for_a_value = @int_cast(i32, xs.len) + v + w + table[1] + table[2] + pts[0].x + pts[1].y + LIMIT + SMALL;
    if (long_name_for_a_value > 1000) and (long_name_for_a_value < 2000) and (xs.len > 3) and (xs.len < 10) and (xs[3] != long_name_for_a_value) {
        return 1;
    }
    let spread = Point.{
        x: 5, y: 6,
    };
    return v + spread.x;
}

test "classify"{ _ = classify(3) catch 0; }
