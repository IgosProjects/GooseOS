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

#include <blockdev/ramdisk.hpp>
#include <limine/limine.h>
#include <utils.hpp>
#include <core.hpp>
#include <console/console.hpp>

extern "C" struct limine_module_request module_request;

using namespace GooseOS;

// Returns the currently used ramdisk
Storage::Ramdisk::Ramdisk Storage::Ramdisk::GetCurrentRamdisk() {
    // Check if limine gave us an ramdisk
    assert((module_request.response), "RAMDISK: Limine did not give us an ramdisk! Is it loaded as an module?");

    // Find our ramdisk in the list
    Console::OK("RAMDISK: Amount of modules given by Limine: %u", module_request.response->module_count);

    struct limine_file* ramdisk_file;
    struct Storage::Ramdisk::Ramdisk ramdisk;

    for (u64 i = 0; i < module_request.response->module_count; i++) {
        struct limine_file* module_file = module_request.response->modules[i];

        // Check if the module is valid
        if (!module_file || !module_file->string) {
            Console::Error("RAMDISK: Skipped an empty or invalid module at index %u\n", i);
            continue;
        }

        // Check if the module is our ramdisk using the string property
        if (Utils::strcmp(module_file->string, "ramdisk") == ktrue) {
            ramdisk_file = module_file;
            goto found_ramdisk; // FIX OF FIX: NEVER return inside of an FOR statement!
        }
    }

    found_ramdisk:

    assert((ramdisk_file != nullptr), "RAMDISK: Choulnt find any valid ramdisk!");
    
    // Update the values
    ramdisk.addr = ramdisk_file->address;
    ramdisk.size = ramdisk_file->size;

    return ramdisk; // Return it
}
