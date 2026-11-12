/* The wide long double functions' values (std/c/rt/math_ld.ax), against
   correctly rounded results (written by
   cmd/axiom/internal/tests/tools/gen_ld_math_tests.py; do not edit). */
/* flags: --c-std=c11 */
#include "print.h"
#include <float.h>
#include <math.h>
#include <string.h>

typedef unsigned long long U;

#if __AXIOM_LDBL_WIDTH__ != 64
static long double ld(U lo, U hi)
{
    long double v;
    unsigned char b[16];
    memcpy(b, &lo, 8);
    memcpy(b + 8, &hi, 8);
    memset(&v, 0, sizeof v);
    memcpy(&v, b, sizeof v);
    return v;
}

/* r within n ulps of want (NaN for NaN; zeros and infinities exact) */
static int ok(long double r, long double want, int n)
{
    long double u;
    if (isnan(want))
        return isnan(r);
    if (want == 0 || isinf(want))
        return r == want && signbit(r) == signbit(want);
    if (isnan(r) || isinf(r) || signbit(r) != signbit(want))
        return 0;
    u = ilogbl(want) < LDBL_MIN_EXP - 1 ? LDBL_TRUE_MIN : scalbnl(1.0L, ilogbl(want) - (LDBL_MANT_DIG - 1));
    return fabsl(r - want) <= n * u;
}

#if __AXIOM_LDBL_WIDTH__ == 80
#define ULPS 1
static const U t_expl[][4] = {
    {0xb172000000000000, 0x400c, 0xfa13dba329dd768d, 0x7ffe},
    {0xb172178d4fdf3800, 0x400c, 0xffe560e6623252fc, 0x7ffe},
    {0xb16c000000000000, 0xc00c, 0x92cf78ab801959c5, 0x1},
    {0xb21c000000000000, 0xc00c, 0x1, 0x0},
    {0xb2a0000000000000, 0xc00c, 0x0, 0x0},
    {0xb2c0000000000000, 0xc00c, 0x0, 0x0},
    {0xa2425ff75e150000, 0x3f9b, 0x8000000000000000, 0x3fff},
    {0xa2425ff75e150000, 0xbf9b, 0x8000000000000000, 0x3fff},
    {0x8000000000000000, 0x3ffe, 0xd3094c70f034de4c, 0x3fff},
    {0x8000000000000000, 0x3fff, 0xadf85458a2bb4a9b, 0x4000},
    {0xba9b178f75fafc86, 0xc00a, 0xbc12c7519986bc1a, 0x2f2b},
    {0xd831f4f815235e2d, 0xbfff, 0xbd21e441d396e518, 0x3ffc},
    {0x84b3614c4c51f1d5, 0xbffc, 0xe0e26e3fcfcb8f9b, 0x3ffe},
    {0x95c3bd56d268dadb, 0xc00a, 0xf9f2d6e290cf7ed4, 0x327d},
    {0x914f3bb651468cd2, 0xbffe, 0x911ec3c6e7510611, 0x3ffe},
    {0x8b78e31f39f488f9, 0xbfe0, 0xfffffffdd21c7385, 0x3ffe},
    {0xb86f4846a6e4e301, 0x400b, 0xc9ca28b34e0e1243, 0x6141},
    {0xcc8977f66e3792ff, 0x3ffd, 0xbedb01a8c0ff75ce, 0x3fff},
    {0xb4ae1a9370599c49, 0x3ff0, 0x8001695e333defd5, 0x3fff},
    {0xea7eb9bd497c33c1, 0x4009, 0xad752264541306b8, 0x4a91},
    {0xc6cb7ba70c89f030, 0x3fff, 0x973b9c2732f4c191, 0x4001},
    {0xf489df04d675d790, 0xbff3, 0xffe170973b045bc2, 0x3ffe},
    {0xf9bb2e9e59c7c227, 0x400b, 0x8e046abe9fb398fe, 0x6d08},
    {0xa8fdaf4561d02031, 0xbfff, 0x88bd7dd34cce946e, 0x3ffd},
    {0xed109b189d9bc259, 0x3fce, 0x8000000000007688, 0x3fff},
    {0xa83489271576c958, 0xc00c, 0x927d4c553dfb4ec0, 0x354},
};
static const U t_exp2l[][4] = {
    {0xffff999999999800, 0x400c, 0xeedb4008bd1656cf, 0x7ffe},
    {0xfffa000000000000, 0xc00c, 0x5a827999fcef3242, 0x0},
    {0x8070666666666800, 0xc00d, 0x1c, 0x0},
    {0x80dd000000000000, 0xc00d, 0x0, 0x0},
    {0x8000000000000000, 0x3ffd, 0x9837f0518db8a96f, 0x3fff},
    {0xaa5106553c92b4e3, 0xc00c, 0xd65940a9e4f53a39, 0x156a},
    {0x97bb830c4a6ea1d0, 0xc000, 0xc5f9cd24cf358976, 0x3ffc},
    {0xa8ca19f2c6b89b8f, 0x4007, 0xbf327593898bd63b, 0x4150},
    {0xd9cfa3480b46cdd1, 0xbfff, 0x9d680d732ae9de1f, 0x3ffd},
    {0xa1e71afe7d9ab119, 0xc00b, 0x8a50c639f86922c2, 0x2bc2},
    {0xcb78604f2de0c5bd, 0x3fff, 0xc09ea181582d225c, 0x4000},
    {0xa36d56c3824196d2, 0xc003, 0xbe3b3f52649ffbbc, 0x3fea},
    {0x956156d6d99cbdb5, 0x3ffe, 0xbfcec2d2d4bf3eff, 0x3fff},
    {0xd3bfe95c13682810, 0xc00b, 0x80fc0dc25b0d3dc3, 0x2587},
    {0xce79f082a82582d5, 0xbffe, 0x925e173acf6ef52a, 0x3ffe},
    {0xa8c4002546b13f8c, 0xc00b, 0xb502aa92d557ba13, 0x2ae6},
    {0xe3de46acad2398f7, 0x3fff, 0xdbd38ec240fed7b3, 0x4000},
    {0x8983ece22460c902, 0x400c, 0xfcb56c1ceffff44b, 0x625f},
    {0xf225502dae5a0fc3, 0x3ffb, 0x8aee9840066d5294, 0x3fff},
    {0xbdaf5d2be6fa4c5a, 0xc00c, 0x8eea34db4defd5a6, 0x1093},
    {0xa806aa8a547368c2, 0x3ffd, 0xa0b1c44f702184fa, 0x3fff},
};
static const U t_expm1l[][4] = {
    {0xf79687aed3eec800, 0x3fab, 0xf79687aed3eec800, 0x3fab},
    {0xf79687aed3eec800, 0xbfab, 0xf79687aed3eec800, 0xbfab},
    {0xff972474538ef000, 0x3ff7, 0x804b7e8b3242900b, 0x3ff8},
    {0xff972474538ef000, 0xbff7, 0xfe989f85192d5367, 0xbff7},
    {0xf000000000000000, 0x4005, 0x8b6e454f7b3858b6, 0x40ac},
    {0xc800000000000000, 0xc006, 0x8000000000000000, 0xbfff},
    {0xbce42ecc17e93037, 0x4002, 0x82df8e6646dbd7b5, 0x4010},
    {0xac1024737c386f82, 0xbfd1, 0xac1024737c36a0eb, 0xbfd1},
    {0xed827c376d7ab0f7, 0x4001, 0xd0f58b3b24203d44, 0x4009},
    {0xa1147e5ac23b21a4, 0x3ff9, 0xa2ac951ab3c7042c, 0x3ff9},
    {0xfb481418f80b97c8, 0xc003, 0xfffffffffff99218, 0xbffe},
    {0xbe34b7f4841fedcb, 0x3ff9, 0xc06e67ae307840ad, 0x3ff9},
    {0xdeb922efc97453a8, 0x4003, 0x8f8797d952ac73da, 0x4027},
    {0xcf5e7f55b5958617, 0xbfbe, 0xcf5e7f55b5958617, 0xbfbe},
    {0x988f62a018de636d, 0xc003, 0xffffffe990354264, 0xbffe},
    {0xade54cce47ea75e0, 0x3fd0, 0xade54cce47eb621f, 0x3fd0},
    {0xdb0ea0f9308fbb2e, 0x4001, 0xeaab4cefbd4fbb45, 0x4008},
    {0x89fc69dd6c5dfd21, 0x3fd6, 0x89fc69dd6c832d32, 0x3fd6},
    {0xd6c2b8331c9282c2, 0xc001, 0xffb03c25ce58fb4b, 0xbffe},
    {0x9bd2ab823b6ba67d, 0x3fed, 0x9bd2c3387016d6c4, 0x3fed},
    {0x95c055bdc50d1bd4, 0x4004, 0x810335aae5337121, 0x4035},
    {0xdcce4b0a9079f2c3, 0x3fc3, 0xdcce4b0a9079f2cf, 0x3fc3},
};
static const U t_logl[][4] = {
    {0x8000000000000001, 0x3fff, 0xffffffffffffffff, 0x3fbf},
    {0x8000000000000000, 0x3fff, 0x0, 0x0},
    {0xb57fff94a0359800, 0x3fff, 0xb2cdaa6939be6412, 0x3ffd},
    {0xb583f91e646f1800, 0x3fff, 0xb2d8e0b2f30e004d, 0x3ffd},
    {0x8000000000000000, 0x4000, 0xb17217f7d1cf79ac, 0x3ffe},
    {0x20, 0x0, 0xb20d5bcccaa70f36, 0xc00c},
    {0x0, 0x0, 0x8000000000000000, 0xffff},
    {0xc3be74cfef7e66f4, 0x7a55, 0xa1bf8a019ae2807d, 0x400c},
    {0xdd7ed68585c51c06, 0x3ffe, 0x943ffd24332dbda6, 0xbffc},
    {0xe8893e5bd73f4292, 0x2303, 0xa0b470c5a88e097c, 0xc00b},
    {0xaedad742568ac092, 0x3ffe, 0xc32f6ed7bf9e57a9, 0xbffd},
    {0xc17c472dad316040, 0x251f, 0x950383f3f36d9696, 0xc00b},
    {0xb4d3780ed89d888b, 0x3fff, 0xb0e610bad9bcd07d, 0x3ffd},
    {0xa53b7c827b43bcd2, 0x364a, 0xd746e4baf40e437e, 0xc009},
    {0x894c46ad423d8aa4, 0x3fff, 0x8f9cb8ef391fde0b, 0x3ffb},
    {0xf76203e15ff21674, 0x670, 0x9f93758dc2fc84ad, 0xc00c},
    {0xedab4a36d5686aac, 0x3ffe, 0x982996132d3f692c, 0xbffb},
    {0xc7813a2f8d4eb951, 0x45b6, 0xfda12917fe4ca616, 0x4008},
    {0xa6312f32bdbfabf4, 0x3fff, 0x85b0c41f7a7f97d9, 0x3ffd},
    {0x85a5bc7644c9c438, 0x75e5, 0x95705a155e9179b7, 0x400c},
    {0x8982d41aa4e910a4, 0x3fff, 0x92c9d23cdfe99288, 0x3ffb},
    {0xcf268a62f043a1a7, 0x4d5d, 0x9446e5a6f68bd944, 0x400a},
    {0xb79851bd78de8419, 0x3fff, 0xb8aded95b17d49f6, 0x3ffd},
};
static const U t_log2l[][4] = {
    {0x8000000000000000, 0x4002, 0xc000000000000000, 0x4000},
    {0x8000000000000000, 0x3fff, 0x0, 0x0},
    {0x200000000000, 0x0, 0x8020000000000000, 0xc00d},
    {0xa39dbfb3f7c27cf4, 0x681b, 0xa0716aac063f43d9, 0x400c},
    {0xb42ea374cae1a1dc, 0x3fff, 0xfc9376093dcd2c71, 0x3ffd},
    {0xcae20bea32f87d56, 0xc0b, 0xcfcd578c3a83c95e, 0xc00c},
    {0xaa81465377fc4f2b, 0x3fff, 0xd3cc8ff1d36aa6b6, 0x3ffd},
    {0xf9faf92df872d404, 0x6e1e, 0xb87fdcd8d9bbb9bf, 0x400c},
    {0x9506dd66051ecdf2, 0x3fff, 0xe0b1c54a8545160f, 0x3ffc},
    {0xc663c737c9cfb243, 0x2fb4, 0x8252f144685b213d, 0xc00b},
    {0x92820450c699e056, 0x3fff, 0xc782adb0cf92db6d, 0x3ffc},
    {0xeacbeb8713685093, 0x1453, 0xaeac7fb9e1e28ff2, 0xc00c},
    {0xacc182134e31eb1d, 0x3ffe, 0x9141b5b6e0a52713, 0xbffe},
    {0xa30b8a3b4d426631, 0x1abf, 0x94fe9a7e6e4e4256, 0xc00c},
    {0x9ddab20e4cbd8c3e, 0x3fff, 0x9adaab458da1f523, 0x3ffd},
    {0x8706f58e39e74421, 0x102b, 0xbf4fb10b1ac00b0d, 0xc00c},
    {0x8d1393d675f71633, 0x3fff, 0x8fb39dbf54007be6, 0x3ffc},
    {0xb0ab65e835f22841, 0x53db, 0x9ee3b82268b15252, 0x400b},
    {0x96335084bccf4dbe, 0x3fff, 0xec487eee80064f26, 0x3ffc},
};
static const U t_log10l[][4] = {
    {0xfa00000000000000, 0x4008, 0xc000000000000000, 0x4000},
    {0xfffffffffffffffe, 0x3ffe, 0xde5bd8a937287196, 0xbfbe},
    {0xd1a082435e30ad46, 0x6042, 0x9b66bc4c8523f371, 0x400a},
    {0xa6b5d4a922ce8b66, 0x3fff, 0xeb034e981c2e8584, 0x3ffb},
    {0xa6fa53958dd9603d, 0x2840, 0xe4bb1191ecbffc69, 0xc009},
    {0x80f18fb5a2adc4eb, 0x3fff, 0xd10c45f2865d925d, 0x3ff6},
    {0xdb53c04971d72589, 0x7cd7, 0x92889de3ed6f1c60, 0x400b},
    {0xa7ec91721e50405e, 0x3fff, 0xf17726d3e72d9604, 0x3ffb},
    {0xb1ea81d30488bcf1, 0x796c, 0x8a4cae057c10e47e, 0x400b},
    {0xa3d4f0acd70f83a1, 0x3fff, 0xdb858034373bc650, 0x3ffb},
    {0xd848614be8152215, 0x520d, 0xadf2d498f4b898e1, 0x4009},
    {0xab5b3e610026570e, 0x3fff, 0x81bb21a2127454eb, 0x3ffc},
    {0xc7b3c617c1361752, 0x31c5, 0x89050eeb6a01d0e1, 0xc009},
    {0xbc2bd8354d5035d7, 0x3fff, 0xab5c0d89ba6a9a06, 0x3ffc},
    {0x82ff2c9b6f1ff91c, 0x46bd, 0x81e59ee4fb780a1f, 0x4008},
    {0x958082f39b1d1e13, 0x3fff, 0x8a1c6ca0827536f8, 0x3ffb},
    {0xd310d16993e9c7fc, 0x1f49, 0x9d89ba450ca60453, 0xc00a},
    {0xdc220996087b937a, 0x3ffe, 0x8641a4fc6b373169, 0xbffb},
};
static const U t_log1pl[][4] = {
    {0xa2425ff75e150000, 0x3f9b, 0xa2425ff75e150000, 0x3f9b},
    {0xa2425ff75e150000, 0xbf9b, 0xa2425ff75e150000, 0xbf9b},
    {0xfffffffffffffffe, 0xbffe, 0xaeac4f97f2883bc5, 0xc004},
    {0xb780346dc5d63800, 0x3ff6, 0xb73e8f84d3c500f4, 0x3ff6},
    {0xbe0ded288ce70000, 0xbff6, 0xbe549c7d18542040, 0xbff6},
    {0xbf21e44003ace000, 0x43e3, 0xacb1a23fc3fda9ac, 0x4008},
    {0xc06bf602109f9695, 0xbfe1, 0xc06bf604532810c9, 0xbfe1},
    {0xa1a227098ecfbf4f, 0x3ffd, 0x8c791a0ce44a9fb2, 0x3ffd},
    {0xff6fc15f3d8e6b1b, 0x401a, 0x9b3f51bebd67132d, 0x4003},
    {0xe258c79c7f319819, 0xbffd, 0x9563475924e8d311, 0xbffe},
    {0xf0ac5acb7f562392, 0xbfc6, 0xf0ac5acb7f562403, 0xbfc6},
    {0xec852f5e54e97f14, 0x3ffd, 0xc271b0b5ca6cd504, 0x3ffd},
    {0xa0f514d3d7f51c60, 0x3ff7, 0xa09036160d12a985, 0x3ff7},
    {0xca612002afb83385, 0x3ffa, 0xc589dcc71f6d1b73, 0x3ffa},
    {0xf02cd26c1613a2d6, 0xbffe, 0xb2265ea14f6d81ec, 0xc000},
    {0xd5a785016b3096bb, 0x3ff6, 0xd54e8e651bf120a3, 0x3ff6},
    {0x905af6f3663dcf0e, 0x3ffe, 0xe4f4204d9afac58c, 0x3ffd},
    {0xcaadcb7317f146c0, 0xbfc3, 0xcaadcb7317f146ca, 0xbfc3},
    {0xbe18fbffc008a628, 0xbffe, 0xadb24220c6cc29b9, 0xbfff},
    {0xb3858d6b9d27b5ac, 0x402b, 0xf6b1a5a38bbbf48a, 0x4003},
    {0x839bac1954014b66, 0x3fff, 0xb507513641956234, 0x3ffe},
};
static const U t_sinl[][4] = {
    {0xbf21e44003ace000, 0x43e3, 0xd160b580871c2b72, 0xbffe},
    {0xd1ba8323fe558c61, 0x73e6, 0x90de0837a745738a, 0x3ffe},
    {0xf333333333333333, 0x7ffe, 0xffffa2c0956d9c37, 0x3ffe},
    {0x8b61313bbabce000, 0x3f7a, 0x8b61313bbabce000, 0x3f7a},
    {0xb180000000000000, 0x4007, 0xfcde8183e8ef14ed, 0xbfef},
    {0xcb1c800000000000, 0x400f, 0xa077ef8f5c6f3414, 0xbfef},
    {0xc90fdaa22168c235, 0x4000, 0xece675d1fc8f8cbb, 0xbfbe},
    {0xc9b1c0d91de6bd5d, 0xc001, 0xa1e3844f29c0e7bd, 0xbff9},
    {0xcd0ee888d4bac37c, 0xc170, 0xdb83d42701de6c63, 0xbffe},
    {0xfe5260d70621f9ce, 0x63ca, 0x975295b74da841d7, 0xbffd},
    {0xdc6387aea9de9b50, 0x3fff, 0xfd165cfd799e926b, 0x3ffe},
    {0xecd9574f1128a768, 0x43da, 0x9f5312246af74e1d, 0xbff9},
    {0x837d2b5ed96a5e03, 0xdc9c, 0xc6d9873f434a8b53, 0x3ffe},
    {0x887c97441ed2dd2e, 0x4002, 0xc7a18bec209a8abc, 0x3ffe},
    {0xc3873ef5185fa619, 0x427d, 0x970d02606359df4a, 0x3ffe},
    {0xb4014fa5f2c6cece, 0x6ed1, 0xd6695f523d63a481, 0xbffe},
    {0x98b52d615a826a8a, 0x4001, 0xff8b23b4b7d95180, 0xbffe},
    {0xebb5a8b0ad3b76eb, 0x43b1, 0xfb5b691fe029bd24, 0x3ffe},
    {0xe46170adfdabe157, 0x5626, 0x8bc3375843e63fcc, 0x3ffe},
    {0x837bacaba74f8233, 0xc002, 0xef40ce738aba19e8, 0xbffe},
    {0xb8803964455263c3, 0x410a, 0xd43e1f48a693a850, 0xbffe},
    {0xabffe6e2bd299a1f, 0xff52, 0xbf34016ed72cd010, 0xbffd},
    {0x8b9c5ecece08facf, 0x4002, 0xa4be66d38b7d859c, 0x3ffe},
};
static const U t_cosl[][4] = {
    {0xbf21e44003ace000, 0x43e3, 0x934c81156e260612, 0xbffe},
    {0xd1ba8323fe558c61, 0x73e6, 0xd31143593ff9dec5, 0x3ffe},
    {0x8b61313bbabce000, 0x3f7a, 0x8000000000000000, 0x3fff},
    {0xb180000000000000, 0x4007, 0xfffffffe0c725f3d, 0xbffe},
    {0xc90fdaa22168c235, 0x3fff, 0xece675d1fc8f8cbb, 0xbfbd},
    {0xc90fdaa22168c235, 0x3ffe, 0xb504f333f9de6484, 0x3ffe},
    {0x899634a95460064e, 0x3ff8, 0xfffdb06fffdcfd68, 0x3ffe},
    {0x98996e59e4812a4b, 0xc181, 0xa8ca8e1533104f1c, 0x3ffe},
    {0x9acd109f31f53726, 0xd8b0, 0xff2168e8f10e9127, 0x3ffe},
    {0xd6bffd7527aea410, 0xc000, 0xfa2aceabdd1c2428, 0xbffe},
    {0xcd0c4921dcf90c17, 0x40ff, 0xfa41bc9c5c011e9f, 0xbffe},
    {0xb89a7c93d2419e7b, 0x5d59, 0x95699150ea1db6ac, 0xbffc},
    {0xad87af9a4aa570ec, 0x4001, 0xa6f3a8c0608d07c0, 0x3ffe},
    {0xcec00b2a7fe2ceef, 0xc09d, 0x8ebc15f25dca3a60, 0xbffd},
    {0xbd9974005ba78b68, 0xdb6a, 0xc49917afc7315679, 0x3ffd},
    {0xa314f8d17c7e106f, 0x4001, 0xbfc63176e75a4636, 0x3ffd},
    {0xe138c79de1a75f28, 0x40ab, 0xe8e8a5673827d899, 0xbffd},
    {0xb3479c381da47da8, 0x77d5, 0xeb6a85f5a124bc60, 0xbffd},
    {0xaa89a44b714439bc, 0xc001, 0x9419e223076b8496, 0x3ffe},
    {0x98764e907ca3c269, 0x4130, 0xecbb847067f748bd, 0x3ffc},
    {0xce6ef951281659d7, 0xcd73, 0x8e0e29eb30e3ff50, 0x3ffc},
    {0xc7c8a15716dec118, 0x4000, 0xfff2ee0125038265, 0xbffe},
};
static const U t_tanl[][4] = {
    {0xc90fdaa22168c235, 0x3fff, 0x8a51e04daabda35f, 0xc040},
    {0x878678326eac9000, 0x4048, 0xd07bce0db592bba5, 0xbfff},
    {0x8b61313bbabce000, 0x3f7a, 0x8b61313bbabce000, 0x3f7a},
    {0xc90fdaa22168c235, 0x3ffe, 0x8000000000000000, 0x3fff},
    {0x950b0296c970da41, 0x4001, 0x91da471031ede854, 0x4003},
    {0xb3b7504381859359, 0xd1a4, 0x8bb062b30f240498, 0x4000},
    {0xd03146956cc38109, 0x4001, 0xe807e4c28b9afba9, 0x3ffc},
    {0x8ab19a06a23a499b, 0xd96f, 0xb652e214ecddbf68, 0x3fff},
    {0xf5359482d80beb62, 0x4001, 0xa553d426e68f281b, 0x4001},
    {0xee45febbcc917181, 0x6c8c, 0xd1b57bfff1c445e5, 0x3ffb},
    {0xf946daee2f2fe446, 0x4001, 0xf9564525e440845b, 0x4002},
    {0xb0215862c93c9b5c, 0xe8ef, 0xa09eea91a47a242c, 0xbfff},
    {0xca9e5cc349864123, 0x3ffd, 0xd5e74590e114dfc5, 0x3ffd},
    {0x85bc4bb5b8957c2f, 0x6861, 0xbf29b2a8ad8f9a17, 0x3ffc},
    {0xc1ce6b912ad05884, 0xc001, 0xec3dd6eacbb1973b, 0x3ffc},
    {0xa5087e4e47404324, 0x714b, 0x89c766c25e99bd19, 0x3fff},
    {0xf4053544d024a5cf, 0xc001, 0x89b1af9460f53e98, 0xc001},
    {0xce3bcdbe6c75a43e, 0xd0a7, 0xcf28b47e73bacd99, 0x3ffe},
    {0x875e757b3a912e7a, 0xc002, 0xb87b0a4d22ada5f7, 0x3fff},
    {0x942fd2a25fc34c87, 0x626d, 0xd12f8f61aadaba1e, 0xbfff},
};
static const U t_atanl[][4] = {
    {0x9c3d73864f3805c0, 0xc17, 0x9c3d73864f3805c0, 0xc17},
    {0xd1ba8323fe558c61, 0x73e6, 0xc90fdaa22168c235, 0x3fff},
    {0x8000000000000000, 0x3ff9, 0xfffaaadddb94d5bc, 0x3ff8},
    {0x8000000000000000, 0x4005, 0xc70fe54c65b19889, 0x3fff},
    {0x8000000000000000, 0x3fff, 0xc90fdaa22168c235, 0x3ffe},
    {0xb9da6d6ae4b98b9a, 0xc026, 0xc90fdaa220b8728c, 0xbfff},
    {0xe7c03bc2dd897f5a, 0x3fff, 0x887873647cbb3fc0, 0x3fff},
    {0xbbaa594c7e266eb6, 0xbff2, 0xbbaa592ae057fdcc, 0xbff2},
    {0xd6d65c2f06f7ebde, 0x3fff, 0x8448c7e062003aae, 0x3fff},
    {0xf13dab07775c4bc9, 0x4018, 0xc90fda802c391313, 0x3fff},
    {0xdb6b76dbcf34e0f4, 0xbfff, 0x857756d92babb47e, 0xbfff},
    {0xa0a051b5a2a1adfa, 0x3ff9, 0xa09b0cf8423a2e84, 0x3ff9},
    {0x99a727a2cb7372a7, 0x4000, 0x968b5906502e69b6, 0x3fff},
    {0xa1c808b0162f1de6, 0x3fe8, 0xa1c808b0162dc54f, 0x3fe8},
    {0xcda6770194f4eed1, 0x3ffb, 0xccf694520f1dc188, 0x3ffb},
    {0xb48e1016034f25be, 0xc009, 0xc8f92b1dcaf4e265, 0xbfff},
    {0xf3e5d6dae9bc38ee, 0xbfff, 0x8b33250e8bb1b400, 0xbfff},
    {0xe8831fda1aae74bc, 0xbfdd, 0xe8831fda1aae74bc, 0xbfdd},
    {0xaad6470aec73de2e, 0xc000, 0x9b2e6692418e12d1, 0xbfff},
    {0xe258e61247424fac, 0x4021, 0xc90fdaa20f5028f8, 0x3fff},
    {0xb2c4c6991380bbd3, 0xbfff, 0xf30c4ec6a20c2881, 0xbffe},
};
static const U t_sinhl[][4] = {
    {0xb174000000000000, 0x400c, 0xce2773666cc8cc68, 0x7ffe},
    {0xb176000000000000, 0xc00c, 0x8000000000000000, 0xffff},
    {0xa2425ff75e150000, 0x3f9b, 0xa2425ff75e150000, 0x3f9b},
    {0x8000000000000000, 0x3ffd, 0x815666ce84080f33, 0x3ffd},
    {0x86068915e2b26013, 0xc004, 0xa1f5b2db4602aa80, 0xc02e},
    {0xc6e881f92eee681f, 0x3fd9, 0xc6e881f92eee681f, 0x3fd9},
    {0x8cf005f6ef18e886, 0xc004, 0xe3efdaf5cbd8a0a0, 0xc030},
    {0xfb1fbabde2544f72, 0xbfcc, 0xfb1fbabde2544f72, 0xbfcc},
    {0xa6d6e3af09dab24d, 0x4004, 0x907799127186e7e1, 0x403a},
    {0xc609b7dbc080a678, 0xbfee, 0xc609b7dbd44132bd, 0xbfee},
    {0x814d7e5a65212345, 0xc004, 0xc6ed475965c24371, 0xc02c},
    {0xc86bb861402eebf6, 0x4008, 0xbfe88d1d6a5b623a, 0x4482},
    {0xce6188931c87ca36, 0xc004, 0xad30840a04cf6df5, 0xc048},
    {0xbefb547509e68faf, 0xc004, 0xebe99f7323633ecc, 0xc042},
    {0xa1228db11fba583d, 0x4004, 0x8ad407037f1fd831, 0x4038},
    {0x81e0d425c60cc3ba, 0x3fd3, 0x81e0d425c60cc3ba, 0x3fd3},
    {0x81d14280490bdf94, 0xc001, 0xe715fa28d2bc05e4, 0xc003},
    {0x877b208fdc40393f, 0xbff7, 0x877b39dbd1659cdc, 0xbff7},
    {0xae718464777c1967, 0x4004, 0xf1b72f08660e4fa3, 0x403c},
    {0x9a00204e7c31c773, 0xbff2, 0x9a002057c5fe4abd, 0xbff2},
};
static const U t_coshl[][4] = {
    {0xb174000000000000, 0x400c, 0xce2773666cc8cc68, 0x7ffe},
    {0xb178000000000000, 0xc00c, 0x8000000000000000, 0x7fff},
    {0xa2425ff75e150000, 0x3f9b, 0x8000000000000000, 0x3fff},
    {0xbdd63cc9a17d94c5, 0x4001, 0xbc87e8a632a11d8f, 0x4006},
    {0xa72024cb5e71d008, 0x3fee, 0x800000001b46bd03, 0x3fff},
    {0xa1d4c416af35fbe4, 0xc002, 0xc0f2622350e3f523, 0x400c},
    {0x9ac23bef4d9f7a96, 0x3fe9, 0x800000000005d8e4, 0x3fff},
    {0x9deffd3244dbbe92, 0x4003, 0xb2c4b8c2c0950ebb, 0x401a},
    {0x92ffc292bb5a5093, 0xbfdb, 0x8000000000000000, 0x3fff},
    {0xe6ea3017a3075b72, 0x4002, 0xe22a0b1342eb5030, 0x4012},
    {0xa625a0d944ccfda4, 0x3ffe, 0x9beadfaa81330a18, 0x3fff},
    {0xc3722b616781ca26, 0xc004, 0xb40c2f3138d6456a, 0x4044},
    {0x8cddb749a55caead, 0xbfff, 0xd5a89df57cf0ab9f, 0x3fff},
    {0xec8974778c4a6139, 0xc004, 0x9ef9db51f8f98d48, 0x4053},
    {0xdfdf0efda4c47c17, 0x3fe4, 0x800000000000030f, 0x3fff},
    {0xe14f71732c1093c7, 0x4003, 0xc6548c47e2209c8c, 0x4026},
    {0xe4a0375138f88c55, 0x3fcb, 0x8000000000000000, 0x3fff},
    {0x8442966d6c73f4f9, 0x4003, 0xe6f0adbf3cdb2ebd, 0x4015},
    {0xa0042cc0665d8d2b, 0x3ff3, 0x8000006405380ef4, 0x3fff},
};
static const U t_tanhl[][4] = {
    {0xa2425ff75e150000, 0x3f9b, 0xa2425ff75e150000, 0x3f9b},
    {0xf000000000000000, 0x4003, 0x8000000000000000, 0x3fff},
    {0xc800000000000000, 0x4004, 0x8000000000000000, 0x3fff},
    {0x8000000000000000, 0xbffe, 0xec9a9ebab4579b29, 0xbffd},
    {0xf755592187c4f281, 0xc000, 0xffc660ec7c8b9afd, 0xbffe},
    {0x843df612011637a2, 0xbfd8, 0x843df612011637a2, 0xbfd8},
    {0xf4cb956d6d75650a, 0x3ffd, 0xe3b414c8633c0850, 0x3ffd},
    {0x873dc7d85f169105, 0x4003, 0xfffffffffffed56a, 0x3ffe},
    {0xbb9511eaf093d25d, 0xc001, 0xfffef047509315c1, 0xbffe},
    {0xdb4a9d86bd0bf349, 0x3fdc, 0xdb4a9d86bd0bf349, 0x3fdc},
    {0x96874fc24a16b389, 0xc002, 0xffffffc62762e277, 0xbffe},
    {0xc21fc2b6acc5c9cb, 0x3fca, 0xc21fc2b6acc5c9cb, 0x3fca},
    {0xa856f171b4c28660, 0x4001, 0xfffc7775d5c0f917, 0x3ffe},
    {0xb525ecf2f3bc4bce, 0xbfdf, 0xb525ecf2f3bc4bce, 0xbfdf},
    {0xe4c7380b5737149a, 0x4001, 0xffffeb4d49eba465, 0x3ffe},
    {0xc9616e8c96108adf, 0x3ff4, 0xc9616bf3f81338d8, 0x3ff4},
    {0x9a4333a841dc775b, 0xc001, 0xfff77bb8bb93bf2a, 0xbffe},
    {0xf3964f52c9c433ca, 0x3fe1, 0xf3964f52c9c433b8, 0x3fe1},
    {0xe232f8893e1d400e, 0xc001, 0xffffe7ae7afea79e, 0xbffe},
    {0xdbc63a71df49b22f, 0x3fef, 0xdbc63a710751d9e7, 0x3fef},
};
static const U t_cbrtl[][4] = {
    {0xd800000000000000, 0x4003, 0xc000000000000000, 0x4000},
    {0x20, 0x0, 0x8000000000000000, 0x2a97},
    {0x8000000000000000, 0x7ffe, 0x8000000000000000, 0x5554},
    {0xb88556da4ae17007, 0x4bc9, 0x90987a76b758e3a6, 0x43ed},
    {0xba9a244db56ab149, 0x7ac9, 0xe664182ffc565993, 0x5397},
    {0xb89d67fc06a6e3a1, 0x5ebb, 0xe591f96618417a29, 0x4a3d},
    {0xfb4a148e2985fa7e, 0x7513, 0xc9eeef23aeceda83, 0x51b0},
    {0xa697db9b9101a263, 0x7788, 0x8bc098dde6ec10ea, 0x5282},
    {0xa53ae50dd4681aa6, 0xbfb, 0xaf98687850a8e0b2, 0x2ea8},
    {0x999680a5f16b753e, 0xb12, 0xd7e9c070ecbe50c4, 0x2e5a},
    {0xe8d45a6bde43acbc, 0x6ea5, 0xf807f0447264c9dc, 0x4f8b},
    {0x877056e13ec2982c, 0x27b4, 0x826edd7a1c30217f, 0x37e6},
    {0xb6af0e59178ae308, 0xae1, 0xb5928d5ecdc7dfb6, 0x2e4a},
    {0xc670db03f70e796c, 0x35b5, 0x9424d43dd74387a5, 0x3c91},
    {0x923423bdc7b27fc2, 0x633a, 0xa894269bd65016d9, 0x4bbd},
    {0xf6c64569e8021afd, 0x6ce1, 0x9f4f14d9b99b3b05, 0x4ef5},
};
static const U t_asinl[][4] = {
    {0x8000000000000000, 0x3fff, 0xc90fdaa22168c235, 0x3fff},
    {0x8000000000000000, 0xbfff, 0xc90fdaa22168c235, 0xbfff},
    {0x8000000000000000, 0x3fff, 0xc90fdaa22168c235, 0x3fff},
    {0xa2425ff75e150000, 0x3f9b, 0xa2425ff75e150000, 0x3f9b},
    {0xe3a877acd9108b6b, 0xbffe, 0x8c42d02b7019264e, 0xbfff},
    {0xa7f9b4de2605038c, 0xbff9, 0xa7fcb86ccd5b6dbb, 0xbff9},
    {0xe4c26d05472673b4, 0xbffe, 0x8d79f2174c3bf57a, 0xbfff},
    {0xd3cec0e0fbead1ee, 0x3fc5, 0xd3cec0e0fbead1ee, 0x3fc5},
    {0xd110ddd45a7838cc, 0xbffd, 0xd75c449dc59f4055, 0xbffd},
    {0xda523c7de00e1a82, 0xbfe3, 0xda523c7de00e1aec, 0xbfe3},
    {0x8363146a584d71f1, 0x3ffe, 0x89f8480ffa66ebbe, 0x3ffe},
    {0xfbfe1fb838215e41, 0xbfd8, 0xfbfe1fb838215e41, 0xbfd8},
    {0xaea7405c86d3ad3c, 0x3ffe, 0xc035d524e8d8444f, 0x3ffe},
    {0x9d7bf38ae7b11097, 0xbfd8, 0x9d7bf38ae7b11097, 0xbfd8},
    {0xf0cd1e65b6906c99, 0x3ffd, 0xfab3175c9f139b2d, 0x3ffd},
    {0xebf62b79c78e38a8, 0xbfe4, 0xebf62b79c78e3abf, 0xbfe4},
    {0xfb82420abfb3d36d, 0xbffe, 0xb10cde7d60a244ee, 0xbfff},
    {0xf98dabf4fd60b342, 0x3fee, 0xf98dabf524e6cec0, 0x3fee},
    {0x83fdb2873b76e36c, 0xbffe, 0x8aac959c1d14c545, 0xbffe},
    {0x9e7b5904938ebc6a, 0x3fc5, 0x9e7b5904938ebc6a, 0x3fc5},
};
static const U t_acosl[][4] = {
    {0x8000000000000000, 0x3fff, 0x0, 0x0},
    {0x8000000000000000, 0xbfff, 0xc90fdaa22168c235, 0x4000},
    {0x8000000000000000, 0xbfff, 0xc90fdaa22168c235, 0x4000},
    {0xa2425ff75e150000, 0x3f9b, 0xc90fdaa22168c235, 0x3fff},
    {0xd610c0c4f6d063ac, 0xbffe, 0xa3e91451b8ed832a, 0x4000},
    {0xd8b0375fb7d04de9, 0xbff8, 0xcac13e4cef260601, 0x3fff},
    {0xd1e6ae83528fa705, 0x3ffd, 0x92fe349a6753ad30, 0x3fff},
    {0xf9a6b650a5192014, 0xbff2, 0xc917a7d7d52a7b12, 0x3fff},
    {0xa08dc737d05c8b16, 0x3ffe, 0xe49476dcf2acd172, 0x3ffe},
    {0x9bdaa8939b1271a4, 0x3fe2, 0xc90fda9d42937d98, 0x3fff},
    {0xb2ba990534b24281, 0x3ffd, 0x9b6b2502f755d89b, 0x3fff},
    {0xfd14cb8c98d91ce3, 0xbfe9, 0xc90fde967496f498, 0x3fff},
    {0xdb5c7b8e8842d55c, 0x3ffe, 0x8aa6f04ffe86c3a2, 0x3ffe},
    {0xab44e5771839c938, 0xbfe6, 0xc90fdaf7c3db7dc1, 0x3fff},
    {0xb608182c0cc5cd38, 0x3ffe, 0xc7a0560903057b5d, 0x3ffe},
    {0xb4403497247d8681, 0x3ff3, 0xc904969ed43d492f, 0x3fff},
    {0x9e74c41c57ad6744, 0xbffc, 0xdcf2e834f3b4219b, 0x3fff},
    {0xb47588dd28c90967, 0xbff6, 0xc96a156e08ff2f53, 0x3fff},
    {0xb7d21773d3adc96f, 0x3ffd, 0x9a0eb2d33f20dc92, 0x3fff},
    {0xcddd7490e39e3d89, 0x3fe2, 0xc90fda9bb27d1dae, 0x3fff},
};
static const U t_asinhl[][4] = {
    {0xd1ba8323fe558c61, 0x73e6, 0x8fec2252d887ff4c, 0x400c},
    {0xa2425ff75e150000, 0xbf9b, 0xa2425ff75e150000, 0xbf9b},
    {0xfd559f14c654d771, 0xbfdb, 0xfd559f14c654d771, 0xbfdb},
    {0x9c712756f8a1a457, 0x4017, 0x8c3c13cadc7a96b3, 0x4003},
    {0xab649bff00a9b47d, 0xc019, 0x980e230f1f9153fd, 0xc003},
    {0xec30090a0188d41c, 0x3fe2, 0xec30090a0188d3fa, 0x3fe2},
    {0xa31503eb7d93bf6d, 0xc01e, 0xb3622acf1a5b2557, 0xc003},
    {0xbc0306eb39a8a49a, 0x4036, 0x9ccd88adf75618fd, 0x4004},
    {0x8bfbb3e43b21376f, 0x4031, 0x8dc28ef4a94e58d0, 0x4004},
    {0x9dc2aca94efc3da0, 0x3fdc, 0x9dc2aca94efc3da0, 0x3fdc},
    {0xaeac31467d83b83d, 0x3ff5, 0xaeac2de3154857e0, 0x3ff5},
    {0xa4f8170fc21bb861, 0xc014, 0xf80c25f9a892a078, 0xc002},
    {0xb76fbcc16d663718, 0x4030, 0x8c119c233fc26d4f, 0x4004},
    {0x9298e205dc9bfe60, 0x3fc4, 0x9298e205dc9bfe60, 0x3fc4},
    {0xb498929f5064450d, 0x4055, 0xf29796f9f45220be, 0x4004},
    {0xe9d9c5bb2c5457c3, 0x4006, 0xc4bb0f735408963e, 0x4001},
    {0xb9c65897ca6e6b06, 0xbfff, 0x956ff69604078bee, 0xbfff},
    {0x8be2938df4ffcbb2, 0xc007, 0xca77c1923c710874, 0xc001},
};
static const U t_acoshl[][4] = {
    {0x8000000000000000, 0x3fff, 0x0, 0x0},
    {0x8000000000000001, 0x3fff, 0x8000000000000000, 0x3fe0},
    {0xd1ba8323fe558c61, 0x73e6, 0x8fec2252d887ff4c, 0x400c},
    {0xe9382237056eb468, 0x4012, 0xe76801e742a3b817, 0x4002},
    {0xa3c43b024ad79112, 0x404d, 0xdc0528e35ae2c61a, 0x4004},
    {0x866cee50bbfda7d4, 0x4032, 0x905edb470a06add9, 0x4004},
    {0xe861a3370d9656e5, 0x4029, 0xf336a3341504c71d, 0x4003},
    {0xd010ec74f32d1a07, 0x4028, 0xecc8b7fc7b90f81e, 0x4003},
    {0xdf1e26b13ccaf47b, 0x4004, 0x96dcf4e48e501461, 0x4001},
    {0xdb4c7f1c22e5e0f8, 0x4029, 0xf2bff77af3c55587, 0x4003},
    {0xc5d0eb7834f1bc4a, 0x4047, 0xcc23e497472fbac8, 0x4004},
    {0x81beeb7a9e727dba, 0x404d, 0xdb16b4c81f3d5f8f, 0x4004},
    {0x83d56f18a5ce90c3, 0x402c, 0xff5072d4ab5c1c02, 0x4003},
    {0xa8884f5ef6004e15, 0x401c, 0xa88e5d1c4789ec70, 0x4003},
    {0xb8e9e32dc7813312, 0x4048, 0xcea49b9fce3bb5d0, 0x4004},
    {0x9f2c3a3ddeadefcc, 0x4032, 0x910be45b7b798dbe, 0x4004},
    {0xf7bb689543ecf78f, 0x4013, 0xf376688e120fe5fc, 0x4002},
    {0xe56f0e6f6dc71c1d, 0x401e, 0xb61d463af16564e8, 0x4003},
    {0xcb0c17a9d3240a0b, 0x4026, 0xe17f94d5f5c38d1f, 0x4003},
};
static const U t_atanhl[][4] = {
    {0x8000000000000000, 0x3fff, 0x8000000000000000, 0x7fff},
    {0xa2425ff75e150000, 0xbf9b, 0xa2425ff75e150000, 0xbf9b},
    {0xee6d136be42e9b5c, 0xbffe, 0xd5911af9bda7dc4a, 0xbfff},
    {0x9fb44451f4d99701, 0x3fe4, 0x9fb44451f4d9984c, 0x3fe4},
    {0xe5b742baafccaa0a, 0xbffe, 0xbaaa8702e5718285, 0xbfff},
    {0xdba28acdcd170fc0, 0x3fe2, 0xdba28acdcd170ff6, 0x3fe2},
    {0xab97cac591328e35, 0xbffa, 0xabb1845c7a6d1845, 0xbffa},
    {0x8dd573507b6492dc, 0x3fc4, 0x8dd573507b6492dc, 0x3fc4},
    {0xc7150e3c89d42995, 0xbffb, 0xc7b68084e55339a7, 0xbffb},
    {0x89e6d960df50b7e3, 0xbfd1, 0x89e6d960df50b7e3, 0xbfd1},
    {0xf2cab21c941770f6, 0xbffe, 0xe86807479fd12c02, 0xbfff},
    {0xd751c1b5ce62c85f, 0xbfdc, 0xd751c1b5ce62c85f, 0xbfdc},
    {0xd0094a3f2fa865b8, 0x3ffe, 0x913fc8ab18933bc5, 0x3fff},
    {0xa62763cb79cc74fd, 0xbffe, 0xc60cfc3e348ed6da, 0xbffe},
    {0x836696b144a1ae0d, 0xbfe2, 0x836696b144a1ae19, 0xbfe2},
    {0xaafbb25e39f2fa6e, 0xbffe, 0xce9421f43ee04922, 0xbffe},
    {0xa6463e14b34db6a1, 0xbfdb, 0xa6463e14b34db6a1, 0xbfdb},
};
static const U t_powl[][6] = {
    {0x8000000000000000, 0x4000, 0xfffe000000000000, 0x400c, 0xb504f333f9de6484, 0x7ffe},
    {0x8000000000000000, 0x4000, 0x8071000000000000, 0xc00d, 0x17, 0x0},
    {0x8000000000000001, 0x3fff, 0x8ac7230489e80000, 0x403e, 0xbd40caf4113ebe73, 0x4000},
    {0x8000000000000000, 0xc000, 0xc000000000000000, 0x4000, 0x8000000000000000, 0xc002},
    {0x8000000000000000, 0xc000, 0xc9f2c9cd04675000, 0x4062, 0x8000000000000000, 0x7fff},
    {0xa000000000000000, 0x4002, 0x9a20000000000000, 0x400b, 0xd72cb2a95c7ef6cd, 0x7ffe},
    {0x8000000000000000, 0x3ffe, 0x80dc000000000000, 0x400d, 0x0, 0x0},
    {0x9ce671165ef79ce9, 0x4007, 0xc7af82024c359001, 0x4004, 0x82fb4a9c00831349, 0x419d},
    {0xdbcabd8a9513c32a, 0x3fff, 0xf0aeefb5a1d7afe1, 0x4002, 0xd4c84532c2c1a061, 0x400a},
    {0xcfe65b90260e352c, 0x7e30, 0xe6d2d78f9b6980fd, 0xbffc, 0x82cbb4d62563ee6e, 0x31fa},
    {0xddb9a92c6753b356, 0x4002, 0xa73217ff989bd8df, 0xc006, 0xed01f56cd8c39632, 0x3d84},
    {0x893f0cd82773bfcd, 0x3fff, 0xb6246130c6c7b95e, 0xc00c, 0x822a2d53ad1974ad, 0x3b6a},
    {0xf0f04a6ff913d996, 0x5067, 0xd5e9ffc1b1b3a399, 0x3ffe, 0x9c805d0d38a1aeee, 0x4db5},
    {0xfc393de1597fb0a0, 0x3ff8, 0xeb5bbca7d62d9a00, 0x4002, 0xabe07afcd1fb481e, 0x3fa6},
    {0xa05eec20a71967b4, 0x3fff, 0xcb7c253fc83d96f7, 0x4007, 0xa5e4b90046559e05, 0x4083},
    {0xa8a35f6de2add586, 0x7537, 0xa9c6889cff6f3d0b, 0x3ffe, 0xb5a1ae1cad1f6d23, 0x634a},
    {0xb755753f23071165, 0x4005, 0xb8174401fc8069df, 0x4006, 0xf9db4fb92b65787c, 0x44ae},
    {0xeb2e08009ff4d827, 0x3fff, 0xefe62d9f9c2d961f, 0xc002, 0xe55241eb626e4b3b, 0x3ff1},
    {0x81d658ed2a6f4304, 0x3784, 0xaf070f9a7449b88d, 0x3ffa, 0x963da282d12dd729, 0x3fa2},
    {0xcf07b6fa264e151b, 0x3ffc, 0xacf0bed3cff7d5b4, 0x4006, 0x8dbaf9300db2bb04, 0x3e70},
    {0x9534046fc6656ead, 0x3fff, 0xbd0985f227634c03, 0x400a, 0xe58bd4d2226819b2, 0x429b},
    {0x877d6902a1de6709, 0x79a8, 0xeae586199d687d12, 0x3ffc, 0x85a2ae643217fa7d, 0x4d39},
    {0xac6847f74a946e08, 0x3ffc, 0xbc99660ae3244f4f, 0x4004, 0xe0612b317cdc7ce8, 0x3f85},
};
static const U t_atan2l[][6] = {
    {0x9c3d73864f3805c0, 0xc17, 0x8000000000000000, 0xbfff, 0xc90fdaa22168c235, 0x4000},
    {0x8000000000000000, 0xbfff, 0x9c3d73864f3805c0, 0xc17, 0xc90fdaa22168c235, 0xbfff},
    {0x8000000000000000, 0x3fff, 0x8000000000000000, 0x3fff, 0xc90fdaa22168c235, 0x3ffe},
    {0xa2425ff75e150000, 0x3f9b, 0xc9f2c9cd04675000, 0x4062, 0xcdb0255565313563, 0x3f37},
    {0xc5992e7c7901c94a, 0x3fed, 0xf951fb79498eda5f, 0x4019, 0xcae4741220111cc4, 0x3fd2},
    {0xedfe57876e7bd57d, 0xc019, 0x9d23596a108cce2e, 0xc00f, 0xc924fb7fbe6c3cff, 0xbfff},
    {0xd2fed067a243942c, 0x401f, 0xf3d2dff89d943d5a, 0x3ff4, 0xc90fdaa2215644eb, 0x3fff},
    {0xa6aaf184fd469663, 0x401d, 0x9a42a187899ac447, 0xc00d, 0xc910511abd04ec21, 0x3fff},
    {0xf76a0474e80033b8, 0x401e, 0xc66c27c9db69ef01, 0x3ff5, 0xc90fdaa221356e7b, 0x3fff},
    {0xd2a6a1a52dc66a9e, 0x3fe5, 0xb48cb279f291c0d5, 0x3fe3, 0xae0b0e990b587269, 0x3fff},
    {0xb12de3651857b250, 0x3fe5, 0xd2e787c17671596f, 0xbfd9, 0xc9196075a6f0d130, 0x3fff},
    {0xd0461cfc80c67963, 0x4007, 0xa5a91fbe500cb019, 0xc022, 0xc90fda9812990a5d, 0x4000},
    {0xe637b69c2c803cf7, 0xbfe3, 0xd414729dfed05474, 0xbfe6, 0xc06e32a462c593b8, 0xc000},
    {0xe4bfe95cca2d8450, 0x3ffb, 0xcf4f4a01439f67c0, 0x3fe3, 0xc90fda2e20a49ec8, 0x3fff},
    {0xff9fd1b2434d881c, 0x4022, 0x946286abd3a81143, 0x4013, 0xc90f4607c5fa0b4e, 0x3fff},
    {0xb9ba97761e78f738, 0x3feb, 0xda7d9e8944c2e7e1, 0x4021, 0xd99d2205af284904, 0x3fc8},
    {0xebe8f8e6c99ee4c6, 0xc00e, 0xf6e574a363d752f0, 0x3ffc, 0xc90fb924a1a4d237, 0xbfff},
    {0xd07ed0261392b042, 0xbffc, 0x9b7aa2fa1a8763b7, 0x3fdc, 0xc90fdaa1c1f50601, 0xbfff},
    {0xcf04cdf68fb2f271, 0xbff2, 0xb6a792fb0d285363, 0xc00b, 0xc90fda7ddcaef674, 0xc000},
    {0xe64e6a73d79efe0e, 0x3fd7, 0xfc8957f4677e0803, 0xc024, 0xc90fdaa22168c235, 0x4000},
};
static const U t_hypotl[][6] = {
    {0x8000000000000000, 0x7ffe, 0x8000000000000000, 0x7ffe, 0xb504f333f9de6484, 0x7ffe},
    {0x20, 0x0, 0x20, 0x0, 0x2d, 0x0},
    {0xc000000000000000, 0x4000, 0x8000000000000000, 0x4001, 0xa000000000000000, 0x4001},
    {0x9c3d73864f3805c0, 0xc17, 0x8000000000000000, 0x3fff, 0x8000000000000000, 0x3fff},
    {0xbf7da5b0124262b1, 0x77a6, 0xaf4f6499d0bdece4, 0x1e89, 0xbf7da5b0124262b1, 0x77a6},
    {0xa62e46e1a573e9de, 0x3ffa, 0xbb7997fe7703ce33, 0xbfff, 0xbb8c00dbabc4c30f, 0x3fff},
    {0xc4bfa45c989b9825, 0xd73a, 0x97ffb0da9460b1f9, 0xd962, 0x97ffb0da9460b1f9, 0x5962},
    {0xcf36515b4d3395bc, 0x3ffb, 0x845bc9ac32076a6c, 0x4002, 0x845e5271e9c08458, 0x4002},
    {0xd694c58f28cf8c87, 0x1758, 0x92cf660e7242fb85, 0x3678, 0x92cf660e7242fb85, 0x3678},
    {0xa993ac2dba150bff, 0xbffe, 0xfa41f369afdaa23a, 0xbffe, 0x9726609db5b5f850, 0x3fff},
    {0xf0512098a512187c, 0xe38c, 0xd8b1829a7048dd6a, 0x1b06, 0xf0512098a512187c, 0x638c},
    {0xb6469ddc53668079, 0xbffe, 0xaad2e730ae481e83, 0xc000, 0xb0cc214aaef01ee1, 0x4000},
    {0xd8af6a5afa47889e, 0x5731, 0xc35da67ba4e272c9, 0xadf8, 0xd8af6a5afa47889e, 0x5731},
    {0xfb1cf7c36f935ee2, 0x3ffb, 0xc7b14ed39dccef75, 0xc004, 0xc7b1764c61e94386, 0x4004},
    {0x8992eea5ea64a958, 0xddff, 0xc71b60e54acb0b39, 0x47c2, 0x8992eea5ea64a958, 0x5dff},
    {0x943efb50aa683f14, 0xc002, 0xaf976eb21303ddf4, 0x3ffe, 0x94a6d4892262d62f, 0x4002},
    {0xac31e9c7904c301e, 0x9e5a, 0xb414c6eb052b24dc, 0x61ce, 0xb414c6eb052b24dc, 0x61ce},
    {0xece023741cfffda9, 0x3fff, 0xa49db0adbab6026a, 0x3fff, 0x903ab7d4d075b0eb, 0x4000},
    {0x90aebd0f23d637d5, 0x80d, 0xe602ff2aa982665a, 0x5bdb, 0xe602ff2aa982665a, 0x5bdb},
    {0xf028c1f21ffaf487, 0xc001, 0xcfd700fffe2143c9, 0xbffc, 0xf03f3cbacf223dc0, 0x4001},
};
static const U t_fmal[][8] = {
    {0x8000000000000000, 0x3fff, 0x8000000000000000, 0x3fff, 0x8000000000000000, 0x3f37, 0x8000000000000000, 0x3fff},
    {0x8000000000000000, 0x5f3f, 0x8000000000000000, 0x5f3f, 0x8000000000000000, 0xfe7f, 0x0, 0x0},
    {0x8000000000000000, 0x20bf, 0x8000000000000000, 0x1f2f, 0x8000000000000000, 0x53, 0x8000000000000000, 0x53},
    {0x8000000000000000, 0x3fff, 0x8000000000000000, 0x3fff, 0x8000000000000000, 0xbfff, 0x0, 0x0},
    {0xcaed3d108d4fdc4b, 0x4062, 0xcf2b8d333e786ed8, 0xc03d, 0xea3a6b9161d70faf, 0xbfb3, 0xa4385a26ed758388, 0xc0a1},
    {0xfb5da31a2d0b7653, 0x3fff, 0xe9057b7bf0b6e5bb, 0x3fff, 0x95ccd2f671785c4d, 0xbfff, 0x99e7320b797c7209, 0x4000},
    {0xead2e1a3d016a17a, 0xbfa3, 0xd2501ee905f92992, 0xbfb4, 0xa2901d18a02750f3, 0xc0c5, 0xa2901d18a02750f3, 0xc0c5},
    {0xec5704ce0ef990b9, 0x3fff, 0xa130a0678dec0395, 0x3fff, 0x839dcf4d86e9e37e, 0xc000, 0x898e78f7a16b4610, 0x3ffd},
    {0xbfaa233c6a600c1f, 0x4022, 0xa1056e69be96f6b0, 0x3f9c, 0xccd5b559efb2d3cb, 0x40be, 0xccd5b559efb2d3cb, 0x40be},
    {0xe2d29dda6054cdf7, 0x3fff, 0xacc91fa14c78219e, 0x3fff, 0xad31092a4e18b527, 0xbfff, 0x84fe61bbdaebe38a, 0x3fff},
    {0xd1649c05a88cb298, 0xc033, 0xa796c631d8bd8b24, 0xc057, 0xec476bb8115bdf47, 0x406d, 0x8913f4d98b098c8f, 0x408c},
    {0x95f0b2eca9559b07, 0x3fff, 0x9f6a78674efdd023, 0x3fff, 0xcd1a3152d8c013db, 0xbfff, 0x92e3d59033ad54cb, 0xbffc},
    {0x9ee26bfe597103f3, 0x402a, 0xe664d8b11334fb0c, 0xbfe1, 0xa57ae41464d297bf, 0xbfdc, 0x8efe03f5a54dc294, 0xc00d},
    {0xb2fdbe305b914486, 0x3fff, 0xc150e38aa90e4105, 0x3fff, 0xb3b81ce3754c443a, 0xc000, 0xb23908767a190411, 0xbffe},
    {0xcb407c8dea1e4aa2, 0xbfb0, 0xaffc797fcd5238e5, 0x404b, 0xc6f93c335df54274, 0x4082, 0xc6f93c335df54274, 0x4082},
    {0x820c49f173e59a9d, 0x3fff, 0xd31f1e1b2b03663a, 0x3fff, 0x9651a289030db6dd, 0xc000, 0xac46cb3b39ea3454, 0xbffe},
    {0xa0ed7aa602c46acd, 0xc05b, 0x96d3e59376f66209, 0xbfa8, 0xcee45be6aeb8f255, 0x4065, 0xcee45be6aeb8f255, 0x4065},
    {0xf16de18ee30de2ca, 0x3fff, 0xdd5bde179a1c91fd, 0x3fff, 0x877f84077baf79ae, 0xbfff, 0x8d02bd3755c62e00, 0x4000},
    {0x818a33e43711a3e2, 0x3fed, 0x82447718a087261f, 0xbff9, 0xe96914ddc24f2d5f, 0x404c, 0xe96914ddc24f2d5f, 0x404c},
    {0xff84f9303522077a, 0x3fff, 0x86a1553b4c3bb28f, 0x3fff, 0x88e510a6f95ab4ea, 0xbfff, 0x83dc339d99dabeea, 0x3fff},
};
#else
#define ULPS 2
static const U t_expl[][4] = {
    {0x0, 0x400c62e400000000, 0xed1967480b308def, 0x7ffef427b74653ba},
    {0x7000000000000000, 0x400c62e42f1a9fbe, 0xa5f88227633eed73, 0x7ffeffcac1ccc464},
    {0x0, 0xc00c62d800000000, 0xb38931df9c2dc574, 0x1259ef1570032},
    {0x0, 0xc00c643800000000, 0x1a5740a46ac6d, 0x0},
    {0x0, 0xc00c654000000000, 0x2, 0x0},
    {0x0, 0xc00c658000000000, 0x0, 0x0},
    {0x0, 0x3f9b4484bfeebc2a, 0x1448, 0x3fff000000000000},
    {0x0, 0xbf9b4484bfeebc2a, 0xffffffffffffd76f, 0x3ffeffffffffffff},
    {0x0, 0x3ffe000000000000, 0xbc972dfefab6df34, 0x3fffa61298e1e069},
    {0x0, 0x3fff000000000000, 0x95355fb8ac404e7a, 0x40005bf0a8b14576},
    {0x104717199e57368e, 0x400a82dcafd0b985, 0xc5b738f874108ae3, 0x516ffd1718e3679f},
    {0xd630bc4bbc15ac17, 0xbfffd5c226b3c818, 0xabd396518d59efd, 0x3ffc46e3f7344320},
    {0xe4a79da1f5bd08e1, 0xbff7148740fb187f, 0x79dd6ebc9b28247c, 0x3ffefdd81bc68523},
    {0xd3700e190896b6e, 0x400b25ea532b9980, 0xaa8318cb9529fc28, 0x5a7f65d00fbe11a4},
    {0xce7f5f3eaf8bc97, 0xbffeda8ce8a68515, 0x6b3b94bf0a6c7460, 0x3ffd954b8d596f10},
    {0xa14aec6e4c8bda81, 0x3ff2abddb101a946, 0x4549820701a5ae44, 0x3fff000d5f46ed45},
    {0x6ae01f80ae20372a, 0xc006dc7fc4c8fb28, 0x8d3dd880b3754baa, 0x3ea73685fef8a9f2},
    {0x48a32bcfbe76a780, 0xbffd334e3c042d1e, 0x9657fddaabd2366e, 0x3ffe7b42833580a0},
    {0xabb83f3083637f86, 0xbffaf743f9f8bb58, 0x18e9e813a8473925, 0x3ffee17e1a748f16},
    {0xf583255f614b6a70, 0xc00adc2df08cd6a5, 0x8d2686e7d98fa1d5, 0x2a871b49ff77aad8},
    {0xf352f76206d07762, 0xbffd5925b3fe5260, 0x89415e5cb36e4b91, 0x3ffe6d800af50d8a},
    {0xa6021a7106490e2e, 0x3fceaf562301dd87, 0xd7ab1180eec42dd9, 0x3fff000000000000},
    {0x663b826fc9d82abc, 0xc00b104e317a876f, 0x6031b6f97bfb3a5c, 0x2771442c12006814},
    {0x648de27252943604, 0x3fff90ad7b02fab4, 0xb10d93828101c7eb, 0x40013222dfb538f7},
    {0x711c216a9b1b1a30, 0x3fec105b182e09ec, 0x44bed69d6df7bbb1, 0x3fff0000220b6549},
    {0x27dc74c7e3760d4a, 0x400ba93131eef1e2, 0x78da2587616da6e4, 0x6655b22bbef6b8e4},
};
static const U t_exp2l[][4] = {
    {0x3000000000000000, 0x400cffff33333333, 0xad9dd3f5a70ec1ac, 0x7ffeddb680117a2c},
    {0x0, 0xc00cfff400000000, 0x6484597d89b3754b, 0xb504f333f9de},
    {0xd000000000000000, 0xc00d00e0cccccccc, 0x37b719b5b7a35b, 0x0},
    {0x0, 0xc00d01ba00000000, 0x1, 0x0},
    {0x0, 0x3ffd000000000000, 0x52de8d5a46305c86, 0x3fff306fe0a31b71},
    {0xaa16d347bbc35c12, 0xc00cde961ce86f3c, 0x606e4b88161b93b2, 0x42c2d7913d048b1},
    {0x198691e4fb375579, 0xc000193e6c63df86, 0x74b85d6dbbbe8e3f, 0x3ffcbe9516391e4d},
    {0xb4e355bf584f48d3, 0xc00ca1ae98d0f382, 0xeece98577b13149b, 0xbc9211770d5c3a0},
    {0x6735edb1df52f644, 0x3fff99ab53fc800a, 0xf46ca2a4c586ed5a, 0x40008418a0c5fd53},
    {0x9084db4171ae1afa, 0x4007971a601922c0, 0x6732a514dc2967d6, 0x419612f35d3c61ee},
    {0x966096acdaf3742d, 0xbffe83ec48546c92, 0xd4456ca446d2e35c, 0x3ffe2ed30fd61b0b},
    {0x41740710d1081131, 0x400c926170fa3b2c, 0x6988c17954dd5668, 0x724b220d289f2a8d},
    {0x672eabf19b2b7bb9, 0x3fff14f7d5a14c94, 0xff36cb7c639462fe, 0x40000ef45859df1f},
    {0x6c2c4c6a75d13898, 0x400b2ed583bd516e, 0x7d10166ccd4d670e, 0x52ec45152bcc6e65},
    {0xb40b2877eb074d94, 0xc0003c98ef54ea8d, 0x26ffff92f9e45ad4, 0x3ffc70c577b78f9f},
    {0x2c91d3faa7fc5211, 0x400c0b8dc68fb975, 0x822fb0f5abeaa698, 0x6170a64003fa0600},
    {0x3f821058e42ce822, 0x3fff3df279c8f819, 0xaf888119d0950106, 0x40002ebfe44e0e3d},
    {0xe97fd4ef5d32b9bc, 0x400cd728820d2d20, 0xdf151089951a28c6, 0x7ae40b850f8da7b5},
    {0x694bcf53437e2fd9, 0xbffff8c16a87d104, 0xba8cec773ba7e1e4, 0x3ffd051230a7d7fe},
    {0xf4727c2de69efbb6, 0x400c53c191af2169, 0xf3a23e1735464f7e, 0x6a772547ad38526a},
    {0x242d6d5ec691429b, 0xc0004c0a2e942bdc, 0x235c3da841189eee, 0x3ffc53300134a864},
};
static const U t_expm1l[][4] = {
    {0x9000000000000000, 0x3fabef2d0f5da7dd, 0x900000001dee7a4b, 0x3fabef2d0f5da7dd},
    {0x9000000000000000, 0xbfabef2d0f5da7dd, 0x8fffffffe21185b5, 0xbfabef2d0f5da7dd},
    {0xe000000000000000, 0x3ff7ff2e48e8a71d, 0x2015166382fcf8ea, 0x3ff80096fd166485},
    {0xe000000000000000, 0xbff7ff2e48e8a71d, 0xa6ceffccb896e646, 0xbff7fd313f0a325a},
    {0x0, 0x4005e00000000000, 0xb16b968eed5a9fc0, 0x40ac16dc8a9ef670},
    {0x0, 0xc006900000000000, 0x0, 0xbfff000000000000},
    {0xd2bee66c1b8f8ad0, 0x40048266fe807e25, 0x320e3f396f951426, 0x40449ae26d167699},
    {0xe0e504b004d69d1d, 0x3fd9dcec65c4b6c3, 0xe0ce8366df46aaea, 0x3fd9dcec65c4bdb4},
    {0xb2fcd59364aff481, 0x4001268acb0448f3, 0x458c98173431bcd6, 0x40058ad27966e175},
    {0x8521846bb202659e, 0x3fd9f466b9836bf3, 0xc7fd47f14d7e54e2, 0x3fd9f466b9837397},
    {0x1130dc51953c1a90, 0xc001257ebe7a43cb, 0xe7f50595daddece5, 0xbffefac7b994c42c},
    {0x4953bc3091d9eecf, 0x3fcc6ebfb51b3721, 0x6a2a47c72adfdfc8, 0x3fcc6ebfb51b3721},
    {0x3b9e915e1def3b92, 0xc0035f5cc53ace2f, 0x9edd7e613f48cc9f, 0xbffefffffffd81a6},
    {0xcbde78bd1fbdb28b, 0xbfd74c04a9af35b2, 0x7dd25ea4dfb47bc9, 0xbfd74c04a9af34db},
    {0x764e979f3c6074c3, 0xbfff5c5571449f23, 0x7a1e8993fd1fe603, 0xbffe7cadc8e1f3d0},
    {0xbf87a9c4e8fceabf, 0x3fc9b7c840d95ad9, 0xc56eaa9b06b02f1f, 0x3fc9b7c840d95ad9},
    {0xd7c1b400a4aac2e, 0xc004426b4e67ec0f, 0xff8c2a1c90df5896, 0xbffeffffffffffff},
    {0x2d9c9f20b1dd9203, 0x3fb1af3836e6dc98, 0x2d9c9f265e9a3c6e, 0x3fb1af3836e6dc98},
    {0x3456665120b25098, 0xc003bafa9d7b09f0, 0x43838e53343fc48c, 0xbffefffffffffdeb},
    {0x611dccc4582ccbe4, 0x3fb0b8c3e269eb9b, 0x611dccc74f0ebc13, 0x3fb0b8c3e269eb9b},
    {0xbe1f49069a6a1416, 0xc00359eeb2fbfe84, 0x1610d55d10268eeb, 0xbffefffffffc7fb7},
    {0xdf945421b7d65519, 0x3fc0c2445405f73c, 0xdf976c1601780d0a, 0x3fc0c2445405f73c},
};
static const U t_logl[][4] = {
    {0x1d83c94fb6d2b, 0x3fff000000000000, 0xfffe4c7046255871, 0x3fbfd83c94fb6d2a},
    {0xffffa18d7bcdb6f7, 0x3ffeffffffffffff, 0x22d84c4eeeb0, 0xbfbc79ca10c92424},
    {0x3000000000000000, 0x3fff6affff29406b, 0xc8247afef4bbafb4, 0x3ffd659b54d2737c},
    {0x3000000000000000, 0x3fff6b07f23cc8de, 0x991a47a5e3bcf1, 0x3ffd65b1c165e61c},
    {0x0, 0x4000000000000000, 0xf35793c7673007e6, 0x3ffe62e42fefa39e},
    {0x40000000000000, 0x0, 0x1e6c8068b5aa51ed, 0xc00c641ab799954e},
    {0x10, 0x0, 0xf29a9cd42972ef73, 0xc00c652ff9df0885},
    {0x2a655f6eb35ffbf6, 0x40366c6e3924267f, 0x7ec15f5dda720718, 0x400433cf5e001fa0},
    {0x8886c89eace3a3d6, 0x3ffe70d4d5f87811, 0xe14fbe5899a3a842, 0xbffd4fdb4e5245a9},
    {0xc0ee48e98d161bcf, 0x51ed6ba0bbc24ed6, 0x7739e6925f26dc96, 0x400a8dbcb03a9b99},
    {0x92b5766ed94385ba, 0x3fff15c9cb0da63d, 0xf1cae121db0bef34, 0x3ffb4e9179b2562c},
    {0xcaf612d80d1b8176, 0x5e2e3a6562e1b939, 0x737e687405ad4008, 0x400b4ec275b84ab3},
    {0x41864476a2e54f85, 0x3ffe5a9255e03cfb, 0x5517c8727726d275, 0xbffd8f99cbdd006c},
    {0xdb8f62f7213a6462, 0x3b14e5a4199f22f1, 0xf09baa775e67c096, 0xc008b40418d72451},
    {0xef74afb144619689, 0x3fff2ff955c78ae4, 0x146e02ee6b1ecbe1, 0x3ffc5fc62002a4e8},
    {0x752938e456d43ca2, 0x3945a2f3d7e4f9b3, 0xb21e74c51fea76a3, 0xc0092a46d6df69b9},
    {0x7fe4a707ac8a111e, 0x3ffe5dd158a3b668, 0x5bdefbf753304ab2, 0xbffd860ddf6cd1eb},
    {0xf7e6d002691a5848, 0x7af3547fe8017846, 0x16bef0aba41e2ab6, 0x400c46ea19920f16},
    {0x68a3028464acca56, 0x3fff53727e824f49, 0x69fb7fba9170c3e7, 0x3ffd20e9984cb0b5},
    {0x58e0807918b361a9, 0x6d7316b3338fb7ec, 0x7909c805719fe8c, 0x400bf818ba94371a},
    {0xfb221b0f5c474885, 0x3ffe4885d4ae8869, 0x927ab201fbcdfa8b, 0xbffdc65e216cc976},
    {0x18eb6de35e7f1a27, 0x767b903fda5ea01a, 0xbcedd1a8c2596bc8, 0x400c2e23b5d4dea2},
    {0x578da3a3cd6e318e, 0x3fff48eff86fad06, 0xcd017d9ad225c7f5, 0x3ffd00b4def3f414},
};
static const U t_log2l[][4] = {
    {0x0, 0x4002000000000000, 0x0, 0x4000800000000000},
    {0x2f3942192484, 0x3fff000000000000, 0xfb3c5a8e9b30af7a, 0x3fbd1084738e52ab},
    {0x0, 0x40000000, 0x0, 0xc00d004000000000},
    {0x13795badf64d1441, 0x21e7db10b2d93ce1, 0x87141567126468fb, 0xc00be171ba7148ff},
    {0x9272ba7932a247c, 0x3fff376b858bab13, 0x15355a7ae22e0a63, 0x3ffd2180c7ba9185},
    {0x32de764b6b9b00b7, 0x798bcc819b1cd05e, 0xd880af8a6de4f8c9, 0x400ccc66c6d04e38},
    {0xc29edfd5c64e7466, 0x3fff7bf810523739, 0x7e1d5ee260169862, 0x3ffe23b4afd618ee},
    {0xc70039feebb44908, 0x5fa91773146ceb06, 0x6cafb6cc43d48816, 0x400bfaa205ea14d3},
    {0xf302ece40446da8e, 0x3fff13c7ab43a8e5, 0xbc32832c75d05095, 0x3ffbb7cd96bc4c19},
    {0x9bc6a70ec9099bff, 0x50d43d5a463be76b, 0xe2e7547e150d43a9, 0x400b0d54f586cc8f},
    {0x8021f8edb827031a, 0x3ffef1a9d86f1b88, 0x8bc5699fc5aa7ab, 0xbffa4fa757ce56c2},
    {0x65387205919c68fe, 0x21da3728db566f67, 0x453277574d80f95b, 0xc00be24b7eee6478},
    {0xc6dee4ef222beda9, 0x3fff78446ff87fc4, 0xe3e8170957e2be93, 0x3ffe1c797807cdf6},
    {0x5e02c9dc0d64b2f2, 0x7d32a5c88f30cac5, 0xa7d35341841a7306, 0x400ce99dc34b6217},
    {0x87a1a36ce7aad5fb, 0x3ffec56b66d7dbf3, 0x1bf4d5be32cae9d4, 0xbffc67023ed401c5},
    {0x7d4347e1a08c1472, 0x390a734befa66b51, 0x8f170a6677a12185, 0xc009bd1dab283e8c},
    {0x14220a6f22441f82, 0x3fff157f0a562da3, 0x6615ff9a0e51a18e, 0x3ffbdc770cecc847},
    {0x3aaae56f1d56f304, 0x4cbb4f62635aa835, 0xc16d38c1ae316dd, 0x400a978c78304c2b},
    {0xfb7a2ebd33be2372, 0x3ffe6eabb32db513, 0xca2318bd147015dc, 0xbffded37f7b1132d},
};
static const U t_log10l[][4] = {
    {0x0, 0x4008f40000000000, 0x0, 0x4000800000000000},
    {0xfffc4f86d60925aa, 0x3ffeffffffffffff, 0x35611ae6885fae6c, 0xbfbe9a2df666417d},
    {0x59d96a16e814c51, 0x682ffe85f3254bfd, 0x5026fc0aa3a11e47, 0x400a83297bdeead2},
    {0x11ceb819cb399eb8, 0x3fff0ad56670c586, 0x6b61d82e0a035980, 0x3ff926ebbdd01595},
    {0xa84e334a60164121, 0x39a34a4084c2f8aa, 0x67c1f95ecb630b85, 0xc007e9f75aae1033},
    {0xab6495f10dafd44b, 0x3fff53ecfe2ffe2f, 0x3ba8bd5365d9731, 0x3ffbf865f3a2418a},
    {0xc305cc0b001edecd, 0x37caf0d9dbd81232, 0x8d061a36ab629a68, 0xc0083c16884de7b7},
    {0x56f29eb05014d7d6, 0x3ffe6a1d873bbae4, 0xcd7c6ed81340a7e, 0xbffc3410fda61cb0},
    {0xdfb2d238b5f73d91, 0x235bf015bb69c9a1, 0x731695577d8cbf46, 0xc00a13dbaae7261e},
    {0xfb7b6086b9eb4571, 0x3ffe89ed4095284a, 0x647c541add06edcc, 0xbffbd2591072bd25},
    {0x6feb5e116f2fe504, 0x25bbd94c4ac67451, 0x9815025121c31bdd, 0xc009f9f6f68406be},
    {0xaac21d51d56fe149, 0x3fff416c0a3a30da, 0x6557f8f4d9816fb6, 0x3ffb94d4b7b034e6},
    {0xbb936e57b578e8cc, 0x7432e7444611b97d, 0x8c0f8f6160cf244b, 0x400af6de2ffed5ef},
    {0x1b78354ce0fc8fb1, 0x3ffe3f39dc5d41d4, 0x604a4354cd4a09a0, 0xbffca4310ca7e1d2},
    {0x66f7f45a67b05d92, 0x33d1cdc4951839dd, 0xb07e876d9553c126, 0xc008d52d7bdeaae1},
    {0xb4db1b14013332bf, 0x3ffeb85b22858f31, 0xc7f34d5b3eb66f61, 0xbffb0c262040b892},
    {0x9f1d7eef154b15e8, 0x13be8c9ec5adcb03, 0x1eb6746a20ad913e, 0xc00aaa45b7cbb8b2},
    {0xdfe4911e7df1e118, 0x3ffeb0dd82022f00, 0x9fb97d5ccf7b5686, 0xbffb2aab401f08ff},
};
static const U t_log1pl[][4] = {
    {0x0, 0x3f9b4484bfeebc2a, 0xfffffffffffff325, 0x3f9b4484bfeebc29},
    {0x0, 0xbf9b4484bfeebc2a, 0xcdb, 0xbf9b4484bfeebc2a},
    {0xfffc4f86d60925aa, 0xbffeffffffffffff, 0x97af2356a51796c6, 0xc0045dfe30ee7550},
    {0x7000000000000000, 0x3ff66f0068db8bac, 0x1e7b7dc01cba341, 0x3ff66e7d1f09a78a},
    {0x0, 0xbff67c1bda5119ce, 0x407ffb20a819bbd2, 0xbff67ca938fa30a8},
    {0xc000000000000000, 0x43e37e43c8800759, 0x53579980e21f9a88, 0x40085963447f87fb},
    {0x6a607c2cda782ded, 0x400adb6e7091be1d, 0xd5ac4104d0f8b598, 0x400207ce40142772},
    {0xdb32b405c1d8af94, 0x3ffe6fd96e5ede3f, 0xfa28b4c566054eca, 0x3ffe1535cc092d31},
    {0x19a79497984650d9, 0xbfcd1a18dfb9850e, 0x4082ee8186346fe9, 0xbfcd1a18dfb9850e},
    {0x6cb03a7e4b31202e, 0x3fff05716fef6f3f, 0xe1ec23b608fc5e3, 0x3ffe684e450b5f09},
    {0xb8da12b59432f7cc, 0xbfd367b51408b481, 0x844504e3964334da, 0xbfd367b51408b491},
    {0x818a99d6000b5cc1, 0x3ffd351d3798a392, 0x7243c77ac24783d3, 0x3ffd0e21e21f7c3e},
    {0xd8cd433ed1b5a74a, 0xbfd71b9d4063ec98, 0xf353bdbb33de42c2, 0xbfd71b9d4063ed35},
    {0x60fbc51b87fc2332, 0x3ffe83d96546e5f6, 0x5633bd50d561b383, 0x3ffe20b7d674b689},
    {0xb4bdcc234310a05a, 0xbffe80fd70892f73, 0x15b28abccf94d9f1, 0xbfff64e1096b84c5},
    {0xccf07edde2e4b8af, 0xbfef66e6a5d0a6b2, 0xc4235beb5865db8, 0xbfef66e7a16685a0},
    {0xbd52a90c72687265, 0x3fffe3ec74ab262b, 0x4c1e4145953370d8, 0x3fff0fb5ea7e912b},
    {0xdcb1e1351e619d0c, 0x3fffac4288406031, 0x516a3a13157f7a94, 0x3ffef760cb6c54a6},
    {0x829943b23868066f, 0xbfcb25d6ad7eedbf, 0x8d236c9b90f81c3b, 0xbfcb25d6ad7eedbf},
    {0xe2efe4e4f853ba0, 0x3fff2df0309d3a8a, 0xb1ee08834eafade2, 0x3ffe8ee25f527ba0},
};
static const U t_sinl[][4] = {
    {0xc000000000000000, 0x43e37e43c8800759, 0x56e40287d53b348d, 0xbffea2c16b010e38},
    {0x18c21ab905450cc3, 0x73e6a3750647fcab, 0x41dd929a03bad4bd, 0x3ffd6d83fd6946e2},
    {0x6666666666666666, 0x7ffee66666666666, 0x3d0442d5fb40cd5a, 0x3ffe11e8a4884359},
    {0xc000000000000000, 0x3f7a16c262777579, 0xc000000000000000, 0x3f7a16c262777579},
    {0x0, 0x4007630000000000, 0x29dade2cb6b5cb81, 0xbfeff9bd0307d1de},
    {0x0, 0x400f963900000000, 0x682724556afaf8fe, 0xbfef40efdf1eb8de},
    {0x8469898cc51701b8, 0x4000921fb54442d1, 0xa67cc74020bbea64, 0x3f8dcd129024e088},
    {0x686b277597a24bd2, 0xc0019a0e2b6cdbdd, 0x208ccf814f5cf93c, 0xbffbfa5127efac51},
    {0x58423dda3dfba331, 0xc3742df141ad8673, 0x9742bdedccb7ec4, 0x3ffdc03547c926b0},
    {0x2cae46587ec962f9, 0x7d89d040a6fd91b1, 0xb5b9b987ba0c264f, 0x3ffee2e9de502729},
    {0xd9b11657fb80c608, 0x3ffcbbb5d413a3eb, 0x7be2afcc54f82094, 0x3ffcb83f44d57ef3},
    {0x65e292eb98b5cfa7, 0xc39bf820de509ce0, 0xa04e163c7cb76b88, 0x3ffd3bacb8673366},
    {0x29160325854e1056, 0xdf8dc17a014b8ffd, 0xb1c8a7df4979afbd, 0x3ffcbd5ecefd2e0a},
    {0x5ce8f607e075f80e, 0xbffcd108eb0eb0b4, 0xf503068d5574f9d7, 0xbffccd0c879335b0},
    {0x1c25060106c6e0b3, 0x40998b1b0e1792b3, 0xa99d5731e93fa5ba, 0xbffe80fb6b131745},
    {0xdb34ae2cd4db5d04, 0x5088da9467d602c5, 0x6408c43d383d1909, 0xbffef64a9c0b5f49},
    {0x5a3bbfe702477395, 0xc0021b605da076bb, 0x89b9dda05cd169c0, 0xbffe13f97a573b94},
    {0xccc3a6d1c4bcef11, 0x406c4a6141b8a5e3, 0x2fe1c99dcdab4c24, 0x3ffdad4dce1eb7aa},
    {0x7dbc60d90cb5327, 0xfe7da92b6ed98a65, 0x1808f646e3d0bd3c, 0x3ffe8e0a28001f92},
    {0x83e399ba04c1cba9, 0x4002187a0c999cdd, 0x84ae5bfe382a88d9, 0x3ffe39ddf60ec7c8},
    {0x19262edd1a4fed89, 0xc2a0580567a2e8e3, 0x8c22a7a232d7ac61, 0x3ffef6636d76c0c5},
    {0x19f9407860b48657, 0xced37cb843b85dc3, 0x74da5dea7bf5a482, 0x3ffde79eed4bbf96},
    {0x1f8095297585d2ea, 0x4002275db196166b, 0xe25c1516cd55a220, 0x3ffc8c02f5456b28},
};
static const U t_cosl[][4] = {
    {0xc000000000000000, 0x43e37e43c8800759, 0xc24554d69475184, 0xbffe2699022adc4c},
    {0x18c21ab905450cc3, 0x73e6a3750647fcab, 0x1077024a8b3c7305, 0xbffede457e6279aa},
    {0xc000000000000000, 0x3f7a16c262777579, 0x0, 0x3fff000000000000},
    {0x0, 0x4007630000000000, 0xbe79dd9afe996639, 0xbffefffffffc18e4},
    {0x8469898cc51701b8, 0x3fff921fb54442d1, 0xa67cc74020bbea64, 0x3f8ccd129024e088},
    {0x8469898cc51701b8, 0x3ffe921fb54442d1, 0xc908b2fb1366ea96, 0x3ffe6a09e667f3bc},
    {0x369b970a7b5c8d13, 0xc001f37f2e6b275c, 0x24d755968559d0e, 0x3ffa94101800484e},
    {0x2526263b7df63976, 0xc1bdd068f05bc8ad, 0xe16121a7989737e1, 0x3ffe83b1ecc06dca},
    {0x242c36e65a5d982b, 0xdcbed09cf69a6387, 0x8f8176d0ada29db9, 0xbffd9257a204b247},
    {0x3efb3c420cd31fdc, 0xbffd5da10ff553b1, 0x440a13b31867a34d, 0x3ffee271e6547bea},
    {0x6a195e299eff0762, 0xc17b57f75a22a7bf, 0x30c92bb3fdc14c61, 0x3ffeffc97a106c87},
    {0x659caa5941ea9cd9, 0x6b4affa2c8da1e86, 0x606323483e1d8f16, 0x3ffefa2b613ed41f},
    {0xf191171cf82e6f2, 0x3fff0d47d5f9ed13, 0xc6de1807f84e6133, 0x3ffdfbd8237944ee},
    {0x68537c9d4108aa9f, 0xc0d705b637668903, 0x4d0c7dfb90d31917, 0x3ffde5c387d6d430},
    {0x685c0e5e219e4bb3, 0xf7590da1ce1f00e8, 0x693063ad62f1c16a, 0xbffefdb7fd7e19ea},
    {0x2b8149654030d313, 0x3fffed2836f2726d, 0x42eceee84f2c77b, 0xbffd6481d5c0d8a1},
    {0x94351511fc64a236, 0x43b9f649696e5cbb, 0xbca4cc219e99a50e, 0xbffe97cfa9974e2b},
    {0x3a892bac8b72a5d8, 0xe1c11dafd763b576, 0x1001696d4d22b89b, 0xbffed9b6423f34d5},
    {0xa5ac6ce723ed471f, 0x3fff4e0fef39ce58, 0xe42036963db187cf, 0x3ffd0d0cee1bd2ca},
    {0xed9283ee1171d0a9, 0x42e8013dfa060c1e, 0x2d8d70091c60f2c4, 0xbffc523bdf01923d},
    {0x68d554a696c411b5, 0xfbcb6facbdd8375c, 0x4a45d0bf82387ebe, 0xbffc4d2b83e6d13b},
    {0xb29b3c5d7461640f, 0xc002294e1354157b, 0x3fa9e6c5dd697b38, 0xbffefb690b18a0ad},
};
static const U t_tanl[][4] = {
    {0x8469898cc51701b8, 0x3fff921fb54442d1, 0x93a2ee60193c957b, 0x40711c46bd572779},
    {0x2000000000000000, 0x40480f0cf064dd59, 0x7749e043d5cdf750, 0xbfffa0f79c1b6b25},
    {0xc000000000000000, 0x3f7a16c262777579, 0xc000000000000000, 0x3f7a16c262777579},
    {0x8469898cc51701b8, 0x3ffe921fb54442d1, 0x0, 0x3fff000000000000},
    {0xee80bdedc319bcde, 0x4001054bb6489b50, 0xd8f165b96beae99e, 0x3fff5f5fce98ac41},
    {0xf5d4116160af656b, 0xff380c33ef37bded, 0xdd91969bc1d4599d, 0x3fff0b0555b04b34},
    {0x6299f3aa405cfd20, 0xc0007140924bfd7f, 0x72ea6b6601182f35, 0x3ffd0ce92c8e9f15},
    {0x653e4e8b17e1f268, 0x4c7eaafdd6ee5fca, 0xf061423ba896860, 0x3ffd068781934118},
    {0x699fd3a7c88c9793, 0xc001b739e9a2e6e9, 0x72e33f45e22f0479, 0xbffe4f3e9e73c70f},
    {0x7a80c1e86ab1411, 0xdee252c3b0fb1b86, 0x71703d976666aa5, 0xbfff100b97144b34},
    {0x8de89ce54f9b8bbf, 0xbffde9b7c7b86161, 0x60141b6cb15513a6, 0xbffe0968953c4569},
    {0xac6e1c1c20f1c6f3, 0x7eea53da04999e8d, 0xb279e6bfc7d2a4, 0x3ffd64959f2af6b5},
    {0x2c382455b13de04e, 0xc0004a41a4980d45, 0x2ad481c615216815, 0x3ffe420b293e5bbe},
    {0x68cc577ca2b8481e, 0xdcdd27be09f62983, 0xc4068af083656de5, 0xbffae2789f783cf9},
    {0x5211c698291615c5, 0xc000d06a7db9355c, 0x97c5531411c1875c, 0xbffe0ee672d4aff9},
    {0xacc467e28676c1f, 0xead6ca2dd39b72e9, 0xde7cf60c6debbb1a, 0x3ffdcf4a6840cc63},
    {0xf2eba94fcc496574, 0xc0013df5c0e28136, 0x77b559a994123391, 0x4000e98ec19b3590},
    {0x29a6439c996c1f23, 0x744ee61dac4d6771, 0x8e82684c04832095, 0x3ffeb4680aa8bbd0},
    {0xb13403f3d825d1f5, 0xc0016904a2dd5acc, 0x16699e759ecf16b5, 0x3ffe7f0599e511c0},
    {0xf22e60bc2e65eb1, 0x618e43bd4b790db4, 0x4fa62b2de8e2e283, 0xbffcf6de819e930c},
};
static const U t_atanl[][4] = {
    {0xb8049732d11a23d, 0xc17387ae70c9e70, 0xb8049732d11a23d, 0xc17387ae70c9e70},
    {0x18c21ab905450cc3, 0x73e6a3750647fcab, 0x8469898cc51701b8, 0x3fff921fb54442d1},
    {0x0, 0x3ff9000000000000, 0xab77cf18ac802bef, 0x3ff8fff555bbb729},
    {0x0, 0x4005000000000000, 0x311299ee93be0160, 0x3fff8e1fca98cb63},
    {0x0, 0x3fff000000000000, 0x8469898cc51701b8, 0x3ffe921fb54442d1},
    {0x97004c88aa6a8a75, 0xbfe5ca05ab9abab7, 0x7874cff6f6a0e837, 0xbfe5ca05ab9abab7},
    {0x7e245a698a2b69af, 0xbfffdbb4a88d2e50, 0x9e6672ad81813643, 0xbfff13bcb499c9c5},
    {0x4627c22270169ed7, 0x400a91866d022c3e, 0x342b6d2c338252, 0x3fff920b4e4cf9e9},
    {0x17ba06cb00f7cf60, 0xbffe83aa9921e1e0, 0x2015f7909d3c5cd6, 0xbffe4bcf9e496774},
    {0x7c42158bd81b4242, 0x3ffb2a59e5cacfc3, 0x7ed1fa10a5c5bdf4, 0x3ffb29d33fb23843},
    {0xdeb40d51814dde8f, 0x3fff418be3ee8ae9, 0x502ceef0c1f44e26, 0x3ffecbfbd9bebbed},
    {0x6225bae32cacde0, 0x3ffdab3ec72790fa, 0x25a86e87f06974b, 0x3ffd94c24c9e55fc},
    {0x4349856126f67bea, 0x3ffe6a8ce84b076e, 0xc3a86dd648e3ff57, 0x3ffe3b777417189d},
    {0x4302f9439fa503e3, 0x401072fe7a443c00, 0xd976b49ab972ce5a, 0x3fff921f5cf11867},
    {0x6d4c5a3bb6fc2181, 0xbff8a1c62156b0f2, 0xef6267d4de019b34, 0xbff8a1c055ffd31c},
    {0xc550da00880dc4ae, 0xc01778e947a9248b, 0xc32b59b0fd249d93, 0xbfff921fb4966270},
    {0x26fc46ef1be210a6, 0xbfff61c4cb4bee59, 0xc67852d648526fc7, 0xbffee38616493529},
    {0xab8ca74029e1dbe, 0xbfe05cce98fa7c3b, 0xab56b0c6112d23f, 0xbfe05cce98fa7c3b},
    {0x33f39f579074bca1, 0x40006d1870f5a8f9, 0x9f0f1a0f7c1f3a52, 0x3fff3bccc234a598},
    {0x11d19f460dd74cdc, 0xc016b6268076d83e, 0x8b435b88a8c0a7db, 0xbfff921fb4191cd2},
    {0xf5b210d1e8535334, 0x3fffd297c040be3c, 0xc2956407834282ba, 0x3fff11a8ec091b8b},
};
static const U t_sinhl[][4] = {
    {0x0, 0x400c62e800000000, 0x98cfae8ca4db64a7, 0x7ffe9c4ee6ccd991},
    {0x0, 0xc00c62ec00000000, 0x0, 0xffff000000000000},
    {0x0, 0x3f9b4484bfeebc2a, 0x0, 0x3f9b4484bfeebc2a},
    {0x0, 0x3ffd000000000000, 0x1e6674cdf3fc8eab, 0x3ffd02accd9d0810},
    {0x7d76bb43b065e27, 0xc004254e02cbcb37, 0x72405aa3b9de0ffc, 0xc032db9e3f105643},
    {0xad8b89b5a34f648, 0xbfeb6f66b9d39bf3, 0x2a12f97aa410581a, 0xbfeb6f66b9d39c71},
    {0xabd7b06b7de96ab5, 0xc004c3ed6ded5e72, 0xac9cb6308f8e9fc9, 0xc04f69d5beb88780},
    {0x8d459fb5aad981c6, 0xbfecac35ad817f42, 0x47542d883fbed69d, 0xbfecac35ad818261},
    {0x9a2530561a7f6e4d, 0x40044a8decbb5433, 0x1ad3f12a140cfb8e, 0x403987088ce2bb7b},
    {0x1803e24310be8c22, 0x3ff8592898ecc5cf, 0xdeadb0ceca43c2eb, 0x3ff8592a3b396c08},
    {0xc223f777a11672b4, 0x4003112f6a4a11db, 0x823c205496ae1f, 0x40168cea21b50bf3},
    {0xeb23e831ef5b9ecf, 0xbfe9a6daaf8bc6c1, 0xefab1427f7ae1765, 0xbfe9a6daaf8bc6cd},
    {0x7584a3015701cb29, 0xc0027ad6d6455031, 0x3e4d189f87947b32, 0xc00f0e88e60da57a},
    {0xef62c942d31919af, 0x4004617ffe53f1cd, 0x7ff291d643d18285, 0x403dae43e77389ae},
    {0xd010c30045b42b06, 0x40022ae3ac6a584b, 0x290c8b51918977fa, 0x400b63dde107b049},
    {0xf6b7153c1003e3f8, 0x3fc825452470b813, 0xf6b7153c1003e3f9, 0x3fc825452470b813},
    {0xe9b64d0be1e4ccbe, 0xc002c940d3208b4e, 0xb6387d033477c8dd, 0xc012880d90af002a},
    {0xdd95d4b6a431ac01, 0x3fd2d984886e80c5, 0xdd95d4b6a4752c8d, 0x3fd2d984886e80c5},
    {0xb22f8788f2383248, 0xc00369a4aef26561, 0x3354c17a7cda7bad, 0xc01e866692537861},
    {0x84d3173a2fab7d06, 0xbfff7b303caa013c, 0x56ba9cddb250d2dd, 0xc0000aefa99a97a6},
};
static const U t_coshl[][4] = {
    {0x0, 0x400c62e800000000, 0x98cfae8ca4db64a7, 0x7ffe9c4ee6ccd991},
    {0x0, 0xc00c62f000000000, 0x0, 0x7fff000000000000},
    {0x0, 0x3f9b4484bfeebc2a, 0x0, 0x3fff000000000000},
    {0x685f03ec0ee9be10, 0x400006767a29d32a, 0xfd206d1b9c079bfc, 0x4000f99fe87d8022},
    {0x93c4c53c283b9e8, 0x3fe8f271c74f2495, 0x94fdbc02f2c62274, 0x3fff000000000007},
    {0xb8184141dae5d48f, 0x400447ad1a69b7fe, 0x78691009a0ded213, 0x403910e035db28f9},
    {0x36660da188f530cd, 0x3fee21e98e03d7db, 0x24f710fd398bd963, 0x3fff00000000290a},
    {0x2a2f71c7dc87d994, 0x4003433aec9042e1, 0x792b4b67a6592ecd, 0x401b1b1938b20057},
    {0x1c17b952c09ab88f, 0xbffead9129d4b5a1, 0x819a950d698403fa, 0x3fff5f832d528ae9},
    {0x2199a23e1e1740a4, 0xc0025dfcfbdbe1a7, 0xf63360e03339ef84, 0x400db743b0e72eba},
    {0x3b8b9f89b2983100, 0x3fd77903e66e9899, 0x1159e3e60, 0x3fff000000000000},
    {0x24b7912f1f87e695, 0xc00489bcd807fe92, 0xee8d4e4256a7a902, 0x404500f6d1f47550},
    {0xe2d3d7fea5079675, 0x3fed94fd2bf2c62b, 0x8068b179dcb8e28b, 0x3fff000000001405},
    {0xfa24cc9813a084f6, 0xc0049772ad9ed192, 0x1c6139e975a8514b, 0x4047648899e8695e},
    {0x2e50c6dfde73f374, 0x3fecdde81b4cae1f, 0x55905a85a2ef1034, 0x3fff0000000006f8},
    {0x3e975f001ed5468, 0xc0046f3307706dc5, 0x2a283601ccafc60a, 0x40402a15dd82ff15},
    {0x9de3c394f51377ce, 0x3ff8d203def5d293, 0x69122deea169c262, 0x3fff0006a0ab81a4},
    {0x77fbba29dd0c1d42, 0x4003eb8b9947bf87, 0x1a4199f54ccfea16, 0x402a3ffce7edb4ce},
    {0xae3880c5af71823b, 0x3fe4735774b5bc43, 0x4354d344e90ceac, 0x3fff000000000000},
};
static const U t_tanhl[][4] = {
    {0x0, 0x3f9b4484bfeebc2a, 0x0, 0x3f9b4484bfeebc2a},
    {0x0, 0x4003e00000000000, 0xfffffffff528f373, 0x3ffeffffffffffff},
    {0x0, 0x4004900000000000, 0x0, 0x3fff000000000000},
    {0x0, 0xbffe000000000000, 0x365128ee21c65b09, 0xbffdd9353d7568af},
    {0xd27d8febe3008a8, 0xc00084910f939183, 0x5c55ad02a04bac3b, 0xbffefda45a4bf120},
    {0x48de71812f6a7e8d, 0xbfc9a237eecdb599, 0x48de71812f6a7e76, 0xbfc9a237eecdb599},
    {0x5570ca9a6c4997cc, 0xc001f88aa1a41d53, 0xc4bc1717d6f0fe50, 0xbffefffff6773325},
    {0xcb0edbb6b50dd71b, 0x3ff8660344d83b5f, 0xf1ca783dbb24bff1, 0x3ff865ff9f4d9cd7},
    {0x31aa4d09bf4b4ddd, 0xc000cc030b15b6ec, 0x34175483b8486a83, 0xbffeff3a02fe9a69},
    {0xddbd0327b9dffc94, 0xbffd5754a256fd51, 0x76b21169cbc00668, 0xbffd4b04cb6a727c},
    {0xc8a4df9d0d17d106, 0x4001d1415fd22a7f, 0x926bbd7c546958f9, 0x3ffeffffdf74b62b},
    {0x29568de0bd53978a, 0xbffdb509cef65a37, 0x4e8036b114106252, 0xbffd9c4d8b7b90ea},
    {0xef6c2eaafa02ba21, 0x4001eb59123c9350, 0x16c0b8920ba79677, 0x3ffefffff199b0e4},
    {0x78dd0365d544f02a, 0x3fc9e6aeee0fecd3, 0x78dd0365d544f005, 0x3fc9e6aeee0fecd3},
    {0x93d4c501a0977b6a, 0x400062a00daf8baa, 0xc91e5f80548509e1, 0x3ffefbffc01b7b4e},
    {0xb6db9e3b582b2c19, 0xbfca29fbc14dddf3, 0xb6db9e3b582b2bf7, 0xbfca29fbc14dddf3},
    {0x5b7734c3538fab51, 0xbffdb831be99b7f4, 0xbdd89e38766046a8, 0xbffd9ef1a886449b},
    {0x214961fbe410e0b9, 0xbffa717c4f5ba2bf, 0x1a896a6d6d8866b6, 0xbffa713c38c751a9},
    {0x2e1b1f4448ad3477, 0x40016829ddbbeaf3, 0x6e1dfd3122d29607, 0x3ffefffc9b8e5403},
    {0x907291e365ab368c, 0xc00369115cfc4294, 0xfffe26d981fb53d6, 0xbffeffffffffffff},
};
static const U t_cbrtl[][4] = {
    {0x0, 0x4003b00000000000, 0x0, 0x4000800000000000},
    {0x40000000000000, 0x0, 0x0, 0x2a97000000000000},
    {0x0, 0x7ffe000000000000, 0x0, 0x5554000000000000},
    {0x17746e53d69789d2, 0x2f3d469676212583, 0x369666c25fc65915, 0x3a6915a5adb21468},
    {0xae28d24056914370, 0x1eb63c3a85db2b58, 0xbbdd21291b084330, 0x34e6b406fdc658d4},
    {0xce7a87114ffa8c4b, 0x39d7ee0cae2b85c2, 0xb53b1fa322f229b4, 0x3df1f9f1f7fb6651},
    {0x69e670e7b1b92a6b, 0x6f7c080dc6646bd2, 0x5f5337d946299117, 0x4fd345e326f91e12},
    {0x98be8f04f1adb8fd, 0x537d991a58e0f3f0, 0xa0d13fe15e73b9ae, 0x467e791797b1aa63},
};
static const U t_asinl[][4] = {
    {0x0, 0x3fff000000000000, 0x8469898cc51701b8, 0x3fff921fb54442d1},
    {0x0, 0xbfff000000000000, 0x8469898cc51701b8, 0xbfff921fb54442d1},
    {0xffffa18d7bcdb6f7, 0x3ffeffffffffffff, 0xf1bc78828efd586e, 0x3fff921fb543a752},
    {0x0, 0x3f9b4484bfeebc2a, 0x0, 0x3f9b4484bfeebc2a},
    {0x415bfa8364008dd2, 0x3ffc06f5be7ee638, 0x9962db6d4c06cb39, 0x3ffc07b019307bbf},
    {0xbf9ebb4bedbac4ac, 0x3ffdeedddfaf7b6d, 0xdaee7b9b22d967c0, 0x3ffe023e9f362f4f},
    {0xe89dc424219b83f0, 0xbffee4700856c92f, 0xc48e2a2d2ca81637, 0xbfff3dbe1a233d00},
    {0x5027963165d4f633, 0xbfc727ea26ff6519, 0x5027963165d4f633, 0xbfc727ea26ff6519},
    {0xacf0576104f93e6a, 0x3ffe0fbe78ba2697, 0x21c67a115a97a8ab, 0x3ffe1e74dbf33d48},
    {0xc84c7195d1be7df4, 0xbfd950a836836536, 0xc84c719be2513688, 0xbfd950a836836536},
    {0x1487fa42ced787f5, 0xbffed02cfc40fd30, 0xf189204c521a1666, 0xbfff229886dd836c},
    {0x8cc9fe0344e5629c, 0xbfe5731fcb1d5b47, 0x94e9e97ed0160fbb, 0xbfe5731fcb1d5b47},
    {0xcee4e988149a4647, 0xbffd9fbc828213ac, 0xf07332377b9c6e5a, 0xbffdac1923d931fa},
    {0x4c3100a4cf179e5d, 0xbfda182330473e20, 0x4c3100b2c9460280, 0xbfda182330473e20},
    {0xd436cfd79f598cac, 0xbffe94c89a694dc9, 0xc2f69cb69c2701e5, 0xbffed2d467c7a1c5},
    {0x676cb10539dcbf7c, 0x3fe3a87dcf707558, 0x682f377242786f08, 0x3fe3a87dcf707558},
    {0x89a73370ed79c2b4, 0xbffd684a2364bd92, 0x397567741b086959, 0xbffd702bb5ee677b},
    {0x80c86dbe75551c0e, 0x3fc554c5674d95e0, 0x80c86dbe75551c0e, 0x3fc554c5674d95e0},
    {0xcdd6669d3ea0e5ab, 0x3ffe7a279c22122b, 0x358d1ea6a89ff63f, 0x3ffea97478ab939f},
    {0x587c275826a23c06, 0xbfda0fec69df0616, 0x587c2764ef3434e6, 0xbfda0fec69df0616},
};
static const U t_acosl[][4] = {
    {0x0, 0x3fff000000000000, 0x0, 0x0},
    {0x0, 0xbfff000000000000, 0x8469898cc51701b8, 0x4000921fb54442d1},
    {0xffffa18d7bcdb6f7, 0xbffeffffffffffff, 0x3b130107aa0a2d13, 0x4000921fb543f512},
    {0x0, 0x3f9b4484bfeebc2a, 0x8469898cc516ed70, 0x3fff921fb54442d1},
    {0xdfcf3a1bcb5777a2, 0x3ffecb017a10b819, 0x32668d0c55dd7407, 0x3ffdd6044e64f184},
    {0x7a358169a3a129f4, 0x3fcd82b9ab9ec070, 0x23bb1ea514fae32b, 0x3fff921fb54442d1},
    {0x7967a3525529d145, 0xbffe317f203fd3a2, 0xd4db070260fb9979, 0x40001ae609659e56},
    {0xc6bdedc3227b70cb, 0xbfd043bfae919654, 0xbe8e6aff1c08f34, 0x3fff921fb54442d4},
    {0x6b45a1bb4f5f206, 0x3ffe54df0b8600b1, 0x6aece5fab408473c, 0x3ffeaf3f2adc2333},
    {0x3adc113d0c94cb6d, 0xbfcf19d9a47bf82b, 0x9e432e08bd423c94, 0x3fff921fb54442d2},
    {0x190c0b3339536192, 0xbffef10d45990641, 0xd0e919e81f730b60, 0x4000731e2a36ceb3},
    {0x6b5e6d3e0c56992a, 0x3ff42788788b19d2, 0x238ff902782585c, 0x3fff91fac435109c},
    {0xcf64b1ee5f14fe33, 0x3ffd51d8542beb03, 0x82b8282168231cad, 0x3fff3c0ccca6e91f},
    {0x5fba3edfb7cfa47e, 0xbfe7729e14e6ebb1, 0x6b553b6df5ef7209, 0x3fff921fb6b6e0e6},
    {0xa195c4c7f5d13866, 0x3ffc6b47e8a6e65e, 0x1a4c5bc1078bc306, 0x3fff6478e0724132},
    {0x775cab03a8bdd4, 0x3ff0e1d3e68c713e, 0xbe20982c22d89b9b, 0x3fff921bf19c75af},
    {0x4aa672ee3cf12c36, 0x3ffb5dd49061ca9e, 0x55dd6f488555152a, 0x3fff7c3b98727f5c},
    {0xf3810539ad287add, 0xbfc8a8358f575637, 0x87b9f4ab73c3719f, 0x3fff921fb54442d1},
    {0xa78a57bc2a9bcb1b, 0xbffe6134fff3c3ce, 0xd9e67210ce22769c, 0x40002a81e039d8ab},
    {0xdc86832a6f95d65a, 0xbfdf12fd10ef5116, 0x9558daa3a19db9c5, 0x3fff921fb54555ce},
};
static const U t_asinhl[][4] = {
    {0x18c21ab905450cc3, 0x73e6a3750647fcab, 0xfe988f0fe96a9844, 0x400c1fd844a5b10f},
    {0x0, 0xbf9b4484bfeebc2a, 0x0, 0xbf9b4484bfeebc2a},
    {0xf2b1cd9cb4f8f7c5, 0x3ff7a15d1a79a9be, 0x3e8dd624aa83cc36, 0x3ff7a15c6196c15e},
    {0x26a66cde64f412be, 0x3fc77990dd980122, 0x26a66cde64f412bd, 0x3fc77990dd980122},
    {0xe259832f7c469523, 0x405b72936f134681, 0x55f89e2a410ae0fc, 0x400503548ce167ba},
    {0x4c36e011a2a265db, 0xc006f10ab10a53cc, 0x4f05ebe1c0e45d94, 0xc0018d5afb6639e5},
    {0xef60f82b10d90e25, 0x40153a3c51b12c2f, 0x74ce22398029e032, 0x4003025b8c832b39},
    {0xfe291079f9ffa2ee, 0x4025e53c3ab30c87, 0x8218ae65954a94f0, 0x4003bac151149152},
    {0x58dcd0c109246563, 0x3ff1a3060d0b97c9, 0xd478846516a6b32, 0x3ff1a3060cffe61d},
    {0xc302b427a7888410, 0x3fd90bf61041bd0f, 0xc302b42498a2f4a2, 0x3fd90bf61041bd0f},
    {0x22aea090b2058409, 0xc05439e178d6194a, 0x5333768765eb847, 0xc004de8415058a00},
    {0x984fcc84f1778b82, 0xbff704d6ff417746, 0xef0b19b961e2ca5f, 0xbff704d6d21f98e6},
    {0xf9aea1179d026706, 0xc0353a15cc243023, 0xf31f10adab3aab64, 0xc004329ee31a50e0},
    {0xdd2597d9898fda63, 0x40589e3d424c2e30, 0x774ac729bb42d7be, 0x4004f6ea8596fe9b},
    {0xe0855959b7371200, 0x401a74083b025321, 0xe622c0706b9a2210, 0x40033c82c29f50b7},
    {0x5413c878c44e746f, 0xc03f994d6d45f119, 0x8a34f603e7ddab30, 0xc0046c30d4f19637},
    {0x2ac4ec59e0ecbd9f, 0x405af4c5c04c4521, 0xa12d2b0a1f96ff11, 0x400501c3165d828b},
    {0x60158784585ed806, 0x403ac5d8a36da81d, 0xddcc086df18251d6, 0x4004514a924d1070},
};
static const U t_acoshl[][4] = {
    {0x0, 0x3fff000000000000, 0x0, 0x0},
    {0x1d83c94fb6d2b, 0x3fff000000000000, 0x3464e2f8b31927b, 0x3fdfebb76fb678a0},
    {0x18c21ab905450cc3, 0x73e6a3750647fcab, 0xfe988f0fe96a9844, 0x400c1fd844a5b10f},
    {0x35b70567e3a5d350, 0x402aa90712042528, 0x8e6e9657c59ade84, 0x4003f0165585232a},
    {0xebf9a26dbf70301, 0x4027d48fc295d3e7, 0x5ce9db41fb43bf, 0x4003d0605b2e0275},
    {0xd3c6ce0ef13ac520, 0x404bec7a6aac44b1, 0x2c9ec8b9cb1ca50d, 0x4004b0367d03cd6a},
    {0xab9db98278dee60c, 0x4048bf8387af0e47, 0x3075833fe7dad1a9, 0x40049ecfb5f4251a},
    {0x8e65b9091682efc5, 0x403b799da9716906, 0xd2056d042b236b04, 0x4004555d8c707913},
    {0xee8836111466c515, 0x401b4dee69f32c09, 0x76d5558ea3d0745f, 0x400345df56bf24f9},
    {0x77ee572b342a0a4e, 0x404132f261607137, 0x2cd4a61ef3bc032d, 0x400474fa96e0e30b},
    {0x9967544d890caa3c, 0x402a5f2dc1f17d64, 0x1b6a82f559c88792, 0x4003ed08938942b1},
    {0xbb6e09ea8f7514dc, 0x405685c10f0940d5, 0x503c07b27262c7e5, 0x4004eb569c6f6373},
    {0x97be1cd3f9539acf, 0x40206874cfec854b, 0x95f19a4b868ab3a5, 0x40037e8c1297359a},
    {0x85114ae2e6b4d4bd, 0x40430cfc7373f639, 0x5af1f67d2604deef, 0x40047f035ab54c1e},
    {0xa1bbd455809afa5c, 0x40492886b7722480, 0x7c61e899de9b2318, 0x4004a11060a051a2},
    {0xa7535aa6b5e18b11, 0x400af659a160d0e4, 0xf3982768915606ae, 0x40021fbd87c60681},
    {0xa60b6002158a4353, 0x3fff7c0710658637, 0x557f9eabb1c7f246, 0x3ffee5967187c89e},
    {0x29a55dc6f671f68b, 0x401f9c1c686b8ca8, 0xea71cfcdb8acbdc, 0x400375997c6a6adb},
    {0x7fe83e0dd02b10a7, 0x40056865e4b25d0a, 0x929bceac993820d0, 0x40014c6b013c3bc6},
};
static const U t_atanhl[][4] = {
    {0xffffa18d7bcdb6f7, 0x3ffeffffffffffff, 0x2052ec036fd2e87f, 0x400375f57369e934},
    {0x0, 0xbf9b4484bfeebc2a, 0x0, 0xbf9b4484bfeebc2a},
    {0xef29481c6b7644b5, 0xbffd865148a6ced1, 0xbca0f0cd8c4d0389, 0xbffd9b0fb9dd10a4},
    {0x5a4971c459a97e1, 0x3ffa51ace1856c93, 0x9c1c3808b88bf45b, 0x3ffa51dde3f0b906},
    {0x75ee399ff6e455ff, 0xbffefb80842994cd, 0x7172f047a113804, 0xc0005b19a5d21db5},
    {0xfb911ff4874ec5d5, 0x3feba09adec59935, 0xbfc85e9303344cb8, 0x3feba09adec59aa5},
    {0x4b7eadc2d4bffad0, 0xbffd0a7c8e9ce268, 0x4a77e0f48b6046ad, 0xbffd10c26b7e47d3},
    {0x5726eb5b958dd0cf, 0xbfe772adb5ec86c9, 0x5a34e1a31b69c383, 0xbfe772adb5ec86ca},
    {0x5b33badf3658df7f, 0xbffe21287fc906f2, 0x72d86ba9d77c4e13, 0xbffe47939ff9356a},
    {0xbaa2cd3d6ddb6644, 0x3ffdde7e7fff43fc, 0x9a294b2358ddcbd3, 0x3ffe035efe1264a3},
    {0x6d658ab86157472, 0xbffd078052236738, 0x652157e649dacbe0, 0xbffd0d8f545a6387},
    {0x76186ea09375e322, 0xbfc380bd38b0279b, 0x76186ea09375e322, 0xbfc380bd38b0279b},
    {0xd35c69d224754ec2, 0x3ffe2efefbefc655, 0x95d87d398f754988, 0x3ffe5c5f5af15719},
    {0x933bb71eb181dc41, 0xbfe286fd0a6b5203, 0x9387b7e179006d44, 0xbfe286fd0a6b5203},
    {0x75d87ccd0f82e528, 0xbffe161739481c3e, 0xe19816ad066b97bc, 0xbffe379b5d255f73},
    {0x6983c3c61fbefacc, 0xbfe49b4627d5d165, 0x6f0b1415aabca55f, 0xbfe49b4627d5d165},
    {0xf6f78013939431a5, 0xbff62b238a75b821, 0x4224e35d244ee085, 0xbff62b23ac7f5661},
    {0xc9e5a2fbcd445a90, 0xbfe70919badff4b4, 0x28a880265bff469c, 0xbfe70919badff4b5},
};
static const U t_powl[][6] = {
    {0x0, 0x4000000000000000, 0x0, 0x400cfffc00000000, 0xc908b2fb1366ea95, 0x7ffe6a09e667f3bc},
    {0x0, 0x4000000000000000, 0x0, 0xc00d00e200000000, 0x2d413cccfe7799, 0x0},
    {0x1d83c94fb6d2b, 0x3fff000000000000, 0x0, 0x403e158e460913d0, 0xc1ef16dd42ba2687, 0x40005bf0a8b14576},
    {0x0, 0xc000000000000000, 0x0, 0x4000800000000000, 0x0, 0xc002000000000000},
    {0x0, 0xc000000000000000, 0xa000000000000000, 0x406293e5939a08ce, 0x0, 0x7fff000000000000},
    {0x0, 0x4002400000000000, 0x0, 0x400b344000000000, 0xed99d037e3d04b75, 0x7ffeae596552b8fd},
    {0x0, 0x3ffe000000000000, 0x0, 0x400d01b800000000, 0x1, 0x0},
    {0x36beb10b17007ff1, 0x400961a876e05790, 0x7f767b2d0c64b407, 0x4004f2a0b200315d, 0x41f273fbda6b2190, 0x428b44ae87382064},
    {0xe5503a18d030fc70, 0x3fff1ca2e80f14f8, 0xb405d4411f43c569, 0x4004fb472a640d1f, 0xa7a870bf0a3bfa80, 0x40089feca5b00138},
    {0x9b0dee142510abab, 0x5ea8e464f6cb34dc, 0x3072dd71929f893c, 0xbffc3cebdc35146a, 0xcfd7c12cdb8dd616, 0x3b403087e3281d59},
    {0x22f00d23c1405072, 0x3ff9b9932d8376e3, 0x17c1e9a9e22f31dc, 0xc005bcc3b79c5463, 0xd7c35cd7c72eb6c1, 0x42429df657206ed4},
    {0x83ac03bc3d605c3, 0x3fff8b67eb4049e1, 0x1c2cc59987818b61, 0x4003f83c622187ab, 0x80a95b6196faa293, 0x4012b34416947ba7},
    {0xbdf6860b7e1d9da, 0x42bfb002ba9acc3, 0xc992555ede1c54cb, 0xbffe185b4140ab82, 0xfc664c4671dd0bf7, 0x60c10b0d6e887b85},
    {0x9264a3fc6f7543ac, 0x3ffbf6fff02b5556, 0xa4523e341f427a0a, 0x40057204e4739e60, 0xf3894facf330f41c, 0x3ee715fa86133f15},
    {0x13425f3b71aca1bb, 0x3ffed4c26e3a0a6a, 0x1e99acc38f5534e, 0xc00db05084918ce9, 0x27a8f9af72be6870, 0x4dc1071682f27c29},
    {0xc693588bb54050fa, 0x146f1311b13705ad, 0x46e19c0ab3daa877, 0x3ffd8c702d12b96d, 0xca9dded612cbd66b, 0x2f217ecfabd1b674},
    {0xa697e50ff8c9d8c0, 0x3ff98dfcc3de2866, 0x88a9958efa5b9887, 0x4002b0ac8b736f49, 0xf4cff25b55558ae7, 0x3fb66543911b1c9c},
    {0x86ccf0af01c7faaa, 0x3ffff4077a71dcfc, 0xa0ead01006062fd, 0xc005b0268149ec5b, 0xde74940b6a8b8830, 0x3f9691ac1d3d1e43},
    {0x4e17e856f20f9dd2, 0x8e893a298b0f8d2, 0xcc1f98776484cec4, 0x3ffd0e7e86924fb2, 0x36ce7098f3d4bab0, 0x3171be63a8f3310c},
    {0x11a0898471ac4417, 0x3ff8c36851881c9a, 0xbbacbd18d87cb5e, 0x400643ed003d1bdc, 0x6e43b92fad41df6d, 0x3c15bb32f374d974},
    {0x5f140767b0e35333, 0x3ffe9ec018952608, 0x544ddaffd5b74a73, 0xc004f90df73765ec, 0xdb4330af5a8e329d, 0x4012232f65aca07c},
    {0x1af8109a01b39e3b, 0x44a597141cd360b4, 0x3fd238ca99565ec7, 0xbffd63b721911c4d, 0xe8ccd9d34df3032d, 0x3e614edb2584d293},
    {0x3bbdfb06af68a40c, 0x400620c9c18f6208, 0x9653682d09919787, 0x4002f51e3ba5e399, 0xd5cadbdbd8f34f7d, 0x406f448f3a3c0161},
};
static const U t_atan2l[][6] = {
    {0xb8049732d11a23d, 0xc17387ae70c9e70, 0x0, 0xbfff000000000000, 0x8469898cc51701b8, 0x4000921fb54442d1},
    {0x0, 0xbfff000000000000, 0xb8049732d11a23d, 0xc17387ae70c9e70, 0x8469898cc51701b8, 0xbfff921fb54442d1},
    {0x0, 0x3fff000000000000, 0x0, 0x3fff000000000000, 0x8469898cc51701b8, 0x3ffe921fb54442d1},
    {0x0, 0x3f9b4484bfeebc2a, 0xa000000000000000, 0x406293e5939a08ce, 0x6ac5524bb3f6c14e, 0x3f379b604aaaca62},
    {0x5bc86c3c2c18b6c8, 0x400f3d6c603c73a7, 0x64ffe62118f91cb, 0xc01551e8ee64306e, 0x9626265e32c3d8d3, 0x4000903ec8a41b86},
    {0xf8adfa049c1b9bf0, 0xbfee1b28a889a173, 0x1c5c6d55c86cdddf, 0xc01f6b08de982a5c, 0x527e6332c4cc8450, 0xc000921fb54442d1},
    {0x8423e9f636b15882, 0x4024ff546e61270f, 0xb17e68163b4f2cf9, 0xbfe0a4b92fbf89c4, 0x846996b6f7eb80fe, 0x3fff921fb54442d1},
    {0xecc1713228bea61e, 0xc01916700520dd65, 0x32700d1c5c923f7e, 0x4016c49d5820a1f2, 0xa347d47a32b0857b, 0xbfff5ece0cafb3ec},
    {0x8465f42883737fb1, 0xc01ad496061b7121, 0x8bb6b54e354cc93e, 0xbfd8d3e6cae6e8a8, 0x8469c974d62b7b2f, 0xbfff921fb54442d1},
    {0x768b9671d66b43a1, 0x4023e8d4d784b931, 0x2e2ff193dd2cb50c, 0xbfe219ec6ab79588, 0x8469d35f0ae6cd62, 0x3fff921fb54442d1},
    {0xb5e9bde5809300d9, 0x400e668902637640, 0x45d87d799069bc99, 0xbfff61621ac4ef82, 0x30a6ca67d990b9bd, 0x3fff9221ade8d25f},
    {0x2cf5634140e9eb00, 0xbff81a6d89912011, 0xe3d095a18abad4a6, 0x401f8cb2dfb5b66f, 0xf605e550ce5e2c63, 0xbfd76c841f805582},
    {0x710b3528d0a4fdeb, 0x3fdf7cb5c695deab, 0x29d9e31f03589982, 0x4013c767b99a978a, 0x34c423f11e1a1d21, 0x3fcaac05b2ab30b2},
    {0xc40bc3992e4c5d3, 0x40067c75c550b25e, 0xfea7569a8ea79462, 0xc007cd55be322af0, 0xa3c86c6315bfc4f, 0x4000600ffd17e2ad},
    {0x5658ce6818dbb26d, 0x3ffda7a654606c4d, 0x9f8d9b329ce7ddb8, 0xbfef2d16c790b333, 0xff5c01c085a2e003, 0x3fff92228d06b255},
    {0xb987703ef918eb99, 0xc025a610e75286fd, 0xec7d996df4f4ac7e, 0xc01c018e4da031d0, 0x6ab8d8354563d3f7, 0xbfff926dd128a7cc},
    {0xa9d3b34ba4b9a974, 0x40116f9f4bdae2bd, 0xbd7a8a9377e597ce, 0x3fee836a7afdcefb, 0x70948a0a40a65baa, 0x3fff921fb5442118},
    {0x87b430a4b6851576, 0xc0165523bb7032d7, 0x9db44555df50b9ae, 0xc00e3141940e7b4e, 0x842bd8cb61e696ba, 0xbfff9304c77fd8f0},
    {0x1cf0d6779913b9f7, 0xc01e355b48f0cd11, 0x2c15be200501d3ae, 0x400416ba693512b6, 0x4091dd8f5f15a818, 0xbfff921fb50a98f2},
    {0x74c8f5b5f5f4a9a0, 0xc00d2dc49aa92108, 0xd002c3e1ea13bb9b, 0x4024b3c3f5020766, 0xaa28c9d7c5f78c8d, 0xbfe7628f7b67a781},
};
static const U t_hypotl[][6] = {
    {0x0, 0x7ffe000000000000, 0x0, 0x7ffe000000000000, 0xc908b2fb1366ea95, 0x7ffe6a09e667f3bc},
    {0x40000000000000, 0x0, 0x40000000000000, 0x0, 0x5a827999fcef32, 0x0},
    {0x0, 0x4000800000000000, 0x0, 0x4001000000000000, 0x0, 0x4001400000000000},
    {0xb8049732d11a23d, 0xc17387ae70c9e70, 0x0, 0x3fff000000000000, 0x0, 0x3fff000000000000},
    {0xa16098590c0b0d2f, 0x96d31da846bba167, 0xa59480368f85e724, 0xdc92d40a0be2bb43, 0xa59480368f85e724, 0x5c92d40a0be2bb43},
    {0xc7db227c2f5d3, 0x3ffa323ccf0920cf, 0xe138f7551cb7f5c8, 0x400125e09ad33b4c, 0xf0079f428c86c643, 0x400125e3190cb74c},
    {0xd2abf123b651da66, 0x84ff712ce3d9f47d, 0x86f7aa610d0de43, 0x61c3aadf35bbec62, 0x86f7aa610d0de43, 0x61c3aadf35bbec62},
    {0xa0721d6cd6fddc25, 0xc0026489fd84cf62, 0xb8b773c823ac3f5f, 0x4002f166720cb25b, 0xa267a890ba24f81, 0x400331fe393ad1c6},
    {0xbcc8e2f650fc0202, 0xb0ae6e699cce1002, 0x8220b6c3687974fe, 0xd7cb7c4a1ccd3eaf, 0x8220b6c3687974fe, 0x57cb7c4a1ccd3eaf},
    {0xca2ee3cbcb685d98, 0xc000f1887c59e59a, 0xd4faadb5dd010b8, 0x400443fba24d5ea4, 0x7ffcec3210eecac3, 0x40044578c886a798},
    {0x3fc95879b12fae7, 0xe4cbf153dc4138d0, 0x52e800a9a58b251e, 0x6dadc03672207225, 0x52e800a9a58b251e, 0x6dadc03672207225},
    {0x4f0aa7b594f7b28b, 0xbffed0dd22bac6a8, 0x56b886860f0fcb8c, 0xbfff3e9ecb766bcd, 0x4c4cdb0b906dcf3d, 0x3fff8a63d7cd476c},
    {0x56489f9241cf7457, 0xc636408d24c01112, 0xd8be291dfd680ef1, 0x6fc11f80ed4e8bd8, 0xd8be291dfd680ef1, 0x6fc11f80ed4e8bd8},
    {0x49cf3afd4f92babe, 0xc001e2a5d5a87de1, 0x8d8d115d6ff5eb93, 0xc0039b9d70e57b81, 0x541c86df263bbc37, 0x4003acefa73ef72b},
    {0x76b3f3c57a09b76, 0x767de102d9b6101f, 0x9fab64765e4f3ca9, 0xc6a5a66092e98aaf, 0x76b3f3c57a09b76, 0x767de102d9b6101f},
    {0x9797a61e53ef6e95, 0x3ffeb1ab615def96, 0x16e5a39ec902b8e7, 0xc000232780cd85cc, 0x6910be34b7f43916, 0x400036af5ad0298a},
    {0xc06d5c3470622c13, 0xa6b4e68f35880e2b, 0x9b7c316ae4b18795, 0x2cec21102e4b6741, 0x9b7c316ae4b18795, 0x2cec21102e4b6741},
    {0xa5cf0f7a350a1dbf, 0x40010f65b39f611f, 0xd3320d1202b7e85, 0xc004fc00c8a13fc7, 0x6f2264944b0e8676, 0x4004fd22720d4b9b},
    {0x6e51f7b21e8d7157, 0x743678c6fa62d9d7, 0xd53b2616f7ca9db, 0xf6926aeab059ec4b, 0xd53b2616f7ca9db, 0x76926aeab059ec4b},
    {0xee83bd786790facd, 0xbffe1bc1a3619a77, 0x32d175d6730c2580, 0x40031e1fb23fe5b1, 0xf4f99b95980404b7, 0x40031e42dd29a6c7},
};
static const U t_fmal[][8] = {
    {0x0, 0x3fff000000000000, 0x0, 0x3fff000000000000, 0x0, 0x3f37000000000000, 0x0, 0x3fff000000000000},
    {0x0, 0x5f3f000000000000, 0x0, 0x5f3f000000000000, 0x0, 0xfe7f000000000000, 0x0, 0x0},
    {0x0, 0x20bf000000000000, 0x0, 0x1f2f000000000000, 0x0, 0x53000000000000, 0x1000, 0x53000000000000},
    {0x2f3942192484, 0x3fff000000000000, 0xffffa18d7bcdb6f7, 0x3ffeffffffffffff, 0x0, 0xbfff000000000000, 0x77579761b013eca4, 0xbf8e0000116c2627},
    {0xb8c396225d83a1e6, 0x3fb3151d20fec557, 0xeb7d65160f0a793e, 0xc00b1949671580b8, 0x1c81a09f30a65113, 0x3f9e64d2091f44df, 0x7de7cc63e16e5a87, 0xbfbf307c6e1b5bdb},
    {0x49f7d98d3b48bb09, 0x3ffffca0d85f9abf, 0x8da81d0e1814c562, 0x3fff8488d68e7f0a, 0x3898ec4658676643, 0xbfff727ea2e2caf4, 0x9c983f05511bd468, 0x3fff917504caa0e0},
    {0x68cf0606360d2a5e, 0x3fd50b7dccbbb050, 0x1a66342d838be8e8, 0xc0585737211bde4a, 0x7be89b6b4a5b6914, 0xbfbb14c523b16e98, 0xecb75e20cc569b34, 0xc02e669f27eecb7b},
    {0xcc230f2cfc629388, 0x3fffb771cba4f206, 0x8175c61af5255fa3, 0x3fffbf6454aa271a, 0xf2fb4682460c1f6e, 0xc000e4f2bab38fa7, 0x9836a407d3208787, 0xbffe93d312124988},
    {0xa3870fd9af1cfec0, 0x3fde84d26b8df9e2, 0x5e9d51c4ff03b64e, 0x4062d47b25ef3d96, 0xc3595de085782976, 0x4014a1a147d02b53, 0x7295ea0834623d98, 0x404263c5dbab80cf},
    {0xb4bd0d7e7e6357de, 0x3fff99e886a2bdb1, 0x8915a8c149446924, 0x3fff5f3f1cfa2092, 0xfee74e049f9e20aa, 0xc0007e03f9dcf533, 0x5b4bfbad61e0d31f, 0xbffe933a29036c9f},
    {0x2b8bdbf4fe81545a, 0x4040de3bb711e503, 0x887ade9179aa0454, 0x4036695f2a13cbd2, 0x773d89e69e0d2d5e, 0x4082019b033f4d09, 0xd1319cf7454bbbf9, 0x408201ef65beca87},
    {0xc12e8c2451b94b84, 0x3fff699e82be8aab, 0xd21c318f2b63cf97, 0x3ffff282ebc13064, 0xb7f7b16e366228ea, 0xbffff8086f8422f9, 0xde73a3590f165158, 0x3ffe904db4aca4d8},
    {0x1975325a6b39b2eb, 0x3fb1b56244aaa15c, 0x2694bcad221e2504, 0x4052749ba4a086d7, 0xc62e3333b86796cf, 0xc0a0c8323851614d, 0xc62e3333b86796cf, 0xc0a0c8323851614d},
    {0x475f0dd60ef9c061, 0x3fff014695cfd44c, 0x8a9363fbf8ded287, 0x3fff537d4a9c2eb0, 0x14ea86a67f68c5a5, 0xc0005faa4627c8fe, 0x55965a6913e74ec, 0xbfff6a26297ad823},
    {0x1faff3eac9344715, 0xc03fad80615fd18c, 0xcb13add52e9f3d71, 0xc055e44a91c3fb5f, 0x94351b37fabe294b, 0xc041c5062d8ac8d6, 0x4374da0ceea92b0a, 0x40969641e9e1e9c7},
    {0xb077e40e8e6eaa91, 0x3fff9faebceb5556, 0xcbea3796f21e734, 0x3fffadfed0b91136, 0x1aabfdf479effeb9, 0xc000fe8aaa5ef8df, 0x2a1ba875e7d00c18, 0xbfff42dfbfdda785},
    {0x4828c9ac6896746b, 0xc05b1181bb37435d, 0xe81122ab909f9a40, 0xc041fed2e8e2148c, 0xb5434b8c40b37209, 0x3fe88d23ef050893, 0x90cf8463defbadc4, 0x409e10e0e41963d7},
    {0x8de819553eb83ec3, 0x3fff84ff26007a16, 0xd84e1c17a1e9e71b, 0x3fffad806ba13e6d, 0xa0caf93b03c80379, 0xbfff7b013fea1845, 0x423670800bddfe8, 0x3fff11a175e35cc5},
    {0xb137a216126c837c, 0x3fbdf17dce4cc15c, 0x1a53f4106c4904ce, 0x3f9c48dd56d0d795, 0x52a710967817db56, 0x3f85bd6c76b2345b, 0x359165080747fcd5, 0x3f85bd6c76b234ab},
    {0x24dceb9ba49adae6, 0x3fffcdbc3c1e9203, 0xd47cd7f55bb709b9, 0x3fffcefc8de6b330, 0x9090050dcb711635, 0xbfff4ffe78686d1c, 0x6187940b753ac955, 0x3ffff312c152b3ea},
};
#endif

#define N(t) (sizeof t / sizeof t[0])

int main(void)
{
    unsigned i;
    long bad;
    bad = 0;
    for (i = 0; i < N(t_expl); i++) {
        if (!ok(expl(ld(t_expl[i][0], t_expl[i][1])), ld(t_expl[i][2], t_expl[i][3]), ULPS))
            bad++;
    }
    print_str("expl", 4);
    print_i64(bad);
    bad = 0;
    for (i = 0; i < N(t_exp2l); i++) {
        if (!ok(exp2l(ld(t_exp2l[i][0], t_exp2l[i][1])), ld(t_exp2l[i][2], t_exp2l[i][3]), ULPS))
            bad++;
    }
    print_str("exp2l", 5);
    print_i64(bad);
    bad = 0;
    for (i = 0; i < N(t_expm1l); i++) {
        if (!ok(expm1l(ld(t_expm1l[i][0], t_expm1l[i][1])), ld(t_expm1l[i][2], t_expm1l[i][3]), ULPS))
            bad++;
    }
    print_str("expm1l", 6);
    print_i64(bad);
    bad = 0;
    for (i = 0; i < N(t_logl); i++) {
        if (!ok(logl(ld(t_logl[i][0], t_logl[i][1])), ld(t_logl[i][2], t_logl[i][3]), ULPS))
            bad++;
    }
    print_str("logl", 4);
    print_i64(bad);
    bad = 0;
    for (i = 0; i < N(t_log2l); i++) {
        if (!ok(log2l(ld(t_log2l[i][0], t_log2l[i][1])), ld(t_log2l[i][2], t_log2l[i][3]), ULPS))
            bad++;
    }
    print_str("log2l", 5);
    print_i64(bad);
    bad = 0;
    for (i = 0; i < N(t_log10l); i++) {
        if (!ok(log10l(ld(t_log10l[i][0], t_log10l[i][1])), ld(t_log10l[i][2], t_log10l[i][3]), ULPS))
            bad++;
    }
    print_str("log10l", 6);
    print_i64(bad);
    bad = 0;
    for (i = 0; i < N(t_log1pl); i++) {
        if (!ok(log1pl(ld(t_log1pl[i][0], t_log1pl[i][1])), ld(t_log1pl[i][2], t_log1pl[i][3]), ULPS))
            bad++;
    }
    print_str("log1pl", 6);
    print_i64(bad);
    bad = 0;
    for (i = 0; i < N(t_sinl); i++) {
        if (!ok(sinl(ld(t_sinl[i][0], t_sinl[i][1])), ld(t_sinl[i][2], t_sinl[i][3]), ULPS))
            bad++;
    }
    print_str("sinl", 4);
    print_i64(bad);
    bad = 0;
    for (i = 0; i < N(t_cosl); i++) {
        if (!ok(cosl(ld(t_cosl[i][0], t_cosl[i][1])), ld(t_cosl[i][2], t_cosl[i][3]), ULPS))
            bad++;
    }
    print_str("cosl", 4);
    print_i64(bad);
    bad = 0;
    for (i = 0; i < N(t_tanl); i++) {
        if (!ok(tanl(ld(t_tanl[i][0], t_tanl[i][1])), ld(t_tanl[i][2], t_tanl[i][3]), ULPS))
            bad++;
    }
    print_str("tanl", 4);
    print_i64(bad);
    bad = 0;
    for (i = 0; i < N(t_atanl); i++) {
        if (!ok(atanl(ld(t_atanl[i][0], t_atanl[i][1])), ld(t_atanl[i][2], t_atanl[i][3]), ULPS))
            bad++;
    }
    print_str("atanl", 5);
    print_i64(bad);
    bad = 0;
    for (i = 0; i < N(t_sinhl); i++) {
        if (!ok(sinhl(ld(t_sinhl[i][0], t_sinhl[i][1])), ld(t_sinhl[i][2], t_sinhl[i][3]), ULPS))
            bad++;
    }
    print_str("sinhl", 5);
    print_i64(bad);
    bad = 0;
    for (i = 0; i < N(t_coshl); i++) {
        if (!ok(coshl(ld(t_coshl[i][0], t_coshl[i][1])), ld(t_coshl[i][2], t_coshl[i][3]), ULPS))
            bad++;
    }
    print_str("coshl", 5);
    print_i64(bad);
    bad = 0;
    for (i = 0; i < N(t_tanhl); i++) {
        if (!ok(tanhl(ld(t_tanhl[i][0], t_tanhl[i][1])), ld(t_tanhl[i][2], t_tanhl[i][3]), ULPS))
            bad++;
    }
    print_str("tanhl", 5);
    print_i64(bad);
    bad = 0;
    for (i = 0; i < N(t_cbrtl); i++) {
        if (!ok(cbrtl(ld(t_cbrtl[i][0], t_cbrtl[i][1])), ld(t_cbrtl[i][2], t_cbrtl[i][3]), ULPS))
            bad++;
    }
    print_str("cbrtl", 5);
    print_i64(bad);
    bad = 0;
    for (i = 0; i < N(t_asinl); i++) {
        if (!ok(asinl(ld(t_asinl[i][0], t_asinl[i][1])), ld(t_asinl[i][2], t_asinl[i][3]), ULPS))
            bad++;
    }
    print_str("asinl", 5);
    print_i64(bad);
    bad = 0;
    for (i = 0; i < N(t_acosl); i++) {
        if (!ok(acosl(ld(t_acosl[i][0], t_acosl[i][1])), ld(t_acosl[i][2], t_acosl[i][3]), ULPS))
            bad++;
    }
    print_str("acosl", 5);
    print_i64(bad);
    bad = 0;
    for (i = 0; i < N(t_asinhl); i++) {
        if (!ok(asinhl(ld(t_asinhl[i][0], t_asinhl[i][1])), ld(t_asinhl[i][2], t_asinhl[i][3]), ULPS))
            bad++;
    }
    print_str("asinhl", 6);
    print_i64(bad);
    bad = 0;
    for (i = 0; i < N(t_acoshl); i++) {
        if (!ok(acoshl(ld(t_acoshl[i][0], t_acoshl[i][1])), ld(t_acoshl[i][2], t_acoshl[i][3]), ULPS))
            bad++;
    }
    print_str("acoshl", 6);
    print_i64(bad);
    bad = 0;
    for (i = 0; i < N(t_atanhl); i++) {
        if (!ok(atanhl(ld(t_atanhl[i][0], t_atanhl[i][1])), ld(t_atanhl[i][2], t_atanhl[i][3]), ULPS))
            bad++;
    }
    print_str("atanhl", 6);
    print_i64(bad);
    bad = 0;
    for (i = 0; i < N(t_powl); i++) {
        if (!ok(powl(ld(t_powl[i][0], t_powl[i][1]), ld(t_powl[i][2], t_powl[i][3])), ld(t_powl[i][4], t_powl[i][5]), ULPS))
            bad++;
    }
    print_str("powl", 4);
    print_i64(bad);
    bad = 0;
    for (i = 0; i < N(t_atan2l); i++) {
        if (!ok(atan2l(ld(t_atan2l[i][0], t_atan2l[i][1]), ld(t_atan2l[i][2], t_atan2l[i][3])), ld(t_atan2l[i][4], t_atan2l[i][5]), ULPS))
            bad++;
    }
    print_str("atan2l", 6);
    print_i64(bad);
    bad = 0;
    for (i = 0; i < N(t_hypotl); i++) {
        if (!ok(hypotl(ld(t_hypotl[i][0], t_hypotl[i][1]), ld(t_hypotl[i][2], t_hypotl[i][3])), ld(t_hypotl[i][4], t_hypotl[i][5]), ULPS))
            bad++;
    }
    print_str("hypotl", 6);
    print_i64(bad);
    bad = 0;
    for (i = 0; i < N(t_fmal); i++) {
        if (!ok(fmal(ld(t_fmal[i][0], t_fmal[i][1]), ld(t_fmal[i][2], t_fmal[i][3]), ld(t_fmal[i][4], t_fmal[i][5])), ld(t_fmal[i][6], t_fmal[i][7]), ULPS))
            bad++;
    }
    print_str("fmal", 4);
    print_i64(bad);
    return 0;
}
#else
/* (long double is double: the double functions) */
int main(void)
{
    print_str("expl", 4);
    print_i64(0);
    print_str("exp2l", 5);
    print_i64(0);
    print_str("expm1l", 6);
    print_i64(0);
    print_str("logl", 4);
    print_i64(0);
    print_str("log2l", 5);
    print_i64(0);
    print_str("log10l", 6);
    print_i64(0);
    print_str("log1pl", 6);
    print_i64(0);
    print_str("sinl", 4);
    print_i64(0);
    print_str("cosl", 4);
    print_i64(0);
    print_str("tanl", 4);
    print_i64(0);
    print_str("atanl", 5);
    print_i64(0);
    print_str("sinhl", 5);
    print_i64(0);
    print_str("coshl", 5);
    print_i64(0);
    print_str("tanhl", 5);
    print_i64(0);
    print_str("cbrtl", 5);
    print_i64(0);
    print_str("asinl", 5);
    print_i64(0);
    print_str("acosl", 5);
    print_i64(0);
    print_str("asinhl", 6);
    print_i64(0);
    print_str("acoshl", 6);
    print_i64(0);
    print_str("atanhl", 6);
    print_i64(0);
    print_str("powl", 4);
    print_i64(0);
    print_str("atan2l", 6);
    print_i64(0);
    print_str("hypotl", 6);
    print_i64(0);
    print_str("fmal", 4);
    print_i64(0);
    return 0;
}
#endif

/* expect: expl */
/* expect: 0 */
/* expect: exp2l */
/* expect: 0 */
/* expect: expm1l */
/* expect: 0 */
/* expect: logl */
/* expect: 0 */
/* expect: log2l */
/* expect: 0 */
/* expect: log10l */
/* expect: 0 */
/* expect: log1pl */
/* expect: 0 */
/* expect: sinl */
/* expect: 0 */
/* expect: cosl */
/* expect: 0 */
/* expect: tanl */
/* expect: 0 */
/* expect: atanl */
/* expect: 0 */
/* expect: sinhl */
/* expect: 0 */
/* expect: coshl */
/* expect: 0 */
/* expect: tanhl */
/* expect: 0 */
/* expect: cbrtl */
/* expect: 0 */
/* expect: asinl */
/* expect: 0 */
/* expect: acosl */
/* expect: 0 */
/* expect: asinhl */
/* expect: 0 */
/* expect: acoshl */
/* expect: 0 */
/* expect: atanhl */
/* expect: 0 */
/* expect: powl */
/* expect: 0 */
/* expect: atan2l */
/* expect: 0 */
/* expect: hypotl */
/* expect: 0 */
/* expect: fmal */
/* expect: 0 */
