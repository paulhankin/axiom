## Security fixes

- At `-O2`, the compiler took the popcount of a 64-bit value with no known bound to
  be at most 63, and dropped the bounds check of an index made from it.
- A git dependency's `rev` must be a full commit hash (40 or 64 lower-case hex digits),
  and its URL cannot start with `-`. A revision such as `:/text`, which picks a commit by
  its message, could hold `..` and put the checkout outside the cache.
- The search for a file's `axiom.toml` stops at a directory anyone may write to, such
  as `/tmp`, so another user cannot plant a manifest (and its git dependencies) above
  your files.
- `axiom test` and `axiom profile` build their program in a new private directory,
  not at a predictable name in `/tmp` that another user could take over. With no
  `HOME`, the cache is a private directory for the run instead of `/tmp/.cache/axiom`.
- `std/crypto/argon2` bounds the number of passes as well as the memory: parameters
  asking for more than `MAX_WORK_KIB` (16 GiB, passes times memory) fail with
  `error.TooExpensive`, so a hash with `t=4294967295` cannot make `verify_encoded` run
  for hours.
- `std/crypto/rsa`: PKCS #1 v1.5 signatures with SHA-1 used SHA-512's DigestInfo, so
  valid SHA-1 signatures were refused and the ones made were malformed.
- `std/crypto/x509`:
  - A wildcard `*.S` under a CA with permitted DNS subtrees was judged by the one name
    `a.S`, so a CA permitted only `a.example.com` could issue `*.example.com`. A
    wildcard is now permitted only if every name it matches is.
  - A CA whose name constraints include a kind std does not enforce (directoryName,
    rfc822Name, URI and the rest, or a minimum or maximum) was treated as if they were
    absent. A path through such a CA is now refused with `error.NameConstraint`.
  - A duplicated extension after 32 others was missed, so a second basicConstraints
    could override the first. Duplicates are refused wherever they are.
  - KeyUsage honours its bit string's unused-bits count: padding bits no longer count
    as usages, and non-DER padding is refused.
- `std/crypto/chacha20poly1305.xor_stream` traps instead of letting its 32-bit block
  counter wrap to block 0, which would reuse the keystream. `seal` and `open` already
  refused messages that long.
- On Windows, `process.run` and `Command.spawn` refuse to run a batch file (`.bat` or
  `.cmd`) with `InvalidArgument`: cmd.exe, which runs it, would read an argument such
  as `x&calc` as a command. Run `cmd.exe` explicitly for a script.
- On Windows, a program named without a path that is not on the child's `PATH` gives
  `NotFound`; it was also looked for in the current directory, where a program planted
  there could run instead.
- The http router refuses `\` and `:` in route parameters, as it does `/`, `..` and NUL,
  so `/files/{name}` cannot reach outside its directory on Windows.
- `os.unsetenv` removes every entry for a variable when the environment the program
  started with held several (it removed only the first, so a child could still get the
  variable), and `os.setenv` leaves exactly one.
- `term.set_title` also leaves out C1 control characters (U+0080 to U+009F) and invalid
  UTF-8, so a title cannot end its escape sequence early and start others.
- `fs.copy_file` gives a new file the source's permission bits (less the umask) instead
  of 0644, so a copy of a private file is private too.
- `fs.map` takes the size and kind of the file from the file it opened, not from an
  earlier `stat` of the path.
- On macOS, a child process inherits only its standard streams, not other descriptors
  open in the parent (such as a pipe another thread had just made).
