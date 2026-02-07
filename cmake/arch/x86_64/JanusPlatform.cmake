 # x86_64 specific flags
if(JANUS_COMPILER_CLANG)
    set(JANUS_ARCH_FLAGS
        -target x86_64-elf
        -mno-red-zone
    )
else()
    # GCC x86_64
    set(JANUS_ARCH_FLAGS
        -m64
        -march=x86-64
        -mno-red-zone
    )
endif()