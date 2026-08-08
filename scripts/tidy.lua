#!/usr/bin/env lua
--- clang-tidy runner for JANUS.
--
-- Runs clang-tidy on all kernel C sources extracted from a build directory's
-- compile_commands.json, mirroring the CI static-analysis step.
--
-- Usage: lua scripts/tidy.lua [OPTIONS]
-- Run with --help for full usage information.
--
-- @copyright Copyright (C) 2026 Frederik Tobner
-- @license   GNU Affero General Public License v3.0 or later

local SCRIPT_DIR = arg[0]:match("^(.*)/[^/]+$") or "."
package.path = SCRIPT_DIR .. "/lib/?.lua;" .. package.path

local ansi      = require("ansi")
local shell     = require("shell")
local cli       = require("cli")
local project   = require("project")
local time      = require("time")
local progress  = require("progress")
local toolchain = require("toolchain")

local C           = ansi.C
local die         = cli.die
local capture     = shell.capture
local short_path  = project.short_path
local now         = time.now
local fmt_elapsed = time.fmt_elapsed

local ROOT = project.root()

-- The CI pipeline pins clang-tidy-18.  Using a different version may silence
-- or add diagnostics relative to CI, giving misleading local results.
local TOOL = toolchain.pinned_tool("clang-tidy", "18", { "17", "16" })

local USAGE = [[
Usage: lua scripts/tidy.lua [OPTIONS]

Runs clang-tidy on JANUS kernel C sources using a build directory's
compile_commands.json. Mirrors the CI static-analysis step.

Options:
  -p, --preset PRESET   CMake preset whose build dir to use (e.g. x86_64-clang)
                        Defaults to the first available *-clang build directory.
      --filter PATTERN  Only analyse files whose path matches PATTERN
  -f, --fix             Apply clang-tidy suggested fixes in-place
  -v, --verbose         Show full clang-tidy output even for passing files
  -h, --help            Print this message and exit

Exit codes:
  0   All files passed clang-tidy
  1   One or more files produced diagnostics
]]

local opts = {
    preset  = nil,
    filter  = nil,
    fix     = false,
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
        elseif a == "-f" or a == "--fix" then
            opts.fix = true
        elseif a == "-v" or a == "--verbose" then
            opts.verbose = true
        else
            die("unknown option: %s  (try --help)", a)
        end
        i = i + 1
    end
end

--- Return the build directory for a preset name.
---
--- @param preset string CMake preset name
--- @returns string build directory path
local function build_dir_for(preset)
    return ROOT .. "/build-" .. preset
end

--- Discover available *-clang build directories that contain a
--- compile_commands.json, sorted for deterministic selection.
---
--- @returns table list of build directory paths
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

--- Extract kernel .c source paths from compile_commands.json.
--- Uses Lua pattern matching to avoid a jq dependency.
---
--- @param build_dir string path to a build directory containing compile_commands.json
--- @returns table list of kernel .c source paths
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

--- Overwrite the current line with a progress indicator.
---
--- @param i     integer current file index
--- @param total integer total number of files
--- @param path  string current file path
--- @param msg   string message to display
local function status(i, total, path, msg)
    progress.status(i, total, short_path(path), msg)
end

--- Finalise a status line with a result symbol and elapsed time.
---
--- @param i       integer current file index
--- @param total   integer total number of files
--- @param path    string current file path
--- @param ok      boolean true if the file passed clang-tidy
--- @param elapsed integer elapsed seconds for this file
local function status_done(i, total, path, ok, elapsed)
    progress.status_done(i, total, short_path(path), ok, elapsed)
end

--- Print indented clang-tidy output.
---
--- @param text string clang-tidy stdout+stderr output
local function show_output(text)
    if not text or text:match("^%s*$") then return end
    for line in text:gmatch("[^\n]+") do
        io.write(string.format("  %s\n", line))
    end
    io.write("\n")
end

--- Main entry point: run clang-tidy on all kernel .c sources in the build directory.
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

        local fix_flag = opts.fix and " --fix" or ""
        local cmd    = string.format(
            "%s -p %q%s %q 2>&1", TOOL, BUILD_DIR, fix_flag, path)
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
