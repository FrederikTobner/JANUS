# Building TinyOS: The Book

This directory contains the source for the TinyOS book, written using [mdBook](https://rust-lang.github.io/mdBook/).

## Building the Book

### Prerequisites

Install mdBook:
```bash
cargo install mdbook
```

### Build and Serve

```bash
# Build the book (output in book/)
mdbook build

# Serve locally with auto-reload
mdbook serve
```

Then open http://localhost:3000 in your browser.

### Structure

- `book.toml` - Book configuration
- `src/` - Markdown source files
  - `SUMMARY.md` - Table of contents (controls chapter order)
  - `*.md` - Individual chapters

## Writing Guidelines

- Keep chapters focused on one concept
- Include code snippets directly from the TinyOS source when possible
- Add diagrams for complex concepts (ASCII art is fine!)
- End each chapter with practical exercises
- Update SUMMARY.md when adding new chapters

## Publishing

To publish to GitHub Pages:
```bash
mdbook build
# Push the book/ directory to gh-pages branch
```
