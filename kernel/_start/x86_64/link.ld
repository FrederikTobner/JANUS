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

/**
 * @file link.ld (link-multiboot2.ld)
 * @brief JANUS kernel linker script for Multiboot2 protocol (x86_64)
 * 
 * Physical address kernel for GRUB/Multiboot2 bootloader.
 * The kernel sets up its own paging after boot.
 * 
 * Memory layout:
 * - Load/run address: 0x100000 (1MB) - above BIOS reserved area
 * - Multiboot2 header must be in first 32KB of kernel image
 * 
 * Note: This is x86_64-specific. Multiboot2 is not available on ARM.
 */

ENTRY(_start_multiboot2)

SECTIONS {
    /* Kernel loads at 1MB */
    . = 0x100000;
    
    /* Multiboot2 header must be first (GRUB requirement: first 32KB) */
    .multiboot ALIGN(8) : {
        *(.multiboot)
    }
    
    /* Code section */
    .text ALIGN(4K) : {
        *(.text.multiboot2_entry)
        *(.text)
        *(.text.*)
    }
    
    /* Read-only data */
    .rodata ALIGN(4K) : {
        *(.rodata)
        *(.rodata.*)
    }
    
    /* Initialized data */
    .data ALIGN(4K) : {
        *(.data)
        *(.data.*)
    }
    
    /* Uninitialized data */
    .bss ALIGN(4K) : {
        *(COMMON)
        *(.bss)
        *(.bss.*)
    }
    
    /* Kernel bounds symbols */
    kernel_start = 0x100000;
    kernel_end = .;
    kernel_size = kernel_end - kernel_start;
}
