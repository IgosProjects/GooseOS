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

global outb
global inb
global outw
global inw

; Sends out a byte value on a port
outb:
    mov dx, di
    mov al, sil
    out dx, al ; Output the value
    ret ; Return

; Reads a byte from a port
inb:
    mov dx, di
    in al, dx ; Read a value from the port
    ret ; Return

; Outputs a 16 bit value to a port
outw:
    mov dx, di
    mov ax, si
    out dx, ax
    ret

; Reads a 16 bit value from a port
inw:
    mov dx, di
    in ax, dx
    ret