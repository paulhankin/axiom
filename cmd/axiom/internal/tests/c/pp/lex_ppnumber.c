/* pp-numbers take letters, digits, ., and e+ e- (p+ p- from C99): 0x1e+1 is one, and invalid. */
/* flags: --c-std=c99 */
#define s(x) #x
s(0x1e+1) s(1.2.3) s(.5.e+-) s(1p+2) s(1_a) s(1..2) s(0xe+foo)
