--- CLI diagnostics helpers shared by the JANUS Lua scripts.
--
-- @copyright Copyright (C) 2026 Frederik Tobner
-- @license   GNU Affero General Public License v3.0 or later

local ansi = require("ansi")

local M = {}

--- Print an error message to stderr and exit with status 1.
---
--- @param fmt string printf-style format string
--- @param ... any printf-style arguments
function M.die(fmt, ...)
    io.stderr:write(string.format("%serror:%s " .. fmt .. "\n", ansi.C.red, ansi.C.reset, ...))
    os.exit(1)
end

return M
