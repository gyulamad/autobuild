#pragma once

// EGA color palette
static const unsigned int EGA_COLORS[16] = {
    0x00000000, // Black
    0x0000AA00, // Blue
    0x00AA0000, // Green
    0x00AAAA00, // Cyan
    0xAA000000, // Red
    0xAA00AA00, // Magenta
    0xAA550000, // Brown
    0xAAAAAA00, // Light Gray
    0x55555500, // Dark Gray
    0x5555FF00, // Light Blue
    0x55FF5500, // Light Green
    0x55FFFF00, // Light Cyan
    0xFF555500, // Light Red
    0xFF55FF00, // Light Magenta
    0xFFFF5500, // Yellow
    0xFFFFFF00,  // White
};

#define EGA_BLACK           EGA_COLORS[0]
#define EGA_BLUE            EGA_COLORS[1]
#define EGA_GREEN           EGA_COLORS[2]
#define EGA_CYAN            EGA_COLORS[3]
#define EGA_RED             EGA_COLORS[4]
#define EGA_MAGENTA         EGA_COLORS[5]
#define EGA_BROWN           EGA_COLORS[6]
#define EGA_LIGHT_GRAY      EGA_COLORS[7]
#define EGA_DARK_GRAY       EGA_COLORS[8]
#define EGA_LIGHT_BLUE      EGA_COLORS[9]
#define EGA_LIGHT_GREEN     EGA_COLORS[10]
#define EGA_LIGHT_CYAN      EGA_COLORS[11]
#define EGA_LIGHT_RED       EGA_COLORS[12]
#define EGA_LIGHT_MAGENTA   EGA_COLORS[13]
#define EGA_YELLOW          EGA_COLORS[14]
#define EGA_WHITE           EGA_COLORS[15]
