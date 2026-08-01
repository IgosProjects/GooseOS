#include <types.hpp>

// Generic allocator namespace
namespace GooseOS::Memory::Allocator {
    // Initilizes the memory allocator, in this function we allocate the heap and setup everything needed
    void Init();

    // Allocates a certain amount of bytes from the kernel heap
    void* kmalloc(size amount);

    // Frees the memory at ptr and allows for future allocations there!
    int kfree(void* ptr);
}
