use regex::Regex;

pub fn process_side_notes(content: &str) -> String {
    // Match [!side] ... [/!side] blocks
    // Handle both inline and multi-line side notes
    let re = Regex::new(r"(?ms)\[!side\](.*?)\[/!side\]").unwrap();
    
    re.replace_all(content, |caps: &regex::Captures| {
        let note_content = caps.get(1).map_or("", |m| m.as_str().trim());
        format!("<aside class=\"side-note\">\n\n{}\n\n</aside>", note_content)
    }).to_string()
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

        assert_eq!(process_side_notes(input), expected);
    }

    #[test]
    fn test_multiple_margin_notes() {
        let input = r#"Text 1.

[!margin]
Note 1.
[/!margin]

Text 2.

[!margin]
Note 2.
[/!margin]

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

    #[test]
    fn test_no_margin_notes() {
        let input = "Just some regular text.";
        assert_eq!(process_margin_notes(input), input);
    }
}
