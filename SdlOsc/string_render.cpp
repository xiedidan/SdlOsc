#include "stdafx.h"
#include <iostream>

#pragma comment(lib, "freetype264")

#include "sdl\include\SDL.h"
#include "sdl\include\SDL_opengl.h"
#include <GL/GL.h>
#include <GL/glu.h>

#include "common.h"
#include "sdl_service.h"

#include <ft2build.h>
#include FT_FREETYPE_H
#include "string_render.h"

#define THEME_ENERY
#include "theme.h"

using namespace std;

// freetype 2
FT_Library ftLib;
FT_Face ftFace;
FT_Error ftErr;

// font color
GLubyte fontColor[4] = TITLE_CHAR_COLOR;
GLfloat levelStep = 1 / (GLfloat)FONT_AA_LEVEL;

// charmap
FT_Glyph_Metrics charMetrics[ASCII_CHAR_COUNT];
GLubyte* charmap[ASCII_CHAR_COUNT][FONT_AA_LEVEL];

FT_Error initFreeType() {
	// init FreeType2
	ftErr = FT_Init_FreeType(&ftLib);
	if (ftErr != FT_Err_Ok) {
		// TODO : ft init error - ignored for now
		cout << "FT_Init_FreeType() failed." << endl;
		return ftErr;
	}
	
	ftErr = FT_New_Face(ftLib, FONT_NAME, 0, &ftFace);
	if (ftErr == FT_Err_Unknown_File_Format) {
		// TODO : file could be read, but its format is unknown
		cout << "FT_New_Face() found unknown format." << endl;
		return ftErr;
	}
	else if (ftErr != FT_Err_Ok) {
		// TODO : couldn't read font file
		cout << "FT_New_Face() failed. " << ftErr << endl;
		return ftErr;
	}

	ftErr = FT_Set_Char_Size(ftFace, 0, FONT_SIZE * 64, SCREEN_RESOLUTION, SCREEN_RESOLUTION);
	if (ftErr != FT_Err_Ok) {
		// TODO : error handler - ignored for now
		cout << "FT_Set_Char_Size() failed." << endl;
		return ftErr;
	}

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // bitmap row aligns to 1 byte
	glPixelStorei(GL_UNPACK_LSB_FIRST, 1);

	return ftErr;
}

void initCharmap() {
	for (int i = 0; i < ASCII_CHAR_COUNT; i++) {
		// 1. render to face
		FT_UInt  glyph_index;
		glyph_index = FT_Get_Char_Index(ftFace, i);
		ftErr = FT_Load_Glyph(ftFace, glyph_index, FT_LOAD_DEFAULT);
		if (ftErr)
			continue;
		ftErr = FT_Render_Glyph(ftFace->glyph, FT_RENDER_MODE_NORMAL);
		if (ftErr)
			continue;

		// 2. calc charmap geometry
		int moreRowBits = ftFace->glyph->bitmap.width % FONT_AA_LEVEL;
		int rowBytes = ftFace->glyph->bitmap.width / FONT_AA_LEVEL;
		if (moreRowBits != 0)
			rowBytes++;
		
		// 3. fill charmap
		for (int j = 0; j < FONT_AA_LEVEL; j++) {
			charmap[i][j] = (GLubyte*)malloc(rowBytes * ftFace->glyph->bitmap.rows);
			memset(charmap[i][j], 0, rowBytes * ftFace->glyph->bitmap.rows);
		}

		for (int j = 0; j < ftFace->glyph->bitmap.rows; j++) {
			for (int k = 0; k < ftFace->glyph->bitmap.width; k++) {
				int revI = ftFace->glyph->bitmap.rows - j - 1;
				int index = revI * ftFace->glyph->bitmap.width + k;
				GLubyte level = ftFace->glyph->bitmap.buffer[index] >> 5;
				if (ftFace->glyph->bitmap.buffer[index] != 0) {
					int byteIndex = j * rowBytes + k / BITS_PER_BYTE;
					int bitOffset = k % BITS_PER_BYTE;
					(charmap[i][level])[byteIndex] |= 1 << bitOffset;
				}
			} // k loop
		} // j loop

		// save char geometry
		memcpy((FT_Glyph_Metrics*)&(charMetrics[i]), (FT_Glyph_Metrics*)&(ftFace->glyph->metrics), sizeof(FT_Glyph_Metrics));
	} // i loop
}

void renderString(int x, int y, char* string) {
	char* currChar = string;
	int currX = x;
	int currY = y;
	
	glPushMatrix();
	glLoadIdentity();

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ZERO);

	while (*currChar != '\0') {
		renderChar(currX, currY, *currChar);
		currX += charMetrics[*currChar].horiAdvance >> 6;
		currChar++;
	}

	glDisable(GL_BLEND);

	glPopMatrix();
}

// a printf() style wrapper
void printGL(int x, int y, char* fmt, ...) {
	va_list args;

	char strBuf[STRING_BUFFER_SIZE];
	memset(strBuf, 0, STRING_BUFFER_SIZE);

	va_start(args, fmt);
	vsprintf_s(strBuf, fmt, args);
	va_end(args);

	renderString(x, y, strBuf);
}

// helper
void renderChar(int x, int y, char ascii) {
	GLubyte currColor[4] = TITLE_CHAR_COLOR;
	currColor[3] = fontColor[3] * levelStep;
	FT_Glyph_Metrics metrics = charMetrics[ascii];

	for (int i = 0; i < FONT_AA_LEVEL; i++) {
		glColor4ubv(currColor);
		glRasterPos2i(x, y);
		glBitmap(metrics.width >> 6,
			metrics.height >> 6,
			-1 * metrics.horiBearingX >> 6, 
			(metrics.height - metrics.horiBearingY) >> 6, 
			0, 0, 
			charmap[ascii][i]);
		currColor[3] += fontColor[3] * levelStep;
	}
}

