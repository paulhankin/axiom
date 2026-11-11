/* C99: a narrow string literal joined to a wide one is wide. */
/* flags: --emit=c-tokens --c-std=c99 */
"a" L"b" "c"
