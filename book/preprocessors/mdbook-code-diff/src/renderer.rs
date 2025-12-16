use crate::parser::{CodeLine, DiffMetadata, LineType, Placement};

fn html_escape(text: &str) -> String {
    text.replace('&', "&amp;")
        .replace('<', "&lt;")
        .replace('>', "&gt;")
        .replace('"', "&quot;")
}

fn render_annotation(metadata: &DiffMetadata) -> String {
    let mut html = String::from("  <div class=\"code-diff-annotation\">\n");

    if let Some(file) = &metadata.file {
        html.push_str(&format!(
            "    <span class=\"file-path\">{}</span>\n",
            html_escape(file)
        ));
    }

    if let Some(placement) = &metadata.placement {
        let placement_text = match placement {
            Placement::After(loc) => format!("after {}", loc),
            Placement::Before(loc) => format!("before {}", loc),
            Placement::Replace(loc) => format!("replace {}", loc),
        };
        html.push_str(&format!(
            "    <span class=\"placement\">{}</span>\n",
            html_escape(&placement_text)
        ));
    }

    html.push_str("  </div>\n");
    html
}

fn render_code_lines(lines: &[CodeLine], language: &str) -> String {
    let mut html = String::from("  <div class=\"code-diff-lines\" data-language=\"");
    html.push_str(language);
    html.push_str("\">");

    for line in lines {
        let class = match line.line_type {
            LineType::Context => "diff-context",
            LineType::Add => "diff-add",
            LineType::Remove => "diff-remove",
        };

        html.push_str("\n    <div class=\"diff-line ");
        html.push_str(class);
        html.push_str("\"><code>");
        html.push_str(&html_escape(&line.content));
        html.push_str("</code></div>");
    }

    html.push_str("\n  </div>\n");
    html
}

pub fn render_diff_block(
    language: &str,
    metadata: &DiffMetadata,
    lines: &[CodeLine],
) -> String {
    let mut html = String::from("<div class=\"code-diff-container\">\n");

    // Add annotation if we have metadata
    if metadata.file.is_some() || metadata.placement.is_some() {
        html.push_str(&render_annotation(metadata));
    }

    // Add code block
    html.push_str(&render_code_lines(lines, language));

    html.push_str("</div>\n");
    html
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_html_escape() {
        assert_eq!(html_escape("<div>"), "&lt;div&gt;");
        assert_eq!(html_escape("a & b"), "a &amp; b");
    }

    #[test]
    fn test_render_simple_diff() {
        let metadata = DiffMetadata {
            file: Some("test.c".to_string()),
            placement: Some(Placement::After("main".to_string())),
        };
        let lines = vec![
            CodeLine {
                line_type: LineType::Add,
                content: "int x = 5;".to_string(),
            },
        ];

        let html = render_diff_block("c", &metadata, &lines);
        
        assert!(html.contains("code-diff-container"));
        assert!(html.contains("test.c"));
        assert!(html.contains("after main"));
        assert!(html.contains("language-c"));
        assert!(html.contains("diff-add"));
        assert!(html.contains("int x = 5;"));
    }
}
