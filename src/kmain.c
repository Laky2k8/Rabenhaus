#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limine.h>

#include "rhMemory.h"
#include "rhConsole.h"

void heap_init(void);

// Fonts
extern char _binary_src_fonts_vgafont_sfn_start;
extern char _binary_src_fonts_vera_sfn_start;
extern char _binary_src_fonts_freesans_sfn_start;


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

    // Draw diagonal line first to verify framebuffer works
    for(size_t i = 0; i < 100; i++)
    {
        fb_ptr[i * (framebuffer->pitch / 4) + i] = 0xFFFF0000;  // Red diagonal
    }
    
    // Initialize console and check for errors
    int err = console_init(framebuffer, (char *)&_binary_src_fonts_vera_sfn_start);
    
    // Draw error code as colored squares (each square = one error code)
    // Square at x=0: Green if OK, Red if error
    uint32_t color = (err == SSFN_OK) ?  0xFF00FF00 : 0xFFFF0000;
    for(int y = 0; y < 50; y++)
        for(int x = 0; x < 50; x++)
            fb_ptr[y * (framebuffer->pitch / 4) + x] = color;

    // Draw specific error indicator squares at x=60
    // SSFN_ERR_ALLOC = -1 (Cyan)
    // SSFN_ERR_BADFILE = -2 (Magenta) 
    // SSFN_ERR_NOFACE = -3 (Yellow)
    // SSFN_ERR_INVINP = -4 (White)
    // SSFN_ERR_BADSTYLE = -5 (Orange)
    // SSFN_ERR_BADSIZE = -6 (Purple)
    // SSFN_ERR_NOGLYPH = -7 (Gray)
    
    uint32_t err_colors[] = {
        0xFF00FF00,  // 0 = OK (Green)
        0xFF00FFFF,  // -1 = ALLOC (Cyan)
        0xFFFF00FF,  // -2 = BADFILE (Magenta)
        0xFFFFFF00,  // -3 = NOFACE (Yellow)
        0xFFFFFFFF,  // -4 = INVINP (White)
        0xFFFF8000,  // -5 = BADSTYLE (Orange)
        0xFF8000FF,  // -6 = BADSIZE (Purple)
        0xFF808080,  // -7 = NOGLYPH (Gray)
    };
    
    int err_idx = (err <= 0 && err >= -7) ? -err : 0;
    uint32_t err_color = err_colors[err_idx];
    
    for(int y = 0; y < 50; y++)
        for(int x = 60; x < 110; x++)
            fb_ptr[y * (framebuffer->pitch / 4) + x] = err_color;

    // Only try to write if init succeeded
    if (err == SSFN_OK) {
        console_setPos(50, 150);
        console_write("Rabenhaus v0.0.1\n");
    }

    console_setPos(50, 100);

    console_write("Rabenhaus v0.0.1\n");
    console_write("By Laky2k8, 2025\n");
    console_write("----------------\n\n");
    console_write("Árvíztűrő tükörfúrógép");




    hcf();
}