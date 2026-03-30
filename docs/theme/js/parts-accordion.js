/**
 * Parts Accordion – auto-collapses inactive book parts in the sidebar.
 *
 * Scans the sidebar for <li class="part-title"> elements, groups the
 * chapter items that follow each one, then:
 *   1. Determines which part the current page belongs to.
 *   2. Collapses every other part.
 *   3. Adds click-to-toggle on each part title.
 */
(function () {
    "use strict";

    /* ---- Wait for sidebar (works with mdbook's iframe toc) -- */
    function ready(fn) {
        if (document.readyState !== "loading") fn();
        else document.addEventListener("DOMContentLoaded", fn);
    }

    ready(function () {
        var sidebar =
            document.getElementById("sidebar") ||
            document.querySelector(".sidebar-scrollbox") ||
            document;

        var partTitles = sidebar.querySelectorAll("li.part-title");
        if (!partTitles.length) return;

        /* ---- Build a map:  partTitle → [sibling chapter-items] -- */
        var parts = [];

        partTitles.forEach(function (titleEl) {
            var group = { title: titleEl, items: [] };

            /* Walk subsequent siblings until the next part-title (or end).
               Note: despite the HTML source nesting part-title inside
               chapter-item, browsers auto-close the outer <li>, so
               part-title ends up as a flat sibling in <ol class="chapter">. */
            var node = titleEl.nextElementSibling;

            while (node) {
                /* If the node *contains* a part-title, we've hit the next part */
                if (
                    node.classList.contains("part-title") ||
                    node.querySelector(".part-title")
                ) {
                    break;
                }
                group.items.push(node);
                node = node.nextElementSibling;
            }

            parts.push(group);
        });

        /* ---- Detect the active part from the current page URL --- */
        var currentHref = window.location.pathname;

        function containsActive(group) {
            for (var i = 0; i < group.items.length; i++) {
                var links = group.items[i].querySelectorAll("a[href]");
                for (var j = 0; j < links.length; j++) {
                    /* Resolve relative hrefs to absolute for comparison */
                    var linkPath = new URL(links[j].href, window.location.origin)
                        .pathname;
                    if (currentHref === linkPath || currentHref.endsWith(linkPath)) {
                        return true;
                    }
                }
            }
            return false;
        }

        /* ---- Collapse / expand helpers ------------------------- */
        function collapseGroup(group) {
            group.title.classList.add("collapsed");
            group.items.forEach(function (el) {
                el.classList.add("part-collapsed");
            });
        }

        function expandGroup(group) {
            group.title.classList.remove("collapsed");
            group.items.forEach(function (el) {
                el.classList.remove("part-collapsed");
            });
        }

        /* ---- Initial state: expand active part, collapse rest -- */
        var activeIdx = -1;
        parts.forEach(function (group, idx) {
            if (containsActive(group)) activeIdx = idx;
        });

        /* Fallback: if nothing matched keep everything expanded */
        if (activeIdx === -1) return;

        parts.forEach(function (group, idx) {
            if (idx === activeIdx) {
                expandGroup(group);
                group.title.classList.add("active-part");
            } else {
                collapseGroup(group);
            }
        });

        /* ---- Click handler: toggle, accordion-style ------------ */
        parts.forEach(function (group, idx) {
            group.title.style.cursor = "pointer";

            group.title.addEventListener("click", function (e) {
                e.preventDefault();
                e.stopPropagation();

                var isCollapsed = group.title.classList.contains("collapsed");

                if (isCollapsed) {
                    /* Collapse all first (accordion) */
                    parts.forEach(function (g) {
                        collapseGroup(g);
                        g.title.classList.remove("active-part");
                    });
                    /* Then expand the clicked one */
                    expandGroup(group);
                    group.title.classList.add("active-part");
                } else {
                    /* Clicking the already-open part collapses it */
                    collapseGroup(group);
                    group.title.classList.remove("active-part");
                }
            });
        });
    });
})();
