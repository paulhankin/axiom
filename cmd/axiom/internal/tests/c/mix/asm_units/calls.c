/* C's side: the assembly routine and table (asm_primes, in table.s). */
long long asm_sum(const long long *p, long long n);
extern const long long asm_primes[];
extern const unsigned char asm_greeting[];

long long c_checks(void) {
    return asm_sum(asm_primes, 6) + (asm_greeting[0] == 'h');
}
