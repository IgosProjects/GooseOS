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
global ISREntry
global StubEntry
extern ISRHandler
global idt_load
extern IDTPtr

; This function is called externally by C++ to load the IDT!
idt_load:
	lidt [IDTPtr]
	ret

; This is the function called by the CPU when an exception happens,
; we CANNOT use C or a higher level language cause the CPU pushes stuff on the stack depending on if its with and error code or not!
; So we have to prepare the stack for our C++ handler to run
ISREntry:
    ; Save registers
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15

    ; Now the stack is ready for C++, so we can call our ISRHandler function defined in C++
    mov rdi, rsp ; Push the stack to C++
    call ISRHandler

    ; Restore the registers
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax

    add rsp, 16 ; Remove error code and interrupt number from stack
    iretq ; Return from interrupt

; This is used in testing to prevent the CPU freaking out before IRQs are registered.
; And used cause we dont have APIC support yet
StubEntry:
    iret ; Return from interrupt

%macro isr_err_stub 1
global isr%1

isr%1:
    ; CPU already pushed error code
    push qword %1 ; Push interrupt number
    jmp ISREntry
%endmacro

%macro isr_no_err_stub 1
global isr%1

isr%1:
    push qword 0 ; Push dummy code
    push qword %1 ; Push interrupt number
    jmp ISREntry
%endmacro

isr_no_err_stub 0 ; ISR 0
isr_no_err_stub 1 ; ISR 1
isr_no_err_stub 2 ; ISR 2
isr_no_err_stub 3 ; ISR 3
isr_no_err_stub 4 ; ISR 4
isr_no_err_stub 5 ; ISR 5
isr_no_err_stub 6 ; ISR 6
isr_no_err_stub 7 ; ISR 7
isr_err_stub 8 ; ISR 8(double fault)
isr_no_err_stub 9 ; ISR 9
isr_err_stub 10 ; ISR 10
isr_err_stub 11 ; ISR 11
isr_err_stub 12 ; ISR 12
isr_err_stub 13 ; ISR 13
isr_err_stub 14 ; ISR 14
isr_no_err_stub 15 ; ISR 15
isr_no_err_stub 16 ; ISR 16
isr_err_stub 17 ; ISR 17
isr_no_err_stub 18 ; ISR 18
isr_no_err_stub 19 ; ISR 19
isr_no_err_stub 20 ; ISR 20
isr_err_stub 21 ; ISR 21
isr_no_err_stub 22 ; Reserved for future use
isr_no_err_stub 23 ; Reserved for future use
isr_no_err_stub 24 ; Reserved for future use
isr_no_err_stub 25 ; Reserved for future use
isr_no_err_stub 26 ; Reserved for future use
isr_no_err_stub 27 ; Reserved for future use
isr_no_err_stub 28 ; ISR 28
isr_err_stub 29 ; ISR 29
isr_err_stub 30 ; ISR 30
isr_no_err_stub 31 ; Reserved for future use