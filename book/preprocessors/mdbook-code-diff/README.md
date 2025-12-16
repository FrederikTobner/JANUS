# mdbook-code-diff

A preprocessor for mdBook that enables Crafting Interpreters-style code diffs with syntax highlighting.

## Features

- Visual diff indicators (colored borders on new/removed lines)
- Dimmed context lines for existing code
- Side annotations showing file path and placement
- Full syntax highlighting for all languages
- Perfect copy-paste (no `+` or `-` symbols in output)
- Responsive design (annotations move inline on mobile)
- Dark theme support
- Print-friendly styles

## Installation

```bash
cd book/preprocessors/mdbook-code-diff
cargo build --release
cargo install --path .
```

## Usage

In your markdown files, use the `language-diff` syntax:

````markdown
```c-diff
file: kernel/memory.c
after: init_memory()
---
 void init_memory(void) {
     setup_heap();
 }
 
+void kmalloc(size_t size) {
+    return allocate(size);
+}
```
````

### Syntax

- **Language**: `<lang>-diff` where `<lang>` is any language (c, asm, ld, rust, etc.)
- **Metadata**: Between ` ``` ` and `---`
  - `file: path/to/file` — File path for annotation
  - `after: location` — Place after this location
  - `before: location` — Place before this location
  - `replace: location` — Replace this location
- **Code lines**:
  - ` ` (space) prefix = existing code (dimmed)
  - `+` prefix = new code (highlighted green)
  - `-` prefix = removed code (strikethrough red)

## Configuration

Add to `book.toml`:

```toml
[preprocessor.code-diff]
command = "mdbook-code-diff"

[output.html]
additional-css = ["theme/css/code-diff.css"]
additional-js = ["theme/js/code-diff.js"]
```

## Testing

```bash
cargo test
```

## License

MIT
