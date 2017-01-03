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

uint32_t simBufRate = FTDI_DATA_RATE / FTDI_READ_BUF_SIZE;
uint32_t simSignalFreq = 1000;

void startDataSimulatorThread(SIM_DATA_TYPE type) {
	SIM_DATA_TYPE* pType = (SIM_DATA_TYPE*)malloc(sizeof(SIM_DATA_TYPE));
	*pType = type;

	simThreadQuitFlag = false;
	readThreadBufferLock = SDL_CreateSemaphore(1);
	simThread = SDL_CreateThread(simThreadFunc, "SimThread", pType);
}

int simThreadFunc(void* data) {
	SIM_DATA_TYPE type = *((SIM_DATA_TYPE*)data);
	if (data != NULL)
		free(data);

	while (!simThreadQuitFlag) {
		byte* buffer = (byte*)malloc(FTDI_READ_BUF_SIZE);
		int res;

		switch (type) {
		case Random:
			// create random buffer
			srand(time(NULL));
			for (int i = 0; i < FTDI_READ_BUF_SIZE; i++) {
				buffer[i] = rand() % 256;
			}

			res = SDL_SemWaitTimeout(readThreadBufferLock, FTDI_READ_WAIT_TIMEOUT);
			if (res == SDL_MUTEX_TIMEDOUT) {
				continue;
			}

			if (bufferQueue.size() < FTDI_READ_BUF_COUNT)
				bufferQueue.push(buffer);

			// cout << "simThreadFunc bufferQueue.size(): " << bufferQueue.size() << endl;
			SDL_SemPost(readThreadBufferLock);

			// control generating speed
			if (SDL_GetTicks() - simTicks < simBufferDelay)
				SDL_Delay(simBufferDelay - SDL_GetTicks() + simTicks);
			simTicks = SDL_GetTicks();
			break;

		case Sine:
			buffer[i] = sin(w * t);
			break;

		default:
			// create random buffer
			srand(time(NULL));
			for (int i = 0; i < FTDI_READ_BUF_SIZE; i++) {
				buffer[i] = rand() % 256;
			}

			res = SDL_SemWaitTimeout(readThreadBufferLock, FTDI_READ_WAIT_TIMEOUT);
			if (res == SDL_MUTEX_TIMEDOUT) {
				continue;
			}

			if (bufferQueue.size() < FTDI_READ_BUF_COUNT)
				bufferQueue.push(buffer);

			// cout << "simThreadFunc bufferQueue.size(): " << bufferQueue.size() << endl;
			SDL_SemPost(readThreadBufferLock);

			// control generating speed
			if (SDL_GetTicks() - simTicks < simBufferDelay)
				SDL_Delay(simBufferDelay - SDL_GetTicks() + simTicks);
			simTicks = SDL_GetTicks();
			break;
		}
	}

	return 0;
}