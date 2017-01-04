#include "stdafx.h"
#include <iostream>

#pragma comment(lib, "glew32")
#pragma comment(lib, "opengl32")
#pragma comment(lib, "glu32")

#include "ftdi\include\ftd2xx.h" // for byte typedef
#include "sdl\include\SDL.h"
#include "sdl\include\SDL_thread.h"
#include "glew\include\glew.h"

#include "imgui\imgui.h"
#include "imgui_service.h"

#include <ft2build.h>
#include FT_FREETYPE_H
#include "string_render.h"

#include "common.h"
#include "pipeline.h"
#include "cursor_service.h"
#include "sdl_service.h"

#define THEME_ENERY
#include "theme.h"

using namespace std;

// global
int windowWidth = 1280;
int windowHeight = 720;
int divWidth = windowWidth / RULER_X_COUNT;
int divHeight = windowHeight / RULER_Y_COUNT;

// fps control
uint32_t fpsDelay = 1000 / FPS_TARGET;
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
	GLenum ret = glewInit();
	if (ret != GLEW_OK) {
		SDL_Quit();
		exit(EXIT_GLEW_INIT_FAILED);
	}

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

	windowWidth = width;
	windowHeight = height;

	divWidth = windowWidth / RULER_X_COUNT;
	divHeight = windowHeight / RULER_Y_COUNT;
}

SDL_Window* initSDL(int width, int height) {
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		sdlExit(1, "SDL init failed.");
	}

	divWidth = windowWidth / RULER_X_COUNT;
	divHeight = windowHeight / RULER_Y_COUNT;

	// require at least OpenGL 3.0 Compatibility Profile, for now, we use OpenGL 4.5
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);

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

	return sdlWindow;
}

void renderBuffer() {
	SDL_GL_SwapWindow(sdlWindow);
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
			glVertex3d(0, windowHeight * i / (RULER_Y_COUNT + 1), -1.0f);
			glVertex3d(windowWidth, windowHeight * i / (RULER_Y_COUNT + 1), -1.0f);
		}
	}

	for (int i = 1; i < RULER_X_COUNT + 1; i++) {
			glVertex3d(windowWidth * i / (RULER_X_COUNT + 1), 0, -1.0f);
			glVertex3d(windowWidth * i / (RULER_X_COUNT + 1), windowHeight, -1.0f);
	}
	glEnd();
	glDisable(GL_LINE_STIPPLE);

	glBegin(GL_LINES);
	glVertex3d(0, windowHeight / 2, -1.0f);
	glVertex3d(windowWidth, windowHeight / 2, -1.0f);
	glEnd();

	glPopMatrix();
}

void drawFrame(GLfloat* vertices, int pixelCount) {
	GLubyte channel1Color[4] = CHANNEL1_COLOR;
	int renderCount = 0;
	int lastRenderCount = 0;
	bool finishFlag = false;

	// draw
	glPushMatrix();

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(VERTEX_DIM, GL_FLOAT, 0, vertices);

	glLineWidth(1.0f);
	glColor4ubv(channel1Color);

	/* render method 1 - glVertex2f()
	while (renderCount < pixelCount) {
		if (!drawingFlag) {
			glBegin(GL_LINES);
			glLoadIdentity();
			drawingFlag = true;
		}

		if (renderCount > 0 && vertices[renderCount * VERTEX_DIM] < vertices[(renderCount - 1) * VERTEX_DIM]) {
			glEnd();
			drawingFlag = false;
		}
		else {
			glVertex2f(vertices[renderCount * VERTEX_DIM], vertices[renderCount * VERTEX_DIM + 1]);
		}

		renderCount++;
	} // while

	if (drawingFlag)
		glEnd();
	*/

	/* render method 2 - glArrayElement()
	glBegin(GL_LINES);
	for (int i = 0; i < pixelCount; i++)
		glArrayElement(i);
	glEnd();
	*/

	// render method 3 - glDrawArrays()
	while (renderCount < pixelCount) {
		// look for screen edge
		if (renderCount > 0 && vertices[renderCount * VERTEX_DIM] < vertices[(renderCount - 1) * VERTEX_DIM]) {
			// got edge
			glDrawArrays(GL_LINE_STRIP, lastRenderCount, renderCount - lastRenderCount);
			lastRenderCount = renderCount;
			finishFlag = true;
		}
		else {
			finishFlag = false;
		}

		renderCount++;
	}

	if (!finishFlag) {
		glDrawArrays(GL_LINE_STRIP, lastRenderCount, renderCount - lastRenderCount);
	}

	glPopMatrix();
}

// render interface
void startRender() {
	renderFlag = true;

	// start render thread
	renderThread = SDL_CreateThread(renderThreadFunc, "renderThread", NULL);
}

void stopRender() {
	int retValue;
	cout << "Stopping render... ";

	renderFlag = false;
	SDL_WaitThread(renderThread, &retValue);

	cout << "Done" << endl;
}

int renderThreadFunc(void* data) {
	// must create local GL context in render thread, use main thread created sdlWindow
	SDL_GLContext glContext = SDL_GL_CreateContext(sdlWindow);
	// now init our local GL context
	initGL(windowWidth, windowHeight);

	// init FreeType2
	initFreeType();
	initCharmap();

	// init ImGUI
	ImGui_Service_Init(sdlWindow);
	
	// render loop
	while (renderFlag) {
		// check resize
		int w = 0;
		int h = 0;
		SDL_GL_GetDrawableSize(sdlWindow, &w, &h);
		if (windowWidth != w || windowHeight != h)
			resizeGL(w, h);

		ImGui_Service_NewFrame(sdlWindow);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		drawRuler();

		// get next frame from pipeline
		GLfloat* vertArr = NULL;
		int pixelCount = getArrays(&vertArr, 0);
		// draw frame
		drawFrame(vertArr, pixelCount);
		free(vertArr);

		// TODO : draw strings
		printGL(windowWidth / 2 + 400, 96, "OpenGL %s", glGetString(GL_VERSION));
		printGL(windowWidth / 2 + 400, 64, "%s", glGetString(GL_VENDOR));
		printGL(windowWidth / 2 + 400, 32, "%s", glGetString(GL_RENDERER));

		drawCursor();

		// draw ImGUI
		{
			GLbyte clearColor[4] = BACKGROUND_COLOR;
			ImVec4 clear_color = ImColor(clearColor[0], clearColor[1], clearColor[2]);

			ImGuiWindowFlags window_flags = 0;
			// window_flags |= ImGuiWindowFlags_NoTitleBar;
			// window_flags |= ImGuiWindowFlags_ShowBorders;
			window_flags |= ImGuiWindowFlags_NoResize;
			// window_flags |= ImGuiWindowFlags_NoMove;
			window_flags |= ImGuiWindowFlags_NoScrollbar;
			window_flags |= ImGuiWindowFlags_NoCollapse;
			// window_flags |= ImGuiWindowFlags_MenuBar;

			ImGui::Begin("Perf. Counter", 0, window_flags);
			ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			ImGui::Text("Mouse Pos: (%d, %d)", mouseX, mouseY);
			ImGui::End();
		}

		ImGui::Render();

		renderBuffer();

		// 7. frame rate control
		/*
		if (SDL_GetTicks() - fpsTicks < fpsDelay)
			SDL_Delay(fpsDelay - SDL_GetTicks() + fpsTicks);

		fpsTicks = SDL_GetTicks();
		*/
	}

	// clean up ImGUI
	ImGui_Service_Shutdown();

	return 0;
}
