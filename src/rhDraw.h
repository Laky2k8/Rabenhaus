#ifndef RH_DRAW_H
#define RH_DRAW_H

#include <stdint.h>
#include <stddef.h>
#include <limine.h>

// TGA Header (stole it from the interwebz)
typedef struct {
	uint8_t magic1; // 0
	uint8_t colormap; // 1
	uint8_t encoding; // 2
	uint16_t cmaporig, cmaplen;
	uint8_t cmapent;
	uint16_t x; // 12
	uint16_t y; // 14
	uint16_t w; // 12
	uint16_t h; // 14
	uint8_t bpp; // 16
	uint8_t pixeltype; // 17
} __attribute__((packed)) tga_header_t;

// Helper to pack color
static inline uint32_t make_color(uint8_t r, uint8_t g, uint8_t b)
{
	return (r << 16) | (g << 8) | b;
}

// Draw a pixel (with alpha!)
void put_pixel(struct limine_framebuffer *fb, int x, int y, uint32_t color)
{

	volatile uint32_t *fb_ptr = fb->address;

	// Bounds check
	if((x < 0 || x >= fb->width) || (y < 0 || y >= fb->height))
	{
		return;
	}

	// Calculate index
	size_t index = y * (fb->pitch / 4) + x;

	// Source (sprite) RGB values
	uint8_t sourceA = (color >> 24) & 0xFF;
	uint8_t sourceR = (color >> 16) & 0xFF;
	uint8_t sourceG = (color >> 8) & 0xFF;
	uint8_t sourceB = (color) & 0xFF;

	// Skip if transparent
	if(sourceA == 0) return;

	// Overwrite if fully opaque
	if(sourceA == 255)
	{
		fb_ptr[index] = color;
		return;
	}

	// Destination (background or whatever is already there) values
	uint32_t bgColor = fb_ptr[index];
	uint8_t destR = (bgColor >> 16) & 0xFF;
	uint8_t destG = (bgColor >> 8) & 0xFF;
	uint8_t destB = (bgColor) & 0xFF;

	// Alpha blending
	uint8_t finalR = ((sourceR * sourceA) + (destR * (255 - sourceA))) >> 8;
	uint8_t finalG = ((sourceG * sourceA) + (destG * (255 - sourceA))) >> 8;
	uint8_t finalB = ((sourceB * sourceA) + (destB * (255 - sourceA))) >> 8;

	fb_ptr[index] = make_color(finalR, finalG, finalB);
}


// TGA Drawing
void draw_sprite(struct limine_framebuffer *fb, int x, int y, void *tga_buffer, int scale)
{
	tga_header_t *header = (tga_header_t *)tga_buffer;

	// Test if correct encoding (RGB uncompressed)
	if(header->encoding != 2) return;

	uint8_t *image_data = (uint8_t*)tga_buffer + 18; // Data starts after header

	int width = header->w;
	int height = header->h;
	int bpp = header->bpp / 8;

	for(int iy = 0; iy < height; iy++)
	{
		for(int ix = 0; ix < width; ix++)
		{
			int offset = (iy * width + ix) * bpp;

			uint8_t b = image_data[offset + 0];
			uint8_t g = image_data[offset + 1];
			uint8_t r = image_data[offset + 2];
			uint8_t a = (bpp == 4) ? image_data[offset + 3] : 255;

			uint32_t color = (a << 24) | (r << 16) | (g << 8) | b;

			for(int sy = 0; sy < scale; sy++)
			{
				for(int sx = 0; sx < scale; sx++)
				{
					put_pixel(fb, x + (ix * scale) + sx, y + (height - 1 - iy * scale) + sy, color);
				}
			}
		}
	}
}

#endif