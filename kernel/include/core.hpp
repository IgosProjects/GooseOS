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

// Checks if the passed in condition returns "true" if so, it will not panic
// If the condition returns "false" it will kernel panic
#define assert(condition, message) \
    do { \
        if (!(condition)) { \
            GooseOS::Core::Panic(message); \
        } \
    } while (0)

namespace GooseOS::Core {
    // Causes a kernel panic and displays the screen
    // NOTE: Only call on CRITICAL issues!
    [[noreturn]] void Panic(const char* r, ...);

    // Stops execution and the CPU forever!
    void Halt();
}
