/* flags: --c-std=c99 */
/* Designated initializers, nested. */
struct p { int x, y; };
struct q { struct p a[2]; int n; };
struct q v = { .a[1].y = 2, .a[0] = { 1, 2 }, .n = 4 };
int arr[5] = { [4] = 1, [1] = 2, 3 };
struct p ps[3] = { [1].x = 5, 7, { .y = 6 }, };
