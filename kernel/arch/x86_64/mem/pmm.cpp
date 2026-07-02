#include <types.hpp>
#include <limine/limine.h>
#include <core.hpp>
#include <mem/pmm.hpp>
#include <utils.hpp>

using namespace GooseOS;

extern "C" volatile struct limine_memmap_request memmap_request; // Get the memmap request from Limine

// Bitmap so we know what is free or taken up
uint8_t* bitmap = nullptr;
size total_pages = 0;
size bitmap_size = 0;

void SetBit(size page_idx)   { bitmap[page_idx / 8] |=  (1 << (page_idx % 8)); }
void ClearBit(size page_idx) { bitmap[page_idx / 8] &= ~(1 << (page_idx % 8)); }
bl TestBit(size page_idx)  { return (bitmap[page_idx / 8] & (1 << (page_idx % 8))); }

// Initilizes the PMM using the provided HHDM offset
void Memory::PMM::Init(u64 HHDMOffset) {
    size entry_count = memmap_request.response->entry_count;
    u64 highest_address = 0;

    // Check if Limine sent us a memory map
    //assert((!memmap_request.response != 0), "PMM: Didnt recive memory map from Limine!");

    // Find the top of usable physical memory
    for (size i = 0; i < entry_count; i++) {
        struct limine_memmap_entry* entry = memmap_request.response->entries[i];
        if (entry->type == LIMINE_MEMMAP_USABLE) {
            u64 top = entry->base + entry->length;
            if (top > highest_address) highest_address = top;
        }
    }

    total_pages = highest_address / 4096;
    bitmap_size = total_pages / 8;

    // Find a chunk of RAM that can hold our bitmap
    u64 bitmap_phys_addr = 0;
    for (size i = 0; i < entry_count; i++ ) {
        struct limine_memmap_entry* entry = memmap_request.response->entries[i];
        
        // Make sure we arent in the first 4 KB region(BIOS data and IVT(legacy x86))
        if (entry->type == LIMINE_MEMMAP_USABLE && entry->length >= bitmap_size && entry->base > 0) {
            bitmap_phys_addr = entry->base;
            break;
        }
    }

    // For some reason the compiler doenst like "bitmap_phys_addr == 0"
    // or "bitmap_phys_addr != 0" so "!bitmap_phys_addr != 0" somehow works??
    // Thanks GCC, why?
    //assert((!bitmap_phys_addr != 0), "PMM: Didnt find a memory region to use for the bitmap!");

    // Set the bitmap pointer using Limine higher-half offset
    bitmap = (u8*)(bitmap_phys_addr + HHDMOffset);

    // Lock everything by defualt
    Utils::memset(bitmap, 0xFF, bitmap_size);

    // Free up the usable addresses
    for (size i = 0; i < entry_count; i++) {
        struct limine_memmap_entry* entry = memmap_request.response->entries[i];
        if (entry->type == LIMINE_MEMMAP_USABLE) {
            size start_page = entry->base / 4096;
            size page_count = entry->length / 4096;
            for (size p = 0; p < page_count; p++) {
                ClearBit(start_page + p);
            }
        }
    }

    // FORCE LOCK PAGE 0! We dont want our allocator returning null pointers
    // Cause that whould be bad i guess.
    SetBit(0);

    // Mark the bitmap address as USED so we dont overmark ourself
    size bitmap_start_page = bitmap_phys_addr / 4096;
    size bitmap_page_count = (bitmap_size + 4095) / 4096;
    for (size p = 0; p < bitmap_page_count; p++) {
        SetBit(bitmap_start_page + p);
    }

    // Protect bootloader and kernel data
    for (size i = 0; i < entry_count; i++) {
        struct limine_memmap_entry* entry = memmap_request.response->entries[i];
        
        // If it's NOT usable, OR if it's the kernel/modules/reclaimable memory, lock it up!
        if (entry->type != LIMINE_MEMMAP_USABLE) {
            size start_page = entry->base / 4096;
            // Round up page count to ensure full coverage of unaligned regions
            size page_count = (entry->length + 4095) / 4096;
            
            for (size p = 0; p < page_count; p++) {
                if ((start_page + p) < total_pages) {
                    SetBit(start_page + p);
                }
            }
        }
    }
}

// Allocates a single 4kb page and returns the pointer
void* Memory::PMM::AllocatePage() {
    for (size i = 0; i < total_pages; i++) {
        if (!TestBit(i)) {
            SetBit(i);
            return (void*)(i * 4096); // Get raw address of page
        }
    }

    // If not able to allocate, panic!
    Core::Panic("PMM: Tried to allocate memory but none was left");
    return nullptr;
}