## Standard library

- `fs.remove_all` and `fs.walk` no longer fail on trees deeper than the process's
  descriptor limit: from 32 levels down they close a directory's handle while its
  subtree is visited and reopen it through the child's `..` afterwards, checking
  (device and inode, through the new host ABI function `ax_dir_stat`) that it is the
  same directory. A hostile tree can no longer stop a cleanup partway.
