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

#include <types.hpp>
#include <console/serial.hpp>
#include <io/ports.hpp>

using namespace GooseOS;

u16 COM1 = 0x3F8; // First COM port on an x86 PC

// Initilizes the serial subsystem
void Console::Serial::Init() {
    IO::outb(COM1 + 1, 0); // Disable UART interrupts by writing 0 to COM1 + 1
    IO::outb(COM1 + 3, IO::inb(COM1 + 3) | 0x80); // Enable DLAB

    // Now we can set the baude rate
    IO::outb(COM1 + 0, 0x03); // Low byte
    IO::outb(COM1 + 1, 0x00); // High byte

    IO::outb(COM1 + 3, IO::inb(COM1 + 3) & ~0x80); // Disable DLAB
    IO::outb(COM1 + 3, 0x03);

    IO::outb(COM1 + 2, 0xC7); // Enable FIFO
    IO::outb(COM1 + 2, 0x08); // Enable modem control
}

// Prints a character to the serial port
void Console::Serial::PrintChar(const char c) {
    while ((IO::inb(COM1 + 5) & 0x20) == 0); // Wait until serial is safe to write
    IO::outb(COM1, c); // Send character
}