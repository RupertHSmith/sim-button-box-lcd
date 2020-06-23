#include "pch.h"
#include "ACTelemetryProcessor.h"

void ACTelemetryProcessor::SetReceiver(std::unique_ptr<AcSDKDataReceiver> sdkDataReceiver)
{
	acSDKDataReceiver = move(sdkDataReceiver);
	receiverSet = true;
}

MEMORY_MAP_STATUS ACTelemetryProcessor::InitMemoryMap()
{
	MEMORY_MAP_STATUS result;
	result = initPhysics();
	if (result)
		return result;

	result = initGraphics();
	if (result)
		return result;

	result = initStatic();
	if (result)
		return result;	

	//TODO: May need code to release mem map for succeeded elements if some fail and we return early
	initialised = true;
	return MEM_MAP_OK;
}

MEMORY_MAP_STATUS ACTelemetryProcessor::initPhysics()
{
	TCHAR szName[] = TEXT("Local\\acpmf_physics");
	m_physics.hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(SPageFilePhysics), szName);
	if (!m_physics.hMapFile)
	{
		return MEM_MAP_CREATE_FAILED;
	}
	m_physics.mapFileBuffer = (unsigned char*)MapViewOfFile(m_physics.hMapFile, FILE_MAP_READ, 0, 0, sizeof(SPageFilePhysics));
	if (!m_physics.mapFileBuffer)
	{
		return MEM_MAP_VIEW_FAILED;
	}
	return MEM_MAP_OK;
}

MEMORY_MAP_STATUS ACTelemetryProcessor::initGraphics()
{
	TCHAR szName[] = TEXT("Local\\acpmf_graphics");
	m_graphics.hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(SPageFileGraphic), szName);
	if (!m_graphics.hMapFile)
	{
		return MEM_MAP_CREATE_FAILED;
	}
	m_graphics.mapFileBuffer = (unsigned char*)MapViewOfFile(m_graphics.hMapFile, FILE_MAP_READ, 0, 0, sizeof(SPageFileGraphic));
	if (!m_graphics.mapFileBuffer)
	{
		return MEM_MAP_VIEW_FAILED;
	}
	return MEM_MAP_OK;
}

MEMORY_MAP_STATUS ACTelemetryProcessor::initStatic()
{
	TCHAR szName[] = TEXT("Local\\acpmf_static");
	m_static.hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(SPageFileStatic), szName);
	if (!m_static.hMapFile)
	{
		return MEM_MAP_CREATE_FAILED;
	}
	m_static.mapFileBuffer = (unsigned char*)MapViewOfFile(m_static.hMapFile, FILE_MAP_READ, 0, 0, sizeof(SPageFileStatic));
	if (!m_static.mapFileBuffer)
	{
		return MEM_MAP_VIEW_FAILED;
	}
	return MEM_MAP_OK;
}

TELEMETRY_STATUS ACTelemetryProcessor::StartUpdates()
{
	if (!receiverSet || !initialised)
		return TELEMETRY_FAILED;

	running = true;
	//Start thread to send data at ~ 40 ms intervals (timing accuracy is not critical)
	updateThread = std::make_unique<std::thread>([&, acSDKDataReceiver = move(acSDKDataReceiver)]() {
		//when we move ownership of acSDKDataReceiver here we no longer have access to its pointer I think
		receiverSet = false;
		while (running)
		{
			SPageFileGraphic* pGraphic = (SPageFileGraphic*)m_graphics.mapFileBuffer;
			SPageFilePhysics* pPhysics = (SPageFilePhysics*)m_physics.mapFileBuffer;
			SPageFileStatic* pStatic = (SPageFileStatic*)m_static.mapFileBuffer;

			acSDKDataReceiver->ReceiveUpdate(*pPhysics, *pGraphic, *pStatic);
			std::this_thread::sleep_for(std::chrono::milliseconds(40));
		}

	});
	return TELEMETRY_READING;
}

ACTelemetryProcessor::ACTelemetryProcessor()
{
}

ACTelemetryProcessor::~ACTelemetryProcessor()
{
	//Clear up thread..
	if (updateThread != nullptr)
	{
		//Then we need to set running false and wait for thread to join
		running = false;
		updateThread->join();
	}
}
