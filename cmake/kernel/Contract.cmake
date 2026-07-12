# JANUS Contract Layer
#
# A "contract" is a type definition that must be shared across subsystem
# boundaries. Unlike the global kernel/include layer (universally-needed
# primitives only) or subsystem headers (private to one subsystem), a
# contract has exactly N named consumers.
#
# Headers live in  kernel/subsys/contracts/<name>/include/contracts/<name>.h
# and are NOT in any default include search path.  Only listed consumers
# receive the include directory; any other target that tries to link the
# generated INTERFACE target is rejected at configure time by
# janus_validate_contracts().
#
# Usage (in kernel/subsys/contracts/CMakeLists.txt):
#
#   janus_add_contract(memmap CONSUMERS boot mm)
#
# Each listed consumer must call
#
#   target_link_libraries(<target> PUBLIC janus_contract_<name>)
#
# in its own CMakeLists.txt.  This is what janus_validate_contracts()
# verifies.

include_guard(GLOBAL)

set(JANUS_CONTRACT_NAMES "" CACHE INTERNAL "All registered contract names" FORCE)

# ---------------------------------------------------------------------------
# janus_add_contract(NAME CONSUMERS consumer1 [consumer2 ...])
#
# Creates an INTERFACE library  janus_contract_<NAME>  that exposes
#   kernel/contracts/<NAME>/include
# as its sole include root.
#
# CONSUMERS is the exhaustive allowlist of CMake targets that may directly
# link this contract target.  janus_validate_contracts() enforces the list.
# ---------------------------------------------------------------------------
function(janus_add_contract NAME)
    cmake_parse_arguments(ARG "" "" "CONSUMERS" ${ARGN})

    if(NOT ARG_CONSUMERS)
        message(FATAL_ERROR "janus_add_contract(${NAME}): CONSUMERS list is empty — every contract must have at least one consumer")
    endif()

    set(CONTRACT_TARGET  "janus_contract_${NAME}")
    set(CONTRACT_INCLUDE "${CMAKE_SOURCE_DIR}/kernel/subsys/contracts/${NAME}/include")

    if(NOT EXISTS "${CONTRACT_INCLUDE}")
        message(FATAL_ERROR
            "janus_add_contract(${NAME}): include directory does not exist:\n"
            "  ${CONTRACT_INCLUDE}")
    endif()

    add_library(${CONTRACT_TARGET} INTERFACE)
    target_include_directories(${CONTRACT_TARGET} INTERFACE "${CONTRACT_INCLUDE}")

    set(JANUS_CONTRACT_NAMES
        "${JANUS_CONTRACT_NAMES};${NAME}"
        CACHE INTERNAL "All registered contract names" FORCE)
    set("JANUS_CONTRACT_CONSUMERS_${NAME}"
        "${ARG_CONSUMERS}"
        CACHE INTERNAL "Allowed direct consumers of contract ${NAME}" FORCE)

    # Register in the global registry so janus_write_mermaid_diagram picks it up.
    # DEPENDENCIES is empty — edges go FROM consumers TO this node, not the reverse.
    janus_register("${CONTRACT_TARGET}" CONTRACT "")
    # Store the short name (e.g. "memmap") as the diagram label.
    set("JANUS_CONTRACT_LABEL_${CONTRACT_TARGET}" "${NAME}"
        CACHE INTERNAL "Short diagram label for ${CONTRACT_TARGET}" FORCE)

    message(STATUS "Contract '${NAME}' registered — consumers: ${ARG_CONSUMERS}")
endfunction()

# ---------------------------------------------------------------------------
# janus_validate_contracts()
#
# Call after all CMake targets are defined (i.e. after all add_subdirectory
# calls in kernel/CMakeLists.txt).
#
# For each registered contract, inspects the LINK_LIBRARIES property of
# every target in the JANUS registry.  A target that directly links
# janus_contract_<N> but is not in the CONSUMERS list for contract <N>
# triggers a fatal configure-time error.
#
# Note: targets that only receive the contract headers *transitively*
# (because a listed consumer links with PUBLIC visibility) are not
# flagged — only direct linkage is validated.
# ---------------------------------------------------------------------------
function(janus_validate_contracts)
    if(NOT JANUS_CONTRACT_NAMES)
        return()
    endif()

    message(STATUS "Validating contract consumers...")

    foreach(contract ${JANUS_CONTRACT_NAMES})
        set(contract_target "janus_contract_${contract}")
        set(allowed "${JANUS_CONTRACT_CONSUMERS_${contract}}")

        foreach(name ${JANUS_REG_KERNEL_NAMES})
            if(NOT TARGET ${name})
                continue()
            endif()

            # LINK_LIBRARIES holds *direct* (private + public) dependencies of
            # STATIC/SHARED targets.  INTERFACE targets use INTERFACE_LINK_LIBRARIES.
            get_target_property(_link_libs  ${name} LINK_LIBRARIES)
            get_target_property(_iface_libs ${name} INTERFACE_LINK_LIBRARIES)

            set(_direct "")
            if(_link_libs)
                list(APPEND _direct ${_link_libs})
            endif()
            if(_iface_libs)
                list(APPEND _direct ${_iface_libs})
            endif()

            if(contract_target IN_LIST _direct)
                if(NOT name IN_LIST allowed)
                    message(FATAL_ERROR
                        "CONTRACT VIOLATION: Target '${name}' directly links "
                        "contract '${contract}' (janus_contract_${contract}) "
                        "but is not in the allowed consumers list: [${allowed}]. "
                        "Add '${name}' to the CONSUMERS list in "
                        "kernel/subsys/contracts/CMakeLists.txt if this dependency is "
                        "intentional.")
                endif()
            endif()
        endforeach()
    endforeach()
endfunction()
