#!/usr/bin/env lua
--- clang-format runner for JANUS.
--
-- Checks or fixes the formatting of all kernel C/H sources against the
-- project's .clang-format style file.
--
-- Usage: lua scripts/format.lua [OPTIONS]
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

--- Execute a command and capture stdout as a string.
local function capture(cmd)
    local f = io.popen(cmd, "r")
    if not f then return nil end
    local out = f:read("*a")
    f:close()
    return out
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

-- The CI pipeline pins clang-format-18.  Using a different version can produce
-- different results (compound-literal spacing, brace-init rules, etc.), giving
-- a false pass locally while CI still fails.
local CI_TOOL = "clang-format-18"

local function find_tool(candidates)
    for _, name in ipairs(candidates) do
        local out = capture(string.format("command -v %s 2>/dev/null", name))
        if out and out:match("%S") then return name end
    end
    return nil
end

local TOOL = find_tool { CI_TOOL }
if not TOOL then
    -- Fall back to whatever is available, but warn loudly.
    TOOL = find_tool { "clang-format-17", "clang-format-16", "clang-format" }
    if not TOOL then
        die("clang-format not found — install clang-format-18 or add it to PATH")
    end
    local ver = capture(TOOL .. " --version 2>/dev/null") or "unknown"
    ver = ver:match("^%s*(.-)%s*$")
    io.stderr:write(string.format(
        "%swarning:%s %s not found; using '%s' (%s)\n"
        .. "         Results may differ from CI (which pins %s).\n\n",
        C.yellow, C.reset, CI_TOOL, TOOL, ver, CI_TOOL))
end

-- ─── Options ──────────────────────────────────────────────────────────────────

local USAGE = [[
Usage: lua scripts/format.lua [OPTIONS]

Checks or fixes the formatting of JANUS kernel sources against .clang-format.

Options:
  -f, --fix             Reformat files in-place (default: check only)
      --filter PATTERN  Only process files whose path matches PATTERN
  -v, --verbose         Print every file, not just failures / fixes
  -h, --help            Print this message and exit

Exit codes:
  0   All files are correctly formatted (check) or all done (fix)
  1   One or more files need formatting (check mode only)
]]

local opts = {
    fix     = false,
    filter  = nil,
    verbose = false,
}

do
    local i = 1
    while i <= #arg do
        local a = arg[i]
        if a == "-h" or a == "--help" then
            io.write(USAGE); os.exit(0)
        elseif a == "-f" or a == "--fix" then
            opts.fix = true
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

-- ─── File discovery ───────────────────────────────────────────────────────────

local function find_sources()
    local raw = capture(string.format(
        "find %q/kernel -type f \\( -name '*.c' -o -name '*.h' \\) | sort",
        ROOT))
    if not raw or raw == "" then
        die("no C/H sources found under %s/kernel", ROOT)
    end
    local files = {}
    for path in raw:gmatch("[^\n]+") do
        if not opts.filter or path:match(opts.filter) then
            files[#files + 1] = path
        end
    end
    if #files == 0 then
        die("no files matched filter '%s'", opts.filter or "(none)")
    end
    return files
end

-- ─── Core operations ──────────────────────────────────────────────────────────

--- Returns true if the file is already correctly formatted.
local function is_clean(path)
    local ok, _ = exec(string.format(
        "%s --dry-run --Werror %q 2>/dev/null", TOOL, path))
    return ok
end

--- Reformat a file in-place.
local function fix_file(path)
    exec(string.format("%s -i %q", TOOL, path))
end

-- ─── Progress helpers ─────────────────────────────────────────────────────────

--- Strip ROOT prefix for readable output.
local function short_path(path)
    return path:gsub("^" .. ROOT .. "/", "")
end

local function print_file_result(i, total, path, label)
    io.write(string.format("[%d/%d] %-58s %s\n",
        i, total, short_path(path), label))
    io.flush()
end

-- ─── Main ─────────────────────────────────────────────────────────────────────

local function main()
    local mode_label = opts.fix and "fix" or "check"
    io.write(string.format("%s── JANUS format %s ──%s  (%s)\n\n",
        C.bold, mode_label, C.reset, TOOL))

    local files = find_sources()
    local n           = #files
    local dirty_count = 0
    local fixed_count = 0
    local clean_count = 0

    for i, path in ipairs(files) do
        local clean = is_clean(path)
        if clean then
            clean_count = clean_count + 1
            if opts.verbose then
                print_file_result(i, n, path, C.green .. "✓" .. C.reset)
            end
        else
            dirty_count = dirty_count + 1
            if opts.fix then
                fix_file(path)
                fixed_count = fixed_count + 1
                print_file_result(i, n, path, C.yellow .. "fixed" .. C.reset)
            else
                print_file_result(i, n, path,
                    C.red .. "✗  needs formatting" .. C.reset)
            end
        end
    end

    -- Summary
    io.write("\n")
    local sep = string.rep("─", 52)
    io.write(sep .. "\n")
    if opts.fix then
        if fixed_count == 0 then
            io.write(string.format(" %sAll %d files already correctly formatted%s\n",
                C.green, n, C.reset))
        else
            io.write(string.format(
                " %sFixed %d of %d files%s  (%d already correct)\n",
                C.yellow, fixed_count, n, C.reset, clean_count))
        end
    else
        if dirty_count == 0 then
            io.write(string.format(
                " %sAll %d files correctly formatted%s\n",
                C.green, n, C.reset))
        else
            io.write(string.format(
                " %s%d of %d files need formatting%s\n",
                C.red, dirty_count, n, C.reset))
            io.write(string.format(
                " %sRun with --fix to apply changes%s\n",
                C.dim, C.reset))
        end
    end
    io.write(sep .. "\n")

    os.exit((not opts.fix and dirty_count > 0) and 1 or 0)
end

main()
