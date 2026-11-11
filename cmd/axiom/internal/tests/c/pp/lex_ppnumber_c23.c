/* C23: ' between digits and letters is part of a pp-number. */
/* flags: --c-std=c23 */
#define s(x) #x
s(1'2'3) s(0x'1) s(1'a'b) 1'000'000 0b1010'0101 0x1'0p1'0
