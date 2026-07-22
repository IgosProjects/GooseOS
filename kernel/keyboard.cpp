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

#include <arch.hpp>
#include <core.hpp>
#include <kconfig.hpp>
#include <io/ports.hpp>
#include <cpu/int.hpp>
#include <console/console.hpp>
#include <input/keyboard.hpp>

using namespace GooseOS;

// US keyboard layout
unsigned char kbd_us[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
  '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0, '\\','z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',   0,
    0,   0, ' '
};

// US keyboard layout(shift, capslock)
unsigned char kbd_us_uppercase[128] = {
    0,  27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '-', '=', '\b',
  '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '|',
    0, '|','Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?',   0,
    0,   0, ' '
};

bl shift = kfalse;

// INTERNAL TO DRIVER
// Interrupt handler for the keyboard
void KeyboardInterruptHandler(CPU::Interrupts::interrupt_frame* int_frame) {
    u8 scancode = IO::inb(0x60);
    
    // NOTE: This handles both left and right shift!
    if (scancode == 0x2A || scancode == 0x36) {shift = true; return;}
    if (scancode == 0xAA || scancode == 0xB6) {shift = false; return;};

    if (scancode & 0x80) return; // Check if the 7th bit is set(key up)

    char letter;

    // Get the normal letter if the not shifted
    if (!shift) letter = kbd_us[scancode];
    if (shift) letter = kbd_us_uppercase[scancode]; // Get the uppercase version of the letter

    if (letter == 0) return; // Check if its a valid ASCII letter

    Console::PrintChar(letter); // Print the inputted letter
}

// Initilizes the keyboard driver and starts accepting keypresses
void Input::Keyboard::Init() {
    Console::INFO("KEYBOARD: Initilizing keyboard driver");
    
    CPU::Interrupts::RegisterInterrupt(KRNL_KEYBOARD_INTERRUPT, (void*)KeyboardInterruptHandler); // Register the interrupt

    // x86 specific!
    // Start and drain all the useless data from PS/2
    #ifdef __x86_64__
        // 2. Wait for the controller to be ready, then send the "Enable First PS/2 Port" command
        // Port 0x64 bit 1 (value 2) must be 0 before writing a command
        while (IO::inb(0x64) & 2); 
        IO::outb(0x64, 0xAE); // Command 0xAE: Enable first PS/2 port

        // 3. Tell the keyboard itself to start scanning keys
        while (IO::inb(0x64) & 2);
        IO::outb(0x60, 0xF4); // Command 0xF4: Enable scanning

        // 4. Thoroughly drain any lingering garbage out of the buffer
        // This ensures the hardware line can drop low to trigger the next edge interrupt
        while (IO::inb(0x64) & 1) {
            IO::inb(0x60);
        }
    #endif
}
