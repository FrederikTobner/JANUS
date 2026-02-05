/*****************************************************************************
 * Copyright (C) 2025 by Frederik Tobner                                     *
 *                                                                           *
 * This file is part of JANUS.                                               *
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
 *****************************************************************************/

/**
 * @file linker.ld
 * @brief JANUS kernel linker script for Limine protocol (AArch64)
 * 
 * Higher-half kernel for Limine bootloader on AArch64.
 * Limine handles paging and maps the kernel at virtual addresses.
 * 
 * Memory layout:
 * - Physical load address: 0x100000 (1MB) 
 * - Virtual address: 0xFFFFFFFF80000000 (-2GB, higher-half)
 * 
 * This linker script mirrors the x86_64 version for consistency.
 * AArch64 page granule is typically 4KB (same as x86_64).
 */

ENTRY(_start_limine)

/* Higher-half virtual address base */
KERNEL_VMA = 0xFFFFFFFF80000000;
/* Physical load address */
KERNEL_LMA = 0x100000;

SECTIONS {
    /* Start at higher-half virtual address */
    . = KERNEL_VMA + KERNEL_LMA;
    
    /* Limine request structures with start/end markers */
    .limine_requests ALIGN(8) : AT(ADDR(.limine_requests) - KERNEL_VMA) {
        KEEP(*(.limine_requests_start))
        KEEP(*(.limine_requests))
        KEEP(*(.limine_requests_end))
    }
    
    /* Code section */
    .text ALIGN(4K) : AT(ADDR(.text) - KERNEL_VMA) {
        *(.text.limine_entry)
        *(.text)
        *(.text.*)
    }
    
    /* Read-only data */
    .rodata ALIGN(4K) : AT(ADDR(.rodata) - KERNEL_VMA) {
        *(.rodata)
        *(.rodata.*)
    }
    
    /* Initialized data */
    .data ALIGN(4K) : AT(ADDR(.data) - KERNEL_VMA) {
        *(.data)
        *(.data.*)
    }
    
    /* Uninitialized data */
    .bss ALIGN(4K) : AT(ADDR(.bss) - KERNEL_VMA) {
        __bss_start = .;
        *(COMMON)
        *(.bss)
        *(.bss.*)
        __bss_end = .;
    }
    
    /* Kernel bounds symbols */
    kernel_vma_offset = KERNEL_VMA;
    kernel_phys_start = KERNEL_LMA;
    kernel_virt_start = KERNEL_VMA + KERNEL_LMA;
    kernel_phys_end = . - KERNEL_VMA;
    kernel_virt_end = .;
    kernel_size = kernel_virt_end - kernel_virt_start;
}
