/* #embed (C23): the resource's bytes as integer constants, and its parameters. */
/* flags: --c-std=c23 -Iinc */
int a[] = {
#embed "inc/abc.bin"
};
int b[] = {
#embed "inc/abc.bin" limit(2) prefix(0, ) suffix(, 9)
};
int c[] = {
#embed "inc/empty.bin" prefix(bad) suffix(bad) if_empty(-1)
};
int d[] = {
#embed "inc/abc.bin" limit(0) if_empty(0)
};
#define LIM 1 + 1
#define NAME "inc/bytes.bin"
int e[] = {
#embed NAME __limit__(LIM) __prefix__(1 + )
};
int f[] = {
#embed <abc.bin> if_empty(x)
};
#if __has_embed("inc/abc.bin") == __STDC_EMBED_FOUND__ && __has_embed("inc/empty.bin") == __STDC_EMBED_EMPTY__
ok1
#endif
#if __has_embed("nope.bin") == __STDC_EMBED_NOT_FOUND__ && __has_embed("inc/abc.bin" limit(0)) == 2
ok2
#endif
#if __has_embed("inc/abc.bin" vendor::param) == 0 && __has_embed("inc/abc.bin" unknown) == 0
ok3
#endif
