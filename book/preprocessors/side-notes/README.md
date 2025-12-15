# mdBook Margin Notes Preprocessor

A custom mdBook preprocessor that adds Crafting Interpreters-style margin notes to your book.

## Installation

The preprocessor is built automatically when you build the book. No separate installation needed.

## Usage

In your markdown files, use the `[!margin]` syntax:

```markdown
GRUB scans the first 32KB of your kernel looking for the Multiboot2 header.

[!margin]
The 32KB scan limit is why we place the header at the very start of the kernel in the linker script.
[/!margin]

When it finds the magic number (0xe85250d6), it validates the checksum...
```

The content between `[!margin]` and `[/!margin]` will be rendered as a margin note on the right side of the page (on wide screens) or as an inline callout on narrow screens.

## Features

- **Markdown support**: Margin notes can contain any markdown formatting (bold, italic, code, links, etc.)
- **Responsive design**: Automatically converts to inline callouts on small screens
- **Multiple notes**: You can have multiple margin notes on the same page
- **Print-friendly**: Margin notes display properly when printing

## Examples

### Simple note

```markdown
[!margin]
This is a simple margin note.
[/!margin]
```

### Note with formatting

```markdown
[!margin]
This margin note has **bold text**, `code`, and [a link](example.com).
[/!margin]
```

### Multi-line note

```markdown
[!margin]
This is a margin note
with multiple lines.

It can even have multiple paragraphs!
[/!margin]
```

## Styling

The margin notes are styled in `theme/margin-notes.css`. You can customize:

- Width and positioning
- Typography
- Colors
- Responsive breakpoints

## Testing

Run the tests:

```bash
cd book/preprocessors/margin-notes
cargo test
```

## How It Works

1. mdBook calls the preprocessor before rendering
2. The preprocessor scans each chapter for `[!margin]...[/!margin]` blocks
3. These blocks are converted to `<aside class="margin-note">` HTML
4. Custom CSS positions them in the margin
5. mdBook's markdown renderer processes the content inside the aside tags
