use anyhow::{Context, Result};
use mdbook_side_notes::process_side_notes;
use serde_json::Value;
use std::io::{self, Read};

fn process_item(item: &mut Value) {
    if let Some(chapter) = item.get_mut("Chapter") {
        if let Some(content) = chapter.get_mut("content").and_then(|v| v.as_str()) {
            let processed = process_side_notes(content);
            chapter["content"] = Value::String(processed);
        }
        
        // Process sub_items recursively
        if let Some(sub_items) = chapter.get_mut("sub_items").and_then(|v| v.as_array_mut()) {
            for sub_item in sub_items {
                process_item(sub_item);
            }
        }
    }
}

fn process_book(mut book: Value) -> Value {
    if let Some(items) = book.get_mut("items").and_then(|v| v.as_array_mut()) {
        for item in items {
            process_item(item);
        }
    }
    book
}

fn main() -> Result<()> {
    // Check if we're being asked what renderers we support
    let args: Vec<String> = std::env::args().collect();
    if args.len() == 3 && args[1] == "supports" {
        // We support all renderers, but margin notes only make sense for HTML
        return Ok(());
    }

    let mut stdin = io::stdin();
    let mut buffer = String::new();
    stdin
        .read_to_string(&mut buffer)
        .context("Failed to read from stdin")?;

    // Parse the input - we expect [context, book]
    let mut input: Vec<Value> = serde_json::from_str(&buffer)
        .context("Failed to parse input JSON")?;

    if input.len() != 2 {
        anyhow::bail!("Expected input to be [context, book], got {} items", input.len());
    }

    // The book is the second element
    let book = input.remove(1);
    let processed_book = process_book(book);

    serde_json::to_writer(io::stdout(), &processed_book)
        .context("Failed to write output")?;

    Ok(())
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_side_note_conversion() {
        let input = r#"Some text here.

[!side]
This is a side note.
[/!side]

More text here."#;

        let expected = r#"Some text here.

<aside class="side-note">

This is a side note.

</aside>

More text here."#;

        assert_eq!(process_margin_notes(input), expected);
    }

    #[test]
    fn test_multiple_side_notes() {
        let input = r#"Text 1.

[!side]
Note 1.
[/!side]

Text 2.

[!side]
Note 2.
[/!side]

Text 3."#;

        let result = process_margin_notes(input);
        assert!(result.contains("<aside class=\"margin-note\">"));
        assert!(result.contains("Note 1."));
        assert!(result.contains("Note 2."));
    }

    #[test]
    fn test_multiline_margin_note() {
        let input = r#"Some text.

[!margin]
This is a margin note
with multiple lines
and **formatting**.
[/!margin]

More text."#;

        let result = process_margin_notes(input);
        assert!(result.contains("with multiple lines"));
        assert!(result.contains("**formatting**"));
    }
}
