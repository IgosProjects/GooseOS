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
#include <core.hpp>
#include <kconfig.hpp>
#include <mem/pmm.hpp>
#include <mem/vmm.hpp>
#include <mem/alloc.hpp>
#include <mem/alloc-backend.hpp>
#include <console/console.hpp>

using namespace GooseOS;

u64 MEM_HeapSizeInKiB = KRNL_MEM_HEAP_SIZE * 1024; // Convert KiB into bytes
void* MEM_HeapStartPointer = nullptr;
void* MEM_HeapEndPointer = nullptr;
void* MEM_CurrentHeapPointer = nullptr; // Allocation pointer, moved forward in kmalloc

bl IsOverflowingHeap(size amount) {
    if ((uintptr_t)MEM_CurrentHeapPointer + amount >= (uintptr_t)MEM_HeapEndPointer) {
        return ktrue;
    }

    return kfalse;
}

// Allocates a certain amount of bytes from the kernel heap
void* Memory::Allocator::kmalloc(size amount) {
    // Since this is a simple bump allocator implementation, we need to:
    // 1. Check if there is enough of the heap left, if not we allocate more until yes
    // 2. Allocate the memory on the heap
    // 3. Move the heap pointer forward
    
    // But the 0th step! Check for null!
    assert((MEM_HeapStartPointer != nullptr), "MEM: Attempt to allocate memory before initilizing allocator");

    bl IsHeapOverflowing = IsOverflowingHeap(amount);

    // If the heap is overflowing, allocate more pages for the heap!
    if (IsHeapOverflowing) {
        // Calculate the amount of needed pages for the heap to grow
        // DO NOT DELETE OR "MODERNIZE": The below check rounds it up so 4097 is 2 pages not 1!
        u64 AmountOfNeededPages = (amount + 4095) / 4096;

        // Allocate all the needed pages and add them to the heap
        for (u64 PageIndex = 0; PageIndex < AmountOfNeededPages; PageIndex++) {
            u64 NewlyAllocatedPage = (u64)Memory::AllocatorBackend::AllocatePageAt((uintptr_t)MEM_HeapEndPointer + (4 * 1024)); // Each page is 4 KiB

            MEM_HeapEndPointer = (void*)NewlyAllocatedPage;
            Console::INFO("ALLOC: Allocated new heap page at %x", (u64)NewlyAllocatedPage);
        }
    }
    
    // Now lets move the pointer forward by the specified amount
    MEM_CurrentHeapPointer += amount;

    // Return the pointer to the heap
    return MEM_CurrentHeapPointer - amount; // Start of the segment not end!
}

// Frees the memory at ptr and allows for future allocations there!
int kfree(void* ptr) {
    /// NOT IMPLEMENTED!

    // Check for nullptr
    if (ptr == nullptr) {
        return Errno::EINVAL; // Invalid argument
    }

    Console::Error("Attempt to call kfree, but bump allocator doesnt support it!");
    return Errno::ENOSYS;
}

// Initilizes the memory allocator, in this function we allocate the heap and setup everything needed
void Memory::Allocator::Init() {
    Console::INFO("KMALLOC: Initilizing allocator!");

    // Allocate the inital heap start page
    // This is where we know is a SAFE spot for our heap
    u64 LastAllocatedPage = (u64)Memory::AllocatorBackend::GetHeapStartPage();

    // Print a debug statement
    Console::INFO("ALLOC: Inital heap page at %x", LastAllocatedPage);

    MEM_HeapStartPointer = (void*)LastAllocatedPage;
    MEM_CurrentHeapPointer = MEM_HeapStartPointer;

    u64 AmountOfNeededPages = MEM_HeapSizeInKiB / 4096;

    // Allocate all the needed pages for the heap
    for (u64 PageIndex = 0; PageIndex < AmountOfNeededPages - 1; PageIndex++) {
        u64 NewlyAllocatedPage = (u64)Memory::AllocatorBackend::AllocatePageAt(LastAllocatedPage + (4 * 1024)); // Each page is 4 KiB

        LastAllocatedPage = NewlyAllocatedPage;
        Console::INFO("ALLOC: Allocated new heap page at %x", LastAllocatedPage);
    }

    // This code runs when we have allready allocated the last page of the heap
    // This is where the end marker is set
    MEM_HeapEndPointer = (void*)(LastAllocatedPage + (1024 * 4)); // Since the pointer points to the start of the page, not the end we gotta move it to the end
}