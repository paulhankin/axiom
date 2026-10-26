## Compiler

- A Linux program can name macOS frameworks and Windows DLLs in extern blocks, as
  macOS and Windows programs already could name each other's: the linker binds their
  functions to the runtime's `ax_unavailable`, which crashes with a stack trace if one
  is called. A program that picks its system at run time (a window through
  `std/os/window`, sound through `std/audio/output`) now builds for Linux too.
