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

Then open <http://localhost:3000> in your browser.

### Structure

- `book.toml` - Book configuration
- `src/` - Markdown source files
  - `SUMMARY.md` - Table of contents (controls chapter order)
  - `chapter-01-introduction/` - Chapter 1: Introduction
  - `chapter-02-getting-started/` - Chapter 2: Getting Started
  - `chapter-03-boot-process/` - Chapter 3: The Boot Process
  - `chapter-04-io/` - Chapter 4: Input/Output
  - `chapter-05-memory/` - Chapter 5: Memory Management
  - `chapter-06-kernel/` - Chapter 6: The Kernel
  - `appendices/` - Appendix content
- `preprocessors/side-notes/` - Custom mdBook preprocessor for side notes
