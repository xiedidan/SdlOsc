#ifndef DATA_SIMULATOR_H
#define DATA_SIMULATOR_H

typedef enum _SIM_DATA_TYPE{
	Random,
	Sine
} SIM_DATA_TYPE;

void startDataSimulatorThread(SIM_DATA_TYPE type);

// helper
int simThreadFunc(void* data);

#endif