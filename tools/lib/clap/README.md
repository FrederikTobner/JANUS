# clap — CLAP is Likely A Parser

Declarative command-line argument parsing library for JANUS C tools.

## Features

- Flags (`--verbose`), options (`--output FILE`), and positional arguments
- Short forms (`-o FILE`)
- Auto-generated `--help` and `--version` output
- Required-argument enforcement with clear error messages
- Default values

## API Sketch

```c
#include <clap/clap.h>

static const clap_arg_t args[] = {
    { CLAP_POSITIONAL, "input",   0,   "FILE",   "Input PSF file",   NULL, true  },
    { CLAP_OPTION,     "output", 'o',  "FILE",   "Output C header",  NULL, true  },
    { CLAP_FLAG,       "verbose",'v',  NULL,     "Verbose output",   NULL, false },
};

static const clap_parser_t parser = {
    .name    = "psf2c",
    .version = "1.0.0",
    .about   = "Convert PSF fonts to C headers",
    .args    = args,
    .nargs   = 3,
};

int main(int argc, char ** argv) {
    clap_result_t * result = clap_parse(&parser, argc, argv);
    const char * input = clap_get(result, "input");
    // ...
    clap_result_free(result);
}
```

## Files

- `include/clap/clap.h` — public API (`clap_parser_t`, `clap_arg_t`, `clap_result_t`)
- `src/clap.c` — implementation
- `tests/` — unit tests (Google Test)
