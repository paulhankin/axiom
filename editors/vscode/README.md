# Axiom for Visual Studio Code

Syntax highlighting for Axiom (`.ax` files, and ```` ```axiom ```` blocks in Markdown), and
a client for the Axiom language server, `axiom lsp`, which gives:

- diagnostics as you type (the same checks as `axiom check`)
- hover: types, signatures and `///` documentation
- go to definition, go to type definition, find references, highlight uses
- rename (across the files of the package, or of the directory)
- completion of names in scope, members after `x.`, module items after `vec.`, error
  names after `error.` and builtins after `@`
- signature help in calls
- the outline (document symbols)
- format document (`axiom fmt`)

## Installing

Build the compiler, which includes the language server, then package and install the
extension:

```
axiom build selfhost/src/main.ax --target=native -O2 -o selfhost/build/axiom      # axiom.exe on Windows
python editors/vscode/package.py
code --install-extension editors/vscode/axiom-0.1.0.vsix
```

The extension needs no Node.js or npm: it is plain JavaScript with no dependencies.

## Settings

| Setting | Default | |
|---|---|---|
| `axiom.path` | `""` | The `axiom` compiler. Empty: `selfhost/build/axiom` in a workspace folder, else `axiom` on the `PATH`. |
| `axiom.serverArgs` | `["--target=native"]` | Options for every check, e.g. `--target=wasm32` or `--std=<dir>`. |
| `axiom.enableLanguageServer` | `true` | Off: highlighting only. |
| `axiom.trace.server` | `false` | Log the protocol messages in the Axiom output channel. |

Commands: **Axiom: Restart Language Server** and **Axiom: Show Language Server Output**.

## How it works

`axiom lsp` (`selfhost/src/lsp.ax`) speaks the Language Server Protocol on standard input
and output, so other editors (Neovim, Helix, Zed, Emacs, Sublime) can use it too: run
`axiom lsp` for `.ax` files. It keeps the open documents and answers each request by
running `axiom ide` (`selfhost/src/ide.ax`) with their unsaved text on standard input:
that loads and checks the program as `axiom check` does, then answers one question from
the checked tree.
