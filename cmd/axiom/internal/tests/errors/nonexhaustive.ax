// expect-error: E0630
union U { A, B, C }
fn f(u: U) u32 {
    match u {
        .A => { return 1; }
        .B => { return 2; }
    }
    return 0;
}
