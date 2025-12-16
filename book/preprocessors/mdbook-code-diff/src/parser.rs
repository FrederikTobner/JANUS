use regex::Regex;

#[derive(Debug, Clone)]
pub enum Placement {
    After(String),
    Before(String),
    Replace(String),
}

#[derive(Debug, Clone)]
pub struct DiffMetadata {
    pub file: Option<String>,
    pub placement: Option<Placement>,
}

pub fn parse_metadata(text: &str) -> DiffMetadata {
    let mut file = None;
    let mut placement = None;

    for line in text.lines() {
        let line = line.trim();
        if line.is_empty() {
            continue;
        }

        if let Some(file_path) = line.strip_prefix("file:") {
            file = Some(file_path.trim().to_string());
        } else if let Some(location) = line.strip_prefix("after:") {
            placement = Some(Placement::After(location.trim().to_string()));
        } else if let Some(location) = line.strip_prefix("before:") {
            placement = Some(Placement::Before(location.trim().to_string()));
        } else if let Some(location) = line.strip_prefix("replace:") {
            placement = Some(Placement::Replace(location.trim().to_string()));
        }
    }

    DiffMetadata { file, placement }
}

#[derive(Debug, Clone, PartialEq)]
pub enum LineType {
    Context,
    Add,
    Remove,
}

#[derive(Debug, Clone)]
pub struct CodeLine {
    pub line_type: LineType,
    pub content: String,
}

pub fn parse_line(line: &str) -> CodeLine {
    if line.is_empty() {
        return CodeLine {
            line_type: LineType::Context,
            content: String::new(),
        };
    }

    match line.chars().next() {
        Some('+') => CodeLine {
            line_type: LineType::Add,
            content: line[1..].to_string(),
        },
        Some('-') => CodeLine {
            line_type: LineType::Remove,
            content: line[1..].to_string(),
        },
        Some(' ') => CodeLine {
            line_type: LineType::Context,
            content: line[1..].to_string(),
        },
        _ => CodeLine {
            line_type: LineType::Context,
            content: line.to_string(),
        },
    }
}

pub fn parse_code_lines(code: &str) -> Vec<CodeLine> {
    code.lines().map(parse_line).collect()
}

pub fn find_diff_blocks(content: &str) -> Vec<(String, String, String, usize, usize)> {
    let re = Regex::new(
        r"(?ms)^```(\w+)-diff\s*\n(.*?)\n---\s*\n(.*?)\n```\s*$"
    ).unwrap();

    let mut blocks = Vec::new();
    for cap in re.captures_iter(content) {
        let language = cap.get(1).unwrap().as_str().to_string();
        let metadata = cap.get(2).unwrap().as_str().to_string();
        let code = cap.get(3).unwrap().as_str().to_string();
        let start = cap.get(0).unwrap().start();
        let end = cap.get(0).unwrap().end();
        
        blocks.push((language, metadata, code, start, end));
    }

    blocks
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_parse_metadata() {
        let input = "file: kernel/linker.ld\nafter: .text section";
        let metadata = parse_metadata(input);
        assert_eq!(metadata.file, Some("kernel/linker.ld".to_string()));
        assert!(matches!(metadata.placement, Some(Placement::After(_))));
    }

    #[test]
    fn test_parse_line_add() {
        let line = "+.rodata ALIGN(4K) : {";
        let parsed = parse_line(line);
        assert_eq!(parsed.line_type, LineType::Add);
        assert_eq!(parsed.content, ".rodata ALIGN(4K) : {");
    }

    #[test]
    fn test_parse_line_context() {
        let line = " .text ALIGN(4K) : {";
        let parsed = parse_line(line);
        assert_eq!(parsed.line_type, LineType::Context);
        assert_eq!(parsed.content, ".text ALIGN(4K) : {");
    }

    #[test]
    fn test_parse_line_remove() {
        let line = "-old_function();";
        let parsed = parse_line(line);
        assert_eq!(parsed.line_type, LineType::Remove);
        assert_eq!(parsed.content, "old_function();");
    }
}
