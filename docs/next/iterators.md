## Language

- `while let P = e { ... }` runs its block while `e` matches the patterns `P`, and
  evaluates `e` again before each iteration (spec §8.4). It mirrors `if let`; `break`,
  `continue` and labels apply to the loop. It is the loop for an iterator:
  `while let .some(line) = lines.next() { ... }`, or `while let .some(ev) =
  events.next()! { ... }` for one that can fail.
