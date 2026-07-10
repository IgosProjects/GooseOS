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

#pragma once
#include <types.hpp>

namespace GooseOS::CPU::APIC {
    // Initilizes the core specific Local APIC chip
    // Can be safely called multiple times for each core! 
    void InitLAPIC();

    // Sends End Of Interrupt to the LAPIC, ONLY CALL IN IRQs!
    void SendEOI();

    // Initilizes the motherboard chip I/O APIC
    // Must be called ONCE on the GP
    void InitIOAPIC(u64 HHDMOffset);

    // Sends an EOI(End Of Interrupt) to the IOAPIC
    void SendIOAPICEOI();
}