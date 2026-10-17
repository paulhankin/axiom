# 0017: An import of an absolute Windows path is taken as relative

- Type: bug
- Status: fixed (absolute import paths are an error, E0218)
- Area: compiler (selfhost): `sema.ax`

## What happens

On Windows, a file that imports a module by an absolute path with a drive,

```
import "C:/Users/me/axiom/std/os/fs.ax" as fs;
```

fails to compile: the path is joined to the importing file's directory.

```
error[E0211]: cannot find module 'C:/Users/me/scratch/C:/Users/me/axiom/std/os/fs.ax'
```

`C:\...` fails the same way. Found running std tests natively on Windows 11.

## Cause

`load_module` in cmd/axiom/src/sema.ax treats an import path as absolute
only if it starts with '/':

```
} else if (ip.len > 0) and (ip[0] == '/') {
    path = u.cat2(ip, ext);
} else {
    path = u.f3("%s/%s%s", dir, ip, ext);
}
```

## Fix

Also take a path with a drive as absolute on Windows. util.ax already has
`drive_len` (private) for `real_path`; make it public, or add a
`u.is_abs_path`, and use it here. Check the C compiler too, if it resolves
imports the same way. A test belongs with the Windows tests (an import by an
absolute path built from the test file's own directory).

## Resolution

Import paths are now relative only, with '/' between their parts: a path
starting with '/', a drive ("C:") or containing '\' is `E0218` in both
compilers (spec §4.1), so an import means the same on every host.
tests/errors/import_absolute.ax, import_drive.ax and import_backslash.ax.
