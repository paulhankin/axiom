# 0010: Better typesetting with fonts that lack a MATH table

- Type: feature
- Status: open
- Area: std/text/math/typeset

With a font that has no OpenType MATH table (Liberation Serif, say), the
typesetter uses STIX Two Math's constants scaled to the font, and stretches
delimiters by scaling the glyph vertically. Results are readable but rough:

- big operators (`\sum`, `\int`) in display style are only mildly enlarged,
  and integrals look thin when scaled;
- the radical sign is scaled as a whole, so its stroke thickens and the
  overbar does not meet it cleanly;
- `\overbrace`, wide accents and arrows are scaled horizontally, distorting
  the stroke.

Possible improvements: build radicals, braces and tall delimiters from the
Unicode pieces many text fonts have (U+239B–U+23B3: bracket and brace
pieces, U+23B7 radical bottom) the way a glyph assembly does; draw the
radical's overbar and vertical part as rules; take the axis height and rule
thickness from the font's minus sign rather than from STIX's constants.
