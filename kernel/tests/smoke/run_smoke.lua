#!/usr/bin/env lua
-- run_smoke.lua — boot a JANUS ISO in QEMU headless and assert on serial output.
--
-- This is a host-side test runner, not a kernel module: it consumes only the
-- build's output (the ISO) plus QEMU configuration and never links kernel code,
-- so it stays out of the kernel's layered dependency graph.
--
-- Wraps QEMU in timeout(1) so a hung boot cannot block forever, read the merged 
-- serial stream via io.popen line by line, -- and match each line against a 
-- profile from profiles.lua. A -pidfile lets us kill QEMU immediately on early 
-- success or a forbidden marker instead of waiting out the whole deadline.
--
-- Exit codes: 0 = pass, 1 = fail, 2 = usage error, 77 = skipped (autotools
-- convention; CTest maps it to "skipped" via SKIP_RETURN_CODE).
--
-- @copyright Copyright (C) 2026 Frederik Tobner 
-- @license   GNU Affero General Public License v3.0 or later

--- Return codes for the test runner, following CTest's SKIP_RETURN_CODE convention.
local RC_PASS, RC_FAIL, RC_SKIP, RC_USAGE = 0, 1, 77, 2

--- Resolve profiles.lua relative to this script, so `require` works regardless of
--- the caller's working directory (CMake also sets LUA_PATH, this is a fallback).
---
--- @returns string path to the directory containing this script
local function script_dir()
    local src = arg and arg[0] or ""
    return src:match("^(.*)[/\\][^/\\]*$") or "."
end
package.path = script_dir() .. "/?.lua;" .. package.path


--- Single-quote a string for safe use as one POSIX shell word.
---
--- @param str string to Single-quote
--- @returns string safely quoted for shell
local function shq(str)
    return "'" .. tostring(str):gsub("'", "'\\''") .. "'"
end

--- Normalise os.execute's return across Lua 5.1 (number) and 5.2+ (boolean).
---
--- @param a boolean or number from os.execute
--- @param _ ignored second return value from os.execute
--- @param code integer exit code from os.execute
local function ok_exit(a, _, code)
    if type(a) == "boolean" then
        return a
    end
    return a == 0 or code == 0
end

--- Is `cmd` runnable (a bare name on PATH or an executable path)?
---
--- @param cmd string command name or path
--- @returns boolean true if the command exists and is executable
local function command_exists(cmd)
    return ok_exit(os.execute("command -v " .. shq(cmd) .. " >/dev/null 2>&1"))
end

--- Does a readable file exist at `path`?
---
--- @param path string path to check
--- @returns boolean true if the file exists and is readable
local function file_exists(path)
    if not path or #path == 0 then
        return false
    end
    local f = io.open(path, "r")
    if f then
        f:close()
        return true
    end
    return false
end

--- Best-effort kill of the QEMU process named in `pidfile`, then remove it.
---
--- @param pidfile string path to a QEMU -pidfile
local function kill_pidfile(pidfile)
    local f = io.open(pidfile, "r")
    if not f then
        return
    end
    local pid = f:read("*n")
    f:close()
    if pid then
        os.execute("kill " .. math.floor(pid) .. " >/dev/null 2>&1")
    end
    os.remove(pidfile)
end

--- Print usage message and exit with RC_USAGE.
---
--- @param msg optional string to prefix the usage message with
local function usage(msg)
    if msg then
        io.stderr:write("run_smoke.lua: ", msg, "\n")
    end
    io.stderr:write(
        "usage: lua run_smoke.lua --qemu <bin> --iso <path> --profile <nominal|fault|kmalloc>\n",
        "                        [--machine \"<flags>\"] [--bios <path>]\n",
        "                        [--timeout <seconds>] [--memory <size>] [--log <path>]\n"
    )
    os.exit(RC_USAGE)
end

--- Parse command-line arguments into a table, with defaults and validation.
--- 
--- @param argv array of strings, typically `arg` from the script's main chunk
--- @returns table with keys: qemu, iso, profile, machine, bios, timeout, memory, log
local function parse_args(argv)
    local a = { machine = "", timeout = "30", memory = "256M" }
    local i = 1
    while argv[i] do
        local key = argv[i]
        local val = argv[i + 1]
        if key == "--qemu" then
            a.qemu = val
        elseif key == "--iso" then
            a.iso = val
        elseif key == "--profile" then
            a.profile = val
        elseif key == "--machine" then
            a.machine = val
        elseif key == "--bios" then
            a.bios = val
        elseif key == "--timeout" then
            a.timeout = val
        elseif key == "--memory" then
            a.memory = val
        elseif key == "--log" then
            a.log = val
        else
            usage("unknown argument: " .. tostring(key))
        end
        if val == nil then
            usage("missing value for " .. tostring(key))
        end
        i = i + 2
    end

    if not a.qemu then
        usage("--qemu is required")
    end
    if not a.iso then
        usage("--iso is required")
    end
    if a.profile ~= "nominal" and a.profile ~= "fault" and a.profile ~= "kmalloc" then
        usage("--profile must be 'nominal', 'fault' or 'kmalloc'")
    end
    return a
end

--- Assemble the timeout(1)-wrapped QEMU command line (stderr merged for popen).
---
--- @param a table of parsed arguments
--- @param pidfile string path to a temporary file for QEMU to write its PID
local function build_command(a, pidfile)
    local parts = {
        "timeout", "-k", "5", tostring(a.timeout),
        shq(a.qemu),
        a.machine, -- arch flags passed through verbatim (already shell words)
        "-cdrom", shq(a.iso),
        "-boot", "d",
        "-m", a.memory,
        "-serial", "file:/dev/stdout", -- kernel console → the pipe we read
        "-display", "none", -- headless
        "-monitor", "none", -- keep the monitor off our stdio
        "-no-reboot", "-no-shutdown", -- a reset ends the run; no reboot spin
        "-L", "pc-bios", -- QEMU's default BIOS search path
        "-pidfile", shq(pidfile),
    }
    if a.bios and #a.bios > 0 then
        parts[#parts + 1] = "-bios"
        parts[#parts + 1] = shq(a.bios)
    end
    return table.concat(parts, " ") .. " 2>&1"
end

--- Return a comma-separated string of the labels of required markers that were not seen.
---
--- @param required array of matcher tables (from profiles.lua)
--- @param seen array of booleans, indexed by the same order as `required`
local function missing_labels(required, seen)
    local out = {}
    for i, r in ipairs(required) do
        if not seen[i] then
            out[#out + 1] = r.label
        end
    end
    return table.concat(out, ", ")
end

--- Dump the captured serial output to stderr, bracketed by markers.
---
--- @param captured array of strings, each a line of serial output
local function dump(captured)
    io.stderr:write("--- captured serial output ---\n")
    for _, line in ipairs(captured) do
        io.stderr:write(line, "\n")
    end
    io.stderr:write("--- end serial output ---\n")
end

--- Main test runner: launch QEMU, read serial output, and assert against the profile.
---
--- @param a table of parsed arguments
--- @returns rc integer exit code, msg string, captured array of serial lines
local function run(a)
    -- Skip (rather than fail) when the environment cannot run the test at all.
    if not command_exists(a.qemu) then
        return RC_SKIP, "SKIP: QEMU not available (" .. a.qemu .. ")"
    end
    if a.bios and #a.bios > 0 and not file_exists(a.bios) then
        return RC_SKIP, "SKIP: firmware not found (" .. a.bios .. ")"
    end

    local profile = require("profiles")[a.profile]
    local pidfile = os.tmpname()
    local logf = a.log and io.open(a.log, "w") or nil

    local command = build_command(a, pidfile)
    if logf then
        logf:write("cprofile: ", a.profile, "\n")
        logf:write("command: ", command, "\n")
        logf:flush()
    end


    local pipe = io.popen(build_command(a, pidfile), "r")
    if not pipe then
        return RC_FAIL, "FAIL: could not launch QEMU"
    end

    local captured = {}
    local seen = {}
    local seen_n = 0

    for line in pipe:lines() do
        captured[#captured + 1] = line
        if logf then
            logf:write(line, "\n")
        end

        for _, f in ipairs(profile.forbidden) do
            if f:matches(line) then
                kill_pidfile(pidfile)
                pipe:close()
                if logf then
                    logf:close()
                end
                return RC_FAIL, "FAIL: forbidden marker seen: " .. f.label, captured
            end
        end

        for i, r in ipairs(profile.required) do
            if not seen[i] and r:matches(line) then
                seen[i] = true
                seen_n = seen_n + 1
            end
        end

        if seen_n == #profile.required then
            kill_pidfile(pidfile) -- all milestones hit → PASS early
            pipe:close()
            if logf then
                logf:close()
            end
            return RC_PASS
        end
    end

    -- EOF: QEMU exited on its own or timeout(1) killed it at the deadline.
    kill_pidfile(pidfile)
    pipe:close()
    if logf then
        logf:close()
    end
    return RC_FAIL, "FAIL: deadline/exit with missing markers: " .. missing_labels(profile.required, seen), captured
end

-- Main entry point: parse args, run the test, report results, and exit with the right code.
local a = parse_args(arg)
local rc, msg, captured = run(a)
if msg then
    io.stderr:write(msg, "\n")
end
if rc == RC_FAIL and captured then
    dump(captured)
end
if rc == RC_PASS then
    print("PASS: smoke/" .. a.profile)
end
os.exit(rc)
