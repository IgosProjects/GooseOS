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

#include <core.hpp>
#include <console/console.hpp>
#include <stdarg.h>

using namespace GooseOS;

// Stops execution and the CPU forever!
void Core::Halt() {
    asm volatile("cli");
    asm volatile("hlt");
}

// Causes a kernel panic and displays the screen
// NOTE: Only call on CRITICAL issues!
[[noreturn]] void Core::Panic(const char* r, ...) {
    Console::EmergencyUnlock();
    va_list a;
    va_start(a, r);

    Console::PrintString("C[c,0]\n========================\n\n");
    Console::PrintString("KERNEL PANIC!\n");
    Console::PrintString("\n========================\n\n");
    Console::PrintString("GooseOS has run into a critical error and has to restart!\n");
    Console::PrintString("Please make a new issue on Github so we can fix it.\n");
    Console::PrintString("\n========================\n\n");

    Console::PrintStringInternal(r, a);
    Console::PrintChar('\n');

    Console::PrintString("\n========================\n\n");

    va_end(a);

    Core::Halt();
}