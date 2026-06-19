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
#include <console/console.hpp>
#include <math.hpp>
#include <utils.hpp>
#include <console/serial.hpp>
#include <console/font-8x8.h>
#include <types.hpp>
#include <spinlock.hpp>

using namespace GooseOS;

GooseOS::Spinlock ConsoleLock;
bl IsConsoleReady = kfalse;

// This is the x86_64 implemenation of the console driver, the console driver needs to provide basic functions
// Used to print to the display and such

// Pointer to framebuffer
static Graphics::Framebuffer* fb_ptr;

// Position of the cursor on the screen
Utils::Vector2 CursorPosition;

// Console state variable
Console::ConsoleState ConsoleConfiguration;

// INTERNAL CONSOLE CONFIG START

// Minimum and maximum font scaling
u64 MaxFontScale = 5;
u64 MinFontScale = 0;

// Terminal color pallete

Graphics::RGBColor TerminalPallete[8] = {
    {255, 0, 0}, // Red
    {0, 255, 0}, // Blue
    {0, 0, 255}, // Green
    {0, 0, 0}, // Black
    {255, 255, 255}, // White
    {0, 189, 252}, // Cyan
    {252, 223, 0}, // Yellow
    {0, 72, 252} // Darkblue
};

// Calculate the terminal pallete size
constexpr u64 TerminalPaletteSize = sizeof(TerminalPallete) / sizeof(TerminalPallete[0]); 

// INTERNAL CONSOLE CONFIG END

// INTERNAL TO DRIVER
// This internal function draws a character at the provided X and Y positions
void DrawChar(Graphics::Framebuffer* fb, char c, Utils::Vector2 position) {
    const u8* glyph = font8x8_basic[(u8)c];

    u64 scale = ConsoleConfiguration.FontScale;

    u64 CursorX = position.x;
    u64 CursorY = position.y;

    for (u32 row = 0; row < 8; row++)
    {
        for (u32 col = 0; col < 8; col++)
        {
            if (glyph[row] & (1 << col))
            {
                for (u32 dy = 0; dy < scale; dy++)
                {
                    for (u32 dx = 0; dx < scale; dx++) {
                        DrawPixel(fb, {CursorX + col * scale + dx, CursorY + row * scale + dy}, ConsoleConfiguration.ForegroundColor);
                    }
                }
            } else {
                for (u32 dy = 0; dy < scale; dy++) {
                    for (u32 dx = 0; dx < scale; dx++) {
                        DrawPixel(fb, {CursorX + col * scale + dx, CursorY + row * scale + dy}, ConsoleConfiguration.BackgroundColor);
                    }
                }
            }
        }
    }
}

// Prints a character to the display
void Console::PrintChar(const char c) {
    Serial::PrintChar(c);
    
    // Handle newlines
    if (c == '\n') {
        CursorPosition.x = 0; // CR
        CursorPosition.y += 1; // LF

        return;
    }
    
    Utils::Vector2 PixelPosition = {CursorPosition.x * (8 * ConsoleConfiguration.FontScale), CursorPosition.y * (8 * ConsoleConfiguration.FontScale)}; // Convert from our positition to pixels
    DrawChar(fb_ptr, c, PixelPosition);
    
    CursorPosition.x += 1; // Increase grid position by one

    // Check if offscreen
    if (CursorPosition.x * 8 >= fb_ptr->width) {
        // If the pixel position(cursor.x * 8) is equal to or higher than the width of the screen
        // make a newline and return

        Console::PrintChar('\n');
        return;
    }
}

// INTERNAL TO DRIVER
// Parses the styling data passed in
const char* ParseStyling(const char* s) {
    s++;
    s++;

    u64 clrvalue = 0; // What color in the pallete to use
    u8 type = 0; // Internal type variable

    u8 bufferindex = 0; // Index in type buffer
    char typebuffer[2]; // Where to apply the color?(foreground or background)

    // Scan for type(fg, bg)
    while (*s != ',' && *s != '\0') {
        typebuffer[bufferindex] = *s;

        // Check if out of bounds
        if (bufferindex >= 2) {
            asm volatile("hlt"); // FIXME: Add proper panic
        }

        bufferindex++;
        s++; // Increase string pointer
    }

    // If a reset tag was spotted, skip everything until the closing tag and reset everything
    if (typebuffer[0] == 'r') {
        // Skip everything until closing tag
        while (*s != ']' && *s != '\0') {
            s++;
        }
        s++;

        // Set the default values
        ConsoleConfiguration.BackgroundColor = {0, 0, 0};
        ConsoleConfiguration.ForegroundColor = {255, 255, 255};

        return s; // Continue with code execution
    }

    // If a clear tag was detected, clear the screen with the passed in color
    // We will set type to 2 for this
    if (typebuffer[0] == 'c') {
        type = 2;
    }

    // Check if valid type, if not set to defualt(foreground)
    if (((typebuffer[0] != 'f' && typebuffer[0] != 'b') && typebuffer[1] != 'g')) {
        type = 0;
        if (typebuffer[0] == 'c') type = 2;
    }
            
    // Check if foreground or background
    if (typebuffer[0] == 'f' && typebuffer[1] == 'g') {
        type = 0;
    } else if (typebuffer[0] == 'b' && typebuffer[1] == 'g') {
        type = 1;
    }

    // Scan for color code
    while (*s != ']' && *s != '\0') {
        if (*s >= '0' && *s <= '9') {
            clrvalue = clrvalue * 10 + (*s - '0');
            }
        if (*s == '\0') break; // Stop if end of string
            s++;
    };

    s++; // Skip the ']' closing tag

    // Now by all specifications ever written by humans, we shall not trust random input
    // So we need to clamp it
    u64 clrvalueclamped = Math::Clamp(0, TerminalPaletteSize, clrvalue);
            
    // Now we can safely set the color acording to the type
    if (type == 0) {
        ConsoleConfiguration.ForegroundColor = TerminalPallete[clrvalueclamped];
    } else if (type == 1) {
        ConsoleConfiguration.BackgroundColor = TerminalPallete[clrvalueclamped];
    } else if (type == 2) {
        // Here we will handle clearing the screen, we just loop thru all the pixels and done!
        u64 fb_width = fb_ptr->width;
        u64 fb_height = fb_ptr->height;
        
        // Draw all the pixels on the screen
        for (u64 hi = 0; hi < fb_height; hi++) {
            for (u64 wi = 0; wi < fb_width; wi++) {
                Graphics::DrawPixel(fb_ptr, {wi, hi}, TerminalPallete[clrvalueclamped]);
            }
        }

        // Set the background color of text cause it may look weird if not set
        ConsoleConfiguration.BackgroundColor = TerminalPallete[clrvalueclamped];
        CursorPosition.x = 0;
        CursorPosition.y = 0;
    }
            
    return s;
}

// Prints a string to the display
// INTERNAL TO DRIVER
// Doesnt lock, that has to be done in other functions
void PrintStringInternal(const char* s) {
    if (!IsConsoleReady) {
        Console::Init(Graphics::GetCurrentFramebuffer());
        IsConsoleReady = ktrue;
    }

    while (*s) {
        if (*s == 'C' && *(s + 1) == '[') {
            s = ParseStyling(s); // Parse the styling
            continue; // Next character
        }

        Console::PrintChar(*s); // Print the character
        s++; // Move the pointer forward
    }
}

// Prints a string to the display
void Console::PrintString(const char* s) {
    ConsoleLock.Lock();

    PrintStringInternal(s);

    ConsoleLock.Unlock();
}

// Outputs a string to the display but with "LOG" before it
void Console::Log(const char* s) {
    ConsoleLock.Lock();

    // FIXME: Add proper printf so this can be easier
    PrintStringInternal("C[fg,6]LOGC[r,] ");
    PrintStringInternal(s);
    Console::PrintChar('\n');

    ConsoleLock.Unlock();
}

// Initilizes the console driver, allows for printing
void Console::Init(GooseOS::Graphics::Framebuffer* fb) {
    if (IsConsoleReady) return;

    Serial::Init();

    if (fb) {
        fb_ptr = fb; // Set the framebuffer pointer
    } else {
        asm volatile("hlt"); // FIXME: Add proper error handling
    }
}

// Sets how scaled to draw the characters when printing
// INTERNAL TO DRIVER
void SetFontScale(u64 scale) {
    ConsoleConfiguration.FontScale = Math::Clamp(MinFontScale, MaxFontScale, scale);
}

// Sets the console state to the passed in one
void Console::SetConsoleState(Console::ConsoleState* state) {
    u64 NewTextScale = state->FontScale; // Store the old one for later
    ConsoleConfiguration = *state; // Set the state

    SetFontScale(NewTextScale); // Clamp the text scale
}