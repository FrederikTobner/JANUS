// Code Diff Syntax Highlighting
// Applies syntax highlighting to diff blocks line-by-line

(function() {
    'use strict';

    function highlightDiffLines(container) {
        const language = container.getAttribute('data-language');
        if (!language || !window.hljs) {
            return;
        }

        const lines = container.querySelectorAll('.diff-line code');
        
        lines.forEach(codeEl => {
            const code = codeEl.textContent;
            
            try {
                // Highlight this single line
                const result = hljs.highlight(code, { 
                    language: language,
                    ignoreIllegals: true 
                });
                
                // Replace content with highlighted version
                codeEl.innerHTML = result.value;
            } catch (e) {
                // If highlighting fails, keep original content
                console.warn('Failed to highlight line:', e);
            }
        });
    }

    function processDiffBlocks() {
        const diffContainers = document.querySelectorAll('.code-diff-lines[data-language]');
        
        diffContainers.forEach(container => {
            highlightDiffLines(container);
        });
    }

    // Run after DOM is loaded
    if (document.readyState === 'loading') {
        document.addEventListener('DOMContentLoaded', processDiffBlocks);
    } else {
        processDiffBlocks();
    }

    // Also run when highlight.js is ready (if loaded async)
    if (window.hljs) {
        processDiffBlocks();
    } else {
        window.addEventListener('load', processDiffBlocks);
    }
})();
