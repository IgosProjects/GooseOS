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
#include <mem/pmm.hpp>
#include <mem/vmm.hpp>
#include <cpu/idt.hpp>
#include <cpu/core.hpp>
#include <console/console.hpp>
#include <limine/limine.h>
#include <apic/apic.hpp>
#include <cpu/gdt.hpp>
#include <acpi/acpi.hpp>

using namespace GooseOS;
extern "C" volatile struct limine_mp_request mp_request; // Get the Limine MP request
extern limine_hhdm_request hhdm_request; // Get the Limine HHDM request

CPU::CoreContext CoreContextes[256]; // Allow up to 16 cores only!
bl APsSafeToRun = kfalse; // Is the system ready to start up APs

// Get the local APIC id, used detect APs
inline u8 GetLocalApicId() {
    u32 eax, ebx, ecx, edx;
    
    // Query CPUID leaf 1
    asm volatile("cpuid"
                 : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
                 : "a"(1));

    // The initial APIC ID is packed into the high byte of EBX (bits 24-31)
    return (ebx >> 24) & 0xFF;
}

void WaitUntilGPReady() {
    while (!APsSafeToRun) {
        asm volatile("pause");
    }
}

// Loads the core context passed in into GS
void LoadCoreContext(CPU::CoreContext* context) {
    uint64_t addr = reinterpret_cast<uint64_t>(context);
    uint32_t low = addr & 0xFFFFFFFF;
    uint32_t high = addr >> 32;
    
    // 0xC0000101 is the IA32_GS_BASE Model Specific Register
    asm volatile("wrmsr" : : "c"(0xC0000101), "a"(low), "d"(high));
}

// Waits until GP is ready, then starts executes AP code
void APEntry(struct limine_mp_info* info) {
    // Here is where EARLY CORE init goes! Not normal code
    CPU::InitGDT();
    CPU::LoadIDT();

    // Set core context(GS)
    uint32_t id = info->processor_id;
    CoreContextes[id].CoreID = id;
    CoreContextes[id].LapicID = info->lapic_id;
    LoadCoreContext(&CoreContextes[id]);

    // Initilize the x2APIC(for IRQs and external interrupts)
    CPU::APIC::InitLAPIC();

    WaitUntilGPReady(); // Wait until its safe and the GP is done loading

    // NOTE: The below code is WHERE everything should go! Its NOT smart to slap all the stuff below this function
    // Cause it will break, severly!
    asm volatile("sti"); // Enable interrupts

    Console::Log("Hi from AP %u!", info->processor_id);
    Core::Halt(); // Stop the AP!
}

// Initilizes the needed architecture specific functions(CPU, interrupts, UART, etc, etc)
void Arch::EarlyInit() {
    // NOTE!!
    // The below code only runs on the GP(General Processor) before APs(Application Processors) are ready!
    
    CoreContextes[0].CoreID= 0;
    CoreContextes[0].LapicID = 0; // FIXME: Get actual LAPIC id
    LoadCoreContext(&CoreContextes[0]);

    // Initilize the GDT and code segments
    CPU::InitGDT();

    // Load the IDT(interrupt descriptor table)
    CPU::InitIDT();
    CPU::LoadIDT();
    
    // Use the "assert" function to check wheter Limine returned an HHDM pointer
    u64 HHDMOffset = hhdm_request.response->offset; // Get the offset from Limine
    
    // FIX ME: Fix the assert, its allways false idk why
    // FIXED BY ME: I fixed it! Somehow but i did!
    assert((hhdm_request.response->offset != 0), "IOAPIC: Failed to recive HHDM from Limine!");

    // Initilize the PMM and VMM so we can correctly map pages
    Memory::PMM::Init(HHDMOffset);

    u64 modern_pml4_phys = (u64)Memory::PMM::AllocatePage();

    // Zero it out
    Memory::VMM::pt_entry* modern_pml4_virt = (Memory::VMM::pt_entry*)(modern_pml4_phys + HHDMOffset);
    for(int i = 0; i < 512; i++) {
        modern_pml4_virt[i] = 0;
    }

    u64 cr3_val;
    asm volatile("mov %%cr3, %0" : "=r"(cr3_val));

    // Clone limines mapping into our table
    Memory::VMM::pt_entry* old_pml4_virt = (Memory::VMM::pt_entry*)((cr3_val & 0x000FFFFFFFFFF000ULL) + HHDMOffset);

    // Copying entries 256 through 511 covers the higher half mappings safely
    for(int i = 256; i < 512; i++) {
        modern_pml4_virt[i] = old_pml4_virt[i];
    }

    // Use our custom page table insteaad of Limine's
    asm volatile("mov %0, %%cr3" :: "r"(modern_pml4_phys) : "memory");

    // Initilize the x2APIC(for IRQs and external interrupts)
    CPU::APIC::InitLAPIC();

    // Initilize the IOAPIC for routing interrupts to cores
    CPU::APIC::InitIOAPIC(HHDMOffset);

    asm volatile("sti"); // Enable interrupts
}

// Initilizes the not so needed arch specific functions
void Arch::LateInit() {
    Console::Log("Starting APs at address %x!", reinterpret_cast<uint64_t>(APEntry));
 
    struct limine_mp_response* mp_response = mp_request.response;
    
    if (mp_response != nullptr) {
        for (u64 i = 0; i < mp_response->cpu_count; i++) {
            struct limine_mp_info* cpu = mp_response->cpus[i];

            // Skip the BSP
            if (cpu->lapic_id == mp_response->bsp_lapic_id) {
                continue;
            }

            // Wake up the AP and run the entry code
            __atomic_store_n(&cpu->goto_address, reinterpret_cast<uint64_t>(APEntry), __ATOMIC_RELEASE);
        }
    }

    APsSafeToRun = ktrue;

    Console::OK("Welcome to GooseOS on x86_64");
    Console::Log("Finished Arch::LateInit!");
}