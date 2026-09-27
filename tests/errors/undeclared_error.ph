// expect-error: E0650
fn f() !void {
    return error.NotDeclaredAnywhere;
}
