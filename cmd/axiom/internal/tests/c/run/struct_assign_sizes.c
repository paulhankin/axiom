/* Structure assignment of each size class (inline moves, a loop, a block
   copy): onto another object, and onto itself through two pointers (the
   overlap C allows, 6.5.16.1p3); and structure initialization. */
#include "host.h"

struct s3 { unsigned char b[3]; };
struct s16 { unsigned char b[16]; };
struct s40 { unsigned char b[40]; };
struct s100 { unsigned char b[100]; };
struct s256 { unsigned char b[256]; };
struct s300 { unsigned char b[300]; };
struct s1000 { unsigned char b[1000]; };
struct s2048 { unsigned char b[2048]; };
struct s2100 { unsigned char b[2100]; };
struct s3000 { unsigned char b[3000]; };

static unsigned long long h;
static void mix(const unsigned char *p, int n) {
    for (int i = 0; i < n; i++) h = h * 31 + p[i];
}

static struct s3 g3[2];
static void assign3(struct s3 *d, const struct s3 *q) { *d = *q; }
static void t3(int k) {
    for (int i = 0; i < 3; i++) {
        g3[0].b[i] = (unsigned char)(i * 7 + k);
        g3[1].b[i] = (unsigned char)(i * 3 + 1);
    }
    assign3(&g3[k], &g3[k]);
    mix(g3[k].b, 3);
    assign3(&g3[1 - k], &g3[k]);
    mix(g3[1 - k].b, 3);
    struct s3 l = g3[0];
    l.b[3 - 1] ^= 0x5a;
    g3[1] = l;
    mix(g3[1].b, 3);
    print_u64(h);
}

static struct s16 g16[2];
static void assign16(struct s16 *d, const struct s16 *q) { *d = *q; }
static void t16(int k) {
    for (int i = 0; i < 16; i++) {
        g16[0].b[i] = (unsigned char)(i * 7 + k);
        g16[1].b[i] = (unsigned char)(i * 3 + 1);
    }
    assign16(&g16[k], &g16[k]);
    mix(g16[k].b, 16);
    assign16(&g16[1 - k], &g16[k]);
    mix(g16[1 - k].b, 16);
    struct s16 l = g16[0];
    l.b[16 - 1] ^= 0x5a;
    g16[1] = l;
    mix(g16[1].b, 16);
    print_u64(h);
}

static struct s40 g40[2];
static void assign40(struct s40 *d, const struct s40 *q) { *d = *q; }
static void t40(int k) {
    for (int i = 0; i < 40; i++) {
        g40[0].b[i] = (unsigned char)(i * 7 + k);
        g40[1].b[i] = (unsigned char)(i * 3 + 1);
    }
    assign40(&g40[k], &g40[k]);
    mix(g40[k].b, 40);
    assign40(&g40[1 - k], &g40[k]);
    mix(g40[1 - k].b, 40);
    struct s40 l = g40[0];
    l.b[40 - 1] ^= 0x5a;
    g40[1] = l;
    mix(g40[1].b, 40);
    print_u64(h);
}

static struct s100 g100[2];
static void assign100(struct s100 *d, const struct s100 *q) { *d = *q; }
static void t100(int k) {
    for (int i = 0; i < 100; i++) {
        g100[0].b[i] = (unsigned char)(i * 7 + k);
        g100[1].b[i] = (unsigned char)(i * 3 + 1);
    }
    assign100(&g100[k], &g100[k]);
    mix(g100[k].b, 100);
    assign100(&g100[1 - k], &g100[k]);
    mix(g100[1 - k].b, 100);
    struct s100 l = g100[0];
    l.b[100 - 1] ^= 0x5a;
    g100[1] = l;
    mix(g100[1].b, 100);
    print_u64(h);
}

static struct s256 g256[2];
static void assign256(struct s256 *d, const struct s256 *q) { *d = *q; }
static void t256(int k) {
    for (int i = 0; i < 256; i++) {
        g256[0].b[i] = (unsigned char)(i * 7 + k);
        g256[1].b[i] = (unsigned char)(i * 3 + 1);
    }
    assign256(&g256[k], &g256[k]);
    mix(g256[k].b, 256);
    assign256(&g256[1 - k], &g256[k]);
    mix(g256[1 - k].b, 256);
    struct s256 l = g256[0];
    l.b[256 - 1] ^= 0x5a;
    g256[1] = l;
    mix(g256[1].b, 256);
    print_u64(h);
}

static struct s300 g300[2];
static void assign300(struct s300 *d, const struct s300 *q) { *d = *q; }
static void t300(int k) {
    for (int i = 0; i < 300; i++) {
        g300[0].b[i] = (unsigned char)(i * 7 + k);
        g300[1].b[i] = (unsigned char)(i * 3 + 1);
    }
    assign300(&g300[k], &g300[k]);
    mix(g300[k].b, 300);
    assign300(&g300[1 - k], &g300[k]);
    mix(g300[1 - k].b, 300);
    struct s300 l = g300[0];
    l.b[300 - 1] ^= 0x5a;
    g300[1] = l;
    mix(g300[1].b, 300);
    print_u64(h);
}

static struct s1000 g1000[2];
static void assign1000(struct s1000 *d, const struct s1000 *q) { *d = *q; }
static void t1000(int k) {
    for (int i = 0; i < 1000; i++) {
        g1000[0].b[i] = (unsigned char)(i * 7 + k);
        g1000[1].b[i] = (unsigned char)(i * 3 + 1);
    }
    assign1000(&g1000[k], &g1000[k]);
    mix(g1000[k].b, 1000);
    assign1000(&g1000[1 - k], &g1000[k]);
    mix(g1000[1 - k].b, 1000);
    struct s1000 l = g1000[0];
    l.b[1000 - 1] ^= 0x5a;
    g1000[1] = l;
    mix(g1000[1].b, 1000);
    print_u64(h);
}

static struct s2048 g2048[2];
static void assign2048(struct s2048 *d, const struct s2048 *q) { *d = *q; }
static void t2048(int k) {
    for (int i = 0; i < 2048; i++) {
        g2048[0].b[i] = (unsigned char)(i * 7 + k);
        g2048[1].b[i] = (unsigned char)(i * 3 + 1);
    }
    assign2048(&g2048[k], &g2048[k]);
    mix(g2048[k].b, 2048);
    assign2048(&g2048[1 - k], &g2048[k]);
    mix(g2048[1 - k].b, 2048);
    struct s2048 l = g2048[0];
    l.b[2048 - 1] ^= 0x5a;
    g2048[1] = l;
    mix(g2048[1].b, 2048);
    print_u64(h);
}

static struct s2100 g2100[2];
static void assign2100(struct s2100 *d, const struct s2100 *q) { *d = *q; }
static void t2100(int k) {
    for (int i = 0; i < 2100; i++) {
        g2100[0].b[i] = (unsigned char)(i * 7 + k);
        g2100[1].b[i] = (unsigned char)(i * 3 + 1);
    }
    assign2100(&g2100[k], &g2100[k]);
    mix(g2100[k].b, 2100);
    assign2100(&g2100[1 - k], &g2100[k]);
    mix(g2100[1 - k].b, 2100);
    struct s2100 l = g2100[0];
    l.b[2100 - 1] ^= 0x5a;
    g2100[1] = l;
    mix(g2100[1].b, 2100);
    print_u64(h);
}

static struct s3000 g3000[2];
static void assign3000(struct s3000 *d, const struct s3000 *q) { *d = *q; }
static void t3000(int k) {
    for (int i = 0; i < 3000; i++) {
        g3000[0].b[i] = (unsigned char)(i * 7 + k);
        g3000[1].b[i] = (unsigned char)(i * 3 + 1);
    }
    assign3000(&g3000[k], &g3000[k]);
    mix(g3000[k].b, 3000);
    assign3000(&g3000[1 - k], &g3000[k]);
    mix(g3000[1 - k].b, 3000);
    struct s3000 l = g3000[0];
    l.b[3000 - 1] ^= 0x5a;
    g3000[1] = l;
    mix(g3000[1].b, 3000);
    print_u64(h);
}

int main(void) {
    t3(0);
    t3(1);
    t16(0);
    t16(1);
    t40(0);
    t40(1);
    t100(0);
    t100(1);
    t256(0);
    t256(1);
    t300(0);
    t300(1);
    t1000(0);
    t1000(1);
    t2048(0);
    t2048(1);
    t2100(0);
    t2100(1);
    t3000(0);
    t3000(1);
    return 0;
}
/* expect: 205020232333 */
/* expect: 7945142626481159981 */
/* expect: 15734769067576930719 */
/* expect: 10020771797266242349 */
/* expect: 14062839734220589835 */
/* expect: 1268133091738632693 */
/* expect: 5354257328258885321 */
/* expect: 5180908014756475409 */
/* expect: 1289362168216662843 */
/* expect: 12839322922387136865 */
/* expect: 9211422894249961593 */
/* expect: 13685854714590400213 */
/* expect: 2105336007043105939 */
/* expect: 3168858351531587805 */
/* expect: 17496060399533294727 */
/* expect: 18186861049667876909 */
/* expect: 8588180042739070121 */
/* expect: 15887078612252923641 */
/* expect: 10510158530178321759 */
/* expect: 3149552135326045681 */
