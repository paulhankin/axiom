## C compiler

- An asm statement with memory operands and no `"memory"` clobber now reads and
  writes exactly the objects of its operands, as the GNU dialect's rule says, instead
  of all memory: values of other objects stay in registers across it, and stores to
  them are not forced out.
