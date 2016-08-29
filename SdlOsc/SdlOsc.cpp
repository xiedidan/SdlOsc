// SdlOsc.cpp : Defines the entry point for the console application.

#include "stdafx.h"
#include <iostream>

#include "ftdi\include\ftd2xx.h"
#include "sdl\include\SDL.h"
#include "sdl\include\SDL_timer.h"
#include "glew\include\glew.h"

#include "imgui\imgui.h"
#include "imgui_service.h"

#include "common.h"
#include "ftdi_service.h"
#include "sdl_service.h"
#include "pipeline.h"
#include "event_handler.h"
#include "cursor_service.h"

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
		return EXIT_NO_FTDI_DEVICE;
	}

	cout << endl << "Input port index you want to use: ";
	int index;
	cin >> index;

	// 2. open selected port
	if (!openFtdiPort(index)) {
		cout << "Exiting." << endl;
		return EXIT_FTDI_OPEN_FAILED;
	}

	// 3. create a thread to read usb data
	startFtdiReadThread();
	#else
	startDataSimulatorThread();
	#endif

	// 4. create a thread to prepare frame
	startPipeline();

	// 5. init SDL
	SDL_Window* window = initSDL(WINDOW_WIDTH, WINDOW_HEIGHT);
	atexit(SDL_Quit);

	// 6. start mouse capture
	startCursorThread();

	// 7. create a thread to render
	startRender();

	// 8. handle user inputs - must be in main thread
	while (!quitFlag) {
		while (SDL_PollEvent(&event)) {
			ImGui_Service_ProcessEvent(&event);
			eventHandler(&event);
		}

		Sleep(1);
	}

	// 9. clean up
	stopFtdiReadThread();
	stopPipeline();
	stopCursorThread();
	stopRender();

	return EXIT_SUCCESS;
}
