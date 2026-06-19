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
    // This struct represents one IDT entry in the table
    struct IDTEntry {
        u16 offset_low; // bits 0-15
        u16 selector; // GDT code segment

        u8  ist; // IST index(zero)
        u8  attrs; // Type and attributes

        u16 offset_mid; // Bits 16-31
        u32 offset_high; // Bits 31-63

        u32 zero; // reserved
    } __attribute__((packed));

    // This struct represents a pointer to an IDT table with 255 entries
	struct IDTPointer {
		u16 size; // Size of IDT table - 1
		u64 offset; // Offset in memory to IDT table
	}__attribute__((packed));

    // Initilizes the IDT driver, doesnt automaticly load it!
    // For that, use LoadIDT! Also only run once on GP and load on the GP and all APs
    void InitIDT();

    // Loads the IDT table, can be called on the GP and all other APs!
    void LoadIDT();
} 
