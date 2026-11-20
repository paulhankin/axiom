/* Integer bit manipulation: a bitwise CRC-32, bit reversal, popcount by hand, xorshift, rotations. */
#include <stdio.h>
#include <stdint.h>
static uint32_t crc32_bitwise(const uint8_t *p, int n, uint32_t crc) {
    crc = ~crc;
    for (int i = 0; i < n; i++) {
        crc ^= p[i];
        for (int k = 0; k < 8; k++) crc = (crc >> 1) ^ (0xEDB88320u & -(crc & 1));
    }
    return ~crc;
}
static uint32_t rev32(uint32_t x) {
    x = ((x >> 1) & 0x55555555u) | ((x & 0x55555555u) << 1);
    x = ((x >> 2) & 0x33333333u) | ((x & 0x33333333u) << 2);
    x = ((x >> 4) & 0x0F0F0F0Fu) | ((x & 0x0F0F0F0Fu) << 4);
    x = ((x >> 8) & 0x00FF00FFu) | ((x & 0x00FF00FFu) << 8);
    return (x >> 16) | (x << 16);
}
static int pop(uint64_t x) { int c = 0; while (x) { x &= x - 1; c++; } return c; }
static uint64_t rotl(uint64_t x, int k) { return (x << k) | (x >> (64 - k)); }
int main(void) {
    uint8_t buf[4096];
    for (int i = 0; i < 4096; i++) buf[i] = (uint8_t)(i * 7 + (i >> 3));
    uint32_t crc = 0;
    for (int r = 0; r < 300; r++) crc = crc32_bitwise(buf, 4096, crc);
    uint64_t s0 = 1, s1 = 2, acc = 0;
    for (int i = 0; i < 20000000; i++) {
        uint64_t a = s0, b = s1;
        uint64_t res = rotl(a + b, 17) + a;
        b ^= a; s0 = rotl(a, 49) ^ b ^ (b << 21); s1 = rotl(b, 28);
        acc += rev32((uint32_t)res) ^ (uint64_t)pop(res & 0xffff0000ffffULL);
    }
    printf("bits: %u %llu\n", crc, (unsigned long long)acc);
    return 0;
}
