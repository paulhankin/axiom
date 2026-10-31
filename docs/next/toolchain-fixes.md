## Tools

- `axiom update` in a source checkout on a Mac builds the compiler itself again: its
  stages had different file names, which a Mac executable's signature holds, so they
  never came out the same and it fell back to bootstrap.sh.
- The PDB of a Windows program compiled in parts has the line tables of all its
  functions (those of internal functions were missing).
- At `-O2`, some loops whose conditions compare `k - x` with a constant on a loop
  variable made the compiler run out of stack or memory.
