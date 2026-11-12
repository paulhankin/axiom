/* flags: --c-std=c99 */
/* expect-error: C1086 */
#define OLD gets
#pragma GCC poison gets
OLD
gets
