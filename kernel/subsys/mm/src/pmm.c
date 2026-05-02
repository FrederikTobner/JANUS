/*****************************************************************************
 * Copyright (C) 2025 by Frederik Tobner                                     *
 *                                                                           *
 * This file is part of JANUS.                                              *
 *                                                                           *
 * Permission to use, copy, modify, and distribute this software and its     *
 * documentation under the terms of the GNU Affero General Public License is *
 * hereby granted.                                                           *
 * No representations are made about the suitability of this software for    *
 * any purpose.                                                              *
 * It is provided "as is" without express or implied warranty.               *
 * See the <https://www.gnu.org/licenses/agpl-3.0.en.html>                   *
 * GNU Affero General Public License                                         *
 * License for more details.                                                 *
 ****************************************************************************/

/// @file pmm.c
/// @brief Physical Memory Manager implementation
///
/// Bitmap-based PMM covering 4 GiB of physical address space.
/// The bitmap lives in .bss (128 KiB). Each bit represents one 4 KiB frame.
/// 1 = free, 0 = used. The bitmap is initialised with all frames used,
/// then usable regions from the boot memory map are marked free.
///
/// Design choices following Linux/FreeBSD conventions:
/// - Static allocation (no dynamic allocation in core PMM)
/// - All frames in the first 1 MiB are permanently reserved (DMA, BIOS, VGA)
/// - The kernel image frames are permanently reserved
/// - mm_pmm_alloc_page() panics on OOM rather than returning an error code,
///   because callers in early boot cannot meaningfully handle OOM

#include <mm/pmm.h>

#include <janus/attributes.h>
#include <janus/errno.h>
#include <kio/kio.h>

// ── Constants ──────────────────────────────────────────────────────────────

/// Page size in bytes (4 KiB)
#define PAGE_SIZE           4096ULL

/// Number of bits in a byte
#define BITS_PER_BYTE       8ULL

/// Total physical address space covered by the bitmap: 4 GiB
#define PMM_COVERAGE        (4ULL * 1024ULL * 1024ULL * 1024ULL)

/// Number of page frames tracked (1,048,576)
#define PMM_BITMAP_BITS     (PMM_COVERAGE / PAGE_SIZE)

/// Size of the bitmap in bytes (131,072 = 128 KiB)
#define PMM_BITMAP_BYTES    (PMM_BITMAP_BITS / BITS_PER_BYTE)

/// End of the low 1 MiB reserved region (kept for BIOS, ISA DMA, VGA)
#define PMM_LOW_MEMORY_END  (1024ULL * 1024ULL)

/// Frame index of the first allocatable frame (frame 256 = 1 MiB)
#define PMM_MIN_ALLOC_FRAME (PMM_LOW_MEMORY_END / PAGE_SIZE)

// ── Internal helpers ───────────────────────────────────────────────────────

/// Convert a physical address to a frame index
static inline u64 phys_to_frame(phys_addr_t addr)
{
    return addr / PAGE_SIZE;
}

/// Convert a frame index to a physical address
static inline phys_addr_t frame_to_phys(u64 frame)
{
    return (phys_addr_t) (frame * PAGE_SIZE);
}

/// Test whether a frame is free (bit == 1)
static inline bool pmm_is_free(u8 const * bitmap, u64 frame)
{
    return (bitmap[frame >> 3] & (1U << (frame & 7U))) != 0;
}

/// Mark a frame free (set bit to 1)
static inline void pmm_mark_free(u8 * bitmap, u64 frame)
{
    bitmap[frame >> 3] |= (u8) (1U << (frame & 7U));
}

/// Mark a frame used (clear bit to 0)
static inline void pmm_mark_used(u8 * bitmap, u64 frame)
{
    bitmap[frame >> 3] &= (u8) ~(1U << (frame & 7U));
}

// ── PMM State ──────────────────────────────────────────────────────────────

/// Private PMM state — allocated in .bss, zero-initialized
typedef struct {
    u8 bitmap[PMM_BITMAP_BYTES]; ///< Allocation bitmap (1 = free, 0 = used)
    u64 total_pages;             ///< Total usable page frames
    u64 free_pages;              ///< Currently free page frames
    u64 first_free_hint;         ///< Lowest frame that might be free (never retreats)
    bool initialized;            ///< Set true after successful mm_pmm_init()
} pmm_state_t;

static pmm_state_t g_pmm;

// ── Internal: mark a physical range free ──────────────────────────────────

/// Mark all page-aligned frames within [base, base+length) as free.
/// Frames below PMM_LOW_MEMORY_END or beyond PMM_COVERAGE are skipped.
static void pmm_mark_range_free(phys_addr_t base, u64 length)
{
    // Round base up to page boundary
    phys_addr_t const aligned_base = (base + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
    if (aligned_base >= base + length) {
        return; // region smaller than one aligned page
    }
    u64 const adjusted_length = length - (aligned_base - base);

    // Round length down so we only mark fully-covered frames
    u64 const frame_start = phys_to_frame(aligned_base);
    u64 const frame_end = phys_to_frame(aligned_base + adjusted_length); // exclusive

    for (u64 frame = frame_start; frame < frame_end && frame < PMM_BITMAP_BITS; frame++) {
        // Never free low-memory frames (< 1 MiB)
        if (frame < PMM_MIN_ALLOC_FRAME) {
            continue;
        }
        if (!pmm_is_free(g_pmm.bitmap, frame)) {
            pmm_mark_free(g_pmm.bitmap, frame);
            g_pmm.free_pages++;
            g_pmm.total_pages++;
            if (frame < g_pmm.first_free_hint) {
                g_pmm.first_free_hint = frame;
            }
        }
    }
}

/// Mark all page-aligned frames within [base, base+length) as used.
/// Used to punch holes in usable regions for reserved areas.
static void pmm_mark_range_used(phys_addr_t base, u64 length)
{
    u64 const frame_start = phys_to_frame(base);
    u64 const frame_end = (base + length + PAGE_SIZE - 1) / PAGE_SIZE; // round up

    for (u64 frame = frame_start; frame < frame_end && frame < PMM_BITMAP_BITS; frame++) {
        if (pmm_is_free(g_pmm.bitmap, frame)) {
            pmm_mark_used(g_pmm.bitmap, frame);
            g_pmm.free_pages--;
        }
    }
}

// ── Public API ─────────────────────────────────────────────────────────────

__cold error_t mm_pmm_init(boot_memmap_t const * memmap, phys_addr_t kernel_phys_base, phys_addr_t kernel_phys_end)
{
    if (UNLIKELY(memmap == NULL)) {
        return JANUS_EINVAL;
    }

    // bitmap is already all-zero (.bss) — every frame starts as "used"
    g_pmm.free_pages = 0;
    g_pmm.total_pages = 0;
    g_pmm.first_free_hint = PMM_MIN_ALLOC_FRAME;

    // Pass 1: mark all USABLE and BOOTLOADER_RECLAIMABLE regions free
    for (u32 i = 0; i < memmap->count; i++) {
        mem_region_t const * region = &memmap->entries[i];
        if (region->type == MEM_REGION_USABLE || region->type == MEM_REGION_BOOTLOADER_RECLAIMABLE) {
            pmm_mark_range_free(region->base, region->length);
        }
    }

    if (UNLIKELY(g_pmm.total_pages == 0)) {
        return JANUS_ENOMEM;
    }

    // Pass 2: punch out the kernel image (always reserved)
    if (kernel_phys_end > kernel_phys_base) {
        u64 const klen = kernel_phys_end - kernel_phys_base;
        pmm_mark_range_used(kernel_phys_base, klen);
    }

    g_pmm.initialized = true;
    return JANUS_OK;
}

phys_addr_t mm_pmm_alloc_page(void)
{
    if (UNLIKELY(!g_pmm.initialized)) {
        kpanic("mm_pmm_alloc_page: PMM not initialized");
    }

    // Scan from hint, looking for a non-zero bitmap byte (any free frame)
    u64 const start_byte = g_pmm.first_free_hint >> 3;

    for (u64 byte = start_byte; byte < PMM_BITMAP_BYTES; byte++) {
        if (g_pmm.bitmap[byte] == 0) {
            continue;
        }
        // Found a byte with at least one free frame
        u8 const bits = g_pmm.bitmap[byte];
        u8 const bit = (u8) __builtin_ctz((unsigned) bits);
        u64 const frame = (byte << 3) | bit;

        if (frame < PMM_MIN_ALLOC_FRAME) {
            continue;
        }

        pmm_mark_used(g_pmm.bitmap, frame);
        g_pmm.free_pages--;
        g_pmm.first_free_hint = frame + 1;
        return frame_to_phys(frame);
    }

    // Out of physical memory — return 0 (reserved address, never a valid frame)
    return 0;
}

void mm_pmm_free_page(phys_addr_t phys)
{
    if (UNLIKELY(!g_pmm.initialized)) {
        kpanic("mm_pmm_free_page: PMM not initialized");
    }
    if (UNLIKELY(phys & (PAGE_SIZE - 1))) {
        kpanic("mm_pmm_free_page: unaligned address 0x%llx", (unsigned long long) phys);
    }

    u64 const frame = phys_to_frame(phys);

    if (UNLIKELY(frame >= PMM_BITMAP_BITS)) {
        kpanic("mm_pmm_free_page: address 0x%llx out of tracked range", (unsigned long long) phys);
    }
    if (UNLIKELY(frame < PMM_MIN_ALLOC_FRAME)) {
        kpanic("mm_pmm_free_page: attempt to free reserved low-memory frame at 0x%llx", (unsigned long long) phys);
    }
    if (UNLIKELY(pmm_is_free(g_pmm.bitmap, frame))) {
        kpanic("mm_pmm_free_page: double-free detected at 0x%llx", (unsigned long long) phys);
    }

    pmm_mark_free(g_pmm.bitmap, frame);
    g_pmm.free_pages++;

    if (frame < g_pmm.first_free_hint) {
        g_pmm.first_free_hint = frame;
    }
}

void mm_pmm_get_stats(mm_pmm_stats_t * stats)
{
    stats->total_pages = g_pmm.total_pages;
    stats->free_pages = g_pmm.free_pages;
    stats->used_pages = g_pmm.total_pages - g_pmm.free_pages;
}
