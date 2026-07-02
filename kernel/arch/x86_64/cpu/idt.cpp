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

// External interrupt functions
// Tell C++ to expect raw, unmangled C/Assembly names
extern "C" {
    // ISRs
    void isr0();
    void isr1();
    void isr2();
    void isr3();
    void isr4();
    void isr5();
    void isr6();
    void isr7();
    void isr8();
    void isr9();
    void isr10();
    void isr11();
    void isr12();
    void isr13();
    void isr14();
    void isr15();
    void isr16();
    void isr17();
    void isr18();
    void isr19();
    void isr20();
    void isr21();
    void isr22();
    void isr23();
    void isr24();
    void isr25();
    void isr26();
    void isr27();
    void isr28();
    void isr29();
    void isr30();
    void isr31();

    void StubEntry();

    // IRQs
    void irq32();
    void irq33();
}

// Array of all the ISR handlers that are called upon exception
void (*ISRHandlers[])() = {
    isr0, isr1, isr2, isr3, isr4, isr5, isr6, isr7, isr8, isr9, isr10,
    isr11, isr12, isr13, isr14, isr15, isr16, isr17, isr18, isr19, isr20,
    isr21, isr22, isr23, isr24, isr25, isr26, isr27, isr28, isr29, isr30,
    isr31
};

// Array of all IRQ handlers called upon external interrupt
void (*IRQHandlers[])() = {
    irq32, irq33
};

#include <cpu/idt.hpp>
#include <math.hpp>
#include <console/console.hpp>

using namespace GooseOS;

struct CPU::IDTEntry IDTTable[256]; // Make the IDT table array with 256 entries
struct CPU::IDTPointer IDTPtr; // Make the IDT pointer to point to the IDT array

// INTERNAL TO DRIVER
// Sets the IDT gate at entrynum to the passed in handler
void SetIDTGate(u64 entrynum, u64 handler) {
    u64 addr = (u64)handler;
    entrynum = Math::Clamp(0, 255, entrynum); // Clamp the IDT entry number to the safe range
    
    struct CPU::IDTEntry& entry = IDTTable[entrynum];

    // Handler offset
    entry.offset_low  = addr & 0xFFFF;
    entry.offset_mid  = (addr >> 16) & 0xFFFF;
    entry.offset_high = (addr >> 32);

    entry.selector = 0x08; // Selector in GDT
    entry.ist = 0; // IST table id(0)
    entry.attrs = 0x8E; // Type and atributes
    entry.zero = 0; // Reserved(allways zero)
}

// Initilizes the IDT driver, doesnt automaticly load it!
// For that, use LoadIDT! Also only run once on GP and load on the GP and all APs
void CPU::InitIDT() {
    // The IDT structure is made up of 255 entires(atleast on legacy PIC).
    // Each entry contains a specific structure that tells the CPU where to go on interrupt.
    // We can construct an IDT table very simply!

    // Set all the ISRs
    for (u8 i = 0; i < 32; i++) {
        //Console::Log("Writing IDT entry for ISR!");
       
        u64 handler_addr = reinterpret_cast<u64>(ISRHandlers[i]);;
        SetIDTGate(i, handler_addr);
    }

    // Set all the IRQs
    for (u8 i = 32; i < 34; i++) {
        u64 handler_addr = reinterpret_cast<u64>(IRQHandlers[i - 32]);
        SetIDTGate(i, handler_addr);
    }

    // Set all the the others to be stubs
    for (u8 i = 34; i < 255; i++) {
        //Console::Log("Writing IDT entry for ISR!");
       
        u64 handler_addr = reinterpret_cast<u64>(StubEntry);
        SetIDTGate(i, handler_addr);
    }
}

extern "C" void idt_load(); // Assembly function to load IDT

// Loads the IDT table, can be called on the GP and all other APs!
void CPU::LoadIDT() {
    IDTPtr.offset = (u64)&IDTTable; // Offset to table in memory
    IDTPtr.size = (sizeof(IDTEntry) * 256) - 1; // Lower by 1 cause idk x86 shit

    idt_load();
}