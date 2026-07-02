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

// This namespace provides basic utilites that dont need a new namespace
namespace GooseOS::Utils {
    // Vector2 position struct
    struct Vector2 {
        u64 x;
        u64 y;
    };

    // Sets the bytes at DEST to amount of bytes from CH specifed by COUNT
    // Aka, fancy memory set stuff
    inline void* memset(void* dest, int ch, size count) {
        u8* ptr = (u8*)dest;
        for (size i = 0; i < count; i++) {
            ptr[i] = (u8)ch;
        }
        return dest;
    }
}