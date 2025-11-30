#ifndef RH_KEYBOARD_H
#define RH_KEYBOARD_H

#include <stdint.h>
#include <stdbool.h>

static inline uint8_t inb(uint16_t port)
{
    uint8_t ret;
    asm volatile ( "inb %1, %0" : "=a"(ret) : "Nd"(port) );
    return ret;
}

// PS/2 ports
#define PS2_DATA 0x60
#define PS2_STATUS 0x64

static char scan_code_map[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',   // 0x00 - 0x0E
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',     // 0x0F - 0x1C
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',             // 0x1D - 0x29
    0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,               // 0x2A - 0x36
    '*', 0, ' ', 0                                                              // 0x37 - 0x39
};

bool keyboard_has_data()
{
    // Read Status Register: bit 0 is 1 if there is data available
    return inb(PS2_STATUS) & 1;
}

char keyboard_read()
{
    if(keyboard_has_data())
    {
        uint8_t scancode = inb(PS2_DATA);

        if(scancode & 0x80)
        {
            // Key release event, ignore
            return 0;
        }

        if(scancode < 128)
        {
            return scan_code_map[scancode];
        }
    }

    return 0;
}

#endif