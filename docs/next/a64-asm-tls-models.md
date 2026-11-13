## C front end

- AArch64 inline and file-scope assembly: every code model's thread-local
  sequences link: the large model's initial exec (`:gottprel_g1:`,
  `:gottprel_g0_nc:`) and TLS descriptor (`:tlsdesc_off_g1:`, `:tlsdesc_off_g0_nc:`,
  `.tlsdescldr`, `.tlsdescadd`), the tiny model's descriptor (a literal `ldr` and an
  `adr` of `:tlsdesc:`) and local dynamic (`:tlsldm:`, `:tlsldm_lo12_nc:`), relaxed by
  the linker as the ABI describes.
- The constraint `k`: the stack pointer, as the operand that is the register variable
  declared `asm("sp")`.
- On macOS and Windows on Arm, an absolute address in the four move-wide groups
  (`movz`/`movk` of `:abs_g3:` ... `:abs_g0_nc:`) assembles and links (as `adrp` and
  `add`: the images are position-independent); an address in a `.word` or `.hword`,
  which such an image cannot hold, is an error at the asm.
