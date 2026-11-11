/* flags: --c-std=c99 */
/* expect-error: C2127 */
struct s { union { int a; float b; }; } v;
