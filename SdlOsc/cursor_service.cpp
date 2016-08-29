#include "stdafx.h"
#include <iostream>

#include "sdl\include\SDL.h"
#include "glew\include\glew.h"

#include "cursor_service.h"

int mouseX = 0;
int mouseY = 0;
Uint32 mouseButtonMask = 0;

void drawCursor() {
	cursorHandler();
}

void cursorHandler() {
	mouseButtonMask = SDL_GetMouseState(&mouseX, &mouseY);
}