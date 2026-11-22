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

## Standard library (breaking)

Every iterator in std now has one protocol (spec §15.3): `next(self: *mut Self) ?T`,
or `E!?T` when a step can fail, with `.none` at the end. The end is no longer an
error, a sentinel token or a `bool`. A test in the suite keeps it so. What changed:

- `collections/map`: `Map.iter` and `MapDyn.iter` take `*mut Self`, and `Iter.next()`
  returns `?map.Entry.[K, V]`, an entry of pointers into the map (`key: *K`,
  `value: *mut V`; a value may be changed through it), instead of
  `next(&mut k, &mut v) bool`. `Iter.next_key()` is gone: `Map.keys()` (on `*Self`)
  gives a `Keys.[K]` whose `next()` returns `?*K`.
- `collections/set`: `Iter.next()` returns `?*K` (was `next(&mut k) bool`).
- `collections/lru` and `collections/treemap`: `iter` (and `TreeMap.iter_from`) take
  `*mut Self`, and `Iter.next()` returns `?map.Entry.[K, V]` (was
  `next(&mut k, &mut v) bool`).
- `archive/tar`: `Reader.next()` is `TarError!?Entry`; `csv`: `Reader.next()` is
  `CsvError!?[][]u8`; `crypto/der`: `Reader.next()` is `DerError!?Element`, and the new
  `Reader.element()` reads one element whatever it is (`Corrupt` at the end);
  `encoding/xml`: `Reader.next()` is `XmlError!?Token`. Each returned `error.Done`
  (xml: a `TokenKind.END_DOCUMENT` token, now gone; der: `Corrupt`) at the end.
  `Done` has left `TarError`, `CsvError`, `DerError`, `CookieError`, `OsPathError` and
  `os/fs`'s `FsError`.
- `net/cookie`: `Pairs.next()` is `?Pair`; `os/path`: `Elements.next()` is `?[]u8`;
  `encoding/xml`: `Elements.next()` is `?*Element`; `text/html`: `Children.next()` is
  `?u32`; `crypto/x509`: `PemIter.next()` is `X509Error!?PemBlock`. They failed with
  `Done` or `NotFound` at the end.
- `text/html`: `Tokenizer.next()` is `HtmlError!?Token`, `.none` at the end instead of
  an `EOF` token.
- `text/html` and `text/markdown`: the tree walkers' `Walker.next()` is now
  `Walker.advance()` (a cursor read through its fields).
- `time/cron`: `Schedule.next` and `Schedule.prev`, which are not iterators, are now
  `next_utc` and `prev_utc` (like `next_in` and `prev_in`).

To migrate, write the loop with `while let`, and drop the end test:

```axiom
// before
loop {
    let rec = r.next() catch |e| { if e == error.Done { break; } return e; };
    ...
}
let mut k: u64 = 0;
let mut v: u64 = 0;
while it.next(&mut k, &mut v) { ... k ... v ... }
// after
while let .some(rec) = r.next()! {
    ...
}
while let .some(e) = it.next() { ... e.key.* ... e.value.* ... }
```

A map that was iterated through `*Map` is now iterated through `*mut Map`, or with
`keys()` and `get`.
