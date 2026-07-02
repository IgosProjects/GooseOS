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

#include <apic/apic.hpp>
#include <acpi/acpi.hpp>
#include <console/console.hpp>
#include <limine/limine.h>
#include <types.hpp>
#include <mem/vmm.hpp>
#include <core.hpp>
#include <io/ports.hpp>

using namespace GooseOS;
extern "C" volatile struct limine_rsdp_request rsdp_request; // Get the Limine RSDP request

// Each entry in the MADT table starts with this 16 bit header
struct __attribute__((packed)) MADTEntryHeader {
    u8 type;
    u8 length;
};

// This is the type 0 entry in the MADT table. It stores info about the IOAPIC
struct __attribute__((packed)) MADTEntryIOAPIC {
    MADTEntryHeader header;
    uint8_t IOAPIC_ID;
    uint8_t reserved;
    uint32_t IOAPICAddress; // Physical memory address of IOAPIC
    uint32_t GlobalSystemInterruptBase;
};

MADTEntryIOAPIC* IOAPICData; // Used to store the IOAPIC data later on

#define IOAPIC_REGSEL  0x00 // Register select
#define IOAPIC_IOWIN   0x10 // IO window(data)

// Parses the MADT table entries
// INTERNAL TO DRIVER
void ParseMADT(struct ACPI::MADT* madt, u64 hhdm_offset) {
    // Calculate exactly where the entires are in memory
    uintptr_t madt_end = (uintptr_t)madt + madt->header.length;
    
    // Start at the first entry right after the global flags
    uintptr_t current_entry = (uintptr_t)madt->entries;
    
    while (current_entry < madt_end) {
        MADTEntryHeader* header = (MADTEntryHeader*)current_entry;
        
        switch (header->type) {
            case 0: { // Processor Local APIC(we dont handle this)
                break;
            }
            case 1: { // IOAPIC
                // Store the IOAPIC info in our pointer for later
                IOAPICData = (MADTEntryIOAPIC*)current_entry;

                break;
            }
            case 2: { // Interrupt Source Overrides(used for keyboards and stuff)
                Console::OK("Recived Interrupt Source Override!");

                break;
            }
        }
        
        // Advance the pointer EXACTLY!
        current_entry += header->length;
    }
}

// Writes a 32 bit value to a IOAPIC register(where base is the IOAPIC MMIO address)
void IOAPICWrite(uint64_t base, uint8_t reg, uint32_t val) {
    volatile uint32_t* regsel = (volatile uint32_t*)(base + IOAPIC_REGSEL);
    volatile uint32_t* iowin  = (volatile uint32_t*)(base + IOAPIC_IOWIN);
    
    *regsel = reg;
    *iowin = val;
}

// Reads and returns a value from the specific IOAPIC register(where base is the IOAPIC MMIO address)
uint32_t IOAPICRead(uint64_t base, uint8_t reg) {
    volatile uint32_t* regsel = (volatile uint32_t*)(base + IOAPIC_REGSEL);
    volatile uint32_t* iowin  = (volatile uint32_t*)(base + IOAPIC_IOWIN);
    
    *regsel = reg;
    return *iowin;
}

// Sets a redirect entry in the IOAPIC(for example you can redirect pin 2 aka keyboard to IRQ 0 in the IDT)
void SetRedirectEntry(uint64_t ioapic_base, uint8_t pin, uint8_t vector, uint8_t core_apic_id) {
    uint8_t low_reg = 0x10 + (pin * 2);
    uint8_t high_reg = low_reg + 1;

    // Low 32-bits configuration:
    // Bit 0-7: The IDT Vector number 
    // Bit 8-10: Delivery Mode(000: Fixed)
    // Bit 11: Destination Mode(0: Physical APIC ID)
    // Bit 12: Delivery Status(Read Only)
    // Bit 13: Pin Polarity(0: Active High)
    // Bit 15: Trigger Mode(0: Edge Triggered)
    // Bit 16: Mask Bit(0: UNMASKED / ENABLED, 1: Disabled)
    uint32_t low_val = vector; // Mask bit is 0, so its unmasked

    // High 32-bits configuration:
    // Bit 56-63: Destination field(The APIC ID of the target CPU core)
    uint32_t high_val = (uint32_t)core_apic_id << 24;

    IOAPICWrite(ioapic_base, low_reg, low_val);
    IOAPICWrite(ioapic_base, high_reg, high_val);
}

// INTERNAL TO DRIVER
// Masks an IOAPIC pin so that interrupt doesnt fire
void MaskIOAPICPin(uint8_t pin, u64 base) {
    // Redirection table entries start at 0x10. Each entry is 2 registers wide (8 bytes).
    uint8_t low_reg_offset = 0x10 + (pin * 2);

    // Read the current lower 32-bits of the entry
    uint32_t low_window = IOAPICRead(base, low_reg_offset);

    // Bit 16 is the Interrupt Mask bit. Flip it to 1 to disable.
    low_window |= (1 << 16);

    // Write it back to mask the interrupt
    IOAPICWrite(base, low_reg_offset, low_window);
}

// Initilizes the motherboard chip I/O APIC
// Must be called ONCE on the GP
void CPU::APIC::InitIOAPIC(u64 HHDMOffset) {
    // Mask both legacy PICs
    IO::outb(0x21, 0xFF); // Mask master PIC
    IO::outb(0xA1, 0xFF); // Mask slave PIC

    // Check if limine sent an RSDP pointer
    assert((rsdp_request.response != nullptr), "IOAPIC: Limine did not send an RSDP pointer!");

    // Get the ACPI MADT table pointer
    void* MADTPointer = ACPI::GetTable((ACPI::RSDP*)rsdp_request.response->address, HHDMOffset, "APIC");

    // Print the MADT pointer to the display
    Console::INFO("IOAPIC: Located MADT at 0x%x", MADTPointer);

    // Get the other needed info to start the IOAPIC
    struct ACPI::MADT* MADTTable = (ACPI::MADT*)MADTPointer;

    // Print misc info
    Console::INFO("IOAPIC: MADT table");
    Console::INFO("IOAPIC: LAPIC address: 0x%x Flags: 0x%x Entries(pointer to): 0x%x", MADTTable->local_apic_address, MADTTable->flags, MADTTable->entries);

    // Now that we have the MADT table, we can parse it and get the IOAPIC address and stuff
    ParseMADT(MADTTable, HHDMOffset);

    // Now the IOAPIC address should be filled in, along with the other info
    // Lets print it!
    Console::INFO("IOAPIC: IOAPIC info");
    Console::INFO("IOAPIC: GlobalSystemInterruptBase: 0x%x IOAPIC_ID: 0x%x IOAPICAddress: 0x%x", IOAPICData->GlobalSystemInterruptBase, IOAPICData->IOAPIC_ID, IOAPICData->IOAPICAddress);

    u64 IOAPICAddress = IOAPICData->IOAPICAddress + HHDMOffset;
    u64 PhysAddress = IOAPICData->IOAPICAddress;

    // Get the PML4 virutal address
    u64 cr3_val;
    asm volatile("mov %%cr3, %0" : "=r"(cr3_val));

    // Clear out the lower 12 bits(to be safe)
    u64 pml4_phys = cr3_val & 0x000FFFFFFFFFF000ULL;
    
    // Convert it to a virutal pointer with our HHDM offset
    Memory::VMM::pt_entry* pml4_virt = (Memory::VMM::pt_entry*)(pml4_phys + HHDMOffset);

    // Now lets map the IOAPIC in the page tables
    Memory::VMM::MapPage(pml4_virt, IOAPICAddress, PhysAddress, GooseOS::Memory::VMM::PTE_WRITABLE | GooseOS::Memory::VMM::PTE_WRITABLE | (1ULL << 3) | (1ULL << 4), HHDMOffset);
    
    // Now set the needed redirects
    //SetRedirectEntry(IOAPICAddress, 1, 0x20, 0);
    MaskIOAPICPin(1, IOAPICAddress);

    SetRedirectEntry(IOAPICAddress, 2, 0x21, 0); // Redirect pin 2(keyboard) to entry 32(0x21) in the IDT on core 0(GP)
}