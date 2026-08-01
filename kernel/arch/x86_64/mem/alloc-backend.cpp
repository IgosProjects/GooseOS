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

#include <mem/pmm.hpp>
#include <mem/vmm.hpp>
#include <mem/alloc-backend.hpp>
#include <limine/limine.h>

using namespace GooseOS;
extern limine_hhdm_request hhdm_request;

// This file is a generic allocator backend used to seperate architecture logic and normal allocator logic
// This is NOT the allocator!

extern "C" char kernel_end[];

// Returns a safe start page for the allocator heap
// Usually right after the kernel ends
void* Memory::AllocatorBackend::GetHeapStartPage() {
    return (void*)kernel_end; // The linker allready made this a virtual address since we are allready loaded in the higher half
}

// Allocates a single 4kb page in memory, virt_addr is where it will start
void* Memory::AllocatorBackend::AllocatePageAt(uintptr_t virt_addr) {
    void* AllocatedPagePointer = Memory::PMM::AllocatePage(); // Allocate the page
    
    // Now we need to map it in the page table, this is so we prevent Page Faults
    // But first, find the PML4 table. The PML4 is the root table in x86_64 paging and its allways stored in CR3(Control Register 3)

    // Get the PML4 virutal address by reading CR3
    u64 cr3_val;
    asm volatile("mov %%cr3, %0" : "=r"(cr3_val));

    // Clear out the lower 12 bits(to be safe)
    u64 pml4_phys = cr3_val & 0x000FFFFFFFFFF000ULL;
    
    Memory::VMM::MapPage((Memory::VMM::pt_entry*)((u64)pml4_phys + hhdm_request.response->offset), virt_addr, (u64)AllocatedPagePointer, Memory::VMM::PTE_WRITABLE, hhdm_request.response->offset);
    
    return (void*)virt_addr;
}

// Maps a physical to a virtual address
void Memory::AllocatorBackend::MapVirtual(uintptr_t virt_addr, uintptr_t phys_addr) {
    // Get the PML4 virutal address by reading CR3
    u64 cr3_val;
    asm volatile("mov %%cr3, %0" : "=r"(cr3_val));

    // Clear out the lower 12 bits(to be safe)
    u64 pml4_phys = cr3_val & 0x000FFFFFFFFFF000ULL;

    // Map the address in the page tables using the VMM
    Memory::VMM::MapPage((Memory::VMM::pt_entry*)((u64)pml4_phys + hhdm_request.response->offset), virt_addr, phys_addr, Memory::VMM::PTE_WRITABLE, hhdm_request.response->offset);
}

