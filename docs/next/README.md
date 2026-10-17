# Notes for the next release

Each change that a release's users should hear about gets a file here when it lands: a
new feature, a change that breaks programs (say how to update them), or a notable fix.
Name the file for the change (`std-audio-formats-add.md`), and write a short Markdown
paragraph or list starting with a `##` heading for its area, for example:

```markdown
## Standard library

- `std/audio/formats` reads AAC only once a program adds `aac.DECODER` with
  `formats.add`.
```

`cmd/axiom/release.sh notes <version>` gathers these files into
`docs/releases/<version>.md` and removes them (see `docs/releasing.md`).
