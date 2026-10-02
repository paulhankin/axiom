# 0009: GitHub's `` $`...`$ `` math syntax in Markdown

- Type: feature
- Status: open
- Area: std/text/markdown

GitHub accepts inline math written as `` $`a^2 + b^2`$ `` as well as
`$a^2 + b^2$`; the backtick form avoids the dollar rules' ambiguities
(prices, `$` inside code). With `Options.math` on, std/text/markdown reads
`` $`x^2`$ `` as `$...$` math whose source is `` `x^2` ``, so the backticks
end up in the formula as stray operators:

```
<math>...<mo>`</mo><msup><mi>x</mi><mn>2</mn></msup><mo>`</mo>...</math>
```

Recognize `` $` `` ... `` `$ `` (backticks as in a code span: the same number
on both sides) as MATH whose source is what lies between them.
