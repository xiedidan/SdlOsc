#ifndef PIPELINE_H
#define PIPELINE_H

#define SAMPLE_DEPTH 8
#define SAMPLE_MAX_VALUE 256
#define PIPELINE_READ_TIMEOUT 1000

typedef struct _PIPELINE_THREAD_DATA {
	int channelBytePerFrame;
} PIPELINE_THREAD_DATA;

void startPipeline();
void stopPipeline();

int pipelineThreadFunc(void* data);

int getArrays(GLfloat* vertDest, GLbyte* colorDest, int channelNo);

// helper
int getChannelDataRate(int channelCount);
int getChannelBytePerFrame(int channelDataRate, int frameRate);
int copyChannelFrameData(byte* data, int channelBytePerFrame);

#endif // PIPELINE_H