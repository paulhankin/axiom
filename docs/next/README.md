# Notes for the next release

Each change that a release's users should hear about gets a file here when it lands: a
new feature, a change that breaks programs (say how to update them), or a notable fix.
Name the file for the change (`std-audio-formats-add.md`), and write a short Markdown
paragraph or list starting with a `##` heading for its area, for example:

```markdown
## Standard library

- `lib/audio/formats` reads AAC only once a program adds `aac.DECODER` with
  `formats.add`.
```

A note about the C front end, which is an experiment (`docs/c.md`), is named
`c-<change>.md`: the release notes give those a section of their own.

`cmd/axiom/release.sh notes <version>` gathers these files into
`docs/releases/<version>.md` and removes them (see `docs/releasing.md`).
