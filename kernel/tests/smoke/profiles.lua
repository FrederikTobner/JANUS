-- profiles.lua — smoke-test assertion profiles (single source of truth)
--
-- Each profile lists the serial-console markers that must appear (`required`)
-- and those that must NOT appear (`forbidden`) during a bounded QEMU boot. Keep
-- ALL marker strings here: if the kernel's console text changes, this is the one
-- file to edit.
--
-- A matcher is a table with a `.label` (for diagnostics) and a `:matches(line)`
-- method. `substr` uses a *plain* find so magic characters in markers such as
-- "*** KERNEL PANIC ***" are matched literally.

--- Build a plain-substring matcher.
-- @param s literal substring to search for on each serial line
--
-- @copyright Copyright (C) 2026 Frederik Tobner 
-- @license   GNU Affero General Public License v3.0 or later
local function substr(s)
    return {
        label = s,
        matches = function(_, line)
            return line:find(s, 1, true) ~= nil
        end,
    }
end

--- Build a numeric-threshold matcher.
-- Captures a decimal group from `pattern` and passes when it exceeds `threshold`.
-- @param pattern   Lua pattern with a single "(%d+)" capture
-- @param threshold integer the captured value must be strictly greater than
-- @param label     human-readable description for diagnostics
local function num_gt(pattern, threshold, label)
    return {
        label = label,
        matches = function(_, line)
            local n = line:match(pattern)
            return n ~= nil and tonumber(n) > threshold
        end,
    }
end

return {
    -- Default build: boots, initialises subsystems, reports PMM stats, halts.
    nominal = {
        required = {
            substr("Version:"),
            num_gt("Physical Memory Manager:%s+(%d+)%s+MiB free", 0, "PMM free MiB > 0"),
        },
        forbidden = {
            substr("*** KERNEL PANIC ***"),
            substr("*** CPU EXCEPTION ***"),
        },
        timeout = 30,
    },

    -- JANUS_TEST_FAULTS build (kind 0): the deliberate fault must be trapped and
    -- escalate through the exception reporter to a clean, halting panic.
    fault = {
        required = {
            substr("[fault-test] triggering deliberate fault"),
            substr("*** CPU EXCEPTION ***"),
            substr("*** KERNEL PANIC ***"),
            substr("System halted."),
        },
        forbidden = {}, -- a panic IS the expected outcome here
        timeout = 30,
    },
}
