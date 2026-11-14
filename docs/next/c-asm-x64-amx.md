## C front end

- The x86-64 assembler encodes AMX: the tile registers `tmm0`-`tmm7`, the tile
  configuration (`ldtilecfg`, `sttilecfg`, `tilerelease`), `tilezero`, the tile loads
  and stores (`tileloadd`, `tileloaddt1`, `tilestored`) and the dot products of
  AMX-BF16, AMX-INT8, AMX-FP16 and AMX-COMPLEX, in both syntaxes. Inline assembly
  takes `tmm0`-`tmm7` as clobbers.
