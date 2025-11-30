#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limine.h>

#include "rhMemory.h"
#include "rhText.h"

void heap_init(void);

// Fonts
extern char _binary_src_fonts_vgafont_sfn_start;
extern char _binary_src_fonts_vera_sfn_start;
extern char _binary_src_fonts_freesans_sfn_start;
extern char _binary_src_fonts_comic_sfn_start;

// Console
char *buffer;
int console_x = 50;
int console_y = 100;

int default_x = 50;
int default_y = 100;

int width;
int lines;

void print(const char *str);
void scroll();
void render();

// LIMINE

// Base revision 4: This is the latest base revision described by the Limine boot protocol specification
__attribute__((used, section(".limine_requests")))
static volatile uint64_t limine_base_revision[] = LIMINE_BASE_REVISION(4);

// Framebuffer request
__attribute__((used, section(".limine_requests")))
static volatile struct limine_framebuffer_request framebuffer_request = 
{
	.id = LIMINE_FRAMEBUFFER_REQUEST_ID,
	.revision = 0
};

// Bigger stack size request
__attribute__((used, section(".limine_requests")))
static volatile struct limine_stack_size_request stack_size_request = {
	.id = LIMINE_STACK_SIZE_REQUEST_ID,
	.revision = 0,
	.stack_size = 0x100000 // 1 MB stack
};

// Define the start and end markers for the Limine requests
__attribute__ ((used, section(".limine_requests_start")))
static volatile uint64_t limine_requests_start_marker[] = LIMINE_REQUESTS_START_MARKER;

__attribute__ ((used, section(".limine_requests_end")))
static volatile uint64_t limine_requests_end_marker[] = LIMINE_REQUESTS_END_MARKER;



/* Rabenhaus Kernel */

void kmain(void)
{
	// Initialize heap
	heap_init();

	// Ensure the bootloader understands our base revision
	if(LIMINE_BASE_REVISION_SUPPORTED(limine_base_revision) == false)
	{
		hcf();
	}

	// Ensure we got a framebuffer
	if(framebuffer_request.response == NULL || framebuffer_request.response->framebuffer_count < 1)
	{
		hcf();
	}

	// Fetch the first framebuffer (we assume the framebuffer is RGB with 32-bit pixels)
	struct limine_framebuffer *framebuffer = framebuffer_request.response->framebuffers[0];


	volatile uint32_t *fb_ptr = framebuffer->address;
	
	/*for(size_t i = 0; i < 100; i++)
	{
		fb_ptr[i * (framebuffer->pitch / 4) + i] = 0xFFFF0000;  // Red diagonal
	}*/

	// Loop for whole screen
	for(size_t y = 0; y < framebuffer->height; y++)
	{
		for(size_t x = 0; x < framebuffer->width; x++)
		{
			fb_ptr[y * (framebuffer->pitch / 4) + x] = (1.0 - y) * 0xFFFFFF + y * 0x7FB2FF;  // Vertical gradient from white to blue
		}
	}
	
	int err = textRenderer(framebuffer, (char *)&_binary_src_fonts_vgafont_sfn_start);

	if(err != SSFN_OK)
	{
		hcf();
	}

	// Init console
	width = framebuffer->width;
	lines = framebuffer->height / 16;

	buffer = (char *)malloc(width * lines);
	memset(buffer, ' ', width * lines);

	textRenderer_setPos(50, 100);
	textRenderer_setColor(0x000000, 0xFFFFFF); // Black on white

	textRenderer_write("Rabenhaus v0.0.1\n");
	textRenderer_write("By Laky2k8, 2025\n");
	textRenderer_write("----------------\n\n");
	textRenderer_write("Árvíztűrő tükörfúrógép");

	print("Rabenhaus v0.0.1\n");
	print("By Laky2k8, 2025\n");
	print("----------------\n\n");
	print("Árvíztűrő tükörfúrógép\n");

	render();

	while(true)
	{
		// Just loop infinitely
		continue;
	}

	//hcf();
}

// CONSOLE
void print(const char *str)
{
	while (*str)
	{
		if (*str == '\n')
		{
			console_x = default_x;
			console_y++;
			if (console_y >= lines)
			{
				scroll();
				console_y = lines - 1;
			}
		}
		else
		{
			buffer[console_y * width + console_x] = *str;
			console_x++;
			if (console_x >= width)
			{
				console_x = 0;
				console_y++;
				if (console_y >= lines)
				{
					scroll();
					console_y = lines - 1;
				}
			}
		}
		str++;
	}
	render();
}

void scroll()
{
	// Move all lines up by one
	memmove(buffer, buffer + width, (lines - 1) * width);
	// Clear the last line
	memset(buffer + (lines - 1) * width, ' ', width);
}

void render()
{
	textRenderer_setPos(console_x, console_y);
	for (int i = 0; i < lines; i++)
	{
		textRenderer_write(buffer + i * width);
		textRenderer_setPos(console_x, console_y + (i + 1) * 16);
	}
}