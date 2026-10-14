#!/usr/bin/env lua
-- run_ktest.lua — host-side runner for the optional in-kernel KTest suite.
--
-- This file is intentionally separate from the smoke harness under
-- kernel/tests/smoke. The smoke runner validates boot behaviour and serial output
-- at the ISO level, while this runner validates the kernel's runtime ktest
-- registry and final KTEST: PASS / KTEST: FAIL summary.
--
-- It is a Lua-only host-side wrapper around QEMU and exits with the same
-- conventions as the smoke runner so CTest can classify skipped cases cleanly.

local RC_PASS, RC_FAIL, RC_SKIP, RC_USAGE = 0, 1, 77, 2

local function script_dir()
    local src = arg and arg[0] or ""
    return src:match("^(.*)[/\\][^/\\]*$") or "."
end

package.path = script_dir() .. "/?.lua;" .. package.path

local function shq(str)
    return "'" .. tostring(str):gsub("'", "'\\''") .. "'"
end

local function ok_exit(a, _, code)
    if type(a) == "boolean" then
        return a
    end
    return a == 0 or code == 0
end

local function command_exists(cmd)
    return ok_exit(os.execute("command -v " .. shq(cmd) .. " >/dev/null 2>&1"))
end

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

local function usage(msg)
    if msg then
        io.stderr:write("run_ktest.lua: ", msg, "\n")
    end
    io.stderr:write(
        "usage: lua run_ktest.lua --qemu <bin> --iso <path>\n",
        "                        [--machine \"<flags>\"] [--bios <path>]\n",
        "                        [--timeout <seconds>] [--memory <size>] [--log <path>]\n"
    )
    os.exit(RC_USAGE)
end

local function parse_args(argv)
    local a = { machine = "", timeout = "60", memory = "256M" }
    local i = 1
    while argv[i] do
        local key = argv[i]
        local val = argv[i + 1]

        if key == "--qemu" then
            a.qemu = val
        elseif key == "--iso" then
            a.iso = val
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

    return a
end

local function build_command(a, pidfile)
    local parts = {
        "timeout", "-k", "5", tostring(a.timeout),
        shq(a.qemu),
        a.machine,
        "-cdrom", shq(a.iso),
        "-boot", "d",
        "-m", a.memory,
        "-serial", "file:/dev/stdout",
        "-display", "none",
        "-monitor", "none",
        "-no-reboot", "-no-shutdown",
        "-L", "pc-bios",
        "-pidfile", shq(pidfile),
    }

    if a.bios and #a.bios > 0 then
        parts[#parts + 1] = "-bios"
        parts[#parts + 1] = shq(a.bios)
    end

    return table.concat(parts, " ") .. " 2>&1"
end

local function run(a)
    if not command_exists(a.qemu) then
        return RC_SKIP, "SKIP: QEMU not available (" .. a.qemu .. ")"
    end
    if a.bios and #a.bios > 0 and not file_exists(a.bios) then
        return RC_SKIP, "SKIP: firmware not found (" .. a.bios .. ")"
    end

    local pidfile = os.tmpname()
    local logf = a.log and io.open(a.log, "w") or nil
    local command = build_command(a, pidfile)
    if logf then
        logf:write("command: ", command, "\n")
        logf:flush()
    end

    local pipe = io.popen(command, "r")
    if not pipe then
        return RC_FAIL, "FAIL: could not launch QEMU"
    end

    local captured = {}
    local saw_boot = false
    local saw_run = false
    local saw_pass = false
    local saw_fail = false
    local saw_summary = false
    local summary_run = nil
    local summary_pass = nil
    local summary_fail = nil

    for line in pipe:lines() do
        captured[#captured + 1] = line
        if logf then
            logf:write(line, "\n")
        end

        if line:find("==== ktest ====") then
            saw_boot = true
        end
        if line:match("%[%s*RUN%s*%]%s") then
            saw_run = true
        end
        if line:match("KTEST:%s+PASS") then
            saw_pass = true
        end
        if line:match("KTEST:%s+FAIL") then
            saw_fail = true
        end
        if line:match("KTEST SUMMARY:") then
            saw_summary = true
            local run_count, pass_count, fail_count = line:match(
                "KTEST SUMMARY:%s*(%d+)%s+run,%s*(%d+)%s+passed,%s*(%d+)%s+failed"
            )
            if run_count then
                summary_run = tonumber(run_count)
                summary_pass = tonumber(pass_count)
                summary_fail = tonumber(fail_count)
            end
        end
    end

    kill_pidfile(pidfile)
    if logf then
        logf:close()
    end
    pipe:close()

    if not saw_boot then
        return RC_FAIL, "FAIL: ktest banner was not seen", captured
    end
    if not saw_run then
        return RC_FAIL, "FAIL: no ktest case executed", captured
    end
    if not saw_summary then
        return RC_FAIL, "FAIL: ktest summary was not reported", captured
    end
    if not saw_pass then
        return RC_FAIL, "FAIL: final PASS marker was not seen", captured
    end
    if saw_fail then
        return RC_FAIL, "FAIL: ktest reported one or more failing cases", captured
    end
    if summary_run and summary_pass and summary_fail then
        if summary_run <= 0 then
            return RC_FAIL, "FAIL: summary reported zero tests run", captured
        end
        if summary_pass ~= summary_run or summary_fail ~= 0 then
            return RC_FAIL,
                string.format(
                    "FAIL: summary mismatch (%d run, %d passed, %d failed)",
                    summary_run,
                    summary_pass,
                    summary_fail
                ), captured
        end
    end

    return RC_PASS, "PASS: ktest completed successfully", captured
end

local a = parse_args(arg)
local code, msg, captured = run(a)
if msg then
    io.stderr:write(msg, "\n")
end
if captured and #captured > 0 then
    io.stderr:write("--- ktest serial output ---\n")
    for _, line in ipairs(captured) do
        io.stderr:write(line, "\n")
    end
    io.stderr:write("--- end serial output ---\n")
end
os.exit(code)
