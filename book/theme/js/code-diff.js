// Code Diff Syntax Highlighting
// Applies syntax highlighting to diff blocks

(function() {
    'use strict';

    function highlightDiffBlock(container) {
        const language = container.getAttribute('data-language');
        if (!language) {
            return;
        }

        // Check if highlight.js is available
        if (!window.hljs) {
            setTimeout(() => highlightDiffBlock(container), 100);
            return;
        }

        // Verify the language is loaded
        const langDef = hljs.getLanguage(language);
        if (!langDef) {
            // Language not available, skip silently
            return;
        }

        // Collect all lines
        const lineElements = container.querySelectorAll('.diff-line code');
        const lines = Array.from(lineElements).map(el => el.textContent);

        // Combine all code into one string
        const fullCode = lines.join('\n');

        try {
            // Use highlightAuto or the language-specific method
            // This is more forgiving and works across versions
            let highlighted;
            // Check if we can use the language directly
            if (hljs.highlight && hljs.getLanguage(language)) {
                    try {
                        // Try old API signature
                        const result = hljs.highlight(language, fullCode, true);
                        highlighted = result.value;
                    } catch (err2) {
                        // Both failed, use highlightAuto as fallback
                        const result = hljs.highlightAuto(fullCode, [language]);
                        highlighted = result.value;
                    }
            
            } else {
                // Fallback to auto-detection
                const result = hljs.highlightAuto(fullCode, [language]);
                highlighted = result.value;
            }
            
            // Split back into lines
            const highlightedLines = highlighted.split('\n');
            
            // Apply to each line
            lineElements.forEach((codeEl, index) => {
                if (highlightedLines[index] !== undefined) {
                    codeEl.innerHTML = highlightedLines[index];
                }
            });
            
        } catch (e) {
            // Complete silent fail - the code stays as-is
            // Don't even log to console to avoid spam
        }
    }

    function processDiffBlocks() {
        const diffContainers = document.querySelectorAll('.code-diff-lines[data-language]');
        diffContainers.forEach(highlightDiffBlock);
    }

    // Wait for highlight.js
    function init() {
        if (window.hljs) {
            processDiffBlocks();
        } else {
            setTimeout(init, 100);
        }
    }

    if (document.readyState === 'loading') {
        document.addEventListener('DOMContentLoaded', init);
    } else {
        init();
    }
})();
