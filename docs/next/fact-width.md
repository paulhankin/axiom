## Compiler

- Fixed: at `-O1` and above, a bound check could be folded into a trap when the
  index was derived from a narrow value with a known lower bound (for example
  `n = @as(usize, b) - 127` after `b >= 128` for a byte `b`): the optimizer's
  proof wrapped a constant to the narrow type and mistook a fact about the
  smaller value for one about the larger. Found by the standard library's fuzzer
  in the Zstandard decoder's Huffman table reader.
