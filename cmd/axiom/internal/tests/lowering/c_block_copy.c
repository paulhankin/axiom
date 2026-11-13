/* Copies and fills of a constant size of 256 bytes to 2 KiB: on x86-64 a
   loop of four vector moves an iteration from the first offset at which the
   destination is on a cache line's start (neg, and $63), with the first 64
   and the last bytes copied apart (16-byte moves, or 32 with AVX); on
   AArch64 a loop of two LDP / STP pairs of Q registers moving the pointers
   (post-index), then the last 64 bytes. Longer ones are rep movsb / stosb,
   and calls on AArch64. */
// lower-selfhost: --target=native --cpu=x86-64 -O2 --c-freestanding | and\s+\$0x3f, | 2
// lower-selfhost: --target=native --cpu=x86-64 -O2 --c-freestanding | movdqu\s+%xmm\d+,0x30\(%r\w+,%r\w+,1\) | 2
// lower-selfhost: --target=native --cpu=x86-64 -O2 --c-freestanding | rep movsb | 1
// lower-selfhost: --target=native --cpu=x86-64-v3 -O2 --c-freestanding | vmovdqu\s+%ymm\d+,0x60\(%r\w+,%r\w+,1\) | 2
// lower: --target=aarch64 -O2 --c-freestanding | \bldp\s+q\d+, q\d+, \[x\d+\], #64 | 1
// lower: --target=aarch64 -O2 --c-freestanding | \bstp\s+q\d+, q\d+, \[x\d+\], #64 | 2
// lower: --target=aarch64 -O2 --c-freestanding | \bbl\s+\S+\s+<memmove> | 1
typedef struct { int n; unsigned d[400]; } big;  /* 1604 bytes */
typedef struct { char b[3000]; } huge;

void copy(big *r, const big *a) { *r = *a; }
void zero(big *r) { *r = (big){ 0 }; }
void copy_huge(huge *r, const huge *a) { *r = *a; }
