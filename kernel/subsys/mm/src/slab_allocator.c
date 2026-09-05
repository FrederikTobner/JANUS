#include <janus/attributes.h>
#include <janus/errno.h>

#include <kio/die.h>
#include <mm/pmm.h>
#include <mm/slab_allocator.h>

#define KMALLOC_PAGE_SIZE   4096ULL
#define KMALLOC_MIN_ALIGN   16ULL
#define KMALLOC_SLAB_MAGIC  0x4B4D41C4C534C42ULL
#define KMALLOC_SLAB_HDR    48ULL
#define KMALLOC_PAGE_USABLE (KMALLOC_PAGE_SIZE - KMALLOC_SLAB_HDR) // 4096 - 48 = 4048 bytes usable for objects

typedef struct kmalloc_slab kmalloc_slab_t;
struct kmalloc_slab {
    /// @brief Magic number for slab validation
    u64 magic;
    /// @brief Size of each object in the slab
    u32 object_size;
    /// @brief Number of objects currently allocated in the slab
    u16 free_count;
    /// @brief Total number of objects in the slab
    u16 total_count;
    /// @brief Pointer to the previous slab in the linked list
    kmalloc_slab_t * previous;
    /// @brief Pointer to the next slab in the linked list
    kmalloc_slab_t * next;
    /// @brief Pointer to the free list of objects in the slab
    void * free_list;
    /// @brief Physical address of the backing physical page for the slab
    phys_addr_t phys_addr;
};

typedef struct {
    /// @brief Size of each object in the cache
    u32 object_size;
    /// @brief Heap of slabs that have a free object
    kmalloc_slab_t * partial;
} kmalloc_cache_t;

static u32 const g_class_sizes[] = {
    16,
    32,
    64,
    128,
    256,
    512,
    1024,
    2048,
    (u32) KMALLOC_PAGE_USABLE,
};

#define KMALLOC_NUM_CLASSES (sizeof(g_class_sizes) / sizeof(g_class_sizes[0]))
#define KMALLOC_MAX_SIZE    KMALLOC_PAGE_USABLE

static kmalloc_cache_t g_caches[KMALLOC_NUM_CLASSES];
static kmalloc_stats_t g_stats;
static u64 g_hhdm_offset;
static bool g_initialized;

static u32 kmalloc_size_to_class(u32 size);
static u32 kmalloc_class_of_slab(kmalloc_slab_t const * slab);
static kmalloc_slab_t * kmalloc_grow(kmalloc_cache_t * cache);
static void kmalloc_partial_push(kmalloc_cache_t * cache, kmalloc_slab_t * slab);
static void kmalloc_partial_remove(kmalloc_cache_t * cache, kmalloc_slab_t * slab);
static void kmalloc_fill(void * dest, u8 value, size_t n);
static void kmalloc_copy(void * dest, void const * src, size_t n);

error_t mm_slab_alloc_init(u64 hhdm_offset)
{
    if (g_initialized) {
        return JANUS_EINVAL;
    }

    g_hhdm_offset = hhdm_offset;

    for (u32 i = 0; i < KMALLOC_NUM_CLASSES; ++i) {
        g_caches[i].object_size = g_class_sizes[i];
        g_caches[i].partial = NULL;
    }

    g_stats.alloc_count = 0;
    g_stats.free_count = 0;
    g_stats.bytes_in_use = 0;
    g_stats.pages_in_use = 0;
    g_initialized = true;
    return JANUS_OK;
}

static u32 kmalloc_size_to_class(u32 size)
{
    for (u32 i = 0; i < KMALLOC_NUM_CLASSES; ++i) {
        if (size <= g_class_sizes[i]) {
            return i;
        }
    }
    return KMALLOC_NUM_CLASSES; // Invalid class index
}

static kmalloc_slab_t * kmalloc_grow(kmalloc_cache_t * cache)
{
    // A size class larger than a single page can hold would leave the new slab with
    // zero objects (an empty free_list), which the caller is never prepared to handle.
    // All current classes satisfy object_size <= KMALLOC_PAGE_USABLE, so count is always
    // at least 1, but this guard keeps that invariant explicit and fails safely instead
    // of handing back a slab with nothing to allocate from.
    u32 const count = (u32) (KMALLOC_PAGE_USABLE / cache->object_size);
    if (count == 0) {
        return NULL; // object_size exceeds a single page; misconfigured size class
    }

    phys_addr_t phys_page = mm_pmm_alloc_page();
    if (phys_page == 0) {
        return NULL; // Out of physical memory
    }

    kmalloc_slab_t * slab = (kmalloc_slab_t *) (g_hhdm_offset + phys_page);

    slab->magic = KMALLOC_SLAB_MAGIC;
    slab->object_size = cache->object_size;
    slab->phys_addr = phys_page;
    slab->free_list = NULL;

    u8 * const base = (u8 *) slab + KMALLOC_SLAB_HDR;
    for (u32 i = 0; i < count; ++i) {
        void * obj = base + ((u64) i * cache->object_size);
        *(void **) obj = slab->free_list; // intrusive push to the free list
        slab->free_list = obj;
    }

    slab->total_count = (u16) count;
    slab->free_count = (u16) count;
    kmalloc_partial_push(cache, slab);
    g_stats.pages_in_use++;
    return slab;
}

void * kmalloc(u64 size)
{
    if (!g_initialized) {
        kpanic("kmalloc: allocator not initialized");
    }
    if (size == 0 || size > KMALLOC_MAX_SIZE) {
        return NULL;
    }

    u32 class_index = kmalloc_size_to_class((u32) size);
    kmalloc_cache_t * cache = &g_caches[class_index];

    kmalloc_slab_t * slab = cache->partial;
    if (slab == NULL) {
        slab = kmalloc_grow(cache);
        if (slab == NULL) {
            return NULL; // Out of memory
        }
    }

    void * obj = slab->free_list;
    slab->free_list = *(void **) obj; // Pop from the free free_list
    slab->free_count--;
    if (slab->free_count == 0) {
        kmalloc_partial_remove(cache, slab);
    }

    g_stats.bytes_in_use += slab->object_size;
    g_stats.alloc_count++;

    return obj;
}

void kfree(void * ptr)
{
    if (!g_initialized) {
        kpanic("kfree: allocator not initialized");
    }
    if (ptr == NULL) {
        return;
    }

    kmalloc_slab_t * slab = (kmalloc_slab_t *) ((u64) ptr & ~(KMALLOC_PAGE_SIZE - 1));
    if (slab->magic != KMALLOC_SLAB_MAGIC) {
        kpanic("kfree: invalid pointer or corrupted slab");
    }

    bool const was_full = (slab->free_count == 0);
    *(void **) ptr = slab->free_list; // Push to the free free_list
    slab->free_list = ptr;
    slab->free_count++;

    g_stats.bytes_in_use -= slab->object_size;
    g_stats.free_count++;

    kmalloc_cache_t * cache = &g_caches[kmalloc_class_of_slab(slab)];
    if (was_full) {
        kmalloc_partial_push(cache, slab);
    }
    if (slab->free_count == slab->total_count) {
        kmalloc_partial_remove(cache, slab);
        slab->magic = 0; // Invalidate the slab
        mm_pmm_free_page(slab->phys_addr);
        g_stats.pages_in_use--;
    }
}

void * krealloc(void * ptr, u64 new_size)
{
    if (ptr == NULL) {
        return kmalloc(new_size);
    }
    if (new_size == 0) {
        kfree(ptr);
        return NULL;
    }

    kmalloc_slab_t * slab = (kmalloc_slab_t *) ((u64) ptr & ~(KMALLOC_PAGE_SIZE - 1));
    if (slab->magic != KMALLOC_SLAB_MAGIC) {
        kpanic("krealloc: invalid pointer or corrupted slab");
    }

    if (new_size <= slab->object_size) {
        return ptr; // No need to reallocate
    }

    void * new_ptr = kmalloc(new_size);
    if (new_ptr == NULL) {
        return NULL; // Out of memory
    }

    kmalloc_copy(new_ptr, ptr, slab->object_size);
    kfree(ptr);
    return new_ptr;
}

void * kcalloc(u64 size)
{
    if (size == 0 || size > KMALLOC_MAX_SIZE) {
        return NULL;
    }
    void * ptr = kmalloc(size);
    if (ptr != NULL) {
        kmalloc_fill(ptr, 0, size);
    }
    return ptr;
}

void kmalloc_get_stats(kmalloc_stats_t * stats)
{
    if (stats == NULL) {
        return;
    }
    *stats = g_stats;
}

static void kmalloc_fill(void * dest, u8 value, size_t n)
{
    u8 * d = (u8 *) dest;
    for (size_t i = 0; i < n; ++i) {
        d[i] = value;
    }
}

static void kmalloc_copy(void * dest, void const * src, size_t n)
{
    u8 * d = (u8 *) dest;
    u8 const * s = (u8 const *) src;
    for (size_t i = 0; i < n; ++i) {
        d[i] = s[i];
    }
}

static u32 kmalloc_class_of_slab(kmalloc_slab_t const * slab)
{
    for (u32 i = 0; i < KMALLOC_NUM_CLASSES; ++i) {
        if (slab->object_size == g_class_sizes[i]) {
            return i;
        }
    }
    kpanic("kmalloc_class_of_slab: invalid slab object size");
    return KMALLOC_NUM_CLASSES; // Should never reach here
}

static void kmalloc_partial_push(kmalloc_cache_t * cache, kmalloc_slab_t * slab)
{
    slab->next = cache->partial;
    if (cache->partial != NULL) {
        cache->partial->previous = slab;
    }
    slab->previous = NULL;
    cache->partial = slab;
}

static void kmalloc_partial_remove(kmalloc_cache_t * cache, kmalloc_slab_t * slab)
{
    if (slab->previous != NULL) {
        slab->previous->next = slab->next;
    } else {
        cache->partial = slab->next;
    }
    if (slab->next != NULL) {
        slab->next->previous = slab->previous;
    }
    slab->previous = NULL;
    slab->next = NULL;
}
