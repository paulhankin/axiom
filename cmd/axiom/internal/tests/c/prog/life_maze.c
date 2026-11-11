/* flags: --c-std=c99 */
/* Bit manipulation: Conway's game of life on a 64x64 torus with one
   uint64_t per row (neighbour counts by bit-sliced addition), an
   elementary cellular automaton (rule 30 and rule 110), and a maze carved
   by a randomized depth-first search in a grid of wall bits, solved by a
   breadth-first search whose frontier is a bit set; the path is traced
   back and the maze printed as text rows. Population counts, rotations
   and bit scans are written out by hand. */
#include "host.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define W 64
typedef uint64_t row;

static row board[W], next_board[W];

static row rotl(row x, unsigned k) { k &= 63; return k ? (x << k) | (x >> (64 - k)) : x; }
static row rotr(row x, unsigned k) { k &= 63; return k ? (x >> k) | (x << (64 - k)) : x; }

static int popcount64(uint64_t x) {
    x = x - ((x >> 1) & 0x5555555555555555ull);
    x = (x & 0x3333333333333333ull) + ((x >> 2) & 0x3333333333333333ull);
    x = (x + (x >> 4)) & 0x0f0f0f0f0f0f0f0full;
    return (int)((x * 0x0101010101010101ull) >> 56);
}

static int ctz64(uint64_t x) {
    if (!x) return 64;
    int n = 0;
    if (!(x & 0xffffffffull)) { n += 32; x >>= 32; }
    if (!(x & 0xffffull)) { n += 16; x >>= 16; }
    if (!(x & 0xffull)) { n += 8; x >>= 8; }
    if (!(x & 0xfull)) { n += 4; x >>= 4; }
    if (!(x & 0x3ull)) { n += 2; x >>= 2; }
    if (!(x & 0x1ull)) n += 1;
    return n;
}

/* one step: for each row, add the eight neighbour bit-planes with a
   carry-save adder into three bits s0 s1 s2 (counts 0..7, 8 wraps to 0
   which is "dead" anyway) */
static void life_step(void) {
    for (int y = 0; y < W; y++) {
        row up = board[(y + W - 1) % W], mid = board[y], dn = board[(y + 1) % W];
        row n[8] = { rotl(up, 1), up, rotr(up, 1), rotl(mid, 1), rotr(mid, 1), rotl(dn, 1), dn, rotr(dn, 1) };
        row s0 = 0, s1 = 0, s2 = 0;
        for (int i = 0; i < 8; i++) {
            row c0 = s0 & n[i];
            s0 ^= n[i];
            row c1 = s1 & c0;
            s1 ^= c0;
            s2 |= c1;
        }
        /* alive next: count == 3, or count == 2 and alive */
        row three = s0 & s1 & ~s2;
        row two = ~s0 & s1 & ~s2;
        next_board[y] = three | (two & mid);
    }
    for (int y = 0; y < W; y++) board[y] = next_board[y];
}

static int population(void) {
    int p = 0;
    for (int y = 0; y < W; y++) p += popcount64(board[y]);
    return p;
}

static void place(const char *const *pat, int rows, int x0, int y0) {
    for (int r = 0; r < rows; r++)
        for (int c = 0; pat[r][c]; c++)
            if (pat[r][c] == '#') board[(y0 + r) % W] |= (row)1 << ((x0 + c) % W);
}

static uint64_t board_hash(void) {
    uint64_t h = 0;
    for (int y = 0; y < W; y++) h = (h ^ board[y]) * 0x9e3779b97f4a7c15ull + (uint64_t)y;
    return h;
}

/* ---- elementary automaton ---- */

static uint64_t eca(unsigned rule, uint64_t cells, int steps, int *ones) {
    for (int s = 0; s < steps; s++) {
        uint64_t l = rotl(cells, 1), r = rotr(cells, 1), nx = 0;
        for (unsigned pat = 0; pat < 8; pat++) {
            if (!((rule >> pat) & 1)) continue;
            uint64_t m = ((pat & 4) ? l : ~l) & ((pat & 2) ? cells : ~cells) & ((pat & 1) ? r : ~r);
            nx |= m;
        }
        cells = nx;
        *ones += popcount64(cells);
    }
    return cells;
}

/* ---- maze ---- */

#define MW 23
#define MH 15
/* cell (x, y): bit 0 = wall east, bit 1 = wall south; visited separately */
static unsigned char walls[MH][MW];
static uint32_t visited[MH];

static uint32_t rng = 2463534242u;
static uint32_t xorshift32(void) {
    rng ^= rng << 13;
    rng ^= rng >> 17;
    rng ^= rng << 5;
    return rng;
}

struct pt { signed char x, y; };

static void carve(void) {
    static struct pt stack[MW * MH];
    int sp = 0;
    for (int y = 0; y < MH; y++) {
        visited[y] = 0;
        for (int x = 0; x < MW; x++) walls[y][x] = 3;
    }
    stack[sp++] = (struct pt){ 0, 0 };
    visited[0] |= 1;
    while (sp) {
        struct pt p = stack[sp - 1];
        static const signed char dx[4] = { 1, 0, -1, 0 }, dy[4] = { 0, 1, 0, -1 };
        int opts[4], n = 0;
        for (int d = 0; d < 4; d++) {
            int nx = p.x + dx[d], ny = p.y + dy[d];
            if (nx < 0 || ny < 0 || nx >= MW || ny >= MH) continue;
            if (visited[ny] >> nx & 1) continue;
            opts[n++] = d;
        }
        if (!n) { sp--; continue; }
        int d = opts[xorshift32() % (unsigned)n];
        int nx = p.x + dx[d], ny = p.y + dy[d];
        switch (d) {
        case 0: walls[p.y][p.x] &= ~1u; break;
        case 1: walls[p.y][p.x] &= ~2u; break;
        case 2: walls[ny][nx] &= ~1u; break;
        case 3: walls[ny][nx] &= ~2u; break;
        }
        visited[ny] |= 1u << nx;
        stack[sp++] = (struct pt){ (signed char)nx, (signed char)ny };
    }
}

static bool can_move(int x, int y, int d) {
    switch (d) {
    case 0: return x + 1 < MW && !(walls[y][x] & 1);
    case 1: return y + 1 < MH && !(walls[y][x] & 2);
    case 2: return x > 0 && !(walls[y][x - 1] & 1);
    case 3: return y > 0 && !(walls[y - 1][x] & 2);
    }
    return false;
}

static signed char from_dir[MH][MW];

/* BFS by frontier bit sets; returns the distance to (MW-1, MH-1) */
static int solve(int *explored) {
    uint32_t seen[MH] = { 0 }, frontier[MH] = { 0 }, nf[MH];
    frontier[0] = seen[0] = 1;
    int dist = 0;
    *explored = 1;
    for (;;) {
        if (frontier[MH - 1] >> (MW - 1) & 1) return dist;
        bool any = false;
        for (int y = 0; y < MH; y++) nf[y] = 0;
        for (int y = 0; y < MH; y++) {
            uint32_t f = frontier[y];
            while (f) {
                int x = ctz64(f);
                f &= f - 1;
                static const signed char dx[4] = { 1, 0, -1, 0 }, dy[4] = { 0, 1, 0, -1 };
                for (int d = 0; d < 4; d++) {
                    if (!can_move(x, y, d)) continue;
                    int nx = x + dx[d], ny = y + dy[d];
                    if (seen[ny] >> nx & 1) continue;
                    seen[ny] |= 1u << nx;
                    nf[ny] |= 1u << nx;
                    from_dir[ny][nx] = (signed char)d;
                    ++*explored;
                    any = true;
                }
            }
        }
        if (!any) return -1;
        for (int y = 0; y < MH; y++) frontier[y] = nf[y];
        dist++;
    }
}

static char text[2 * MH + 1][2 * MW + 2];

static void render(void) {
    for (int y = 0; y < 2 * MH + 1; y++) {
        for (int x = 0; x < 2 * MW + 1; x++) text[y][x] = '#';
        text[y][2 * MW + 1] = 0;
    }
    for (int y = 0; y < MH; y++)
        for (int x = 0; x < MW; x++) {
            text[2 * y + 1][2 * x + 1] = ' ';
            if (!(walls[y][x] & 1) && x + 1 < MW) text[2 * y + 1][2 * x + 2] = ' ';
            if (!(walls[y][x] & 2) && y + 1 < MH) text[2 * y + 2][2 * x + 1] = ' ';
        }
    /* trace the path back from the exit */
    int x = MW - 1, y = MH - 1;
    while (x || y) {
        text[2 * y + 1][2 * x + 1] = '.';
        int d = from_dir[y][x];
        static const signed char dx[4] = { 1, 0, -1, 0 }, dy[4] = { 0, 1, 0, -1 };
        int px = x - dx[d], py = y - dy[d];
        text[y + py + 1][x + px + 1] = '.';
        x = px;
        y = py;
    }
    text[1][1] = '.';
}

static const char *const glider[] = { ".#.", "..#", "###" };
static const char *const rpent[] = { ".##", "##.", ".#." };
static const char *const lwss[] = { ".#..#", "#....", "#...#", "####." };
static const char *const blinker[] = { "###" };

int main(void) {
    place(glider, 3, 1, 1);
    place(lwss, 4, 20, 40);
    place(blinker, 1, 50, 10);
    print_i64(population());
    for (int i = 0; i < 4; i++) life_step();
    print_i64(population());
    /* a glider returns to its shape every 4 steps, moved by (1, 1) */
    {
        row b[W];
        for (int y = 0; y < W; y++) b[y] = 0;
        for (int y = 0; y < W; y++) b[y] = board[y];
        (void)b;
    }
    for (int i = 0; i < 4; i++) life_step();
    print_u64(board_hash());
    for (int y = 0; y < W; y++) board[y] = 0;
    place(rpent, 3, 30, 30);
    int maxpop = 0, gen_of_max = 0;
    for (int g = 1; g <= 300; g++) {
        life_step();
        int p = population();
        if (p > maxpop) { maxpop = p; gen_of_max = g; }
    }
    print_i64(maxpop);
    print_i64(gen_of_max);
    print_i64(population());
    print_u64(board_hash());

    int ones30 = 0, ones110 = 0;
    uint64_t c30 = eca(30, (uint64_t)1 << 32, 100, &ones30);
    uint64_t c110 = eca(110, 1, 100, &ones110);
    print_u64(c30);
    print_i64(ones30);
    print_u64(c110);
    print_i64(ones110);
    print_i64(ctz64(c30) + 100 * ctz64(c110));

    int total = 0;
    for (int m = 0; m < 3; m++) {
        carve();
        int explored;
        int d = solve(&explored);
        total += d;
        print_i64(d * 1000 + explored);
    }
    render();
    for (int y = 0; y < 2 * MH + 1; y += 6) print_str(text[y], 2 * MW + 1);
    uint32_t th = 2166136261u;
    int dots = 0;
    for (int y = 0; y < 2 * MH + 1; y++)
        for (int x = 0; x < 2 * MW + 1; x++) {
            th = (th ^ (unsigned char)text[y][x]) * 16777619u;
            dots += text[y][x] == '.';
        }
    print_i64(dots);
    print_u64(th);
    print_i64(total);
    return 0;
}

/* expect: 17 */
/* expect: 17 */
/* expect: 17404277366306503080 */
/* expect: 221 */
/* expect: 148 */
/* expect: 113 */
/* expect: 4634698990366845527 */
/* expect: 11330108872885433499 */
/* expect: 2729 */
/* expect: 3910817228659150766 */
/* expect: 2502 */
/* expect: 100 */
/* expect: 100128 */
/* expect: 104171 */
/* expect: 130184 */
/* expect: ############################################### */
/* expect: #.# ###.# # ###.# # # # # #.###.######### ### # */
/* expect: #####.###.#########.###.###.#.# ### #######.# # */
/* expect: #.# # ###.# ### ########### # ### #.### ### # # */
/* expect: #.#.#.###.### ##### # ### ### ### #.#######.#.# */
/* expect: ############################################### */
/* expect: 261 */
/* expect: 520588633 */
/* expect: 334 */
