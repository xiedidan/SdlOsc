#ifndef STRING_RENDER_H
#define STRING_RENDER_H

#define BITS_PER_BYTE 8
#define ASCII_CHAR_COUNT 128
#define STRING_BUFFER_SIZE 4096

#define FONT_NAME "ARIALUNI.TTF"
#define FONT_SIZE 12
#define SCREEN_RESOLUTION 96

#define FONT_AA_LEVEL 8 // an optimized antialiased char rendering method using glBitmap() - glprogramming.com/red/chapter14.html#name9

FT_Error initFreeType();
void initCharmap();
void renderString(int x, int y, char* string);
void printGL(int x, int y, char* fmt, ...);

// helper
void renderChar(int x, int y, char ascii);

#endif