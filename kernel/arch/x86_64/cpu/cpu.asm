; *
; *	This file is part of gooseOS.
; *
; *	gooseOS is free software: you can redistribute it and/or modify
; *	it under the terms of the GNU General Public License as published by
; *	the Free Software Foundation, either version 3 of the License, or
; *	(at your option) any later version.
; *
; *	gooseOS is distributed in the hope that it will be useful,
; *	but WITHOUT ANY WARRANTY; without even the implied warranty of
; *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
; *	GNU General Public License for more details.
; *
; *	You should have received a copy of the GNU General Public License
; *	along with gooseOS.  If not, see <https://www.gnu.org/licenses/>.
; *
; *	Copyright(c) 2026 EyeDev
; *

[BITS 64] ; We are in 64 bit long mode
section .text ; The text section contains our code

global LoadGDT

; Internal function to load the GDT
LoadGDT:
    lgdt [rdi] ; Load the GDT from the pointer passed in as first argument

    ; Set all standard data segment selectors to point to our Data Segment
    mov ax, 0x10          
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; Reset CS by using a far return
    pop rsi ; Pop the return address
    mov rax, rsp ; Store the original stack

    push qword 0x10 ; Push SS(Stack Segment)
    push rax ; Push stack pointer
    pushfq ; Push RFLAGS
    push qword 0x08 ; Push CS(Code Segment)
    push rsi ; Push the return address back to the stack
    iretq