#include <acpi/acpi.hpp>
#include <types.hpp>

using namespace GooseOS;

// Returns the pointer to a specific ACPI table based on the 4 char signature
void* ACPI::GetTable(ACPI::RSDP* rsdp, u64 hhdm_offset, const char* sig) {
    XSDT* xsdt = (XSDT*)(rsdp->xsdt_address + hhdm_offset);
    size entries = (xsdt->header.length - sizeof(ACPIHeader)) / sizeof(u64);
    
    for (size i = 0; i < entries; i++) {
        ACPIHeader* table = (ACPIHeader*)(xsdt->tables[i] + hhdm_offset);
        
        // Check if the signature is matching by using it as an 32 bit int
        if (*(u32*)table->signature == *(u32*)sig) {
            return (void*)table;
        }
    }
    
    return nullptr;
}