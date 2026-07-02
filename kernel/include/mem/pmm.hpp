#include <types.hpp>

namespace GooseOS::Memory::PMM {
    // Initilizes the PMM using the provided HHDM offset
    void Init(u64 HHDMOffset);

    // Allocates a single 4kb page and returns the pointer
    void* AllocatePage();
} // namespace GooseOS::Memory::PMM
