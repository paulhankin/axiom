# 0008: Missing TeX commands in std/text/math

- Type: feature
- Status: open
- Area: std/text/math (and std/text/math/typeset)

Commands people use that the parser rejects ("unknown command"):

- extensible arrows with labels: `\xrightarrow{f}`, `\xleftarrow[g]{f}`,
  `\xleftrightarrow`, `\xmapsto`;
- `\substack{i < n \\ j < m}` (stacked limits under `\sum`);
- `\sideset{_a^b}{_c^d}\sum`, `\prescript`;
- `\overunderset`, `\underbracket`, `\overbracket`;
- `\operatorname*` exists, but not `\DeclareMathOperator`-style macros or any
  `\newcommand`;
- `\tag{n}` / `\notag` in display math;
- the `alignat`, `multline`, `CD` environments; `array` takes but ignores
  its column specification (no `|` rules, no per-column alignment).

Each needs MathML Core output, text output, and layout in
`text/math/typeset` (the stretchy arrows use the font's horizontal glyph
variants, as `\overrightarrow` does).
