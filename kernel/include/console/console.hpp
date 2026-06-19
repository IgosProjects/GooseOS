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
#include <boot/framebuffer.hpp>

// Define the console namespace
namespace GooseOS::Console {
    struct ConsoleState {
        u64 FontScale = 2; // How much to scale the font?
        
        // COLORS
        Graphics::RGBColor ForegroundColor = {255, 255, 255}; // What color should the text be?
        Graphics::RGBColor BackgroundColor = {0, 0, 0}; // What color should the background of the text be?
    };

    // Prints a string to the display
    void PrintString(const char* s);

    // Prints a character to the display
    void PrintChar(const char c);

    // Initilizes the console driver, allows for printing
    void Init(GooseOS::Graphics::Framebuffer* fb);

    // Outputs a string to the display but with "LOG" before it
    void Log(const char* s);

    // Sets the console state to the passed in one
    void SetConsoleState(ConsoleState* state);
}