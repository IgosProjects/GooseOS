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

// This file implements the ramdisk to act as a block device
// If you are looking for Storage::GetCurrentRamdisk(), it is defined in the boot/ramdisk.cpp file!

#include <blockdev/ramdisk.hpp>
#include <blockdev/blockdevice.hpp>
#include <utils.hpp>
#include <console/console.hpp>

using namespace GooseOS;

// PLEASE NOTICE: All the reading and writing functions will HAVE to have "RAMDISK_" in front of the name
// This is so every block device doesnt interfere when linking!

#define RAMDISK_SectorSize 512 // 512 bytes per sector is standard on all newer SSDs and HDDs

// Reads a sector from the ramdisk pointer(but since we arent using a SSD or HDD we just represent a sector with 512 bytes of memory)
// Returns true if done sucessfully, returns false if not
bl RAMDISK_ReadSector(void* buffer, u64 sector, Storage::BlockDevice dev) {
    // Check if we actually got a valid ramdisk block device
    if (!dev.PrivateDeviceData) {
        Console::Error("ReadSector called with an invalid ramdisk BlockDevice(MISSING_RAMDISK_PTR) BlockDevice: 0x%x", &dev);

        return kfalse;
    }

    // Get the offset to the sector start
    Storage::Ramdisk::RamdiskData* RamdiskData = (Storage::Ramdisk::RamdiskData*)dev.PrivateDeviceData;
    void* RamdiskPointer = RamdiskData->addr;
    void* PointerToStartOfSector = RamdiskPointer + (sector * RAMDISK_SectorSize);

    if (PointerToStartOfSector) {
        Utils::memcpy(buffer, PointerToStartOfSector, RAMDISK_SectorSize); // Copy the contents of the sector into the buffer

        return ktrue;
    }

    // This runs if we chouldnt get a valid pointer to the start of the sector so    
    return kfalse;
}

// Writes a sector to the ramdisk pointer(but we use bytes in memoy so its 512 bytes instead if 1 sector)
// Returns true if done sucessfully, returns false if not
bl RAMDISK_WriteSector(void* buffer, u64 sector, Storage::BlockDevice dev) {
    // Check if we actually got a valid ramdisk block device
    if (!dev.PrivateDeviceData) {
        Console::Error("WriteSector called with an invalid ramdisk BlockDevice(MISSING_RAMDISK_PTR) BlockDevice: 0x%x", &dev);

        return kfalse;
    }

    // Get the offset to the sector start
    Storage::Ramdisk::RamdiskData* RamdiskData = (Storage::Ramdisk::RamdiskData*)dev.PrivateDeviceData;
    void* RamdiskPointer = RamdiskData->addr;
    void* PointerToStartOfSector = RamdiskPointer + (sector * RAMDISK_SectorSize);

    if (PointerToStartOfSector) {
        // PLEASE NOTE: this is just ReadSector but i changed one line, LOL
        Utils::memcpy(PointerToStartOfSector, buffer, RAMDISK_SectorSize); // Copy the contents of the sector into the buffer

        return ktrue;
    }

    // This runs if we chouldnt get a valid pointer to the start of the sector so    
    return kfalse;
}

// NOTE: Do not move into Init, its here so it doesnt vanish on the stack
Storage::Ramdisk::RamdiskData ramdisk_data = {};

// Initilizes the ramdisk driver, modify's the passed in block devices's values
void Storage::Ramdisk::Init(Storage::BlockDevice* blk_device) {
    Storage::Ramdisk::Ramdisk ramdisk = Storage::Ramdisk::GetCurrentRamdisk(); // Get the current ramdisk from our function

    // Fill in the ramdisk data struct
    ramdisk_data.addr = ramdisk.addr;
    ramdisk_data.sector_size = (ramdisk.size * RAMDISK_SectorSize);
    
    // EXTRA INFO(in our case, the ramdisk pointer)
    blk_device->PrivateDeviceData = &ramdisk_data;

    // FUNCTIONS
    blk_device->WriteSector = RAMDISK_WriteSector; // Sector writing function
    blk_device->ReadSector = RAMDISK_ReadSector; // Sector reading function
}