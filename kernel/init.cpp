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

#include <blockdev/blockdevice.hpp>
#include <blockdev/ramdisk.hpp>
#include <boot/framebuffer.hpp>
#include <input/keyboard.hpp>
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

	// Initilize the keyboard
	Input::Keyboard::Init();

	// Initilize the root filesystem, so we can run apps and read files
	Storage::BlockDevice root_blk_dev; // Create an empty block device
	Storage::Ramdisk::Init(&root_blk_dev);

	// READ TEST
	Console::INFO("Testing reading from ramdisk!");
	
	char TestBuffer[512]; // Create an empty 512 byte test buffer
	root_blk_dev.ReadSector(TestBuffer, 0, root_blk_dev); // Read the first sector

	Console::PrintChar(TestBuffer[0]);
	Console::PrintChar(TestBuffer[1]);
	Console::PrintChar(TestBuffer[2]);
	Console::PrintChar(TestBuffer[3]);
	Console::PrintChar(TestBuffer[4]);

	Console::INFO("Framebuffer Address: 0x%x", fb->addr);

	Arch::LateInit(); // Call the Arch::LateInit function to do not so critical stuff

	Console::PrintString("C[c,3]");
	Console::PrintString("C[r,]");

	for (;;) {
		asm volatile("hlt");
	}
}
