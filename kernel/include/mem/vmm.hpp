/*
 *	This file is part of gooseOS.
 *
 *	gooseOS is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation, either version 3 of the License, or
 *	(at your option) any later version.
 *
 *	gooseOS is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with gooseOS.  If not, see <https://www.gnu.org/licenses/>.
 *
 *	Copyright(c) 2026 EyeDev
*/

#pragma once
#include <types.hpp>

// VMM(Virtual Memory Manager) namespace
namespace GooseOS::Memory::VMM {
    typedef u64 pt_entry;

    // Entry types
    constexpr u64 PTE_PRESENT  = (1ULL << 0);
    constexpr u64 PTE_WRITABLE = (1ULL << 1);
    constexpr u64 PTE_USER     = (1ULL << 2);

    // Maps a physical address to a virtual one, if not used on non-mapped addresses it will Page Fault
    void MapPage(pt_entry* pml4_virt, u64 virt_addr, u64 phys_addr, u64 flags, u64 hhdm_offset);
}