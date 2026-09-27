// expect-error: E0410
struct P { x: u32, y: u32 }
fn f() P {
    return P.{ y: 1, x: 2 };
}
