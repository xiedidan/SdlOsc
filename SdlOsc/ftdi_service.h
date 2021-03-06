#ifndef FTDI_SERVICE_H
#define FTDI_SERVICE_H

#define FTDI_DEVINFO_BUF_SIZE 64
#define FTDI_READ_TIMEOUT 1000
#define FTDI_WRITE_TIMEOUT 1000
#define FTDI_READ_BUF_COUNT 2048
#define FTDI_READ_WAIT_TIMEOUT 1000

typedef struct _FTDI_CONFIG {
	int readBufSize = 65536;
	int dataRate = 16384000;
} FTDI_CONFIG;

extern FTDI_CONFIG ftdiConfig;

bool listFtdiPorts();
bool openFtdiPort(int index);
void startFtdiReadThread();
void stopFtdiReadThread();
int getAllData(byte** dataPtr);
// int getData(byte* Data, int bufCount);
// int getDataTimeout(byte* data, int bufCount, int timeout);

// helper
int readThreadFunc(void* data);

#endif // FTDI_SERVICE_H

