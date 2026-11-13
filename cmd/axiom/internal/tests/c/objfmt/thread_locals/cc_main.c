// The platform's compiler's main, linked by the system's linker with this
// compiler's object: its thread-locals through their descriptors.
extern _Thread_local long ours_tl;
long ours_next(void);

int main(void) { return (int)(ours_tl + ours_next() - 71); }
