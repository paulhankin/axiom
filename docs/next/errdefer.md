## Language

- `errdefer S` runs `S` only when the function returns an error from the enclosing
  block (`e!`, `catch return`, or a `return` of an error value), for undoing partial
  work when a function fails partway (spec §8.8). `errdefer` is a keyword.
