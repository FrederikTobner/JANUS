#ifndef JANUS_KMALLOC_H
#define JANUS_KMALLOC_H

#include <janus/types.h>

/// @brief kmalloc heap usage statistics
typedef struct {
    u64 bytes_in_use;
    u64 pages_in_use;
    u64 alloc_count;
    u64 free_count;
} kmalloc_stats_t;

/// @brief Initialize the kernel heap allocator
///
/// Must be called exactly once after mm_pmm_init(). Records the HHDM offset used to address slab pages and clears all size-class state.
/// @param hhdm_offset The HHDM offset used to address slab pages_in_use
/// @return JANUS_OK on success, or an error code on failure
error_t kmalloc_init(u64 hhdm_offset);

/// @brief Allocate at least `size` bytes of memory from the kernel heap
///
/// The returned pointer is guaranteed to be aligned to at least 16 bytes. 
/// Returns NULL if the allocation fails, the size is 0, or the size is larger than the maximum supported allocation size.
/// @param size The number of bytes to allocate
/// @return A pointer to the allocated memory, or NULL on failure
void * kmalloc(u64 size);

/// @brief Allocate zero-initialized memory from the kernel heap
/// The returned pointer is guaranteed to be aligned to at least 16 bytes. 
/// Returns NULL if the allocation fails, the size is 0, or the size is larger than the maximum supported allocation size.
/// @param size The number of bytes to allocate
/// @return A pointer to the allocated memory, or NULL on failure
void * kcalloc(u64 size);

/// @brief Resize a previously allocated memory block
/// The returned pointer is guaranteed to be aligned to at least 16 bytes. 
/// Returns NULL if the reallocation fails, the size is 0, or the size is larger than the maximum supported allocation size.
/// @param ptr The pointer to the previously allocated memory block
/// @param new_size The new size in bytes for the memory block
/// @return A pointer to the reallocated memory, or NULL on failure
void * krealloc(void * ptr, u64 new_size);

/// @brief Free a previously allocated memory block
/// @param ptr The pointer to the previously allocated memory block
void kfree(void * ptr);

/// @brief Get statistics about the kernel heap usage
/// @param stats A pointer to a kmalloc_stats_t structure to be filled with the current heap usage statistics
void kmalloc_get_stats(kmalloc_stats_t * stats);

#endif // JANUS_KMALLOC_H
