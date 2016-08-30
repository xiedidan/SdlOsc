#include "stdafx.h"
#include <iostream>

#include "sdl\include\SDL.h"
#include "sdl\include\SDL_thread.h"
#include "glew\include\glew.h"

#include "common.h"
#include "sdl_service.h"
#define THEME_ENERY
#include "theme.h"
#include "cursor_service.h"

using namespace std;

volatile int mouseX = 0;
volatile int mouseY = 0;
volatile Uint32 mouseButtonMask = 0;
volatile bool cursorServiceFlag = false;

SDL_Thread* cursorThread = NULL;

void drawCursor() {
	GLubyte cursorColor[4] = INFO_CHAR_COLOR;

	glPushMatrix();

	glLoadIdentity();
	glColor4ubv(cursorColor);
	glLineWidth(1.0f);
	glLineStipple(2, 0xaaaa);

	glEnable(GL_LINE_STIPPLE);
	glBegin(GL_LINES);

	// horizontal
	double y = (double)windowHeight - mouseY - 1.0f;
	glVertex3d(0, y, (GLfloat)0.5f);
	glVertex3d(windowWidth, y, (GLfloat)0.5f);

	// vertical
	double x = mouseX + 1.0f;
	glVertex3d(x, 0, (GLfloat)0.5f);
	glVertex3d(x, windowHeight, (GLfloat)0.5f);

	glEnd();
	glDisable(GL_LINE_STIPPLE);

	glPopMatrix();
}

void startCursorThread() {
	cursorServiceFlag = true;
	cursorThread = SDL_CreateThread(cursorThreadFunc, "cursorThread", NULL);
}

void stopCursorThread() {
	int retValue;
	cout << "Stopping cursor service... ";

	cursorServiceFlag = false;
	SDL_WaitThread(cursorThread, &retValue);

	cout << "Done" << endl;
}

int cursorThreadFunc(void* data) {
	while (cursorServiceFlag)
		mouseButtonMask = SDL_GetMouseState((int*)&mouseX, (int*)&mouseY);

	return 0;
}