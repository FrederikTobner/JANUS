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

#include <ktest/ktest.h>
#include <mm/pmm.h>

KTEST(mm, page_alloc_is_page_aligned)
{
    phys_addr_t page = mm_pmm_alloc_page();
    KTEST_ASSERT_NE(page, 0);
    KTEST_ASSERT_EQ(page & 0xFFF, 0);
    mm_pmm_free_page(page);
}
