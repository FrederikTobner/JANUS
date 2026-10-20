--- Toolchain discovery helpers shared by the JANUS Lua scripts.
--
-- @copyright Copyright (C) 2026 Frederik Tobner
-- @license   GNU Affero General Public License v3.0 or later

local ansi  = require("ansi")
local shell = require("shell")
local cli   = require("cli")

local C = ansi.C

local M = {}

--- Find the first executable tool from a list of candidates.
---
--- @param candidates table list of tool names to search for
--- @return string|nil first found tool name, or nil if none found
function M.find_tool(candidates)
    for _, name in ipairs(candidates) do
        local out = shell.capture(string.format("command -v %s 2>/dev/null", name))
        if out and out:match("%S") then return name end
    end
    return nil
end

--- Resolve a tool at the exact version CI pins, warning if only another
--- version is available locally.
---
--- @param base_name      string tool base name, e.g. "clang-tidy"
--- @param pinned_version string exact version pinned by CI, e.g. "18.1.8"
--- @return string resolved tool executable name
function M.pinned_tool(base_name, pinned_version)
    local major = pinned_version:match("^(%d+)")
    local found

    for _, name in ipairs { base_name .. "-" .. major, base_name } do
        if M.find_tool { name } then
            local ver = shell.capture(name .. " --version 2>/dev/null") or ""
            if ver:find(pinned_version, 1, true) then return name end
            found = found or { name = name, ver = ver:match("^%s*(.-)%s*$") }
        end
    end

    local spec = base_name .. "==" .. pinned_version
    if not found then
        cli.die("%s not found — install it with: pipx install %s", base_name, spec)
    end

    io.stderr:write(string.format(
        "%swarning:%s %s %s not found; using '%s' (%s)\n"
        .. "         Results may differ from CI. Install the pinned version with: pipx install %s\n\n",
        C.yellow, C.reset, base_name, pinned_version, found.name, found.ver:match("[^\n]*"), spec))

    return found.name
end

return M
