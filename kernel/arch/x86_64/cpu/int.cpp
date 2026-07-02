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

struct interrupt_frame {
    u64 r15, r14, r13, r12, r11, r10, r9, r8;
    u64 rbp, rdi, rsi, rdx, rcx, rbx, rax;

    u64 int_no;
    u64 error_code;

    u64 rip;
    u64 cs;
    u64 rflags;
} __attribute__((packed));

using namespace GooseOS;

// INTERNAL TO DRIVER
// This function gets the core id using the GS register, used to know what processor encountered the error!
u32 GetCoreIDViaGS() {
    u32 id;
    // Using "eax" or a 32-bit register constraint forces a 32-bit read
    asm volatile("mov %0, %%gs:0" : "=r"(id)); 
    return id;
}

// Handler for CPU issued ISR exceptions
extern "C" void ISRHandler(interrupt_frame* int_frame) {
    //u64 CoreID = GetCoreIDViaGS();
    u64 CoreID = 0;

    Core::Panic("Core %u has encountered a %s", CoreID, ISRNames[int_frame->int_no]); // Panic with the correct name and core
}

// Handler for external device interrupts
extern "C" void IRQHandler() {
    Console::OK("Recived IRQ!");

    CPU::APIC::SendEOI();
}