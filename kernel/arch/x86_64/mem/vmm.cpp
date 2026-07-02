#include <mem/vmm.hpp>
#include <mem/pmm.hpp>

using namespace GooseOS;

// Finds the next table in the page table
Memory::VMM::pt_entry* GetNextTable(Memory::VMM::pt_entry* current_table, size index, u64 hhdm_offset) {
    if (!(current_table[index] & Memory::VMM::PTE_PRESENT)) {
        u64 new_table_phys = (u64)Memory::PMM::AllocatePage();
        if (!new_table_phys) {
            return nullptr;
        }

        u8* new_table_virt = (u8*)(new_table_phys + hhdm_offset);
        for (int i = 0; i < 4096; i++) {
            new_table_virt[i] = 0;
        }

        current_table[index] = new_table_phys | Memory::VMM::PTE_PRESENT | Memory::VMM::PTE_WRITABLE;
    }

    return (Memory::VMM::pt_entry*)((current_table[index] & 0x000FFFFFFFFFF000ULL) + hhdm_offset);
}

// NOTE: Expects the PMM to be initilized
// Maps a physical address to a virtual one, if not used on non-mapped addresses it will Page Fault
void Memory::VMM::MapPage(Memory::VMM::pt_entry* pml4_virt, u64 virt_addr, u64 phys_addr, u64 flags, u64 hhdm_offset) {
    size pml4_idx = (virt_addr >> 39) & 0x1FF;
    size pdpt_idx = (virt_addr >> 30) & 0x1FF;
    size pd_idx   = (virt_addr >> 21) & 0x1FF;
    size pt_idx   = (virt_addr >> 12) & 0x1FF;

    Memory::VMM::pt_entry* pdpt = GetNextTable(pml4_virt, pml4_idx, hhdm_offset);
    Memory::VMM::pt_entry* pd   = GetNextTable(pdpt, pdpt_idx, hhdm_offset);
    Memory::VMM::pt_entry* pt   = GetNextTable(pd, pd_idx, hhdm_offset);

    pt[pt_idx] = (phys_addr & 0x000FFFFFFFFFF000ULL) | flags | Memory::VMM::PTE_PRESENT;
    
    asm volatile("invlpg (%0)" :: "r"(virt_addr) : "memory");
}