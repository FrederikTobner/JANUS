--- Project-root resolution shared by the JANUS Lua scripts.
--
-- @copyright Copyright (C) 2026 Frederik Tobner
-- @license   GNU Affero General Public License v3.0 or later

local shell = require("shell")
local cli   = require("cli")

local M = {}

local cached_root

--- Resolve the project root from the location of the running entry script.
-- The entry script lives at <root>/scripts/<name>.lua, so the root is the
-- parent of the directory that contains the script.
---
--- @return string absolute path to the project root
function M.root()
    if cached_root then return cached_root end

    local abs = shell.capture(string.format("realpath %q 2>/dev/null", arg[0]))
    if not abs then
        cli.die("cannot resolve script path from '%s'", arg[0])
    end
    abs = abs:match("^%s*(.-)%s*$")
    local root = abs:match("^(.*)/[^/]+/[^/]+$")  -- strip /scripts/<name>.lua
    if not root or root == "" then
        cli.die("cannot derive project root from script path '%s'", abs)
    end

    cached_root = root
    return root
end

--- Strip the project root prefix from a path for readable output.
---
--- @param path string absolute path
--- @return string path relative to the project root
function M.short_path(path)
    return (path:gsub("^" .. M.root() .. "/", ""))
end

return M
