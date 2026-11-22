## Language

- `while let P = e { ... }` runs its block while `e` matches the patterns `P`, and
  evaluates `e` again before each iteration (spec §8.4). It mirrors `if let`; `break`,
  `continue` and labels apply to the loop. It is the loop for an iterator:
  `while let .some(line) = lines.next() { ... }`, or `while let .some(ev) =
  events.next()! { ... }` for one that can fail.

## Diagnostics

- `for x in it` over an iterator (a value with `next(self: *mut Self) ?T` or `E!?T`)
  is still `E0412`, now with a fix that writes `while let .some(x) = it.next() { ... }`:
  `machine` when `it` is a mutable variable or a pointer to the iterator, a suggestion
  that first declares `let mut it = ...;` when it is a temporary such as
  `str.lines(s)`, or when `next` can fail (spec §8.5).
