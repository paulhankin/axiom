# Installing Axiom

Axiom is one program, `axiom`, plus the standard library's sources, which the compiler
reads next to its own binary. Releases are archives of the two, one per platform, on
the [releases page](https://github.com/paulhankin/axiom/releases). Axiom is
experimental software with no compatibility promise between releases; see the
disclaimer in the [README](../README.md#disclaimer).

## From a release

Download the archive for your platform and its `SHA256SUMS`:

| Platform | Archive |
|---|---|
| Linux, x86-64 | `axiom-<version>-linux-x86_64.tar.gz` |
| Linux, AArch64 | `axiom-<version>-linux-aarch64.tar.gz` |
| macOS, Apple silicon | `axiom-<version>-macos-aarch64.tar.gz` |
| Windows, x86-64 | `axiom-<version>-windows-x86_64.zip` |

Check and unpack it, then put its `bin/` on the `PATH`. On Linux or macOS:

```sh
sha256sum -c --ignore-missing SHA256SUMS      # shasum -a 256 -c on macOS
tar -xzf axiom-0.1.0-linux-x86_64.tar.gz
export PATH="$PWD/axiom-0.1.0-linux-x86_64/bin:$PATH"
axiom version
```

On Windows, unzip the archive and add its `bin` folder to the `Path` environment
variable, then run `axiom version` in a new terminal.

The archive unpacks to one directory:

```
axiom-<version>-<os>-<arch>/
  bin/axiom              the compiler (axiom.exe on Windows)
  std/                   the standard library's sources, which the compiler reads
  LICENSE-MIT  LICENSE-APACHE  THIRD-PARTY-NOTICES.md  RELEASE-NOTES.md  README.txt
```

Keep `bin/` and `std/` together: the compiler looks for `std/` next to `bin/` (or at
`../share/axiom/std`, so `bin/axiom` and `share/axiom/std` under one prefix work too).
To put the library elsewhere, pass `--std=<dir>` or set `AXIOM_STD=<dir>`.

Notes by platform:

- **Linux.** The binary is static and needs nothing else. Programs you build for
  `--target=native` are static too, unless they link C code.
- **macOS.** The binary is signed ad hoc, not notarized. If you downloaded the archive
  with a browser, macOS may refuse to run it until you clear the quarantine flag:
  `xattr -dr com.apple.quarantine axiom-0.1.0-macos-aarch64`. Downloads with `curl`
  carry no such flag.
- **Windows.** SmartScreen may warn about an unrecognised program the first time; the
  checksum in `SHA256SUMS` is the way to verify what you downloaded. Programs you build
  need no C runtime.

To run WebAssembly output you also need [Node.js](https://nodejs.org) 18 or later:
`node <std>/internal/runtime/host.mjs program.wasm`.

## Updating

`axiom update` replaces the toolchain with another version:

```sh
axiom update            # the latest release
axiom update 0.2.0      # a given release (newer or older)
axiom update head       # the head of main on GitHub (a source checkout only)
```

For an install from a release archive, it downloads the archive for this platform from
the release with `curl`, checks it against the release's `SHA256SUMS`, unpacks it with
`tar` (which macOS and Windows 10 and later include), checks that its compiler
runs, and then replaces `bin/axiom`, `std/` and the other files of the archive, where
they are: the `PATH` stays as it was, though the directory keeps its old name
(`axiom-0.1.0-linux-x86_64`, say). If a step fails, the old toolchain stays. It needs
to be able to write to the directory. A release install cannot update to `head`, which
is not a release: for that, build from source.

For a source checkout (below), it fetches the release's tag, or for `head` (the
default) the head of `main`, from GitHub into the checkout, and moves the checkout to it:
a tag is checked out detached, and `head` fast-forwards `main` (checking it out if the
checkout is at a tag). It refuses a checkout with uncommitted changes, or on another
branch than `main`. Then it builds the compiler, to `cmd/axiom/build/axiom`: with the
compiler it replaces, and then with the result, which must build itself to the same
bytes (the stages of `bootstrap.sh`, without LLVM, in under a minute); if the old
compiler cannot build the new sources, it runs `cmd/axiom/bootstrap.sh`.

## A first program

```sh
mkdir hello && cd hello
cat > hello.ax <<'EOF'
import "std/io" as io;

export fn main() i32 {
    io.stdout().write_all("hello, world\n") catch return 1;
    return 0;
}
EOF
axiom build hello.ax --target=native -o hello && ./hello
axiom build hello.ax -o hello.wasm && node "$(dirname "$(command -v axiom)")/../std/internal/runtime/host.mjs" hello.wasm
```

`axiom new <name>` creates a package (a directory with an `axiom.toml` and `src/lib.ax`
with a test), and `axiom test <name>` runs its tests. The commands and their flags are
listed by `axiom` with no arguments; the language is described in
[`docs/spec.md`](spec.md), and [`examples/`](../examples) has complete programs.

## From source

Building the compiler needs the bootstrap compiler, which is written in C and uses
LLVM 18: on Ubuntu 24.04, `apt install llvm-18-dev lld clang nodejs`; on a Mac,
`brew install llvm@18 node` with `llvm-config` on the `PATH`. Then:

```sh
git clone https://github.com/paulhankin/axiom.git
cd axiom
cmd/axiom/bootstrap.sh                 # about a minute: builds the compiler, three stages, to a fixed point
export PATH="$PWD/cmd/axiom/build:$PATH"
```

`bootstrap.sh` builds a compiler for the generic processor of this machine's
architecture, so the binary runs on any machine of that kind; `TARGET=native
cmd/axiom/bootstrap.sh` builds one tuned to this processor's extensions instead, which
only runs where they exist. The compiler's own README,
[`cmd/axiom/README.md`](../cmd/axiom/README.md), describes it, and
[`docs/releasing.md`](releasing.md) describes how releases are made.
