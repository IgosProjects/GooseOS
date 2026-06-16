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
#include <utils.hpp>

// Framebuffer functions namespace
namespace GooseOS::Graphics {
    // This struct defines the framebuffer, it is used by the console driver and other drivers
    // It also holds the bits per pixel and other needed values to draw pixels!
    struct Framebuffer {
        void* addr;
        u64 width;
        u64 height;
        u64 pitch;
        u16 bpp;
        u8 memory_model;
        u8 red_mask_size;
        u8 red_mask_shift;
        u8 green_mask_size;
        u8 green_mask_shift;
        u8 blue_mask_size;
        u8 blue_mask_shift;
    };
    
    // Struct used to store a single RGB color value
    struct RGBColor {
        u8 R;
        u8 G;
        u8 B;
    }; 

    // Returns the current framebuffer to be used by the drawing logic
    struct Framebuffer* GetCurrentFramebuffer();

    // Draws a pixel to the provided framebuffer, at the provided position
    void DrawPixel(struct Framebuffer* fb, GooseOS::Utils::Vector2 Position, const struct Graphics::RGBColor color);
}
