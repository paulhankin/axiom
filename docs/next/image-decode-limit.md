## Standard library

- The image decoders (`std/graphics/image/formats` and each format's `decode`,
  `decode_all`, `decode_page`, `load`, ...) now refuse an image of more than
  `image.DEFAULT_MAX_PIXELS` pixels (2^26, 64 Mi: 8192 x 8192) with `TooLarge`, before
  allocating anything for it; the limit was `image.MAX_PIXELS` (2^28). A file of a few
  hundred bytes can claim an image that big, and decoding costs memory and time in
  proportion to the size claimed. For animations (GIF, WebP) the limit covers all the
  frames together, as `MAX_PIXELS` did. To decode bigger images from files you trust,
  pass an `image.DecodeOptions` with a higher `max_pixels` (up to `image.MAX_PIXELS`,
  which stays the most an `Image` can have) to the new `formats.decode_with`,
  `decode_from_with` and `load_with`, or to a format's `decode_with` (and
  `gif.decode_all_with`, `webp.decode_all_with`, `tiff.decode_page_with`):

  ```axiom
  let opts = image.DecodeOptions.{ max_pixels: image.MAX_PIXELS };
  let mut img = formats.decode_with(bytes, opts)!;
  ```

  PNG, BMP, QOI, PNM, HDR and JPEG headers claiming more than the limit now fail with
  `TooLarge` rather than `Corrupt`, and AVIF frames and grids with `TooLarge` rather than
  `Unsupported`. `svg` decodes an embedded JPEG only if it fits the room left for images
  (as it did PNGs).
- TIFF decoding is several times faster per pixel (8-bit gray about 0.2 ns, RGB 2–4 ns,
  1-bit 4–5 ns, from 40–100), with half the memory for gray and RGB images, and the
  same output.
