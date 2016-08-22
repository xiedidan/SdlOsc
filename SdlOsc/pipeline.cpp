#include "stdafx.h"
#include <iostream>
#include <queue>

#include "ftdi\include\ftd2xx.h" // for byte typedef
#include "sdl\include\SDL.h"
#include "sdl\include\SDL_opengl.h"
#include "sdl\include\SDL_thread.h"

#include "common.h"
#include "sdl_service.h"
#include "ftdi_service.h"
#include "pipeline.h"

#define THEME_ENERY
#include "theme.h"

using namespace std;

GLbyte channel1Color[4] = CHANNEL1_COLOR;
GLbyte channel2Color[4] = CHANNEL2_COLOR;

// global parameter
int channelCount = 1; // 1 or 2
int timePerDiv[2] = { 100000, 100000 }; // ns
int voltagePerDiv[2] = { 1000, 1000 }; // mV

// last buffer
int lastBufOffset = FTDI_READ_BUF_SIZE; // empty
byte* lastBuf = NULL;

// frame queue
queue<byte*> frameQueue;

// pipeline variables
bool pipelineFlag = false;
SDL_Thread* pipelineThread = NULL;
SDL_sem* pipelineThreadLock = NULL;
bool pipelineEmptyFlag = true;

// vert and color arrays
GLfloat* vertices[2] = { NULL, NULL };
GLbyte* colors[2] = { NULL, NULL };
int renderPixelCount[2] = { 0, 0 };

// pipeline interface
void startPipeline() {
	int channelDataRate = getChannelDataRate(channelCount);
	int channelBytePerFrame = getChannelBytePerFrame(channelDataRate, FPS_TARGET);

	PIPELINE_THREAD_DATA pipelineThreadData;
	pipelineThreadData.channelBytePerFrame = channelBytePerFrame;

	pipelineFlag = true;

	// start pipeline thread
	pipelineThreadLock = SDL_CreateSemaphore(1);
	pipelineThread = SDL_CreateThread(pipelineThreadFunc, "pipelineThread", &pipelineThreadData);
}

void stopPipeline() {
	int retValue;
	cout << "Stopping pipeline... ";

	pipelineFlag = false;
	SDL_WaitThread(pipelineThread, &retValue);

	cout << "Done" << endl;
}

// pipeline thread func
int pipelineThreadFunc(void* data) {
	int lastX[2] = { 0, 0 };

	PIPELINE_THREAD_DATA* pipelineThreadData = (PIPELINE_THREAD_DATA*)data;
	int channelBytePerFrame = pipelineThreadData->channelBytePerFrame;

	int screenTimespan[2] = { timePerDiv[0] * RULER_X_COUNT, timePerDiv[1] * RULER_X_COUNT }; // ns
	int frameTimespan = 1000000000L / FPS_TARGET; // ns

	while (pipelineFlag) {
		if (pipelineEmptyFlag) {
			for (int i = 0; i < channelCount; i++) {
				if (frameTimespan <= screenTimespan[i]) {
					// need multiple (1 or more) frame(s) to fill screen
					int framePerScreen = screenTimespan[i] / frameTimespan;

					// 1. get new frame data
					byte* frameData = (byte*)malloc(channelBytePerFrame);
					int res = copyChannelFrameData(frameData, channelBytePerFrame);
					if (res == 0) {
						break;
					}

					if (frameQueue.size() == framePerScreen) {
						byte* lastFrame = frameQueue.front();
						frameQueue.pop();
						free(lastFrame);
					}
					frameQueue.push(frameData);

					// 2. TODO : prepare next frame - create vert and color array from frameQueue

				}
				else {
					// TODO : consider what happen if 1 sample crosses multiple pixel

					// 1 frame could fill multiple screen
					int screenPerFrame = frameTimespan / screenTimespan[i];
					int channelBytePerScreen = channelBytePerFrame / screenPerFrame;

					// 1. get new frame data
					byte* frameData = (byte*)malloc(channelBytePerFrame);
					int res = copyChannelFrameData(frameData, channelBytePerFrame);
					if (res == 0) {
						break;
					}

					// 2. prepare next frame - create vert and color array from frameData
					// cout << "channelBytePerScreen: " << channelBytePerScreen << endl;
					int channelBytePerPixel = channelBytePerScreen / WINDOW_WIDTH;
					int pixelPerFrame = channelBytePerFrame / channelBytePerPixel;
					renderPixelCount[i] = pixelPerFrame;

					if (vertices[i] != NULL) {
						free(vertices[i]);
						vertices[i] = NULL;
					}
					if (colors[i] != NULL) {
						free(colors[i]);
						colors[i] = NULL;
					}
					vertices[i] = (GLfloat*)malloc(pixelPerFrame * 4 * sizeof(GLfloat));
					colors[i] = (GLbyte*)malloc(pixelPerFrame * 4);

					for (int j = 0; j < pixelPerFrame; j++) {
						// compute pixel value
						int pixelSum = 0;
						for (int k = 0; k < channelBytePerPixel; k++) {
							pixelSum += frameData[j * channelBytePerPixel + k];
						}
						int pixelAverage = pixelSum / channelBytePerPixel;

						// SDL_SemWait(pipelineThreadLock);
						(vertices[i])[j * 4] = (GLfloat)((lastX[i] + j) % WINDOW_WIDTH); // X
						(vertices[i])[j * 4 + 1] = (GLfloat)pixelAverage / (GLfloat)SAMPLE_MAX_VALUE * WINDOW_HEIGHT; // Y
						(vertices[i])[j * 4 + 2] = (GLfloat)0.0f; // Z
						(vertices[i])[j * 4 + 3] = (GLfloat)0.0f; // W

						if (i == 0) {
							(colors[i])[j * 4] = channel1Color[0]; // R
							(colors[i])[j * 4 + 1] = channel1Color[1]; // G
							(colors[i])[j * 4 + 2] = channel1Color[2]; // B
							(colors[i])[j * 4 + 3] = channel1Color[3]; // A
						}
						else {
							(colors[i])[j * 4] = channel2Color[0]; // R
							(colors[i])[j * 4 + 1] = channel2Color[1]; // G
							(colors[i])[j * 4 + 2] = channel2Color[2]; // B
							(colors[i])[j * 4 + 3] = channel2Color[3]; // A
						}
						pipelineEmptyFlag = false;
						// SDL_SemPost(pipelineThreadLock);
					}

					lastX[i] = (lastX[i] + pixelPerFrame) % WINDOW_WIDTH;
					// cout << "lastX: " << lastX[i] << endl;

					free(frameData);
				}
			} // for
		} // if flags
		else {
			Sleep(1);
		}
	} // while (pipelineFlag)

	return 0;
}

int getArrays(GLfloat** vertDest, GLbyte** colorDest, int channelNo) {
	// caller doesn't have to alloc mem, but have to free

	while (pipelineEmptyFlag) {
		Sleep(1);
	}

	// SDL_SemWait(pipelineThreadLock);
	*vertDest = (GLfloat*)malloc(renderPixelCount[channelNo] * 4 * sizeof(GLfloat));
	memcpy(*vertDest, vertices[channelNo], renderPixelCount[channelNo] * 4 * sizeof(GLfloat));

	*colorDest = (GLbyte*)malloc(renderPixelCount[channelNo] * 4);
	memcpy(*colorDest, vertices[channelNo], renderPixelCount[channelNo] * 4);

	pipelineEmptyFlag = true;
	// SDL_SemPost(pipelineThreadLock);

	return renderPixelCount[channelNo];
}

// helper
int getChannelDataRate(int channelCount) {
	int channelBytePerSec = FTDI_DATA_RATE / channelCount;

	return channelBytePerSec;
}

int getChannelBytePerFrame(int channelDataRate, int frameRate) {
	return channelDataRate / frameRate;
}

int copyChannelFrameData(byte* data, int channelBytePerFrame) {
	// assume we could always read enough data frome queue - this is true for our getData() implementation

	// caller have to malloc data
	int lastBufLeftByte = FTDI_READ_BUF_SIZE - lastBufOffset;
	if (lastBufLeftByte >= channelBytePerFrame) {
		// enough bytes in lastBuf, just copy from there
		memcpy(data, lastBuf, channelBytePerFrame);

		if (lastBufLeftByte == channelBytePerFrame) {
			// last buf is empty
			lastBufOffset = FTDI_READ_BUF_SIZE;
		}
		else {
			lastBufOffset += channelBytePerFrame;
		}
	}
	else {
		// get remaining bytes from lastBuf
		if (lastBufLeftByte != 0) {
			memcpy(data, (byte*)(lastBuf + lastBufOffset), lastBufLeftByte);

			// now, last buf is empty, but we could update offset later
		}
		
		// we have to get more data from the queue
		int moreByte = channelBytePerFrame - lastBufLeftByte;
		int moreBuf = moreByte / FTDI_READ_BUF_SIZE;
		int res = getDataTimeout((byte*)(data + lastBufLeftByte), moreBuf, PIPELINE_READ_TIMEOUT);
		if (res == 0) {
			return 0;
		}

		// ... and maybe even 1 more block
		int lastByte = moreByte % FTDI_READ_BUF_SIZE;
		if (lastByte != 0) {
			if (lastBuf == NULL)
				lastBuf = (byte*)malloc(FTDI_READ_BUF_SIZE);

			res = getDataTimeout(lastBuf, 1, PIPELINE_READ_TIMEOUT);
			if (res == 0) {
				return 0;
			}

			memcpy((byte*)(data + (FTDI_READ_BUF_SIZE - lastByte)), lastBuf, lastByte);

			lastBufOffset = lastByte;
		}
		else {
			// just got enough data from the queue
			lastBufOffset = FTDI_READ_BUF_SIZE;
		}
	}

	return channelBytePerFrame;
}