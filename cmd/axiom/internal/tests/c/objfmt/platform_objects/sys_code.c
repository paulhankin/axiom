// A jump table, and a frame of several pages (the stack probes it calls).
int sys_switch(int x) {
    switch (x) {
    case 0: return 11;
    case 1: return 22;
    case 2: return 7;
    case 3: return 99;
    case 4: return 5;
    case 5: return 77;
    default: return -1;
    }
}

int big_frame(int n) {
    volatile char buf[20000];
    for (int i = 0; i < n; i++) {
        buf[i] = (char)i;
    }
    return n > 0 ? (unsigned char)buf[n - 1] + n - 255 : 0;
}

__attribute__((common)) int common_one;
