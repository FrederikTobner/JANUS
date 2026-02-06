#include <boot/arch_requests.h>
#include <boot/protocol/limine.h>
/// Executable address response - provides kernel physical/virtual base addresses
struct limine_executable_address_response {
    u64 revision;
    u64 physical_base;
    u64 virtual_base;
};

/* External Limine request for executable address (defined in limine_requests.c) */
extern volatile struct {
    u64 id[4];
    u64 revision;
    struct limine_executable_address_response * response;
} limine_executable_address_request;

void boot_arch_setup_requests(struct boot_info_requests * reqs)
{
    reqs->limine_executable_address_request = (void *) (unsigned long) &limine_executable_address_request;
}

void boot_arch_extract_executable_address(u64 * phys_base, u64 * virt_base)
{
    if (limine_executable_address_request.response != NULL) {
        *phys_base = limine_executable_address_request.response->physical_base;
        *virt_base = limine_executable_address_request.response->virtual_base;
    } else {
        *phys_base = 0;
        *virt_base = 0;
    }
}
