use serde_json::{json, Value};
use std::io::{self, Read, Write};

mod parser;
mod renderer;

use parser::{find_diff_blocks, parse_code_lines, parse_metadata};
use renderer::render_diff_block;

fn process_content(content: &str) -> String {
    let blocks = find_diff_blocks(content);

    if blocks.is_empty() {
        return content.to_string();
    }

    let mut result = String::new();
    let mut last_pos = 0;

    for (language, metadata_str, code, start, end) in blocks {
        // Add content before this block
        result.push_str(&content[last_pos..start]);

        // Parse and render the diff block
        let metadata = parse_metadata(&metadata_str);
        let lines = parse_code_lines(&code);
        let html = render_diff_block(&language, &metadata, &lines);

        result.push_str(&html);
        last_pos = end;
    }

    // Add remaining content
    result.push_str(&content[last_pos..]);
    result
}

fn process_book(book: &mut Value) {
    if let Some(items) = book.get_mut("items") {
        process_items(items);
    }
}

fn process_items(items: &mut Value) {
    if let Some(arr) = items.as_array_mut() {
        for item in arr {
            if let Some(chapter) = item.get_mut("Chapter") {
                if let Some(content) = chapter.get("content").and_then(|c| c.as_str()) {
                    let processed = process_content(content);
                    *chapter.get_mut("content").unwrap() = Value::String(processed);
                }

                // Recursively process sub-items
                if let Some(sub_items) = chapter.get_mut("sub_items") {
                    process_items(sub_items);
                }
            }
        }
    }
}

fn main() {
    let mut stderr = io::stderr();
    
    // Check command line arguments first
    let args: Vec<String> = std::env::args().collect();
    
    if args.len() > 1 && args[1] == "supports" {
        let response = json!({ "supported": true });
        println!("{}", response);
        return;  // Exit immediately without reading stdin
    }

    let mut stdin = io::stdin();
    let mut stdout = io::stdout();

    // Read input
    let mut buffer = String::new();
    if let Err(e) = stdin.read_to_string(&mut buffer) {
        writeln!(stderr, "Error reading stdin: {}", e).ok();
        std::process::exit(1);
    }

    // Parse JSON input - mdBook passes an array [context, book]
    let input_array: Vec<Value> = match serde_json::from_str(&buffer) {
        Ok(v) => v,
        Err(e) => {
            writeln!(stderr, "Error parsing JSON: {}", e).ok();
            std::process::exit(1);
        }
    };
    
    // The book is the second element of the array
    let mut book_value = if input_array.len() > 1 {
        input_array[1].clone()
    } else {
        writeln!(stderr, "Error: Expected at least 2 elements in input array").ok();
        std::process::exit(1);
    };
    
    process_book(&mut book_value);

    // Output just the modified book (second element)
    let output_json = serde_json::to_string(&book_value).unwrap();
    
    if let Err(e) = writeln!(stdout, "{}", output_json) {
        writeln!(stderr, "Error writing output: {}", e).ok();
        std::process::exit(1);
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_process_simple_diff() {
        let input = r#"Some text before.

```c-diff
file: test.c
after: main
---
+int x = 5;
```

Some text after."#;

        let output = process_content(input);

        assert!(output.contains("Some text before"));
        assert!(output.contains("Some text after"));
        assert!(output.contains("code-diff-container"));
        assert!(output.contains("test.c"));
        assert!(output.contains("int x = 5;"));
        assert!(!output.contains("```c-diff"));
    }

    #[test]
    fn test_process_no_diff() {
        let input = "Just regular markdown text.";
        let output = process_content(input);
        assert_eq!(input, output);
    }
}
