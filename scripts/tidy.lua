#!/usr/bin/env lua
--- clang-tidy runner for JANUS.
--
-- Runs clang-tidy on all kernel C sources extracted from a build directory's
-- compile_commands.json, mirroring the CI static-analysis step.
--
-- Usage: lua scripts/tidy.lua [OPTIONS]
-- Run with --help for full usage information.
--
-- @copyright Copyright (C) 2026 Frederik, TinyOS contributors
-- @license   GNU Affero General Public License v3.0 or later

-- ─── Colours ─────────────────────────────────────────────────────────────────

local _tty_raw   = os.execute("test -t 1") -- luacheck: ignore
local use_colour = (_tty_raw == true) or (_tty_raw == 0)

local function sgr(code)
    if use_colour then return string.format("\27[%sm", code) end
    return ""
end

local C = {
    reset  = sgr("0"),
    bold   = sgr("1"),
    red    = sgr("31"),
    green  = sgr("32"),
    yellow = sgr("33"),
    cyan   = sgr("36"),
    dim    = sgr("2"),
}

-- ─── Helpers ──────────────────────────────────────────────────────────────────

--- Print an error message and exit.
local function die(fmt, ...)
    io.stderr:write(string.format("%serror:%s " .. fmt .. "\n", C.red, C.reset, ...))
    os.exit(1)
end

--- Execute a shell command, return (ok, exit_code).
local function exec(cmd)
    local ok, _, code = os.execute(cmd)
    if ok == true then return true, 0 end
    if type(ok) == "number" then return ok == 0, ok end
    return false, code or 1
end

--- Execute a command and capture stdout+stderr as a string.
local function capture(cmd)
    local f = io.popen(cmd, "r")
    if not f then return nil end
    local out = f:read("*a")
    f:close()
    return out
end

--- Get a monotonic wall-clock timestamp in seconds.
local function now()
    return os.time()
end

--- Format elapsed seconds as a human string.
local function fmt_elapsed(seconds)
    if seconds < 60 then return string.format("%ds", seconds) end
    return string.format("%dm%02ds", math.floor(seconds / 60), seconds % 60)
end

-- ─── Project root ─────────────────────────────────────────────────────────────

local ROOT = (function()
    local abs = capture(string.format("realpath %q 2>/dev/null", arg[0]))
    if not abs then die("cannot resolve script path from '%s'", arg[0]) end
    abs = abs:match("^%s*(.-)%s*$")
    local root = abs:match("^(.*)/[^/]+/[^/]+$")  -- strip /scripts/<name>.lua
    if not root or root == "" then
        die("cannot derive project root from script path '%s'", abs)
    end
    return root
end)()

-- ─── Tool detection ───────────────────────────────────────────────────────────

local function find_tool(candidates)
    for _, name in ipairs(candidates) do
        local out = capture(string.format("command -v %s 2>/dev/null", name))
        if out and out:match("%S") then return name end
    end
    return nil
end

local TOOL = find_tool { "clang-tidy-18", "clang-tidy-17", "clang-tidy-16", "clang-tidy" }
if not TOOL then
    die("clang-tidy not found — install clang-tidy or add it to PATH")
end

-- ─── Options ──────────────────────────────────────────────────────────────────

local USAGE = [[
Usage: lua scripts/tidy.lua [OPTIONS]

Runs clang-tidy on JANUS kernel C sources using a build directory's
compile_commands.json. Mirrors the CI static-analysis step.

Options:
  -p, --preset PRESET   CMake preset whose build dir to use (e.g. x86_64-clang)
                        Defaults to the first available *-clang build directory.
      --filter PATTERN  Only analyse files whose path matches PATTERN
  -v, --verbose         Show full clang-tidy output even for passing files
  -h, --help            Print this message and exit

Exit codes:
  0   All files passed clang-tidy
  1   One or more files produced diagnostics
]]

local opts = {
    preset  = nil,
    filter  = nil,
    verbose = false,
}

do
    local i = 1
    while i <= #arg do
        local a = arg[i]
        if a == "-h" or a == "--help" then
            io.write(USAGE); os.exit(0)
        elseif a == "-p" or a == "--preset" then
            i = i + 1
            if not arg[i] then die("--preset requires an argument") end
            opts.preset = arg[i]
        elseif a == "--filter" then
            i = i + 1
            if not arg[i] then die("--filter requires an argument") end
            opts.filter = arg[i]
        elseif a == "-v" or a == "--verbose" then
            opts.verbose = true
        else
            die("unknown option: %s  (try --help)", a)
        end
        i = i + 1
    end
end

-- ─── Build directory resolution ───────────────────────────────────────────────

--- Return the build directory for a preset name.
local function build_dir_for(preset)
    return ROOT .. "/build-" .. preset
end

--- Discover available *-clang build directories that contain a
--- compile_commands.json, sorted for deterministic selection.
local function discover_clang_build_dirs()
    local raw = capture(string.format(
        "find %q -maxdepth 2 -name compile_commands.json | sort", ROOT))
    if not raw then return {} end
    local dirs = {}
    for path in raw:gmatch("[^\n]+") do
        local dir = path:match("^(.*)/compile_commands%.json$")
        if dir and dir:match("%-clang$") then
            dirs[#dirs + 1] = dir
        end
    end
    return dirs
end

local BUILD_DIR
if opts.preset then
    BUILD_DIR = build_dir_for(opts.preset)
    local cc = BUILD_DIR .. "/compile_commands.json"
    local f = io.open(cc, "r")
    if not f then
        die("compile_commands.json not found in %s\n  Configure first: cmake --preset %s",
            BUILD_DIR, opts.preset)
    end
    f:close()
else
    local dirs = discover_clang_build_dirs()
    if #dirs == 0 then
        die("no *-clang build directories found under %s\n  Configure first: cmake --preset <arch>-clang",
            ROOT)
    end
    BUILD_DIR = dirs[1]
    -- Derive preset name for display
    opts.preset = BUILD_DIR:match("build%-(.+)$") or BUILD_DIR
end

-- ─── compile_commands.json parsing ───────────────────────────────────────────

--- Extract kernel .c source paths from compile_commands.json.
--- Uses Lua pattern matching to avoid a jq dependency.
local function load_sources(build_dir)
    local path = build_dir .. "/compile_commands.json"
    local f = io.open(path, "r")
    if not f then die("cannot open %s", path) end
    local content = f:read("*a")
    f:close()

    local files = {}
    local seen  = {}
    for file in content:gmatch('"file"%s*:%s*"([^"]+)"') do
        -- Mirror the CI grep: kernel/.*\.c$
        if file:match("kernel/.*%.c$") and not seen[file] then
            if not opts.filter or file:match(opts.filter) then
                files[#files + 1] = file
                seen[file]        = true
            end
        end
    end

    if #files == 0 then
        die("no kernel .c files found in %s", path)
    end

    table.sort(files)
    return files
end

-- ─── Progress helpers ─────────────────────────────────────────────────────────

--- Strip ROOT prefix for readable output.
local function short_path(path)
    return path:gsub("^" .. ROOT .. "/", "")
end

--- Overwrite the current line with a progress indicator.
local function status(i, total, path, msg)
    io.write(string.format("\r\27[K[%d/%d] %s%s%s: %s",
        i, total, C.cyan, short_path(path), C.reset, msg))
    io.flush()
end

--- Finalise a status line with a result symbol and elapsed time.
local function status_done(i, total, path, ok, elapsed)
    local sym = ok and (C.green .. "✓") or (C.red .. "✗")
    io.write(string.format("\r\27[K%s[%d/%d]%s %s%s%s %s%s  %s%s%s\n",
        C.bold, i, total, C.reset,
        C.cyan, short_path(path), C.reset,
        sym, C.reset,
        C.dim, fmt_elapsed(elapsed), C.reset))
    io.flush()
end

--- Print indented clang-tidy output.
local function show_output(text)
    if not text or text:match("^%s*$") then return end
    for line in text:gmatch("[^\n]+") do
        io.write(string.format("  %s\n", line))
    end
    io.write("\n")
end

-- ─── Main ─────────────────────────────────────────────────────────────────────

local function main()
    io.write(string.format("%s── JANUS clang-tidy ──%s  (%s)\n",
        C.bold, C.reset, TOOL))
    io.write(string.format("Build dir: %s%s%s\n\n",
        C.cyan, BUILD_DIR, C.reset))

    local files    = load_sources(BUILD_DIR)
    local n        = #files
    local pass     = 0
    local fail     = 0
    local t_total  = now()

    for i, path in ipairs(files) do
        status(i, n, path, "analysing...")
        local t_start = now()

        local cmd    = string.format(
            "%s -p %q %q 2>&1", TOOL, BUILD_DIR, path)
        local output = capture(cmd)
        -- clang-tidy exits 0 even with warnings; treat any diagnostic output
        -- that contains ": error:" or ": warning:" as a failure.
        local has_diag = output and (
            output:match(": error:")   or
            output:match(": warning:") or
            output:match(": note:"))
        local ok = not has_diag

        local elapsed = now() - t_start
        status_done(i, n, path, ok, elapsed)

        if ok then
            pass = pass + 1
            if opts.verbose then show_output(output) end
        else
            fail = fail + 1
            show_output(output)
        end
    end

    -- Summary
    io.write("\n")
    local sep = string.rep("─", 52)
    io.write(sep .. "\n")
    io.write(string.format(" Total time: %s\n", fmt_elapsed(now() - t_total)))
    if fail == 0 then
        io.write(string.format(" %sAll %d files passed clang-tidy%s\n",
            C.green, n, C.reset))
    else
        io.write(string.format(" %s%d of %d files have diagnostics%s\n",
            C.red, fail, n, C.reset))
    end
    io.write(sep .. "\n")

    os.exit(fail == 0 and 0 or 1)
end

main()
