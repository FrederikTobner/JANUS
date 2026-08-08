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

--- Resolve a CI-pinned tool version, falling back to other versions (and
--- finally the unversioned name) with a warning if the pinned version isn't
--- available locally.
---
--- @param base_name         string tool base name, e.g. "clang-tidy"
--- @param pinned_version    string version pinned by CI, e.g. "18"
--- @param fallback_versions table list of fallback version suffixes to try
--- @return string resolved tool executable name
function M.pinned_tool(base_name, pinned_version, fallback_versions)
    local ci_tool = base_name .. "-" .. pinned_version

    local tool = M.find_tool { ci_tool }
    if tool then return tool end

    local candidates = {}
    for _, v in ipairs(fallback_versions or {}) do
        candidates[#candidates + 1] = base_name .. "-" .. v
    end
    candidates[#candidates + 1] = base_name

    tool = M.find_tool(candidates)
    if not tool then
        cli.die("%s not found — install %s or add it to PATH", base_name, ci_tool)
    end

    local ver = shell.capture(tool .. " --version 2>/dev/null") or "unknown"
    ver = ver:match("^%s*(.-)%s*$")
    io.stderr:write(string.format(
        "%swarning:%s %s not found; using '%s' (%s)\n"
        .. "         Results may differ from CI (which pins %s).\n\n",
        C.yellow, C.reset, ci_tool, tool, ver, ci_tool))

    return tool
end

return M
