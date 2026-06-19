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
#include <cpu/idt.hpp>
#include <console/console.hpp>
#include <limine/limine.h>
#include <cpu/gdt.hpp>

using namespace GooseOS;
extern volatile struct limine_mp_request mp_request; // Get the Limine MP request

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

// Waits until GP is ready, then starts executes AP code
void APEntry() {
    CPU::InitGDT();
    CPU::LoadIDT();

    WaitUntilGPReady();

    Core::Halt(); // Stop the AP!
}

// Initilizes the needed architecture specific functions(CPU, interrupts, UART, etc, etc)
void Arch::EarlyInit() {
    // Check if we are running an application processor(AP)
    u8 LocalAPICID = GetLocalApicId();

    struct limine_mp_response* mp_response = mp_request.response; // Get the response
    //assert(mp_response != nullptr, "MP response missing during Arch::EarlyInit!"); // Check if valid pointer 

    // If this check if true, this is an AP!
    if (LocalAPICID != mp_response->bsp_lapic_id) {
        // We need to quickly redirect it!
        APEntry();
        return; // If somehow returned, well uhh return
    }

    // NOTE!!
    // The below code only runs on the GP(General Processor) before APs(Application Processors) are ready!

    // Initilize the GDT and code segments
    CPU::InitGDT();

    // Load the IDT(interrupt descriptor table)
    CPU::InitIDT();
    CPU::LoadIDT();
}

// Initilizes the not so needed arch specific functions
void Arch::LateInit() {
    Console::Log("Starting APs!");
    APsSafeToRun = ktrue;

    Console::Log("Welcome to GooseOS on x86_64");
    Console::Log("Finished Arch::LateInit!");
}