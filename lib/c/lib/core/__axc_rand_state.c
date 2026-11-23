/* __axc_rand_state: the state of rand's generator (rand.c), 1 until srand
   sets it (C23 7.24.2.2p2). */
#include "axc/core_impl.h"

unsigned long long __axc_rand_state = 1;
