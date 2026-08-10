#[[
    BootProtocols.cmake — aarch64 boot-protocol support

    Declares which boot protocols this architecture supports. 
]]

include_guard(GLOBAL)

set(JANUS_BOOT_PROTOCOLS "limine" CACHE STRING
    "Boot protocols (aarch64: limine only)" FORCE)
