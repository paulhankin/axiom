// target.c - architectures, CPU presets and processor extensions. The table
// is std/os/cpu.ax's (names, bits and implications), which the self-hosted
// compiler uses directly; here each extension also has its LLVM name.
#include "target.h"

#include <stdio.h>
#include <string.h>

#include "util.h"

int g_arch = ARCH_WASM32;
int g_os = OS_LINUX;
uint64_t g_features;
const char *g_cpu = "";

typedef struct Feature {
    const char *name;
    int arch;
    uint64_t bit, implies;
    const char *llvm;
} Feature;

#define B(n) ((uint64_t)1 << (n))
enum { X = ARCH_X64, A = ARCH_A64, W = ARCH_WASM32 };
#define AVX512_ALL (B(18) | B(19) | B(20) | B(21) | B(22) | B(23) | B(24) | B(25) | B(26) | B(27) | B(28) | B(29) | B(30))

static const Feature TABLE[] = {
    {"sse3", X, B(0), 0, "sse3"},
    {"ssse3", X, B(1), B(0), "ssse3"},
    {"sse4.1", X, B(2), B(1), "sse4.1"},
    {"sse4.2", X, B(3), B(2), "sse4.2"},
    {"popcnt", X, B(4), 0, "popcnt"},
    {"lzcnt", X, B(5), 0, "lzcnt"},
    {"bmi", X, B(6), 0, "bmi"},
    {"bmi2", X, B(7), 0, "bmi2"},
    {"avx", X, B(8), B(3), "avx"},
    {"avx2", X, B(9), B(8), "avx2"},
    {"fma", X, B(10), B(8), "fma"},
    {"cx16", X, B(11), 0, "cx16"},
    {"aes", X, B(12), 0, "aes"},
    {"pclmul", X, B(13), 0, "pclmul"},
    {"sha", X, B(14), 0, "sha"},
    {"f16c", X, B(15), B(8), "f16c"},
    {"movbe", X, B(16), 0, "movbe"},
    {"adx", X, B(17), 0, "adx"},
    {"avx512f", X, B(18), B(9) | B(10) | B(15), "avx512f"},
    {"avx512cd", X, B(19), B(18), "avx512cd"},
    {"avx512bw", X, B(20), B(18), "avx512bw"},
    {"avx512dq", X, B(21), B(18), "avx512dq"},
    {"avx512vl", X, B(22), B(18), "avx512vl"},
    {"avx512ifma", X, B(23), B(18), "avx512ifma"},
    {"avx512vbmi", X, B(24), B(20), "avx512vbmi"},
    {"avx512vbmi2", X, B(25), B(20), "avx512vbmi2"},
    {"avx512vnni", X, B(26), B(18), "avx512vnni"},
    {"avx512bitalg", X, B(27), B(20), "avx512bitalg"},
    {"avx512vpopcntdq", X, B(28), B(18), "avx512vpopcntdq"},
    {"avx512bf16", X, B(29), B(20), "avx512bf16"},
    {"avx512fp16", X, B(30), B(20) | B(21) | B(22), "avx512fp16"},
    {"gfni", X, B(31), 0, "gfni"},
    {"vaes", X, B(32), B(12) | B(8), "vaes"},
    {"vpclmulqdq", X, B(33), B(13) | B(8), "vpclmulqdq"},
    {"avxvnni", X, B(34), B(9), "avxvnni"},
    {"sha512", X, B(35), B(9), "sha512"},
    {"sm3", X, B(36), B(8), "sm3"},
    {"sm4", X, B(37), B(9), "sm4"},
    {"avx10.1", X, B(38), AVX512_ALL, "avx10.1-512"},
    {"prfchw", X, B(39), 0, "prfchw"},
    {"lse", A, B(0), 0, "lse"},
    {"rcpc", A, B(1), 0, "rcpc"},
    {"dotprod", A, B(2), 0, "dotprod"},
    {"cssc", A, B(3), 0, "cssc"},
    {"aes", A, B(4), 0, "aes"},
    {"sha2", A, B(5), 0, "sha2"},
    {"sha3", A, B(6), B(5), "sha3"},
    {"sha512", A, B(7), B(5), "sha3"},
    {"sm4", A, B(8), 0, "sm4"},
    {"crc", A, B(9), 0, "crc"},
    {"rdm", A, B(10), 0, "rdm"},
    {"fp16", A, B(11), 0, "fullfp16"},
    {"fhm", A, B(12), B(11), "fp16fml"},
    {"bf16", A, B(13), 0, "bf16"},
    {"i8mm", A, B(14), 0, "i8mm"},
    {"sve", A, B(15), B(11), "sve"},
    {"sve2", A, B(16), B(15), "sve2"},
    {"dit", A, B(17), 0, "dit"},
    {"sme", A, B(18), B(13) | B(11), "sme"},
    {"simd128", W, B(0), 0, "simd128"},
    {"bulk-memory", W, B(1), 0, "bulk-memory"},
    {"nontrapping-fptoint", W, B(2), 0, "nontrapping-fptoint"},
    {"sign-ext", W, B(3), 0, "sign-ext"},
    {"mutable-globals", W, B(4), 0, "mutable-globals"},
    {"atomics", W, B(5), 0, "atomics"},
    {"multivalue", W, B(6), 0, "multivalue"},
    {"relaxed-simd", W, B(7), B(0), "relaxed-simd"},
    {"tail-call", W, B(8), 0, "tail-call"},
};
#define NFEAT (int)(sizeof TABLE / sizeof *TABLE)

static void push_str(CharVec *b, const char *s) {
    while (*s) vec_push(*b, *s++);
}

static int family(int a) { return a == ARCH_WASM64 ? ARCH_WASM32 : a; }

uint64_t feature_bit(const char *name) {
    for (int i = 0; i < NFEAT; i++)
        if (TABLE[i].arch == family(g_arch) && !strcmp(TABLE[i].name, name)) return TABLE[i].bit;
    return 0;
}

bool feature_known(const char *name) {
    for (int i = 0; i < NFEAT; i++)
        if (!strcmp(TABLE[i].name, name)) return true;
    return false;
}

const char *feature_names(void) {
    CharVec b = {0};
    for (int i = 0; i < NFEAT; i++) {
        if (TABLE[i].arch != family(g_arch)) continue;
        if (b.len) push_str(&b, ", ");
        push_str(&b, TABLE[i].name);
    }
    vec_push(b, 0);
    return b.data;
}

const char *features_string(uint64_t bits) {
    CharVec b = {0};
    for (int i = 0; i < NFEAT; i++) {
        if (TABLE[i].arch != family(g_arch) || !(bits & TABLE[i].bit)) continue;
        if (b.len) vec_push(b, ',');
        vec_push(b, '+');
        push_str(&b, TABLE[i].name);
    }
    vec_push(b, 0);
    return b.data;
}

uint64_t feature_closure(uint64_t bits) {
    for (;;) {
        uint64_t next = bits;
        for (int i = 0; i < NFEAT; i++)
            if (TABLE[i].arch == family(g_arch) && (bits & TABLE[i].bit)) next |= TABLE[i].implies;
        if (next == bits) return bits;
        bits = next;
    }
}

// The extensions that need `bit`, directly or not (and `bit` itself).
static uint64_t dependents(uint64_t bit) {
    uint64_t d = bit;
    for (int i = 0; i < NFEAT; i++)
        if (TABLE[i].arch == family(g_arch) && (feature_closure(TABLE[i].bit) & bit)) d |= TABLE[i].bit;
    return d;
}

static uint64_t x86_preset(const char *n, bool *ok) {
    uint64_t v2 = B(0) | B(1) | B(2) | B(3) | B(4) | B(11);
    uint64_t v3 = v2 | B(8) | B(9) | B(6) | B(7) | B(10) | B(5) | B(15) | B(16);
    uint64_t v4 = v3 | B(18) | B(19) | B(20) | B(21) | B(22);
    uint64_t haswell = v3 | B(12) | B(13);
    uint64_t skylake = haswell | B(17) | B(39);  // (Broadwell added adx and prfchw)
    uint64_t icelake = skylake | v4 | B(14) | B(23) | B(24) | B(25) | B(26) | B(27) | B(28) | B(31) | B(32) | B(33);
    uint64_t spr = icelake | B(29) | B(30) | B(34) | B(38);
    uint64_t alderlake = skylake | B(14) | B(31) | B(32) | B(33) | B(34);
    uint64_t znver3 = skylake | B(14) | B(32) | B(33);
    uint64_t znver4 = znver3 | v4 | B(23) | B(24) | B(25) | B(26) | B(27) | B(28) | B(29) | B(31);
    *ok = true;
    if (!strcmp(n, "x86-64") || !strcmp(n, "generic")) return 0;
    if (!strcmp(n, "x86-64-v2")) return v2;
    if (!strcmp(n, "x86-64-v3")) return v3;
    if (!strcmp(n, "x86-64-v4")) return v4;
    if (!strcmp(n, "haswell")) return haswell;
    if (!strcmp(n, "skylake")) return skylake;
    if (!strcmp(n, "icelake-server") || !strcmp(n, "icelake-client")) return icelake;
    if (!strcmp(n, "sapphirerapids") || !strcmp(n, "emeraldrapids") || !strcmp(n, "graniterapids")) return spr;
    if (!strcmp(n, "alderlake") || !strcmp(n, "raptorlake") || !strcmp(n, "meteorlake")) return alderlake;
    if (!strcmp(n, "arrowlake") || !strcmp(n, "lunarlake")) return alderlake | B(35) | B(36) | B(37);
    if (!strcmp(n, "znver3")) return znver3;
    if (!strcmp(n, "znver4")) return znver4;
    if (!strcmp(n, "znver5")) return znver4 | B(34);
    *ok = false;
    return 0;
}

static uint64_t a64_preset(const char *n, bool *ok) {
    uint64_t v81 = B(0) | B(10) | B(9);
    uint64_t n1 = v81 | B(2) | B(1) | B(4) | B(5) | B(11);
    uint64_t m1 = n1 | B(6) | B(7) | B(12) | B(17);
    uint64_t v1 = m1 | B(15) | B(13) | B(14);
    uint64_t n2 = n1 | B(15) | B(16) | B(13) | B(14) | B(17) | B(12) | B(6);
    *ok = true;
    if (!strcmp(n, "generic") || !strcmp(n, "armv8-a")) return 0;
    if (!strcmp(n, "armv8.1-a")) return v81;
    if (!strcmp(n, "armv8.2-a")) return v81 | B(2);
    if (!strcmp(n, "armv8.3-a")) return v81 | B(2) | B(1);
    if (!strcmp(n, "neoverse-n1")) return n1;
    if (!strcmp(n, "apple-m1")) return m1;
    if (!strcmp(n, "apple-m2") || !strcmp(n, "apple-m3")) return m1 | B(13) | B(14);
    if (!strcmp(n, "apple-m4")) return m1 | B(13) | B(14) | B(18);
    if (!strcmp(n, "neoverse-v1")) return v1;
    if (!strcmp(n, "neoverse-n2")) return n2;
    if (!strcmp(n, "neoverse-v2") || !strcmp(n, "grace")) return n2 | B(7) | B(8);
    *ok = false;
    return 0;
}

bool cpu_preset(int arch, const char *name, uint64_t *out) {
    bool ok = true;
    if (arch == ARCH_WASM32 || arch == ARCH_WASM64) {
        uint64_t generic = B(0) | B(1) | B(2) | B(3) | B(4) | B(5) | B(6);
        if (!strcmp(name, "mvp")) *out = B(4);
        else if (!strcmp(name, "generic") || !*name) *out = generic;
        else if (!strcmp(name, "bleeding-edge")) *out = generic | B(7) | B(8);
        else ok = false;
        return ok;
    }
    int saved = g_arch;
    g_arch = arch;
    *out = arch == ARCH_X64 ? x86_preset(name, &ok) : a64_preset(name, &ok);
    if (ok) *out = feature_closure(*out);
    g_arch = saved;
    return ok;
}

const char *apply_features(const char *spec) {
    const char *p = spec;
    while (*p) {
        const char *q = strchr(p, ',');
        size_t n = q ? (size_t)(q - p) : strlen(p);
        char item[64];
        snprintf(item, sizeof item, "%.*s", (int)n, p);
        p += n + (q ? 1 : 0);
        if (!*item) continue;
        bool on = true;
        const char *name = item;
        if (*item == '+' || *item == '-') {
            on = *item == '+';
            name++;
        }
        uint64_t bit = feature_bit(name);
        if (!bit) return arena_strndup(name, strlen(name));
        if (on) g_features = feature_closure(g_features | bit);
        else g_features &= ~dependents(bit);
    }
    return NULL;
}

const char *llvm_features(uint64_t bits) {
    CharVec b = {0};
    for (int i = 0; i < NFEAT; i++) {
        if (TABLE[i].arch != family(g_arch) || !(bits & TABLE[i].bit)) continue;
        if (b.len) vec_push(b, ',');
        vec_push(b, '+');
        push_str(&b, TABLE[i].llvm);
        // LLVM 18 split CRC32 (which @x86_crc32 lowers to) off SSE 4.2 into a feature of its
        // own, which a named --cpu does not otherwise turn on
        if (family(g_arch) == ARCH_X64 && !strcmp(TABLE[i].llvm, "sse4.2")) push_str(&b, ",+crc32");
    }
    vec_push(b, 0);
    return b.data;
}

uint64_t features_from_llvm(const char *s) {
    uint64_t bits = 0;
    if (!s) return 0;
    for (int i = 0; i < NFEAT; i++) {
        if (TABLE[i].arch != family(g_arch)) continue;
        char want[48];
        size_t n = (size_t)snprintf(want, sizeof want, "+%s", TABLE[i].llvm);
        for (const char *p = strstr(s, want); p; p = strstr(p + 1, want)) {
            if ((p == s || p[-1] == ',') && (p[n] == ',' || p[n] == 0)) {
                bits |= TABLE[i].bit;
                break;
            }
        }
    }
    // (LLVM's sha3 on AArch64 is SHA3 and SHA512 together)
    if (g_arch == ARCH_A64 && (bits & B(6))) bits |= B(7);
    return bits;
}

uint64_t parse_feature_set(const char *spec, bool *foreign) {
    uint64_t bits = 0;
    *foreign = false;
    const char *p = spec;
    while (*p) {
        const char *q = strchr(p, ',');
        size_t n = q ? (size_t)(q - p) : strlen(p);
        char item[64];
        snprintf(item, sizeof item, "%.*s", (int)n, p);
        p += n + (q ? 1 : 0);
        const char *name = item;
        while (*name == ' ') name++;
        if (*name == '+') name++;
        if (!*name) continue;
        uint64_t b = feature_bit(name);
        if (!b && feature_known(name)) *foreign = true;
        bits |= b;
    }
    return feature_closure(bits);
}
