# CI Workflows

JANUS uses GitHub Actions to verify every pull request and to publish artefacts on every push to `main`. All workflow files live in `.github/workflows/`.

---

## `build.yaml` — Kernel build matrix

**Triggers:** pull requests targeting `main`/`master`.

Builds the kernel across all supported target/compiler combinations:

| Preset | Arch | Compiler |
|---|---|---|
| `x86_64-gcc` | x86\_64 | GCC |
| `x86_64-clang` | x86\_64 | Clang |
| `aarch64-gcc` | AArch64 | GCC |
| `aarch64-clang` | AArch64 | Clang |

Each cell runs three steps: **Configure** → **Build** → **Build ISOs**.

The `x86_64-clang` cell additionally runs two quality gates:

- **Check formatting** — `clang-format --dry-run --Werror` over every `kernel/**/*.{c,h}` file. Fails if any file diverges from `.clang-format`.
- **Static analysis** — `clang-tidy` with `.clang-tidy` enabled checks, run against the `compile_commands.json` produced by the configure step. Errors abort the job.

---

## `tools.yaml` — Tools build, test, and coverage

**Triggers:** pull requests and pushes to `main` when `tools/**` changes.

### `build-and-test` job

Builds and runs CTest for the tools sub-project with both GCC and Clang.

### `coverage` job

Builds the tools with GCC + gcov instrumentation (`TOOLS_ENABLE_COVERAGE=ON`), runs all tests, then uses **gcovr** to produce an HTML coverage report. The report is uploaded as a GitHub Actions artifact (`coverage-report`) with a 30-day retention period.

To enable coverage locally:

```bash
cmake -S tools -B build-tools-coverage \
    -G Ninja \
    -D CMAKE_BUILD_TYPE=Debug \
    -D CMAKE_C_COMPILER=gcc \
    -D TOOLS_ENABLE_COVERAGE=ON
cmake --build build-tools-coverage
ctest --test-dir build-tools-coverage --output-on-failure
gcovr --root tools --object-directory build-tools-coverage \
      --html-details coverage/index.html --print-summary
```

---

## `doxygen.yaml` — API documentation

**Triggers:** pull requests and pushes to `main` when `kernel/**`, `tools/**`, or `Doxyfile` changes.

### `build` job

Installs Doxygen and Graphviz, runs `doxygen Doxyfile`, and uploads `doxygen-out/html/` as a GitHub Actions artifact (`doxygen-html`, 30-day retention). This runs on every push and PR so reviewers can download and browse the API docs for any branch.

### `publish` job (push to `main` only)

Rebuilds the mdBook **and** the Doxygen HTML in a single job, copies the Doxygen output into `docs/build/api/`, then deploys the combined artefact to GitHub Pages. The result is:

| Path | Content |
|---|---|
| `/JANUS/` | mdBook narrative documentation |
| `/JANUS/api/` | Doxygen API reference + call graphs |

The `publish` job uses `concurrency: group: pages` to ensure it never races with `docs.yaml`.

---

## `docs.yaml` — mdBook (narrative docs)

**Triggers:** pushes to `main` when `docs/**` changes.

Builds the mdBook using custom preprocessors (`mdbook-side-notes`, `mdbook-code-diff`, `mdbook-mermaid`) and deploys to GitHub Pages at `/JANUS/`.

> **Note:** When both `docs.yaml` and `doxygen.yaml` would trigger simultaneously (e.g. a commit touching both `docs/` and `kernel/`), the `concurrency: group: pages` lock ensures the second deployment starts only after the first completes.

---

## Quality gate summary

| Check | Workflow | When |
|---|---|---|
| Kernel builds (all 4 presets) | `build.yaml` | Every PR |
| `clang-format` | `build.yaml` | Every PR |
| `clang-tidy` | `build.yaml` | Every PR |
| Tools build + CTest | `tools.yaml` | Every PR touching `tools/` |
| Tools coverage report | `tools.yaml` | Every PR touching `tools/` |
| Doxygen validation | `doxygen.yaml` | Every PR touching `kernel/tools/Doxyfile` |
| Pages deploy (book + API) | `doxygen.yaml` | Push to `main` |
