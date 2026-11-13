// Functions of DLLs, declared as imports.
__declspec(dllimport) const char *__stdcall PathFindFileNameA(const char *);
__declspec(dllimport) int __stdcall lstrlenA(const char *);

const char *base(const char *p) { return PathFindFileNameA(p); }

int len(const char *p) { return lstrlenA(p); }
