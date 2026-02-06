#pragma once
#include <boot/info.h>

struct boot_info_requests {
    void * limine_executable_address_request;
};

void boot_arch_setup_requests(struct boot_info_requests * reqs);

void boot_arch_extract_executable_address(u64 * phys_base, u64 * virt_base);