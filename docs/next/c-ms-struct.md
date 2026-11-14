## C

- In the GNU dialects on x86-64 targets, the attributes `ms_struct` and `gcc_struct` choose
  a structure's or union's layout of bit-fields: the Microsoft one (as on Windows x64) or
  the System V one; `#pragma ms_struct on`, `off` and `reset` give `ms_struct` to the
  structures defined between them. Before, the attributes were unknown and ignored, so such
  structures had the target's own layout.
- Fixed: an address in the initializer of a static object at an offset its type does not
  align (a member of a packed structure, or of one under `#pragma pack`) was stored at the
  next aligned offset, so the member read a wrong pointer.
- Fixed: the compiler crashed on the address of a function declared without a prototype
  that the unit neither defines nor has called before (such as a weak reference tested
  for null: `extern int f() __attribute__((weak)); int (*p)() = f;`).
