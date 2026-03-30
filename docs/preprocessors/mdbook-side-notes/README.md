# mdBook Margin Notes Preprocessor

A custom mdBook preprocessor that adds Crafting Interpreters-style side notes to your book.

## Installation

The preprocessor is built automatically when you build the book. No separate installation needed.

## Usage

In your markdown files, use the `[!side]` syntax:

```markdown
GRUB scans the first 32KB of your kernel looking for the Multiboot2 header.

[!side]
The 32KB scan limit is why we place the header at the very start of the kernel in the linker script.
[/!side]

When it finds the magic number (0xe85250d6), it validates the checksum...
```

The content between `[!side]` and `[/!side]` will be rendered as a side note on the right side of the page (on wide screens) or as an inline callout on narrow screens.

## Features

- **Markdown support**: Margin notes can contain any markdown formatting (bold, italic, code, links, etc.)
- **Responsive design**: Automatically converts to inline callouts on small screens
- **Multiple notes**: You can have multiple side notes on the same page
- **Print-friendly**: Margin notes display properly when printing

## Examples

### Simple note

```markdown
[!side]
This is a simple side note.
[/!side]
```

### Note with formatting

```markdown
[!side]
This side note has **bold text**, `code`, and [a link](example.com).
[/!side]
```

### Multi-line note

```markdown
[!side]
This is a side note
with multiple lines.

It can even have multiple paragraphs!
[/!side]
```

## Styling

The side notes are styled in `theme/side-notes.css`. You can customize:

- Width and positioning
- Typography
- Colors
- Responsive breakpoints

## Testing

Run the tests:

```bash
cd book/preprocessors/side-notes
cargo test
```

## How It Works

1. mdBook calls the preprocessor before rendering
2. The preprocessor scans each chapter for `[!side]...[/!side]` blocks
3. These blocks are converted to `<aside class="side-note">` HTML
4. Custom CSS positions them in the side
5. mdBook's markdown renderer processes the content inside the aside tags
