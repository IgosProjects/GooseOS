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
global _start
extern InitKernel

; Entry function called by the bootloader
_start:
    call InitKernel ; Call the function defined in "init.cpp"

    ; If the kernel returned we should halt the CPU

    cli
    hlt
