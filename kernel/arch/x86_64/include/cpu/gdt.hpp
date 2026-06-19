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

namespace GooseOS::CPU {
    // Each GDT entry is exactly 8 bytes, no more no less!
    struct GDTEntry {
        u16 limit_low;
        u16 base_low;
        u8  base_middle;
        u8  access;
        u8  granularity;
        u8  base_high;
    } __attribute__((packed));

    // The GDT pointer loaded by the CPU, same structure as the IDT one!
    struct GDTPointer {
        u16 size;
        u64 offset;
    } __attribute__((packed));

    // Initilzies the GDT and loads it!
    void InitGDT();
}