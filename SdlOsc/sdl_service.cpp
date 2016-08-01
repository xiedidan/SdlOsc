#include "stdafx.h"
#include <iostream>

#pragma comment(lib, "opengl32")
#pragma comment(lib, "glu32")

#pragma comment(lib, "freetype264")

#include "sdl\include\SDL.h"
#include "sdl\include\SDL_thread.h"
#include "sdl\include\SDL_opengl.h"
#include <GL/GL.h>
#include <GL/glu.h>
#include <ft2build.h>
#include FT_FREETYPE_H

#include "common.h"
#include "sdl_service.h"

#define THEME_ENERY
#include "theme.h"

using namespace std;

// fps control
const uint32_t fpsDelay = 1000 / FPS_TARGET;
uint32_t fpsTicks = 0;

// render thread variables
bool renderFlag = false;
SDL_Thread* renderThread = NULL;

SDL_Window* sdlWindow;
// SDL_GLContext glContext; // moved to render thread

void sdlExit(int exitCode, char* msg) {
	cout << msg << ": " << SDL_GetError() << endl;
	SDL_Quit();
	exit(exitCode);
}

void initGL(int width, int height) {
	SDL_GL_SetSwapInterval(1);

	GLbyte clearColor[4] = BACKGROUND_COLOR;
	glShadeModel(GL_SMOOTH);
	glClearColor((GLfloat)(clearColor[0] / 255.0f), 
		(GLfloat)(clearColor[1] / 255.0f),
		(GLfloat)(clearColor[2] / 255.0f),
		(GLfloat)(clearColor[3] / 255.0f));

	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, width, 0, height, -1.0, 1.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void resizeGL(int width, int height) {
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, width, 0, height, -1.0, 1.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void initSDL(int width, int height) {
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		sdlExit(1, "SDL init failed.");
	}

	// require OpenGL 3.2
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 32);

	sdlWindow = SDL_CreateWindow(PROGRAM_NAME,
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		width,
		height,
		SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
	if (!sdlWindow) {
		sdlExit(2, "SDL window creation failed.");
	}

	// create GL context per render thread - all use the same sdlWindow
	// glContext = SDL_GL_CreateContext(sdlWindow);
}

void renderBuffer() {
	SDL_GL_SwapWindow(sdlWindow);
}

void eventHandler() {

}

void keyEventHandler(SDL_Keysym* keysym) {

}

void drawRuler() {
	GLubyte rulerColor[4] = RULER_COLOR;

	glPushMatrix();

	glLoadIdentity();
	glColor4ubv(rulerColor);
	glLineWidth(2.0f);
	glLineStipple(2, 0xaaaa);
	
	glEnable(GL_LINE_STIPPLE);
	glBegin(GL_LINES);
	for (int i = 1; i < RULER_Y_COUNT + 1; i++) {
		if (i != (RULER_Y_COUNT + 1) / 2) {
			glVertex3d(0, WINDOW_HEIGHT * i / (RULER_Y_COUNT + 1), -1.0f);
			glVertex3d(WINDOW_WIDTH, WINDOW_HEIGHT * i / (RULER_Y_COUNT + 1), -1.0f);
		}
	}

	for (int i = 1; i < RULER_X_COUNT + 1; i++) {
			glVertex3d(WINDOW_WIDTH * i / (RULER_X_COUNT + 1), 0, -1.0f);
			glVertex3d(WINDOW_WIDTH * i / (RULER_X_COUNT + 1), WINDOW_HEIGHT, -1.0f);
	}
	glEnd();
	glDisable(GL_LINE_STIPPLE);

	glBegin(GL_LINES);
	glVertex3d(0, WINDOW_HEIGHT / 2, -1.0f);
	glVertex3d(WINDOW_WIDTH, WINDOW_HEIGHT / 2, -1.0f);
	glEnd();

	glPopMatrix();
}

void drawFrame(GLfloat* vertices, GLfloat* colors) {

}

// render interface
void startRender() {
	renderFlag = true;

	// start render thread
	renderThread = SDL_CreateThread(renderThreadFunc, "renderThread", NULL);
}

void stopRender() {
	int retValue;

	renderFlag = false;
	SDL_WaitThread(renderThread, &retValue);
}

int renderThreadFunc(void* data) {
	// must create local GL context in render thread, use main thread created sdlWindow
	SDL_GLContext glContext = SDL_GL_CreateContext(sdlWindow);
	// now init our local GL context
	initGL(WINDOW_WIDTH, WINDOW_HEIGHT);

	// freetype 2
	FT_Library ftLib;
	FT_Error ftErr;
	
	// init
	ftErr = FT_Init_FreeType(&ftLib);
	if (ftErr != FT_Err_Ok) {
		// TODO : ft init error - ignored for now
		cout << "FT_Init_FreeType() failed." << endl;
	}

	FT_Face ftFace;
	ftErr = FT_New_Face(ftLib, "ARIALUNI.TTF", 0, &ftFace);
	if (ftErr == FT_Err_Unknown_File_Format) {
		// TODO : file could be read, but its format is unknown
		cout << "FT_New_Face() found unknown format." << endl;
	}
	else if (ftErr != FT_Err_Ok) {
	  // TODO : couldn't read font file
		cout << "FT_New_Face() failed. " << ftErr << endl;
	}
	
	ftErr = FT_Set_Char_Size(ftFace, 0, 12 * 64, 96, 96);
	if (ftErr != FT_Err_Ok) {
	  // TODO : error handler - ignored for now
		cout << "FT_Set_Char_Size() failed." << endl;
	}
	
	// render loop
	while (renderFlag) {
		drawRuler();
		// TODO : draw frame

		// TODO : remove this - freetype test
		// ftErr = FT_Load_Char(ftFace, 0x00042, FT_LOAD_RENDER);
		
		FT_UInt  glyph_index;
		glyph_index = FT_Get_Char_Index(ftFace, 0x0041);
		ftErr = FT_Load_Glyph(ftFace, glyph_index, FT_LOAD_DEFAULT);
		if (ftErr)
			continue;
		ftErr = FT_Render_Glyph(ftFace->glyph, FT_RENDER_MODE_NORMAL);
		if (ftErr)
			continue;

		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glPixelStorei(GL_UNPACK_LSB_FIRST, 1);
		int moreRowBits = ftFace->glyph->bitmap.width % 8;
		int rowBytes = ftFace->glyph->bitmap.width / 8;
		if (moreRowBits != 0)
			rowBytes++;

		GLubyte* bitmap[8];
		for (int i = 0; i < 8; i++) {
			bitmap[i] = (GLubyte*)malloc(rowBytes * ftFace->glyph->bitmap.rows);
			memset(bitmap[i], 0, rowBytes * ftFace->glyph->bitmap.rows);
		}

		for (int i = 0; i < ftFace->glyph->bitmap.rows; i++) {
			for (int j = 0; j < ftFace->glyph->bitmap.width; j++) {
				int revI = ftFace->glyph->bitmap.rows - i - 1;
				int index = revI * ftFace->glyph->bitmap.width + j;
				GLubyte level = ftFace->glyph->bitmap.buffer[index] >> 5;
				if (ftFace->glyph->bitmap.buffer[index] != 0) {
					int byteIndex = i * rowBytes + j / 8;
					int bitOffset = j % 8;
					(bitmap[level])[byteIndex] |= 1 << bitOffset;
				}
			}
		}

		glPushMatrix();
		glLoadIdentity();
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ZERO);
		GLubyte fontColor[4] = TITLE_CHAR_COLOR;
		GLubyte currColor[4] = TITLE_CHAR_COLOR;
		currColor[3] = fontColor[3] * 0.125f;

		for (int i = 0; i < 8; i++) {
			glColor4ubv(currColor);
			glRasterPos2i(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);
			glBitmap(ftFace->glyph->bitmap.width, ftFace->glyph->bitmap.rows, 0, 0, 0, 0, bitmap[i]);
			currColor[3] += fontColor[3] * 0.125f;
		}
		glDisable(GL_BLEND);
		glPopMatrix();

		for (int i = 0; i < 8; i++) {
			free(bitmap[i]);
		}

		renderBuffer();

		// 7. frame rate control
		if (SDL_GetTicks() - fpsTicks < fpsDelay)
			SDL_Delay(fpsDelay - SDL_GetTicks() + fpsTicks);

		fpsTicks = SDL_GetTicks();
	}

	return 0;
}

void buildFont() {

}