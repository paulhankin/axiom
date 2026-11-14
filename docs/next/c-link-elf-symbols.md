## Linking (ELF)

- `visibility("protected")` is written as `STV_PROTECTED` to objects and to shared
  libraries' dynamic symbols. A shared library's definitions of default visibility are
  now preemptible, as the ELF rules have them: the library reaches them through its GOT
  and PLT, so that a program's definition of the same name, or its copy of the library's
  object, is the one the library uses too; protected and hidden ones bind within the
  library. A program exports its definitions that the libraries it links refer to or
  define, and the address of a library's function that position-dependent code takes is
  one address in every module. Position-dependent code that would need a copy of a
  library's protected object, or the address of its protected function, is an error.
- Executables and shared libraries have `.eh_frame_hdr` and `PT_GNU_EH_FRAME`, through
  which the platform's unwinder finds the call frame information of the objects linked.
- Indirect functions: the attribute `ifunc("resolver")` in the GNU dialects, and
  `STT_GNU_IFUNC` symbols of the objects linked (global and local, x86-64 and AArch64),
  resolved through `R_*_IRELATIVE` relocations that a static program's start-up applies,
  and the dynamic linker otherwise.
