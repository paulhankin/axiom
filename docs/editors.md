# Editors

`axiom lsp` is a language server over standard input and output: diagnostics as you
type (the same checks as `axiom check`), hover with types, signatures and `///`
documentation, go to definition and type definition, references, rename, completion,
signature help, document symbols and formatting (`axiom fmt`). It needs the compiler
on the `PATH` (or a path to it) and treats `.ax` files; it checks a file as part of its
package when there is an `axiom.toml` above it.

`axiom lsp` accepts `--target=<t>` (the target the checks assume; `native` is a good
default on a development machine, `wasm32` is the compiler's own default) and
`--std=<dir>` (spec §16.1), which the snippets below pass as arguments.

## Visual Studio Code

The extension in [`tools/editors/vscode/`](../tools/editors/vscode/README.md) gives
syntax highlighting (for `.ax` files and ```` ```axiom ```` blocks in Markdown) and the
language server's features. It is plain JavaScript with no dependencies.

From a release: install the compiler ([`docs/install.md`](install.md)) and the
extension's `.vsix` from the [releases page](https://github.com/paulhankin/axiom/releases)
when one is attached, with `code --install-extension axiom-<version>.vsix`. If `axiom`
is not on the `PATH`, set `axiom.path` to the compiler's location in the settings.

From source, with the compiler built (`cmd/axiom/bootstrap.sh`):

```sh
python3 tools/editors/vscode/package.py
code --install-extension tools/editors/vscode/axiom-0.1.0.vsix
```

In a workspace that holds this repository the extension finds `cmd/axiom/build/axiom`
by itself. Its settings: `axiom.path`, `axiom.serverArgs` (default
`["--target=native"]`), `axiom.enableLanguageServer` and `axiom.trace.server`.

## Neovim

Neovim 0.10 or later needs no plugin: register the file type and start the server for
it in `init.lua`.

```lua
vim.filetype.add({ extension = { ax = "axiom" } })
vim.api.nvim_create_autocmd("FileType", {
  pattern = "axiom",
  callback = function(args)
    vim.lsp.start({
      name = "axiom",
      cmd = { "axiom", "lsp", "--target=native" },
      root_dir = vim.fs.root(args.buf, { "axiom.toml", ".git" }),
    })
  end,
})
```

There is no Tree-sitter grammar yet; `vim.bo.commentstring = "// %s"` in the same
callback makes commenting work.

## Helix

In `~/.config/helix/languages.toml`:

```toml
[language-server.axiom]
command = "axiom"
args = ["lsp", "--target=native"]

[[language]]
name = "axiom"
scope = "source.axiom"
file-types = ["ax"]
roots = ["axiom.toml"]
comment-token = "//"
indent = { tab-width = 4, unit = "    " }
language-servers = ["axiom"]
auto-format = true
```

Helix has no grammar for Axiom, so there is no highlighting beyond what the language
server provides (diagnostics, hover, completion, go to definition, formatting).

## Zed

Zed adds languages through extensions, and there is no Axiom extension yet. Until there
is one, Zed can open `.ax` files as plain text but cannot run the language server for
them.

## Emacs

With Eglot (Emacs 29 or later), in `init.el`:

```elisp
(define-derived-mode axiom-mode prog-mode "Axiom"
  "Major mode for Axiom source files."
  (setq-local comment-start "// " comment-end ""
              indent-tabs-mode nil tab-width 4))
(add-to-list 'auto-mode-alist '("\\.ax\\'" . axiom-mode))
(with-eval-after-load 'eglot
  (add-to-list 'eglot-server-programs
               '(axiom-mode . ("axiom" "lsp" "--target=native"))))
(add-hook 'axiom-mode-hook #'eglot-ensure)
```

## Other editors

Any editor with a Language Server Protocol client works the same way: run `axiom lsp`
for files with the `.ax` extension, with the project root at the directory holding
`axiom.toml`. Formatting is also available from the command line as `axiom fmt <file>`,
and `axiom fmt --stdout <file>` prints the formatted text for editors that format
through a filter.
