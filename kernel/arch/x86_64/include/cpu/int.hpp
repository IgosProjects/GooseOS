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

// Main interrupt namespace
namespace GooseOS::CPU::Interrupts {
    struct interrupt_frame {
        u64 r15, r14, r13, r12, r11, r10, r9, r8;
        u64 rbp, rdi, rsi, rdx, rcx, rbx, rax;

        u64 int_no;
        u64 error_code;

        u64 rip;
        u64 cs;
        u64 rflags;
    } __attribute__((packed));


    // Registers a new interrupt handler
    void RegisterInterrupt(u64 int_num, void* handler);
}