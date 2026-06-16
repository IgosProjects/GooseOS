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
#include <types.hpp>
#include <limine/limine.h>

// Get the request from the header
extern struct limine_framebuffer_request framebuffer_request;
using namespace GooseOS; // Use the GooseOS namespace

bl fb_ready = kfalse; // Is the framebuffer safe to use?
static struct GooseOS::Graphics::Framebuffer fb;
static struct GooseOS::Graphics::Framebuffer fb_ptr;

// Generates the color to be used on the specified framebuffer, using the passed in RGB values
// INTERNAL TO THE DRIVER!
uint32_t MakeColor(const Graphics::Framebuffer* fb, u8 r, u8 g, u8 b) {
    auto scale = [](u8 value, u8 bits) -> uint32_t
    {
        if (bits == 0)
            return 0;

        uint32_t max = (1u << bits) - 1;
        return (static_cast<uint32_t>(value) * max) / 255;
    };

    uint32_t red =
        scale(r, fb->red_mask_size)
        << fb->red_mask_shift;

    uint32_t green =
        scale(g, fb->green_mask_size)
        << fb->green_mask_shift;

    uint32_t blue =
        scale(b, fb->blue_mask_size)
        << fb->blue_mask_shift;

    return red | green | blue;
}

// Draws a pixel to the provided framebuffer, at the provided position
void Graphics::DrawPixel(Graphics::Framebuffer* fb, const struct Utils::Vector2 position, const struct Graphics::RGBColor color) {
    uint8_t* base = (u8*)(uintptr_t)fb->addr; // Get address pointer

    // Calculate where the pixel is
    u32 bytes_per_pixel = fb->bpp / 8;
    u32 offset = position.y * fb->pitch + position.x * bytes_per_pixel;

    // Finnaly draw the pixel
    *(u32*)(base + offset) = MakeColor(fb, color.R, color.G, color.B);
}

// INTERNAL
void InitFB() {
    if (framebuffer_request.response) {
        // If Limine responded to us and gave us a framebuffer. We should also check for a valid framebuffer
        // But first, lets store it in our variable
        limine_framebuffer_response* fb_response = framebuffer_request.response;
        
        if (fb_response->framebuffer_count < 1) return;

        // Now lets get the actual framebuffer, and return it
        struct limine_framebuffer* lfb_ptr = fb_response->framebuffers[0]; // Get the first framebuffer
        
        // Check if it exists
        if (!lfb_ptr) {
            return;
        }

        // We need to convert from Limine to our custom framebuffer struct first!
        fb.addr = lfb_ptr->address;
        fb.bpp = lfb_ptr->bpp;
        fb.pitch = lfb_ptr->pitch;
        
        fb.height = lfb_ptr->height;
        fb.width = lfb_ptr->width;

        fb.memory_model = lfb_ptr->memory_model;

        fb.red_mask_shift = lfb_ptr->red_mask_shift;
        fb.red_mask_size = lfb_ptr->red_mask_size;

        fb.blue_mask_shift = lfb_ptr->blue_mask_shift;
        fb.blue_mask_size = lfb_ptr->blue_mask_size;

        fb.green_mask_shift = lfb_ptr->green_mask_shift;
        fb.green_mask_size = lfb_ptr->green_mask_size;

        // Let the rest of the code know its safe
        fb_ready = ktrue;
    }
}

// Returns the current framebuffer to be used by the drawing logic
struct Graphics::Framebuffer* Graphics::GetCurrentFramebuffer() {
    if (!fb_ready) {
        InitFB(); // If not ready, initilize it
    }

    return &fb;
}
