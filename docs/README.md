# JANUS Documentation

This directory contains the JANUS documentation, built with [mdbook](https://rust-lang.github.io/mdBook/).

## Prerequisites

Install mdbook and the required preprocessors via Cargo:

```bash
cargo install mdbook
cargo install mdbook-side-notes
cargo install mdbook-code-diff
cargo install mdbook-mermaid
```

| Tool | Purpose |
|------|---------|
| `mdbook` | Static site generator for the book |
| `mdbook-side-notes` | Renders `[!side]...[/!side]` and `[!margin]...[/!margin]` blocks as margin notes |
| `mdbook-code-diff` | Renders `` ```c-diff `` / `` ```ld-diff `` blocks with add/remove highlighting |
| `mdbook-mermaid` | Renders `` ```mermaid `` blocks as diagrams |

## Building

```bash
cd docs
mdbook build
```

The output is written to `docs/build/`. Open `docs/build/index.html` in a browser to view the site locally.

## Live Preview

For a live-reloading development server:

```bash
cd docs
mdbook serve --open
```

This starts a local server (default `http://localhost:3000`) and rebuilds automatically when any source file changes.

## Structure

```
docs/
├── book.toml              # mdbook configuration (preprocessors, theme, output)
├── src/
│   ├── SUMMARY.md         # Table of contents — defines the sidebar navigation
│   ├── introduction.md    # Landing page
│   ├── wiki/              # General OS / CS concepts (hardware, boot, memory, tools)
│   ├── tutorials/         # Hands-on, step-by-step implementation guides
│   ├── reference/         # JANUS-specific architecture and subsystem design
│   └── project/           # Contributing guidelines, philosophy, setup
├── theme/                 # Custom CSS (side notes, code diffs)
└── additional-js/         # Mermaid JS bundle
```

All pages are listed in [src/SUMMARY.md](src/SUMMARY.md). To add a new page, create the `.md` file in the appropriate subdirectory and add an entry to SUMMARY.md.
