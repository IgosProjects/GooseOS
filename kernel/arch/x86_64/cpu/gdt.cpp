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

#include <cpu/gdt.hpp>

using namespace GooseOS;

// Define our table with 3 entries: Null, Code, Data
struct CPU::GDTEntry GDTTable[3];
struct CPU::GDTPointer GDTPtr;

// Assembly function declaration to actually flush the registers
extern "C" void LoadGDT(u64 gdt_ptr_addr);

// INTERNAL TO DRIVER
// Sets the GDT entry in the table
void SetGDTEntry(u32 index, u8 access, u8 granularity) {
    GDTTable[index].limit_low    = 0;
    GDTTable[index].base_low     = 0;
    GDTTable[index].base_middle  = 0;
    GDTTable[index].access       = access;
    GDTTable[index].granularity  = granularity;
    GDTTable[index].base_high    = 0;
}

// Initilzies the GDT and loads it!
void CPU::InitGDT() {
    // Set up the pointer structure for the CPU
    GDTPtr.size = (sizeof(CPU::GDTEntry) * 3) - 1;
    GDTPtr.offset = (u64)&GDTTable[0];

    // 1. Entry 0x00: Null Descriptor (Always required)
    SetGDTEntry(0, 0, 0);

    // 2. Entry 0x08: Kernel Code Segment 
    // Access 0x9A: Present, Ring 0, Executable, Read/Write
    // Granularity 0x20: Long Mode flag (L-bit) set
    SetGDTEntry(1, 0x9A, 0x20);

    // 3. Entry 0x10: Kernel Data Segment
    // Access 0x92: Present, Ring 0, Read/Write
    SetGDTEntry(2, 0x92, 0x00);

    // Call the assembly function with our pointer
    LoadGDT((u64)&GDTPtr);
}