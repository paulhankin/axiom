/* flags: --c-std=c23 */
/* C23 constants: binary, digit separators, the wb suffix, u8 characters. */
int a = 0b1010;
int b = 1'000'000;
unsigned _BitInt(8) c = 3uwb;
_BitInt(9) e = 5wb;
unsigned char d = u8'x';
