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

    // Copies N amount of bytes from SRC to DEST
    inline void* memcpy(void* dest, const void* src, size n) {
        // Cast the void pointers to byte pointers, that way we can read them byte by byte
        unsigned char* d = (unsigned char*)dest;
        const unsigned char* s = (const unsigned char*)src;

        // Loop through and copy every single byte
        for (size i = 0; i < n; i++) {
            d[i] = s[i];
        }

        // memcpy always returns the original destination pointer
        return dest;
    }

    // Checks if S1 and S2 are the same string, if so it returns true
    inline bl strcmp(const char* s1, const char* s2) {
        while (*s1 && (*s1 == *s2)) {
            s1++;
            s2++;
        }
        return *s1 == *s2;
    }
}