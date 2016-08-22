#include "stdafx.h"
#include <stdlib.h>
#include <iostream>
#include <queue>
#include <time.h>
#include "ftdi\include\ftd2xx.h" // for byte definiation
#include "sdl\include\SDL.h"
#include "sdl\include\SDL_thread.h"

#include "ftdi_service.h"
#include "data_simulator.h"

using namespace std;

extern queue<byte*> bufferQueue;

SDL_Thread* simThread = NULL;
bool simThreadQuitFlag = false;
extern SDL_sem* readThreadBufferLock;

uint32_t simBufferDelay =  1000 * FTDI_READ_BUF_SIZE / FTDI_DATA_RATE;
uint32_t simTicks = 0;

void startDataSimulatorThread() {
	simThreadQuitFlag = false;
	readThreadBufferLock = SDL_CreateSemaphore(1);
	simThread = SDL_CreateThread(simThreadFunc, "SimThread", NULL);
}

int simThreadFunc(void* data) {
	while (!simThreadQuitFlag) {
		// create random buffer
		byte* buffer = (byte*)malloc(FTDI_READ_BUF_SIZE);

		srand(time(NULL));
		for (int i = 0; i < FTDI_READ_BUF_SIZE; i++) {
			buffer[i] = rand() % 256;
		}

		int res = SDL_SemWaitTimeout(readThreadBufferLock, FTDI_READ_WAIT_TIMEOUT);
		if (res == SDL_MUTEX_TIMEDOUT) {
			continue;
		}

		if (bufferQueue.size() < FTDI_READ_BUF_COUNT)
			bufferQueue.push(buffer);
		else
			free(buffer);

		// cout << "simThreadFunc bufferQueue.size(): " << bufferQueue.size() << endl;
		SDL_SemPost(readThreadBufferLock);

		// control generating speed
		if (SDL_GetTicks() - simTicks < simBufferDelay)
			SDL_Delay(simBufferDelay - SDL_GetTicks() + simTicks);
		simTicks = SDL_GetTicks();
	}

	return 0;
}