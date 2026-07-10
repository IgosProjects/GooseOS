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

// This file is responsible for starting up the xAPIC(Legacy APIC), if its not included,
// Interrupts WILL NOT WORK! As so with the entire OS!

#include <apic/apic.hpp>
#include <acpi/acpi.hpp>
#include <console/console.hpp>
#include <limine/limine.h>
#include <types.hpp>
#include <core.hpp>

using namespace GooseOS;

// Initilizes the core specific Local APIC chip
// Can be safely called multiple times for each core! 
void CPU::APIC::InitLAPIC() {
    u32 ecx;
    
    // Check if the x2APIC is even supported
    asm volatile("cpuid" : "=c"(ecx) : "a"(1) : "ebx", "edx");

    // Check if xAPIC supported, if not return
    if (!(ecx & (1 << 21))) {
        Core::Panic("This system does not support the x2APIC! Cannot continue execution!");
        Core::Panic("Panic failed while checking for x2APIC");
    }

    // This code only runs when the x2APIC is supported
    // We can now initilize it and make the CPU start listening to commands

    // Read the APIC base address
    u32 low, high;
    asm volatile("rdmsr" : "=a"(low), "=d"(high) : "c"(0x1B));

    // Enable both the global APIC and the x2APIC
    low |= (1 << 11) | (1 << 10);

    // Write it back and flip the CPU into x2APIC mode
    asm volatile("wrmsr" : : "c"(0x1B), "a"(low), "d"(high) : "memory");

    // Clear the TPR register
    asm volatile("wrmsr" : : "c"(0x808), "a"(0), "d"(0) : "memory");

    // Set the sporious enable value
    u32 spurious_val = 0x1FF; // Bit 8 turned on and vector 0xFF
    asm volatile("wrmsr" : : "c"(0x80F), "a"(spurious_val), "d"(0) : "memory");
}

// Sends End Of Interrupt to the LAPIC, ONLY CALL IN IRQs!
void CPU::APIC::SendEOI() {
    u32 low = 0;
    u32 high = 0;

    // MSR 0x80B is the x2APIC End Of Interrupt register
    asm volatile("wrmsr" :: "a"(low), "d"(high), "c"(0x80B) : "memory");

    // Send an EOI to the IOAPIC
    CPU::APIC::SendIOAPICEOI();
}