#!/usr/bin/env lua
--- Pre-commit verification script for JANUS.
--
-- Builds the kernel across all CMake presets (architecture × compiler)
-- and creates ISO images via the umbrella 'iso' target.
--
-- Usage: lua scripts/regression_check.lua [OPTIONS]
-- Run with --help for full usage information.
--
-- @copyright Copyright (C) 2026 Frederik, TinyOS contributors
-- @license   GNU Affero General Public License v3.0 or later

-- Colours (disabled when stdout is not a tty)
local _tty_raw    = os.execute("test -t 1") -- luacheck: ignore
-- Normalise across Lua versions: 5.3+ returns true/false; 5.1/5.2 returns 0/non-zero
local use_colour  = (_tty_raw == true) or (_tty_raw == 0)

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

--- Print an error message and exit.
local function die(fmt, ...)
    io.stderr:write(string.format("%serror:%s " .. fmt .. "\n", C.red, C.reset, ...))
    os.exit(1)
end

--- Execute a shell command, return (ok, exit_code).
local function exec(cmd)
    local ok, _, code = os.execute(cmd)
    if ok == true then return true, 0 end    -- Lua 5.3+: boolean true on success
    if type(ok) == "number" then             -- Lua 5.1/5.2: numeric 0 on success
        return ok == 0, ok
    end
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

--- Split a string on a separator pattern.  Returns a list (table).
local function split(str, sep)
    local parts = {}
    for part in str:gmatch("([^" .. sep .. "]+)") do
        parts[#parts + 1] = part
    end
    return parts
end

--- Check if a value exists in a list.  Empty list means "accept all".
local function passes_filter(value, filter)
    if #filter == 0 then return true end
    for _, v in ipairs(filter) do
        if v == value then return true end
    end
    return false
end

--- Get a monotonic-ish timestamp in seconds (os.clock = CPU time, but
--- for wall-clock we use os.time which has 1 s resolution — good enough).
local function now()
    return os.time()
end

--- Format elapsed seconds as a human string.
local function fmt_elapsed(seconds)
    if seconds < 60 then
        return string.format("%ds", seconds)
    end
    return string.format("%dm%02ds", math.floor(seconds / 60), seconds % 60)
end

--- Return the number of CPUs (nproc equivalent).
local function nproc()
    local n = capture("nproc 2>/dev/null")
    if n then
        n = tonumber(n:match("%d+"))
        if n then return n end
    end
    return 4  -- safe fallback
end

--- Resolve the project root from the location of this script.
-- The script lives at <root>/scripts/regression_check.lua, so the root
-- is the parent of the directory that contains this file.
local ROOT = (function()
    local abs = capture(string.format("realpath %q 2>/dev/null", arg[0]))
    if not abs then
        die("cannot resolve script path from '%s'", arg[0])
    end
    abs = abs:match("^%s*(.-)%s*$")
    local root = abs:match("^(.*)/[^/]+/[^/]+$")  -- strip /scripts/<name>.lua
    if not root or root == "" then
        die("cannot derive project root from script path '%s'", abs)
    end
    return root
end)()

local USAGE = [[
Usage: lua scripts/regression_check.lua [OPTIONS]

Builds the JANUS kernel and ISO images across all CMake presets.

Options:
  -a, --arch ARCH,...       Filter presets by architecture (default: all)
  -c, --compiler CC,...     Filter presets by compiler     (default: all)
  -j, --jobs N              Parallel jobs per build        (default: nproc)
      --configure-only      Only configure, skip build
      --clean               Remove build dirs before configuring
  -v, --verbose             Show full build output
  -h, --help                Print this message and exit
]]

local opts = {
    arch_filter     = {},
    compiler_filter = {},
    jobs            = nproc(),
    configure_only  = false,
    clean           = false,
    verbose         = false,
}

do
    local i = 1
    while i <= #arg do
        local a = arg[i]
        if a == "-h" or a == "--help" then
            io.write(USAGE)
            os.exit(0)
        elseif a == "-a" or a == "--arch" then
            i = i + 1
            if not arg[i] then die("--arch requires an argument") end
            opts.arch_filter = split(arg[i], ",")
        elseif a == "-c" or a == "--compiler" then
            i = i + 1
            if not arg[i] then die("--compiler requires an argument") end
            opts.compiler_filter = split(arg[i], ",")
        elseif a == "-j" or a == "--jobs" then
            i = i + 1
            local n = tonumber(arg[i])
            if not n or n < 1 then die("--jobs requires a positive integer") end
            opts.jobs = math.floor(n)
        elseif a == "--configure-only" then
            opts.configure_only = true
        elseif a == "--clean" then
            opts.clean = true
        elseif a == "-v" or a == "--verbose" then
            opts.verbose = true
        else
            die("unknown option: %s  (try --help)", a)
        end
        i = i + 1
    end
end

--- Discover all configure preset names from CMakePresets.json.
local function discover_presets()
    local raw = capture(string.format("cd %q && cmake --list-presets=configure 2>/dev/null", ROOT))
    if not raw then die("cmake --list-presets failed — is cmake installed?") end

    local presets = {}
    local skip_header = true
    for line in raw:gmatch("([^\n]+)") do
        -- Skip the header lines (typically 2: blank + "Available ...")
        if skip_header then
            if line:match("^%s+%S") then
                skip_header = false
            end
        end
        if not skip_header then
            local name = line:match("^%s+\"?([%w_%-]+)\"?")
            if name then
                presets[#presets + 1] = name
            end
        end
    end

    if #presets == 0 then
        die("no configure presets found in CMakePresets.json")
    end
    return presets
end

--- Extract architecture and compiler from a preset name like "x86_64-gcc".
local function parse_preset(name)
    local arch, compiler = name:match("^(.-)%-(.+)$")
    if not arch then
        die("preset name '%s' does not match <arch>-<compiler> pattern", name)
    end
    return arch, compiler
end

--- Filter the preset list by the user's --arch / --compiler flags.
local function filter_presets(all_presets)
    local result = {}
    for _, p in ipairs(all_presets) do
        local arch, compiler = parse_preset(p)
        if passes_filter(arch, opts.arch_filter)
        and passes_filter(compiler, opts.compiler_filter) then
            result[#result + 1] = p
        end
    end

    if #result == 0 then
        -- List available values for a helpful error
        local archs, compilers = {}, {}
        for _, p in ipairs(all_presets) do
            local a, c = parse_preset(p)
            archs[a] = true
            compilers[c] = true
        end
        local a_list, c_list = {}, {}
        for k in pairs(archs)     do a_list[#a_list + 1] = k end
        for k in pairs(compilers) do c_list[#c_list + 1] = k end
        die("no presets matched filters\n  available architectures: %s\n  available compilers:     %s",
            table.concat(a_list, ", "), table.concat(c_list, ", "))
    end
    return result
end

--- Status line helper — overwrites the current line.
local function status(index, total, preset, msg)
    local prefix = string.format("[%d/%d]", index, total)
    io.write(string.format("\r\27[K%s%s%s %s%s%s: %s",
        C.bold, prefix, C.reset,
        C.cyan, preset, C.reset,
        msg))
    io.flush()
end

--- Finish a status line with a result symbol.
local function status_done(index, total, preset, ok, elapsed)
    local sym = ok and (C.green .. "✓") or (C.red .. "✗")
    -- Clear the entire line first, then write the final result
    io.write(string.format("\r\27[K%s[%d/%d]%s %s%s%s %s%s  %s%s%s\n",
        C.bold, index, total, C.reset,
        C.cyan, preset, C.reset,
        sym, C.reset,
        C.dim, fmt_elapsed(elapsed), C.reset))
    io.flush()
end

--- Show the tail of a log file on failure.
local function show_log_tail(log_path, lines)
    lines = lines or 30
    local tail = capture(string.format("tail -n %d %q 2>/dev/null", lines, log_path))
    if tail and tail ~= "" then
        io.write(string.format("\n%s── last %d lines of %s ──%s\n",
            C.dim, lines, log_path, C.reset))
        io.write(tail)
        io.write(string.format("%s── end ──%s\n\n", C.dim, C.reset))
    end
end

--- Run the configure phase for a preset.  Returns true on success.
local function phase_configure(preset, build_dir, log_path)
    if opts.clean then
        os.execute(string.format("rm -rf %q", build_dir))
    end

    local cmd = string.format("cd %q && cmake --preset %s >>%q 2>&1", ROOT, preset, log_path)
    local ok, _ = exec(cmd)
    return ok
end

--- Run a single ninja build target.  Returns true on success.
local function phase_build(build_dir, target, log_path)
    local quiet = opts.verbose and "" or " --quiet"
    local cmd = string.format(
        "ninja -C %s %s -j%d%s >>%q 2>&1",
        build_dir, target, opts.jobs, quiet, log_path)
    local ok, _ = exec(cmd)
    return ok
end

local results = {}

local function print_summary()
    -- Column widths
    local col_preset = 7  -- "Preset" header
    for _, r in ipairs(results) do
        if #r.preset > col_preset then col_preset = #r.preset end
    end

    local col_iso  = 3  -- "ISO" header
    local col_time = 4  -- "Time" header

    -- Separator
    local total_width = col_preset + 2 + col_iso + 2 + col_time + 2
    local sep = string.rep("─", total_width)

    -- Header
    io.write("\n" .. sep .. "\n")
    io.write(string.format(" %-" .. col_preset .. "s  %-" .. col_iso .. "s  %-" .. col_time .. "s\n",
        "Preset", "ISO", "Time"))
    io.write(sep .. "\n")

    -- Rows
    local fail_count = 0
    for _, r in ipairs(results) do
        local row = string.format(" %-" .. col_preset .. "s", r.preset)
        local preset_ok = r.configure

        -- ISO column
        local cell
        if r.iso == nil then
            cell = C.dim .. "—" .. C.reset
        elseif r.iso then
            cell = C.green .. "✓" .. C.reset
        else
            cell = C.red .. "✗" .. C.reset
            preset_ok = false
        end
        if not r.configure then preset_ok = false end
        if not preset_ok then fail_count = fail_count + 1 end

        row = row .. "  " .. cell .. string.rep(" ", col_iso - 1)
        row = row .. string.format("  %s%s%s", C.dim, fmt_elapsed(r.elapsed), C.reset)
        io.write(row .. "\n")
    end

    io.write(sep .. "\n")
    if fail_count == 0 then
        io.write(string.format(" %s%d of %d presets passed%s\n\n",
            C.green, #results, #results, C.reset))
    else
        io.write(string.format(" %s%d of %d presets failed%s\n\n",
            C.red, fail_count, #results, C.reset))
    end

    return fail_count
end

local function main()
    io.write(string.format("%s── JANUS pre-commit check ──%s\n\n", C.bold, C.reset))

    -- Discover and filter presets
    local all_presets = discover_presets()
    local presets = filter_presets(all_presets)

    io.write(string.format("Presets: %s%s%s  |  Jobs: %d\n\n",
        C.cyan, table.concat(presets, ", "), C.reset, opts.jobs))

    -- Process each preset
    for i, preset in ipairs(presets) do
        local build_dir = ROOT .. "/build-" .. preset
        local log_path  = build_dir .. "/regression_check.log"

        -- Clear log
        os.execute(string.format("mkdir -p %q && : > %q", build_dir, log_path))

        local t_start = now()
        local r = { preset = preset, elapsed = 0, configure = false, iso = nil }

        -- Phase 1: Configure
        status(i, #presets, preset, "configuring...")
        local conf_ok = phase_configure(preset, build_dir, log_path)
        r.configure = conf_ok

        if not conf_ok then
            r.elapsed = now() - t_start
            status_done(i, #presets, preset, false, r.elapsed)
            show_log_tail(log_path)
            results[#results + 1] = r
            goto continue
        end

        if opts.configure_only then
            r.elapsed = now() - t_start
            status_done(i, #presets, preset, true, r.elapsed)
            results[#results + 1] = r
            goto continue
        end

        -- Phase 2: Build all ISOs via umbrella target
        status(i, #presets, preset, "building iso...")
        local ok = phase_build(build_dir, "iso", log_path)
        r.iso = ok

        r.elapsed = now() - t_start
        status_done(i, #presets, preset, ok, r.elapsed)
        if not ok then
            show_log_tail(log_path)
        end

        results[#results + 1] = r
        ::continue::
    end

    -- Summary
    local fail_count = print_summary()
    os.exit(fail_count == 0 and 0 or 1)
end

main()
