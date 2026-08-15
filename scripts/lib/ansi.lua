--- ANSI colour helpers shared by the JANUS Lua scripts.
--
-- @copyright Copyright (C) 2026 Frederik Tobner
-- @license   GNU Affero General Public License v3.0 or later

local M = {}

-- Colours are disabled when stdout is not a tty.
local _tty_raw = os.execute("test -t 1") -- luacheck: ignore
-- Normalise across Lua versions: 5.3+ returns true/false; 5.1/5.2 returns 0/non-zero
M.use_colour = (_tty_raw == true) or (_tty_raw == 0)

--- Return an ANSI SGR escape sequence for a given code, or an empty string
--- if colour output is disabled.
---
--- @param code string ANSI SGR code (e.g. "31" for red)
--- @return string ANSI escape sequence or empty string
function M.sgr(code)
    if M.use_colour then return string.format("\27[%sm", code) end
    return ""
end

M.C = {
    reset  = M.sgr("0"),
    bold   = M.sgr("1"),
    red    = M.sgr("31"),
    green  = M.sgr("32"),
    yellow = M.sgr("33"),
    cyan   = M.sgr("36"),
    dim    = M.sgr("2"),
}

return M
