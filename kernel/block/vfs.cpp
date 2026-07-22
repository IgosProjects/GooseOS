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

#include <storage/vfs.hpp>
#include <core.hpp>
#include <kconfig.hpp>
#include <utils.hpp>
#include <console/console.hpp>

using namespace GooseOS;

// Special MountedFS struct used in the array, has extra info
struct MountedFS {
    char* prefix;
    Storage::Filesystem* fs;
};

// Max of 32 mounted filesystems
MountedFS MountedFilesystems[KRNL_MAX_MOUNTS];
u8 MountedFSIndex = 0; // PLEASE NOTE: If a person wants to make a huge server cluster or has a bunch of partitions
// THIS WILL BREAK! Anymore than 256 partitions EVEN if KRNL_MAX_MOUNTS is high! NO MORE than 256 partitions

// Checks if something is allready mounted at the provided prefix
bl AllreadyMounted(char* prefix) {
    for (u8 i = 0; i < KRNL_MAX_MOUNTS; i++) {
        if (Utils::strcmp(prefix, MountedFilesystems[i].prefix)) {
            return ktrue; // If one was found, return true!
        }
    }

    return kfalse; // If none were found, return false
}

// Mounts a filesystem at the provided prefix
int Storage::mount(char* prefix, Filesystem* fs) {
    // NOTE 1 OF OSDEV: Check if its NULL before ANYTHING
    if (prefix == nullptr) {
        return EINVAL; // Invalid argument 
    }

    // Check if its gonna overflow
    if (MountedFSIndex + 1 >= KRNL_MAX_MOUNTS) {
        Console::Error("VFS: Attempt to mount more partitons than allowed! KRNL_MAX_MOUNTS: %u, prefix: %s, filesystem: %x", KRNL_MAX_MOUNTS, prefix, fs);
        return -ENFILE; // File Table Overflow
    }

    // Check if allready mounter
    if (AllreadyMounted(prefix)) {
        Console::Error("VFS: A partition is allready mounted at '%s'", prefix);

        return -EEXIST; // File Exists
    };
    
    MountedFS NewMountFS; // Create a new mounted filesystem

    NewMountFS.fs = fs;
    NewMountFS.prefix = prefix;

    MountedFilesystems[MountedFSIndex] = NewMountFS; // Set the filesystem in the mount table
    MountedFSIndex++;

    return -SUCCESS; // Return code 0(success)
}

// Unmounts a filesystem at the provided prefix(duh!)
int Storage::unmount(char* prefix) {
    // SAFETY CHECKS! idk why i just need these so we dont crash
    
    // NOTE 1 OF OSDEV: Check if its NULL before ANYTHING
    if (prefix == nullptr) {
        return EINVAL; // Invalid argument 
    }

    // Check if its even mounted
    if (!AllreadyMounted(prefix)) {
        Console::Error("Cannot unmount non mounted prefix %s", prefix);

        return -ENOENT;
    }

    // Check if the prefix is root
    if (Utils::strcmp(prefix, "/")){
        Console::Error("Attempt to unmount root!");

        return -EBUSY; // Ya cant unmount root! - someone, someday
    }

    // Find the correct filesystem to unmount
    for (u8 i = 0; i < KRNL_MAX_MOUNTS; i++) {
        if (Utils::strcmp(prefix, MountedFilesystems[i].prefix)) {
            // WE FOUND OUR FILESYSTEM!
            // Now we unmount :)

            MountedFilesystems[i] = {}; // Unmount the filesystem
        }
    }


    return -SUCCESS;
}