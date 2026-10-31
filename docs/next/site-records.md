## Compiler

- The call-site records behind stack traces are smaller, and so are programs built
  with stack traces (the compiler itself by 5%): a record keeps its strings (the file,
  the code at the site, the function and the called one) as offsets into one table of
  strings per compilation unit, instead of pointers to strings of their own. The
  compiler builds itself in less memory (552 MB at most at `-O2`, from 584 MB) and
  faster. Tools that read site records from a program's memory read the new layout
  (`std/internal/runtime/trap_format.ax`).
