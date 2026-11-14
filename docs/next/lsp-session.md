## Editors

- `axiom lsp` answers in its own process instead of running `axiom ide` for each
  request, and checks the standard library and the files not being edited once per
  session: after the first check of a program, hover, completion, diagnostics and
  the other requests take milliseconds (a program importing twenty large `std/`
  modules: about 3 ms, from 400 ms; the compiler's own `main.ax`: 10 ms, from
  760 ms). Files changed on disk are checked again. The answers are those of
  `axiom ide`, as before.
