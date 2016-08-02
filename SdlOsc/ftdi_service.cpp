#include "stdafx.h"
#include <iostream>
#include <queue>
#include "ftdi\include\ftd2xx.h"
#include "sdl\include\SDL.h"
#include "sdl\include\SDL_thread.h"

#include "ftdi_service.h"

using namespace std;

FT_HANDLE ftHandle;
char** bufPointers;

SDL_Thread* readThread = NULL;
bool readThreadQuitFlag = false;
SDL_sem* readThreadBufferLock = NULL;

queue<byte*> bufferQueue;

bool listFtdiPorts() {
	FT_STATUS ftStatus;
	DWORD deviceCount;
	
	// get device count
	FT_ListDevices(&deviceCount, NULL, FT_LIST_NUMBER_ONLY);
	if (deviceCount == 0) {
		cout << "listFtdiPorts Error: No FTDI device found." << endl;
		return false;
	}

	cout << "listFtdiPorts: Found " << deviceCount << " FTDI device(s)." << endl;

	// print details of each device
	bufPointers = (char**)malloc(deviceCount * sizeof(char*) + 1);
	for (unsigned int i = 0; i < deviceCount + 1; i++) {
		if (i < deviceCount) {
			char* buf = (char*)malloc(FTDI_DEVINFO_BUF_SIZE);
			bufPointers[i] = buf;
		}
		else {
			// the last item should be NULL
			bufPointers[i] = NULL;
		}
	}
	ftStatus = FT_ListDevices(bufPointers, &deviceCount, FT_LIST_ALL | FT_OPEN_BY_DESCRIPTION);
	if (ftStatus == FT_OK) {
		// print all info
		for (unsigned int i = 0; i < deviceCount; i++) {
			cout << i + 1 << ". " << bufPointers[i] << endl;
		}
	}
	else {
		cout << "listFtdiPorts Error: List FTDI device failed." << endl;
		return false;
	}

	return true;
}

bool openFtdiPort(int index) {
	FT_STATUS ftStatus;
	ftStatus = FT_OpenEx(bufPointers[index - 1], FT_OPEN_BY_DESCRIPTION, &ftHandle);
	if (ftStatus == FT_OK) {
		cout << bufPointers[index - 1] << " is opened successfully." << endl;

		FT_Purge(ftHandle, FT_PURGE_RX | FT_PURGE_TX);
		FT_SetBitMode(ftHandle, 0, 0x40); // sync 245 fifo
		FT_SetUSBParameters(ftHandle, FTDI_READ_BUF_SIZE, 4096);
		FT_SetFlowControl(ftHandle, FT_FLOW_RTS_CTS, 0, 0);	// required to avoid data loss, see appnote "an_130_ft2232h_used_in_ft245 synchronous fifo mode.pdf"
		FT_SetTimeouts(ftHandle, FTDI_READ_TIMEOUT, FTDI_WRITE_TIMEOUT);

		return true;
	}
	else {
		cout << "Open " << bufPointers[index - 1] << " failed." << endl;
		return false;
	}
}

void startFtdiReadThread() {
	readThreadQuitFlag = false;
	readThreadBufferLock = SDL_CreateSemaphore(1);
	readThread = SDL_CreateThread(readThreadFunc, "readThread", NULL);
}

void stopFtdiReadThread() {
	int retValue;
	cout << "Stopping FTDI... ";

	readThreadQuitFlag = true;
	SDL_WaitThread(readThread, &retValue);
	cout << "Done" << endl;
}

int getData(byte* data, int bufCount) {
	// caller have to malloc data

	int size = 0;
	int bufLeft = bufCount;

	while (bufLeft > 0) {
		if (!bufferQueue.empty()) {
			SDL_SemWait(readThreadBufferLock);

			byte* buffer = bufferQueue.front();
			bufferQueue.pop();

			SDL_SemPost(readThreadBufferLock);

			memcpy((byte*)(data + size), buffer, FTDI_READ_BUF_SIZE);
			free(buffer);

			size += FTDI_READ_BUF_SIZE;
			bufLeft--;
		}
		else {
			// wait for 1ms - Sleep() only works on Windows, TODO : port to linux
			Sleep(1);
		}
	}

	return size;
}

int getDataTimeout(byte* data, int bufCount, int timeout) {
	// caller have to malloc data

	int size = 0;
	int bufLeft = bufCount;
	int counter = 0;

	while (bufLeft > 0) {
		if (!bufferQueue.empty()) {
			SDL_SemWait(readThreadBufferLock);

			byte* buffer = bufferQueue.front();
			bufferQueue.pop();

			SDL_SemPost(readThreadBufferLock);

			memcpy((byte*)(data + size), buffer, FTDI_READ_BUF_SIZE);
			free(buffer);

			size += FTDI_READ_BUF_SIZE;
			bufLeft--;
		}
		else {
			// wait for 1ms - Sleep() only works on Windows, TODO : port to linux
			Sleep(1);
			counter++;
			if (counter >= timeout) {
				return size;
			}
		}
	}

	return size;
}

// helper
int readThreadFunc(void* data) {
	while (!readThreadQuitFlag) {
		// read data from ftdi device
		DWORD bytesRead = 0;
		byte* buffer = (byte*)malloc(FTDI_READ_BUF_SIZE);

		//  TODO : what if bytesRead < FTDI_READ_BUF_SIZE or FT_READ < 0?
		FT_Read(ftHandle, buffer, FTDI_READ_BUF_SIZE, &bytesRead);

		int res = SDL_SemWaitTimeout(readThreadBufferLock, FTDI_READ_WAIT_TIMEOUT);
		if (res == SDL_MUTEX_TIMEDOUT) {
			continue;
		}

		if (bufferQueue.size() < FTDI_READ_BUF_COUNT)
			bufferQueue.push(buffer);
		else
			free(buffer);

		// cout << "bufferQueue.size(): " << bufferQueue.size() << endl;
		SDL_SemPost(readThreadBufferLock);
	}

	return 0;
}