#ifndef RH_CONSOLE_H
#define RH_CONSOLE_H

#include <stdint.h>
#include <stddef.h>
#include <limine.h>

#include "rhMemory.h"

// Don't define SSFN_MAXLINES - use dynamic allocation
#define SSFN_IMPLEMENTATION
#include <ssfn.h>

static ssfn_t ctx;
static ssfn_buf_t buf;
static int textRenderermargin_x = 0;

int textRenderer(struct limine_framebuffer *fb, char *font)
{
    int err;
    
    memset(&ctx, 0, sizeof(ssfn_t));

    err = ssfn_load(&ctx, (ssfn_font_t *)font);
    if (err != SSFN_OK) {
        return err;
    }

    err = ssfn_select(&ctx, SSFN_FAMILY_ANY, NULL, SSFN_STYLE_REGULAR, 16);
    if (err != SSFN_OK) {
        return err;
    }

    buf.ptr = (uint8_t *)fb->address;
    buf. w = (int)fb->width;
    buf.h = (int)fb->height;
    buf.p = (uint16_t)fb->pitch;
    buf.x = 0;
    buf.y = 0;
    buf.fg = 0xFFFFFFFF;
    buf.bg = 0x00000000;

    return SSFN_OK;
}

void textRenderer_setPos(int x, int y)
{
    textRenderermargin_x = x;
    buf.x = x;
    buf.y = y;
}

void textRenderer_setColor(uint32_t bg, uint32_t fg)
{
    buf.bg = bg;
    buf.fg = fg;
}

void textRenderer_write(const char* text)
{
    int ret;
    while(*text)
    {
        if(*text == '\n')
        {
            buf.x = textRenderermargin_x;
            buf.y += ctx.line ?  ctx.line : ctx.size;
            text++;
            continue;
        }
        
        ret = ssfn_render(&ctx, &buf, text);
        if(ret > 0)
        {
            text += ret;
        }
        else
        {
            text++;
            break;
        }
    }
}

void textRendererend()
{
    ssfn_free(&ctx);
}

#endif