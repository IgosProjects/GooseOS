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
#include <blockdev/blockdevice.hpp>
#include <types.hpp>

// NOTICE: The two following structs are licensed under the GPLv2 license
// They are also comp

// VNode(Generic file type)
struct VNode {
    const char* name;
    u64 size;
    u32 mode; // Permissions
    void* private_data; // Filesystem specific data(any pointer to anywhere)
};

// File handle(per process opened file)
struct FileHandle {
    VNode* vnode;
    u64 offset;
    u32 flags; // O_RDONLY, O_WRONLY, etc
};

namespace GooseOS::Storage {
    using VnodeOpenFn = int(*)(VNode* vnode, FileHandle** out);
    using VnodeReadFn = ssize(*)(FileHandle* handle, void* buffer, size count);
    using VnodeWriteFn = ssize(*)(FileHandle* handle, const void* buffer, size count);
    using VnodeCloseFn = int(*)(FileHandle* handle);

    // Global filesystem implementation
    struct Filesystem {
        VnodeOpenFn open; // Gets the file handle for the specific file

        VnodeReadFn read; // Reads the file from a file handle
        VnodeWriteFn write; // Writes to the file from a file handle

        VnodeCloseFn close; // Closes the file and gets rid of the handle
    };


    // Mounting and unmounting functions
    int mount(char* prefix, Filesystem* fs);
    int unmount(char* prefix);

    // SYSCALL ENTRY FUNCTIONS, These functions convert the calls to the proper FS calls.
    // Used for syscalls!

    VnodeOpenFn open; // Gets the file handle for the specific file

    VnodeReadFn read; // Reads the file from a file handle
    VnodeWriteFn write; // Writes to the file from a file handle

    VnodeCloseFn close; // Closes the file and gets rid of the handle
}
