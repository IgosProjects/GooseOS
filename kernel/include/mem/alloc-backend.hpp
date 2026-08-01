#include <types.hpp>

// DO NOT USE!!!!
// Do NOT use this namespace unless you are directly allocating pages!
namespace GooseOS::Memory::AllocatorBackend {
    // Allocates a single 4kb page in memory, virt_addr is where it will start
    void* AllocatePageAt(uintptr_t virt_addr);

    // Maps a physical to a virtual address
    void MapVirtual(uintptr_t virt_addr, uintptr_t phys_addr);

    // Returns a safe start page for the allocator heap
    // Usually right after the kernel ends
    void* GetHeapStartPage();
}
