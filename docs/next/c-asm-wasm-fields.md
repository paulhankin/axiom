## C: module fields in WebAssembly assembly

- File-scope asm and assembly sources (`.s`, `.S`) on WebAssembly take every module
  field the program's single memory and function table allow: globals (defined with a
  constant or `global.get` of an imported one, imported from another unit or the host,
  exported), exception tags (`throw $e`, `try_table`'s `catch $e`/`catch_ref $e`, the
  legacy `catch $e`), passive data segments (`memory.init`, `data.drop`), passive and
  declarative element segments (`table.init`, `elem.drop`; `table.copy`), exports of
  globals and tags, a start function, and named function types (`(type $t)`). asm
  statements in C see the names file-scope asm declares. Every instruction the
  `--features` extensions can enable now assembles and is validated. Active data and
  element segments are rejected by rule (the program owns the memory's and the table's
  layout). See docs/c.md, "Inline assembly".
- The WebAssembly linker merges globals and tags across units by name, checks each
  declaration's type against its definition, keeps passive segments apart from the
  memory's image, and emits global and tag imports and exports.
