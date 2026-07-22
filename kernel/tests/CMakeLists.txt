# Host-side smoke tests (CTest). enable_testing() must be at the top level so the
# tests registered later (from the Targets flow) are discoverable by `ctest`.
enable_testing()
include(kernel/SmokeTests)