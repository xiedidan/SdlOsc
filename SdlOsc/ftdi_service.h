#ifndef FTDI_SERVICE_H
#define FTDI_SERVICE_H

#define FTDI_DEVINFO_BUF_SIZE 64
#define FTDI_READ_TIMEOUT 20000
#define FTDI_WRITE_TIMEOUT 1000
#define FTDI_READ_BUF_SIZE 65536
#define FTDI_READ_BUF_COUNT 2048
#define FTDI_DATA_RATE 16384000

bool listFtdiPorts();
bool openFtdiPort(int index);
void startFtdiReadThread();
void stopFtdiReadThread();
int getData(byte* Data, int bufCount);

// helper
int readThreadFunc(void* data);

#endif // FTDI_SERVICE_H

