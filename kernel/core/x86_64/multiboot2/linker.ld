/*****************************************************************************
 * Copyright (C) 2025 by Frederik Tobner                                     *
 *                                                                           *
 * This file is part of TinyOs.                                              *
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
 * @file linker_x86_64.ld
 * @brief TinyOS kernel linker script
 * 
 * Defines the memory layout of the kernel image:
 * - Loads at 1MB (0x100000) - standard higher-half kernel load address
 * - Multiboot header must be in first section
 * - Sections aligned to 4KB page boundaries
 */

/* Entry point symbol defined in boot/boot.asm */
ENTRY(_start)

SECTIONS {
    /* Kernel loads at 1MB - above BIOS reserved area */
    . = 0x100000;
    
    /* Multiboot header must be at the very start */
    .multiboot ALIGN(8) : {
        *(.multiboot)
    }
    
    /* Code section - executable instructions */
    .text ALIGN(4K) : {
        *(.text)
        *(.text.*)
    }
    
    /* Read-only data - string literals, const variables */
    .rodata ALIGN(4K) : {
        *(.rodata)
        *(.rodata.*)
    }
    
    /* Initialized data - global/static variables with initial values */
    .data ALIGN(4K) : {
        *(.data)
        *(.data.*)
    }
    
    /* Uninitialized data - global/static variables zeroed at boot */
    .bss ALIGN(4K) : {
        *(COMMON)
        *(.bss)
        *(.bss.*)
    }
    
    /* Symbols for kernel bounds - useful for memory management */
    kernel_start = 0x100000;
    kernel_end = .;
    kernel_size = kernel_end - kernel_start;
}
