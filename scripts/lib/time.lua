--- Time/duration helpers shared by the JANUS Lua scripts.
--
-- @copyright Copyright (C) 2026 Frederik Tobner
-- @license   GNU Affero General Public License v3.0 or later

local M = {}

--- Get a monotonic-ish timestamp in seconds (wall-clock, 1s resolution).
---
--- @return integer seconds since epoch
function M.now()
    return os.time()
end

--- Format elapsed seconds as a human string.
---
--- @param seconds integer elapsed seconds
--- @return string human-readable elapsed time, e.g. "12s" or "1m03s"
function M.fmt_elapsed(seconds)
    if seconds < 60 then
        return string.format("%ds", seconds)
    end
    return string.format("%dm%02ds", math.floor(seconds / 60), seconds % 60)
end

return M
