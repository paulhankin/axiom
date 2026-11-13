## C front end

- AArch64 inline and file-scope assembly on macOS: the Mach-O thread-local
  descriptor references `sym@TLVPPAGE` and `sym@TLVPPAGEOFF` (an `adrp`/`ldr` pair that
  loads a thread-local's descriptor address through a GOT slot, as the platform
  toolchain addresses one). The assembler makes ARM64_RELOC_TLVP_LOAD_PAGE21 and
  PAGEOFF12; the compiler's Mach-O linker gives each a GOT slot of the symbol's
  descriptor address. On targets other than macOS they are an error (`:tlsdesc:` and
  `:gottprel:` are the thread-local operators there).
