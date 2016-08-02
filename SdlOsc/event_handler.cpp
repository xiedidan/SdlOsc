#include "stdafx.h"
#include <iostream>

#include "sdl\include\SDL.h"

#include "event_handler.h"

extern bool quitFlag;

void eventHandler(SDL_Event* event) {
	if (event->type == SDL_QUIT) {
		quitFlag = true;
	}
}

void keyEventHandler(SDL_Keysym* keysym) {

}

void mouseEventHandler() {

}