/* flags: --c-std=c17 --target=x86_64-windows */
/* check-only */
/* __declspec under an ISO standard when the target is Windows (whose
   headers use it): the same modifiers as in a GNU dialect, on objects,
   structure tags and members, functions, and thread. */
__declspec(align(16)) int x;
struct __declspec(align(32)) T { char c; };
struct S { int a; __declspec(align(64)) int m; };
__declspec(dllexport) int f(void) { return 1; }
__declspec(dllimport) extern int imported;
__declspec(thread) int t;
__declspec(noreturn) void die(void);
__declspec(selectany) int chosen = 3;
_Static_assert(_Alignof(struct T) == 32, "a structure's");
_Static_assert(_Alignof(struct S) == 64, "a member's");
_Static_assert(__builtin_offsetof(struct S, m) == 64, "a member's offset");
