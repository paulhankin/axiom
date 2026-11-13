## C front end

- The AArch64 inline and file-scope assembler has the ABI's relocation operators:
  `:pg_hi21:`, `:pg_hi21_nc:`, the move-wide groups (`:abs_g0:`-`:abs_g3:`, `_nc`, `_s`,
  `:prel_g*:`), the thread-local ones (`:tprel_*:`, `:dtprel_*:`, `:gottprel:`,
  `:gottprel_lo12:`, `:tlsdesc:`, `:tlsdesc_lo12:`, `.tlsdesccall`), the Mach-O
  spellings (`sym@PAGE`, `@PAGEOFF`, `@GOTPAGE`, `@GOTPAGEOFF`) and COFF's
  `:secrel_lo12:`/`:secrel_hi12:`; and `adr`, `b.cond`, `cbz`, `cbnz`, `tbz`, `tbnz`,
  `prfm` and literal loads to symbols of other sections and units. The linkers apply
  them with the ABI's range checks, and relax initial-exec and TLS-descriptor
  sequences of an executable's own thread-locals to local exec.
