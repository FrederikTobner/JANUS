--- In-place progress-line reporting shared by the JANUS Lua scripts.
--
-- @copyright Copyright (C) 2026 Frederik Tobner
-- @license   GNU Affero General Public License v3.0 or later

local ansi = require("ansi")
local time = require("time")

local C = ansi.C

local M = {}

--- Overwrite the current line with a progress indicator.
---
--- @param index integer current item index (1-based)
--- @param total integer total number of items
--- @param name  string name of the item currently being processed
--- @param msg   string message to display
function M.status(index, total, name, msg)
    local prefix = string.format("[%d/%d]", index, total)
    io.write(string.format("\r\27[K%s%s%s %s%s%s: %s",
        C.bold, prefix, C.reset,
        C.cyan, name, C.reset,
        msg))
    io.flush()
end

--- Finish a status line with a result symbol and elapsed time.
---
--- @param index   integer current item index (1-based)
--- @param total   integer total number of items
--- @param name    string name of the item that was processed
--- @param ok      boolean true if the item succeeded
--- @param elapsed number seconds elapsed for this item
function M.status_done(index, total, name, ok, elapsed)
    local sym = ok and (C.green .. "✓") or (C.red .. "✗")
    -- Clear the entire line first, then write the final result
    io.write(string.format("\r\27[K%s[%d/%d]%s %s%s%s %s%s  %s%s%s\n",
        C.bold, index, total, C.reset,
        C.cyan, name, C.reset,
        sym, C.reset,
        C.dim, time.fmt_elapsed(elapsed), C.reset))
    io.flush()
end

return M
