/* #pragma ms_struct (a GNU dialect on x86-64) is written to the
   preprocessed text where it changes, as on or off. */
/* flags: --c-std=gnu17 --target=x86_64 */
#pragma ms_struct on
struct a { char x : 4; short y : 4; };
#pragma ms_struct reset
struct b { char x : 4; short y : 4; };
