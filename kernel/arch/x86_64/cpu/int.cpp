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

// This file contains handlers for CPU interrupts, there are 2 type of them!
// ISRs are CPU issued exception interrupts,
// white IRQs are issued by external hardware(keyboard, mouse, timer)

#include <core.hpp>
#include <console/console.hpp>
#include <cpu/int.hpp>
#include <io/ports.hpp>
#include <apic/apic.hpp>

// Used to get ISR names when fired
const char* ISRNames[32] = {
    "Divide By Zero Exception",          // 0
    "Debug Exception",                   // 1
    "Non-Maskable Interrupt",            // 2
    "Breakpoint Exception",              // 3
    "Overflow Exception",                // 4
    "Bound Range Exceeded",              // 5
    "Invalid Opcode",                    // 6
    "Device Not Available",              // 7
    "Double Fault",                      // 8
    "Coprocessor Segment Overrun",       // 9 (legacy)
    "Invalid TSS",                       // 10
    "Segment Not Present",               // 11
    "Stack-Segment Fault",               // 12
    "General Protection Fault",         // 13
    "Page Fault",                        // 14
    "Reserved",                          // 15
    "x87 Floating-Point Exception",      // 16
    "Alignment Check",                   // 17
    "Machine Check",                     // 18
    "SIMD Floating-Point Exception",     // 19
    "Virtualization Exception",          // 20
    "Control Protection Exception",      // 21
    "Reserved",                          // 22
    "Reserved",                          // 23
    "Reserved",                          // 24
    "Reserved",                          // 25
    "Reserved",                          // 26
    "Reserved",                          // 27
    "Hypervisor Injection Exception",    // 28
    "VMM Communication Exception",       // 29
    "Security Exception",                // 30
    "Reserved"                           // 31
};

using namespace GooseOS;
using HandlerFunc = void(*)(GooseOS::CPU::Interrupts::interrupt_frame* frame);

// Make the handler table
HandlerFunc ExternalInterruptHandlers[32] = { nullptr };

// INTERNAL TO DRIVER
// This function gets the core id using the GS register, used to know what processor encountered the error!
u32 GetCoreIDViaGS() {
    u32 id;
    // Using "eax" or a 32-bit register constraint forces a 32-bit read
    asm volatile("mov %0, %%gs:0" : "=r"(id)); 
    return id;
}

// Handler for CPU issued ISR exceptions
extern "C" void ISRHandler(GooseOS::CPU::Interrupts::interrupt_frame* int_frame) {
    //u64 CoreID = GetCoreIDViaGS();
    u64 CoreID = 0;

    Core::Panic("Core %u has encountered a %s", CoreID, ISRNames[int_frame->int_no]); // Panic with the correct name and core
}

// Normal printf that doesnt need direct vaargs, doenst lock
inline void LogFromIRQ(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    
    // Call the internal PrintString(no spinlocks!)
    Console::PrintStringInternal(fmt, args);
    
    va_end(args);
}

// Registers a new interrupt handler
void CPU::Interrupts::RegisterInterrupt(u64 int_num, void* handler) {
    // Check if in the valid IRQ range
    if (int_num > 256) {LogFromIRQ("CPU::Intterupts::RegisterInterrupt: The x86_64 arch cannot register an interrupt higher than 256, provided id: %u", int_num); return;}
    if (int_num < 31) {LogFromIRQ("CPU::Intterupts::RegisterInterrupt: The x86_64 arch cannot register an interrupt lower than 31, provided id: %u", int_num); return;}

    // Check if a valid pointer
    if (!handler) {LogFromIRQ("CPU::Intterupts::RegisterInterrupt: Invalid handler function pointer for interrupt %u, handler %x", int_num, (u64*)handler); return;}

    ExternalInterruptHandlers[int_num - 32] = (HandlerFunc)handler;
}

// Handler for external device interrupts
extern "C" void IRQHandler(GooseOS::CPU::Interrupts::interrupt_frame* int_frame) {
    // Check if a handler was registered, we do - 32 so we dont go out of bounds
    if (ExternalInterruptHandlers[int_frame->int_no - 32]) {
        ExternalInterruptHandlers[int_frame->int_no - 32](int_frame); // Call the function
    } else {
        // PLEASE DONT FIX: The below IF statement will PREVENT pagefaults
        // It skips the timer printing if not registered!
        if (int_frame->int_no == 32) goto sendeoi;

        Console::INFO("No interrupt handler registered for interrupt %u", int_frame->int_no);
    }

    sendeoi:

    CPU::APIC::SendEOI();
}