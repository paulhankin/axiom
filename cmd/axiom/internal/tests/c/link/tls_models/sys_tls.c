/* thread-locals of every kind: own (initialized, zero, static), the
   program's (extern) */
__thread int own = 3;
static __thread int priv[4] = {1, 2, 3, 4};
__thread int zero;
extern __thread int ours;
int tls_sum(void) { return own + priv[2] + zero + ours; }
int tls_bump(int by) { priv[1] += by; zero += by; return priv[1] * 10 + zero; }
int *tls_addr(void) { return &ours; }
