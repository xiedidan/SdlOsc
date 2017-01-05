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

uint32_t simSignalFreq = 1506;

void startDataSimulatorThread(SIM_DATA_TYPE type) {
	SIM_DATA_TYPE* pType = (SIM_DATA_TYPE*)malloc(sizeof(SIM_DATA_TYPE));
	*pType = type;

	simThreadQuitFlag = false;
	readThreadBufferLock = SDL_CreateSemaphore(1);
	simThread = SDL_CreateThread(simThreadFunc, "SimThread", pType);
}

void stopDataSimulatorThread() {
	int retValue;
	cout << "Stopping data simulator... ";

	simThreadQuitFlag = true;
	SDL_WaitThread(simThread, &retValue);

	// clear buffer queue
	while (!bufferQueue.empty()) {
		byte* buffer = bufferQueue.front();
		free(buffer);
		bufferQueue.pop();
	}

	cout << "Done" << endl;
}

int simThreadFunc(void* data) {
	SIM_DATA_TYPE type = *((SIM_DATA_TYPE*)data);
	if (data != NULL)
		free(data);

	uint32_t tick = 0;

	while (!simThreadQuitFlag) {
		byte* buffer;
		int res;
		int bufferCounter = 0;
		double w = 2 * (double)M_PI / ((double)FTDI_DATA_RATE / (double)simSignalFreq);

		switch (type) {
		case Random:
			// create random buffer
			buffer = (byte*)malloc(FTDI_READ_BUF_SIZE);

			srand(time(NULL));
			for (int i = 0; i < FTDI_READ_BUF_SIZE; i++) {
				buffer[i] = (byte)(rand() % 256);
			}

			res = SDL_SemWaitTimeout(readThreadBufferLock, FTDI_READ_WAIT_TIMEOUT);
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
			break;

		case Sine:
			buffer = (byte*)malloc(FTDI_READ_BUF_SIZE);

			for (int i = 0; i < FTDI_READ_BUF_SIZE; i++) {
				buffer[i] = (byte)(64 * sin(w * tick) + 128);
				tick++;
				// if (tick == FTDI_DATA_RATE / simSignalFreq)
					// tick = 0;
			}

			res = SDL_SemWaitTimeout(readThreadBufferLock, FTDI_READ_WAIT_TIMEOUT);
			if (res == SDL_MUTEX_TIMEDOUT) {
				continue;
			}	

			if (bufferQueue.size() < FTDI_READ_BUF_COUNT)
				bufferQueue.push(buffer);
			else {
				free(buffer);
				cout << "free!" << endl;
			}
				

			SDL_SemPost(readThreadBufferLock);


			// control generating speed
			if (SDL_GetTicks() - simTicks < simBufferDelay)
				SDL_Delay(simBufferDelay - SDL_GetTicks() + simTicks);
			simTicks = SDL_GetTicks();
			break;

		default:
			// create random buffer
			buffer = (byte*)malloc(FTDI_READ_BUF_SIZE);

			srand(time(NULL));
			for (int i = 0; i < FTDI_READ_BUF_SIZE; i++) {
				buffer[i] = (byte)(rand() % 256);
			}

			res = SDL_SemWaitTimeout(readThreadBufferLock, FTDI_READ_WAIT_TIMEOUT);
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
			break;
		}
	}

	return 0;
}