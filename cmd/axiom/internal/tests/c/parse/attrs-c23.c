/* flags: --c-std=c23 */
/* Attributes, attached to what they appertain to. */
[[deprecated("old")]] int a [[maybe_unused]], b;
int [[vendor::type_attr]] c;
struct [[deprecated]] s {
    [[maybe_unused]] int m;
} v;
enum e { E1 [[deprecated]], E2 [[vendor::x(1, 2)]] = 2 };
int *[[vendor::p]] p;
int arr[3] [[vendor::arr]];
void f(int x) [[vendor::fn]];
[[nodiscard]] int g([[maybe_unused]] int x) {
    [[vendor::stmt]] x++;
    [[vendor::label]] l: [[vendor::after]] x--;
    [[vendor::decl]] int y = x;
    switch (y) {
    case 1:
        y++;
        [[fallthrough]];
    default:
        y--;
    }
    return y;
}
[[vendor::attribute_declaration]];
[[]] int z;
[[vendor::args({[(1)]})]] int w;
