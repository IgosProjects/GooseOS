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

// GooseOS storage system
namespace GooseOS::Storage {
    // FUNCTION IDENTIFIERS
    using _BLOCKDEV_ReadFunction = bl(*)(void* buffer, u64 sector, BlockDevice dev);
    using _BLOCKDEV_WriteFunction = bl(*)(void* buffer, u64 sector, BlockDevice dev);

    // GooseOS BlockDevice info specification structure
    struct BlockDeviceInfo {
        // Block device name(for example "Unkown Disk")
        char name[32];

        // Size of the block device in sectors
        u64 sector_size;

        // Size of the block device in bytes
        u64 byte_size;
    };

    // Block device GetInfo function
    using _BLOCKDEV_GetInfo = struct BlockDeviceInfo(*)(BlockDevice dev);

    // GooseOS block device structure
    struct BlockDevice {
        // Reads the data from SECTOR into the specified buffer
        // Returns false if an error happens!
        _BLOCKDEV_ReadFunction ReadSector;

        // Writes the data from BUFFER into the specified sector!
        // Returns false if an error happens!
        _BLOCKDEV_WriteFunction WriteSector;

        // Returns an BlockDeviceInfo struct containing the device info
        _BLOCKDEV_GetInfo GetDeviceInfo;

        // Private data pointer used by the drivers, can contain a pointer to anything. A struct an ramdisk ANYTHING!
        void* PrivateDeviceData;
    };
} 
