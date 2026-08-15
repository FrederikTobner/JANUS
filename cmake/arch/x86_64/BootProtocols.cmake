#[[
    BootProtocols.cmake — x86_64 boot-protocol support

    Declares which boot protocols this architecture supports. 
]]

include_guard(GLOBAL)

set(JANUS_BOOT_PROTOCOLS "multiboot2;limine" CACHE STRING
    "Boot protocols to support (semicolon-separated)")
