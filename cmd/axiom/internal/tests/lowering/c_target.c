/* A function with the attribute target("popcnt") counts bits with popcnt on
   a baseline x86-64 processor, which another function does without. */
// lower-selfhost: --target=native --cpu=x86-64 -O2 --c-freestanding --c-std=gnu17 | \bpopcnt\b | 1
__attribute__((target("popcnt"))) int with(unsigned long x) { return __builtin_popcountl(x); }
int without(unsigned long x) { return __builtin_popcountl(x); }
