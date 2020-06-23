#pragma once
#include "AcSharedFileOut.h"
#include "DisplayController.h"

#define UPDATES_BEFORE_RESET 3

struct DeltaSample {
	float progress;
	int lapTime;
};

struct ResetTimer {
	bool resetEnabled;
	int resetCounter;
};

class AcSDKDataReceiver {
public:
	AcSDKDataReceiver();
	~AcSDKDataReceiver();
	void ReceiveUpdate(const SPageFilePhysics &physicsData, const SPageFileGraphic &graphicsData, const SPageFileStatic &staticData);
private:
	DisplayController displayController;

	//wchar_t currentTrack[15];
	DeltaSample bestLapSamples[20000];
	DeltaSample currentLapSamples[20000];



	float sessionTimeLeft = 0;



	int previousCurrentLaptime;

	float previousPos = 0;


	bool inPits = false;

	int currentLapArrayPointer;
	int bestLapArrayPointer;

	int lapsComplete;

	bool recordingLap;

	bool lapSet;
	bool dataSetRecorded;
	int bestLap;



	ResetTimer lapResetCounter;


	int recordedBest;
	//int previousSamplePosition;
	//int delta;
	float UpdateDelta(const SPageFilePhysics &physicsData, const SPageFileGraphic &graphicsData, const SPageFileStatic &staticData);
	ACData ConvertToACData(const SPageFilePhysics &physicsData, const SPageFileGraphic &graphicsData, const SPageFileStatic &staticData, float delta);
	void SendDataToDisplay(const ACData &acData);
	void ResetSamples();
	void ResetBest();

	void ClearResetCounter();
	void EnableResetCounter();
};