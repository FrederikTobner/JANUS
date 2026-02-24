#include <boot/arch_requests.h>

void boot_arch_setup_requests(struct boot_info_requests_t * reqs)
{
    // No protocol-specific requests needed for x86_64/Multiboot2
    (void) reqs;
}

void boot_arch_extract_executable_address(u64 * phys_base, u64 * virt_base)
{
    // No executable address request for x86_64/Multiboot2 - kernel is identity-mapped
    (void) phys_base;
    (void) virt_base;
}
