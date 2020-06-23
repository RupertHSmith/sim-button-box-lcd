#pragma once
#include "AcSharedFileOut.h"
#include "AcSDKDataReceiver.h"

#include <Windows.h>
#include <memory>
#include <thread>
#include <atomic>

struct SMElement
{
	HANDLE hMapFile;
	unsigned char* mapFileBuffer;
};

typedef int TELEMETRY_STATUS;
#define TELEMETRY_READING 0
#define TELEMETRY_FAILED -1

typedef int MEMORY_MAP_STATUS;

#define MEM_MAP_OK 0
#define MEM_MAP_CREATE_FAILED -1
#define MEM_MAP_VIEW_FAILED -2

class ACTelemetryProcessor {
public:
	ACTelemetryProcessor();
	~ACTelemetryProcessor();
	MEMORY_MAP_STATUS InitMemoryMap();
	void SetReceiver(std::unique_ptr<AcSDKDataReceiver> sdkDataReceiver); //Add receiver type
	TELEMETRY_STATUS StartUpdates();
private:
	//Receiver
	SMElement m_graphics;
	SMElement m_physics;
	SMElement m_static;
	std::unique_ptr<AcSDKDataReceiver> acSDKDataReceiver;
	std::unique_ptr<std::thread> updateThread;
	bool receiverSet;
	bool initialised;
	std::atomic<bool> running;

	MEMORY_MAP_STATUS initPhysics();
	MEMORY_MAP_STATUS initGraphics();
	MEMORY_MAP_STATUS initStatic();
};
