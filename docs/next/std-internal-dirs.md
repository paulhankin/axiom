## Standard library

- The implementation modules of `graphics/shape`, `audio/opus`, `audio/aac`,
  `graphics/image/avif` (the AV1 decoder), `graphics/image/webp` (VP8) and the HTML
  entity table moved under `internal/` directories (`std/graphics/internal/shape/`,
  `std/audio/internal/opus/`, `std/audio/internal/aac/`,
  `std/graphics/image/internal/av1/`, `std/graphics/image/internal/webp/`,
  `std/text/internal/html/`), which programs cannot import (spec §4.1). A program that
  imported one of them uses the parent module instead.
