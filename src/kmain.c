#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limine.h>

#include "rhMemory.h"
#include "rhText.h"
#include "rhKeyboard.h"
#include "rhDraw.h"
#include "rhRand.h"

void heap_init(void);

void reboot()
{
    uint8_t good = 0x02;
    while (good & 0x02)
        good = inb(0x64);
    outb(0x64, 0xFE);
    hcf();
}

char* itoa(int value, char* result, int base) {
	// check that the base if valid
	if (base < 2 || base > 36) { *result = '\0'; return result; }

	char* ptr = result, *ptr1 = result, tmp_char;
	int tmp_value;

	do {
		tmp_value = value;
		value /= base;
		*ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz" [35 + (tmp_value - value * base)];
	} while ( value );

	// Apply negative sign
	if (tmp_value < 0) *ptr++ = '-';
	*ptr-- = '\0';
	while(ptr1 < ptr) {
		tmp_char = *ptr;
		*ptr--= *ptr1;
		*ptr1++ = tmp_char;
	}
	return result;
}
	

// Fonts
extern char _binary_src_fonts_vgafont_sfn_start;
extern char _binary_src_fonts_vera_sfn_start;
extern char _binary_src_fonts_freesans_sfn_start;
extern char _binary_src_fonts_comic_sfn_start;

extern char _binary_src_sprites_player_tga_start[];
extern char _binary_src_sprites_tree_tga_start[];

// Console
char *buffer;
int console_x = 0; // Current cursor position (Column Index)
int console_y = 0; // Current cursor position (Row Index)

int default_x = 50; // Screen Pixel X where console starts
int default_y = 100; // Screen Pixel Y where console starts

int width;
int lines;

int score = 0;

void print(const char *str);
void scroll();
void render();

int player_lane = 0; // -1 - left, 0 - center, 1 - right
bool alive = true;

struct Tree
{
	int x;
	int y;
};

void render_player(struct limine_framebuffer *framebuffer);
void render_trees(struct limine_framebuffer *framebuffer, struct Tree *trees);

// Set up "new tree" at random lane
void new_tree(struct Tree *tree, struct limine_framebuffer *framebuffer)
{
	tree->y = (framebuffer->height - 100) + (rand() % 200);

	// Random integer X between -1 and 1
	int lane = (rand() % 3) - 1;

	tree->x = (framebuffer->width / 2) + lane * 200;
}

struct Tree trees[2];

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

	/* Backbuffer (double buffering) */
	size_t backbuffer_size = framebuffer->pitch * framebuffer->height;
	void *backbuffer = malloc(backbuffer_size);
	if(!backbuffer) hcf();

	struct limine_framebuffer backbuffer_info = *framebuffer;
	backbuffer_info.address = backbuffer;
	
	/*for(size_t i = 0; i < 100; i++)
	{
		fb_ptr[i * (framebuffer->pitch / 4) + i] = 0xFFFF0000;  // Red diagonal
	}*/

	int err = textRenderer(framebuffer, (char *)&_binary_src_fonts_vgafont_sfn_start);

	if(err != SSFN_OK)
	{
		hcf();
	}


	render_player(&backbuffer_info);

	// Set up trees
	for(int i = 0; i < 2; i++)
	{
		new_tree(&trees[i], framebuffer);
	}


	// Init console
	/*width = framebuffer->width / 8;
	lines = (framebuffer->height - default_y) / 16;

	buffer = (char *)malloc(width * lines);
	memset(buffer, ' ', width * lines);

	textRenderer_setPos(50, 50);
	textRenderer_setColor(0, 0xFF000000); // Black on white*/

	textRenderer_setPos(50, 50);
	textRenderer_setColor(0, 0xFF000000);

	textRenderer_write("Rabenhaus v0.0.1\n");
	textRenderer_write("By Laky2k8, 2025\n");
	textRenderer_write("----------------\n\n");
	textRenderer_write("Árvíztűrő tükörfúrógép");

	/*print("Rabenhaus v0.0.1\n");
	print("By Laky2k8, 2025\n");
	print("----------------\n\n");
	print("Árvíztűrő tükörfúrógép\n");

	render();*/

	

	while(alive)
	{
        char c = keyboard_read();
        
        /*if (c != 0) 
		{
            char temp[2] = { c, '\0' };
            print(temp);

        }*/

		if(c == 'a')
		{
			if(player_lane > -2)
			{
				player_lane -= 1;

			}
		}

		if(c == 'd')
		{
			if(player_lane < 2)
			{
				player_lane += 1;

			}
		}

		for(int i = 0; i < 2; i++)
		{
			trees[i].y -= (5 + score / 4); // Increase speed with score


			if(trees[i].y <= 100 && trees[i].x == ((framebuffer->width / 2) + player_lane * 200))
			{
				alive = false;
			}

			if(trees[i].y < -100)
			{
				new_tree(&trees[i], framebuffer);

				score += 1;
			}
		}


		render_player(&backbuffer_info);

		render_trees(&backbuffer_info, trees);

		memcpy(framebuffer->address, backbuffer, backbuffer_size);

		// Print score
		textRenderer_setPos(50, 50);
		textRenderer_setColor(0, 0xFF000000);
		char score_str[20];
		itoa(score, score_str, 10);
		textRenderer_write("Score: ");
		textRenderer_write(score_str);
        
		asm volatile("pause");

	}

	for(size_t y = 0; y < framebuffer->height; y++)
	{

		for(size_t x = 0; x < framebuffer->width; x++)
		{
			fb_ptr[y * (framebuffer->pitch / 4) + x] = 0x0000FF;
		}
	}

	textRenderer_setColor(0, 0xFFFFFFFF);

	textRenderer_write("A problem (You hitting a tree) has been detected and Rabenhaus has been shut down because yeeeowch that must've hurt. To continue:\n");
	textRenderer_write("Press any key and the computer will reboot so you can try again.\n\n");

	char score_str[20];
	itoa(score, score_str, 10);
	textRenderer_write("Your score was: ");
	textRenderer_write(score_str);
	textRenderer_write("\n\n");

	textRenderer_write("Thanks for playing Rabenhaus! - Laky2k8");

	bool anyKeyPressed = false;

	while(!anyKeyPressed)
	{
		char c = keyboard_read();

		if (c != 0) 
		{
			anyKeyPressed = true;
			break;
        }
	}

	reboot();
	

	//hcf();
}


void render_trees(struct limine_framebuffer *framebuffer, struct Tree *trees)
{
	for(int i = 0; i < 2; i++)
	{
        // FIXED: accessing struct members via . instead of ->
		draw_sprite(framebuffer, trees[i].x, trees[i].y, (void*)_binary_src_sprites_tree_tga_start, 4);
	}
}

void render_player(struct limine_framebuffer *framebuffer)
{

	volatile uint32_t *fb_ptr = framebuffer->address;

	// Background
	uint32_t start_color = 0xFFFFFF; // White
	uint32_t end_color = 0x7FB2FF;   // Blue

	// Extract channels
	uint32_t r1 = (start_color >> 16) & 0xFF;
	uint32_t g1 = (start_color >> 8) & 0xFF;
	uint32_t b1 = start_color & 0xFF;

	uint32_t r2 = (end_color >> 16) & 0xFF;
	uint32_t g2 = (end_color >> 8) & 0xFF;
	uint32_t b2 = end_color & 0xFF;

	// Loop for whole screen
	for(size_t y = 0; y < framebuffer->height; y++)
	{
        uint32_t r = (r1 * (framebuffer->height - y) + r2 * y) / framebuffer->height;
        uint32_t g = (g1 * (framebuffer->height - y) + g2 * y) / framebuffer->height;
        uint32_t b = (b1 * (framebuffer->height - y) + b2 * y) / framebuffer->height;

        // Recombine
        uint32_t color = (r << 16) | (g << 8) | b;

		for(size_t x = 0; x < framebuffer->width; x++)
		{
			fb_ptr[y * (framebuffer->pitch / 4) + x] = color;
		}
	}

	draw_sprite(framebuffer, (framebuffer->width / 2) + player_lane * 200, 100, (void*)_binary_src_sprites_player_tga_start, 3);
}

// CONSOLE

void draw_char_at(const char *c, int x, int y)
{
    int pixel_x = default_x + (x * 8);
    int pixel_y = default_y + (y * 16);

    textRenderer_setPos(pixel_x, pixel_y);
    textRenderer_write(c);
}


void print(const char *str)
{
    char *ptr = (char *)str;

    while (*ptr)
    {
        if (*ptr == '\n')
        {
            console_x = 0;
            console_y++;

            // Scroll check logic
            if (console_y >= lines) { scroll(); console_y = lines - 1; render(); }
            
            ptr++; // Move past \n
        }
        else
        {
            char *start = ptr;
            ssfn_utf8(&ptr);
            int len = ptr - start;

            // create a temporary string for just this one character
            char temp[5] = {0}; // Max UTF-8 length is 4 + null terminator
            for(int i = 0; i < len; i++) {
                temp[i] = start[i];
            }

            buffer[console_y * width + console_x] = start[0];

            draw_char_at(temp, console_x, console_y);
            
            console_x++;
            if (console_x >= width)
            {
                console_x = 0;
                console_y++;
                if (console_y >= lines) { scroll(); console_y = lines - 1; render(); }
            }
            
        }
    }
}

void scroll()
{
    memmove(buffer, buffer + width, (lines - 1) * width);
    memset(buffer + (lines - 1) * width, ' ', width);
}

void render()
{
    char line_buf[width + 1];
    line_buf[width] = '\0'; // null termination

    for (int i = 0; i < lines; i++)
    {
        textRenderer_setPos(default_x, default_y + i * 16);

        //safe copy of the line from the big buffer to the temp string
        memcpy(line_buf, buffer + i * width, width);

        textRenderer_write(line_buf);
    }
}