_Pragma("pack(push, 1)") struct s { char c; int i; };
_Pragma("pack(pop)")
int main(void) { return sizeof(struct s); }
