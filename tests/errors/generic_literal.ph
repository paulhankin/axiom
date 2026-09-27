// expect-error: E0461
fn big[T: Int]() T {
    return 1000;
}
