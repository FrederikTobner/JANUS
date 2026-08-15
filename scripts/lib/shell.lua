--- Shell execution helpers shared by the JANUS Lua scripts.
--
-- @copyright Copyright (C) 2026 Frederik Tobner
-- @license   GNU Affero General Public License v3.0 or later

local M = {}

--- Execute a shell command, return (ok, exit_code).
---
--- @param cmd string shell command to execute
--- @return boolean ok, integer exit_code
function M.exec(cmd)
    local ok, _, code = os.execute(cmd)
    if ok == true then return true, 0 end    -- Lua 5.3+: boolean true on success
    if type(ok) == "number" then             -- Lua 5.1/5.2: numeric 0 on success
        return ok == 0, ok
    end
    return false, code or 1
end

--- Execute a command and capture stdout as a string.
---
--- @param cmd string shell command to execute
--- @return string|nil stdout output, or nil if the command could not be executed
function M.capture(cmd)
    local f = io.popen(cmd, "r")
    if not f then return nil end
    local out = f:read("*a")
    f:close()
    return out
end

return M
