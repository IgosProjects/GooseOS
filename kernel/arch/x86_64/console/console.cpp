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

#include <stdarg.h>
#include <boot/framebuffer.hpp>
#include <console/console.hpp>
#include <core.hpp>
#include <math.hpp>
#include <utils.hpp>
#include <console/serial.hpp>
#include <console/font-8x8.h>
#include <types.hpp>
#include <kconfig.hpp>
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
    #ifdef KRNL_USE_SERIAL
        Serial::PrintChar(c);
    #endif // KRNL_USE_SERIAL
    
    #ifdef KRNL_USE_FB

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
    if (CursorPosition.x * (8 * ConsoleConfiguration.FontScale) >= fb_ptr->width) {
        // If the pixel position(cursor.x * 8) is equal to or higher than the width of the screen
        // make a newline and return

        CursorPosition.x = 0;
        CursorPosition.y++;
        return;
    }

    #endif // KRNL_USE_FB
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
            Core::Panic("Formatting buffer out of bounds!"); // FIXME: Add proper panic
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

// INTERNAL TO DRIVER
void PrintUnsignedInternal(u64 value) {
    char buffer[32]; // Holds an 64 bit value with base 10
    u32 i = 0;

    // Handle 0 because loop wont work otherwise
    if (value == 0) {
        Console::PrintChar('0'); 
        return;
    }

    // Extract digits from right to left
    while (value > 0) {
        u32 remainder = value % 10;
        buffer[i++] = '0' + remainder; // Convert to ASCII
        value /= 10;
    }

    // The digits are backward in the buffer, so print them correctly
    while (i > 0) {
        Console::PrintChar(buffer[--i]);
    }
}

// This functions prints a HEX value to the display
// INTERNAL TO DRIVER
void PrintHexInternal(u64 value) {
    char buffer[16]; // A 64 bit int value is maximum 16 digits in hex!
    u32 i = 0;

    // Same as printing numbers, handle zero
    if (value == 0) {
        Console::PrintChar('0');
        return;
    }

    // Extract hex digits from right to left
    while (value > 0) {
        u32 remainder = value % 16;
        
        if (remainder < 10) {
            buffer[i++] = '0' + remainder; // 0 - 9
        } else {
            buffer[i++] = 'A' + (remainder - 10); // A - F
        }
        
        value /= 16;
    }

    // Print the buffer in reverse order
    while (i > 0) {
        Console::PrintChar(buffer[--i]);
    }
}

// Instantly drops the lock on the console and allows for printing
// Used in kpanic
void Console::EmergencyUnlock() {
    ConsoleLock.Unlock(); // INSTANT UNLOCK!
}

// INTERNAL TO DRIVER
// Prints a string to the display
// Doesnt lock, that has to be done in other functions
// Expects va_args directly!
void Console::PrintStringInternal(const char* s, va_list args) {
    if (!IsConsoleReady) {
        Console::Init(Graphics::GetCurrentFramebuffer());
        IsConsoleReady = ktrue;
    }

    while (*s) {
        if (*s == 'C' && *(s + 1) == '[') {
            #ifdef KRNL_USE_FB
            s = ParseStyling(s); // Parse the styling
            #else
                // This only runs if the kernel doesnt use framebuffer, it skips all styling

                s += 2; // Skip 'C' and '['
                while (*s != ']' && *s != '\0') {
                    s++;
                }
                if (*s == ']') s++; // Skip the closing ']'
            #endif
        }

        // Check for "%" to start printf checking
        if (*s == '%') {
            s++; // Skip the "%"

            if (*s == 's') {
                const char* str = va_arg(args, const char*);

                while (*str) {
                    Console::PrintChar(*str);
                    str++;
                }
            } else if (*s == 'u') {
                PrintUnsignedInternal(va_arg(args, u64));
            } else if (*s == 'x') {
                PrintHexInternal(va_arg(args, u64));
            }

            s++; // Skip the type specifier

            continue; // Next character
        }

        Console::PrintChar(*s); // Print the character
        s++; // Move the pointer forward
    }
}

// Prints a string to the display
void Console::PrintString(const char* s, ...) {
    ConsoleLock.Lock();

    va_list a;
    va_start(a, s);

    PrintStringInternal(s, a);

    va_end(a);

    ConsoleLock.Unlock();
}

// Outputs a string to the display but with "INFO" before it
void Console::INFO(const char* s, ...) {
    ConsoleLock.Lock();

    PrintStringInternal("C[fg,5]INFOC[r,] ", {});

    va_list a;
    va_start(a, s);

    PrintStringInternal(s, a);

    Console::PrintChar('\n');

    va_end(a);

    ConsoleLock.Unlock();
}

// Outputs a string to the display but with "LOG" before it
void Console::Log(const char* s, ...) {
    ConsoleLock.Lock();

    PrintStringInternal("C[fg,6]LOGC[r,] ", {});

    va_list a;
    va_start(a, s);

    PrintStringInternal(s, a);

    Console::PrintChar('\n');

    va_end(a);

    ConsoleLock.Unlock();
}

// Outputs a string to the display but with "OK" before it
void Console::OK(const char* s, ...) {
    ConsoleLock.Lock();

    PrintStringInternal("C[fg,1]OKC[r,] ", {});

    va_list a;
    va_start(a, s);

    PrintStringInternal(s, a);

    Console::PrintChar('\n');

    va_end(a);

    ConsoleLock.Unlock();
}

// Initilizes the console driver, allows for printing
void Console::Init(GooseOS::Graphics::Framebuffer* fb) {
    if (IsConsoleReady) return;

    Serial::Init();

    if (fb) {
        fb_ptr = fb; // Set the framebuffer pointer
    } else {
        // PLEASE NOTE: For the next person who has to fix this, its not my fault
        // Its 100% my fault jk, but uhh yeah this CAN cause forever loops if Limine doesnt give a proper framebuffer
        Core::Panic("Invalid framebuffer pointer passed in!"); // FIXME: Add proper error handling
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