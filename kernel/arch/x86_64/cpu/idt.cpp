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

#include <cpu/idt.hpp>
#include <console/console.hpp>

// Initilizes the IDT driver, doesnt automaticly load it!
// For that, use LoadIDT! Also only run once on GP and load on the GP and all APs
void InitIDT() {
    // The IDT structure is made up of 256 entires(atleast on legacy PIC).
    // Each entry contains a specific structure that tells the CPU where to go on interrupt.
    // We can construct an IDT table very simply!

    
}