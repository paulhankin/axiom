/* flags: --c-std=gnu17 */
/* expect-error: C3020 */
/* An integer complex value has no remainder, bitwise or ordering operators,
   as a floating complex value has none. */
int main(void) {
    _Complex int a = 1 + 2i;
    _Complex int b = a % 2;
    return b < a;
}
