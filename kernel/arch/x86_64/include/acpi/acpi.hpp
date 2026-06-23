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

namespace GooseOS::ACPI {
    // Signature before every ACPI table
    struct __attribute__((packed)) ACPIHeader {
        char signature[4];
        u32 length;
        u8 revision;
        u8 checksum;
        char oem_id[6];
        char oem_table_id[8];
        u32 oem_revision;
        u32 creator_id;
        u32 creator_revision;
    };

    // Root ACPI table
    struct __attribute__((packed)) RSDP {
        char signature[8];        // "RSD PTR "
        u8 checksum;
        char oem_id[6];
        u8 revision;
        u32 rsdt_address; // Legacy 32 bit pointer, do not use!
    
        // ACPI 2.0+ fields (64-bit safe)
        u32 length;
        u64 xsdt_address; // Address in PHYSICAL memory of root table!
        u8 extended_checksum;
        u8 reserved[3];
    };

    // Root table with all the pointers
    struct __attribute__((packed)) XSDT {
        ACPIHeader header;
        u64 tables[]; // Array of 64 bit pointers to ACPI tables
    };

    // Returns the pointer to a specific ACPI table based on the 4 char signature
    void* GetTable(RSDP* rsdp, u64 hhdm_offset, const char* sig);
}