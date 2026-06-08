# Doxygen

JANUS uses Doxygen to generate API reference documentation from code comments.

The generated output complements mdBook narrative docs:

- mdBook explains design and concepts
- Doxygen indexes APIs, symbols, and call graphs

## Local Generation

```bash
doxygen Doxyfile
```

HTML output is written to:

```text
doxygen-out/html/
```

Open the index page in a browser to browse module APIs and symbol cross-references.

## CI and Publishing

Doxygen is validated and published by `doxygen.yaml`:

- PRs: build and upload `doxygen-html` artifact
- Push to `main`: publish under `/JANUS/api/` alongside mdBook content

See [CI Workflows](../contributing/ci.md) for full job details.

## Writing Better API Docs

- Use `@file` and `@brief` at file scope.
- Document public structs and invariants.
- Keep parameter and return semantics explicit.
- Prefer stable terminology from [Terminology](../contributing/terminology.md).

