// SdlOsc.cpp : Defines the entry point for the console application.

#include "stdafx.h"
#include <iostream>

#include "ftdi\include\ftd2xx.h"
#include "sdl\include\SDL.h"
#include "sdl\include\SDL_timer.h"
#include "sdl\include\SDL_opengl.h"
#include "imgui\imgui.h"
#include "imgui_impl_sdl.h"

/*
#pragma comment(lib, "opengl32")
#pragma comment(lib, "glu32")
#include <GL/gl.h>
#include <GL/glu.h>
*/

#include "common.h"
#include "ftdi_service.h"
#include "sdl_service.h"
#include "pipeline.h"
#include "event_handler.h"

#define DATA_SIMULATOR 1
#ifdef DATA_SIMULATOR
#include "data_simulator.h"
#endif

using namespace std;

bool quitFlag = false;
SDL_Event event;

int main(int argc, char *argv[])
{
	cout << endl << "*** " << PROGRAM_NAME << " Ver." << MAIN_VER << "." << BRANCH_VER << "." << MINOR_VER << " ***" << endl;
	cout << "Created by " << CREATOR << " " << CREATE_DATE << endl;

	#ifndef DATA_SIMULATOR
	// 1. list ftdi ports
	cout << endl;
	if (!listFtdiPorts()) {
		cout << "Exiting." << endl;
		return 1;
	}

	cout << endl << "Input port index you want to use: ";
	int index;
	cin >> index;

	// 2. open selected port
	if (!openFtdiPort(index)) {
		cout << "Exiting." << endl;
		return 2;
	}

	// 3. create a thread to read usb data
	startFtdiReadThread();
	#else
	startDataSimulatorThread();
	#endif

	// 4. TODO : create a thread to prepare frame
	startPipeline();

	// 5. init SDL
	SDL_Window* window = initSDL(WINDOW_WIDTH, WINDOW_HEIGHT);
	atexit(SDL_Quit);

	// 6. TODO : create a thread to render
	startRender();

	// 7. TODO : handle user inputs - must be in main thread
	while (!quitFlag) {
		while (SDL_PollEvent(&event)) {
			ImGui_ImplSdl_ProcessEvent(&event);
			eventHandler(&event);
		}

		Sleep(1);
	}

	// 8. clean up
	stopFtdiReadThread();
	stopPipeline();
	stopRender();

	return 0;
}
