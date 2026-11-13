int so_counter = 41;
int so_table[3] = {7, 8, 9};
__thread int so_tls = 5;
int so_get(void) { return ++so_counter; }
int so_tls_get(void) { return so_tls; }
