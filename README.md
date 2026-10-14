# Photography Portfolio

Private wildlife portfolio for Neil / @neiltakesflicks. HTML and CSS in the browser, with a dependency-free C11 static gallery generator. No JavaScript, TypeScript, Python, framework, or runtime backend.

## Features

- Editorial gallery and compact contact-sheet mode.
- Native radio controls filter birds / mammals, with keyboard navigation.
- Native HTML popover viewer: Escape to close, previous/next buttons, background scroll lock.
- CSS light/dark toggle. Theme is session-only; it is not saved across reloads.
- Lazy-loaded images and expandable camera metadata.
- GitHub Actions compiles C with strict warnings and address/undefined-behaviour sanitizers, validates input handling and HTML escaping, and produces a downloadable static-site artifact. Nothing is deployed.

## Build

With a C11 compiler installed, from the repository root:

```sh
cc -std=c11 -Wall -Wextra -Werror -pedantic tools/gallery.c -o gallery
./gallery data/photos.tsv templates/page.html > index.tmp.html
# Only replace the current page after a successful generator exit.
mv index.tmp.html index.html
```

Open `index.html` in a current browser. Native popovers and CSS `:has()` require a modern browser. Older browsers can still see the gallery but may not support its enhanced controls.

## Add photographs

Place web-ready images in `assets/photos/`, then edit `data/photos.tsv`. Each line contains exactly ten nonempty tab-separated fields:

`Title | Species | Category | Image URL/path | Alt text | Camera | Lens | Shutter | Aperture | ISO`

The separators above illustrate fields; the actual file uses tabs. Categories are `birds`, `mammals`, or `landscapes`. Images must use `https://` or a path beginning `assets/photos/`. Up to 256 photos are supported. No tabs or newlines inside fields. Use `Not supplied` for unknown metadata. Rebuild after editing the manifest or `templates/page.html`.

The collection contains 45 original photographs by Neil, imported from 46 supplied files with one exact duplicate removed. Four HEIC files were converted for browser compatibility. Web-ready WebP copies and smaller responsive versions are included; originals remain outside the repository. Metadata was extracted during import where available, with location metadata omitted from exported images. `data/photo-sources.json` maps website files to original filenames. The C generator reads the prepared manifest; it does not itself compress images or extract EXIF. Keep original photo backups separately.

## Privacy

Keep this repository private. No GitHub Pages or deployment workflow is configured. The earlier private Sites draft remains separate. CI artifacts are available to users with repository access and expire after seven days.
