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

#include <boot/framebuffer.hpp>
#include <arch.hpp>
#include <core.hpp>
#include <console/console.hpp>

using namespace GooseOS;

// Kernel mode init function
extern "C" void InitKernel() {
	// This function is called by the ASM boot code. In here we initilize all the subsystems and start userspace
	// But for now, we dont do much!

	// Call Arch::EarlyInit to setup the interrupts and other important architecture stuff
	Arch::EarlyInit();

	// Initilize the console
	Graphics::Framebuffer* fb = GooseOS::Graphics::GetCurrentFramebuffer();
	Console::Init(fb);

	Arch::LateInit(); // Call the Arch::LateInit function to do not so critical stuff

	for (;;) {
		asm volatile("hlt");
	}
}
