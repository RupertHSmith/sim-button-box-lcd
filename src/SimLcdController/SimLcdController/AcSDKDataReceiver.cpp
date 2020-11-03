#include "pch.h"
#include "AcSDKDataReceiver.h"
#include <iostream>
#include <string>
#include <cmath>

AcSDKDataReceiver::AcSDKDataReceiver()
{
	displayController.InitUsbDevice();
}

AcSDKDataReceiver::~AcSDKDataReceiver()
{
	
}

void AcSDKDataReceiver::SendDataToDisplay(const ACData &acData, const ACTyreData &acTyreData)
{
	displayController.SendData(acData);
	displayController.SendData(acTyreData);
}

ACTyreData AcSDKDataReceiver::ConvertToACTyreData(const SPageFilePhysics &physicsData, const SPageFileGraphic &graphicsData, const SPageFileStatic &staticData, float delta)
{
	ACTyreData acTyreData;
	acTyreData.packetType = AC_TYRE_DATA;
	//tyre temps
	acTyreData.tyreTemperature.frontL = static_cast<uint16_t>(physicsData.tyreCoreTemperature[0]);
	acTyreData.tyreTemperature.frontR = static_cast<uint16_t>(physicsData.tyreCoreTemperature[1]);
	acTyreData.tyreTemperature.rearL = static_cast<uint16_t>(physicsData.tyreCoreTemperature[2]);
	acTyreData.tyreTemperature.rearR = static_cast<uint16_t>(physicsData.tyreCoreTemperature[3]);
	//tyre wear (this may need to be multiplied by 100)



	acTyreData.tyreWear.frontL = ConvertTyreWear(physicsData.tyreWear[0]);
	acTyreData.tyreWear.frontR = ConvertTyreWear(physicsData.tyreWear[1]);
	acTyreData.tyreWear.rearL = ConvertTyreWear(physicsData.tyreWear[2]);
	acTyreData.tyreWear.rearR = ConvertTyreWear(physicsData.tyreWear[3]);
	//tyre pressure
	acTyreData.tyrePressure.frontL = static_cast<uint16_t>(physicsData.wheelsPressure[0]);
	acTyreData.tyrePressure.frontR = static_cast<uint16_t>(physicsData.wheelsPressure[1]);
	acTyreData.tyrePressure.rearL = static_cast<uint16_t>(physicsData.wheelsPressure[2]);
	acTyreData.tyrePressure.rearR = static_cast<uint16_t>(physicsData.wheelsPressure[3]);
	//for now we'll ignore tyre compound..
	return acTyreData;

}

uint16_t AcSDKDataReceiver::ConvertTyreWear(float wear)
{
	float tyrewearF = 100 * ((wear - 94.F) / 6.f);

	if (tyrewearF <= 0)
		return 0;
	return static_cast<uint16_t>(std::round(tyrewearF));

}

ACData AcSDKDataReceiver::ConvertToACData(const SPageFilePhysics &physicsData, const SPageFileGraphic &graphicsData, const SPageFileStatic &staticData, float delta)
{
	ACData acData;
	acData.packetType = AC_DATA;
	acData.status = graphicsData.status;
	acData.gear = physicsData.gear;
	acData.rpm = physicsData.rpms;
	acData.speedKmh = physicsData.speedKmh;
	acData.drs = physicsData.drs;
	acData.tc = physicsData.tc;
	acData.abs = physicsData.abs;
	acData.pitLimiter = physicsData.pitLimiterOn;
	acData.turboBoost = physicsData.turboBoost;
	acData.maxFuel = staticData.maxFuel;
	acData.maxRpm = staticData.maxRpm;
	acData.maxTurboBoost = staticData.maxTurboBoost;
	acData.lapTimes.bestTime = graphicsData.iBestTime;
	acData.lapTimes.currentTime = graphicsData.iCurrentTime;
	acData.lapTimes.delta = static_cast<uint16_t>(delta * 100);
	acData.fuel = physicsData.fuel; //will need to change all floats 
	acData.position = graphicsData.position;
	return acData;
}

void AcSDKDataReceiver::ResetSamples()
{
	//then reset sampler
	bestLapArrayPointer = 0;
	currentLapArrayPointer = 0;

	//zero array
	for (int ct = 0; ct < 20000; ct++)
	{
		currentLapSamples[ct].lapTime = 0;
		currentLapSamples[ct].progress = 0;
	}
}

void AcSDKDataReceiver::ReceiveUpdate(const SPageFilePhysics &physicsData, const SPageFileGraphic &graphicsData, const SPageFileStatic &staticData)
{
	//CHANGE LAP RECORDING LOGIC TO WORK WITH POINT TO POINT

	float delta = 0;
	if (graphicsData.status == AC_LIVE)
	{
		//User is playing AC
		delta = UpdateDelta(physicsData, graphicsData, staticData);	
	}
	else if (graphicsData.status == AC_OFF)
	{
		ResetBest();
		ResetSamples();
		//Clear reset timer if enabled
		ClearResetCounter();
	}

	//SEND DATA TO DEVICE
	ACData acData = ConvertToACData(physicsData, graphicsData, staticData, delta);
	ACTyreData acTyreData = ConvertToACTyreData(physicsData, graphicsData, staticData, dataSetRecorded);
	//std::cout << acTyreData.tyreWear.rearL << "   AC:   " << physicsData.tyreWear[3] << std::endl;
	SendDataToDisplay(acData, acTyreData);
}

void AcSDKDataReceiver::ResetBest()
{
	recordedBest = 0;
	bestLap = 0;
	lapsComplete = 0;
	//We may be able to get rid of this..
	for (int ct = 0; ct < 20000; ct++)
	{
		bestLapSamples[ct].lapTime = 0;
		bestLapSamples[ct].progress = 0;
	}
}

void AcSDKDataReceiver::ClearResetCounter()
{
	lapResetCounter.resetCounter = 0;
	lapResetCounter.resetEnabled = false;
}

void AcSDKDataReceiver::EnableResetCounter()
{
	lapResetCounter.resetCounter = 0;
	lapResetCounter.resetEnabled = true;
}

float AcSDKDataReceiver::UpdateDelta(const SPageFilePhysics &physicsData, const SPageFileGraphic &graphicsData, const SPageFileStatic &staticData)
{
	//This field should always be decreasing (even in unlimited sessions), so if it has increased then we know session reset
	if ((graphicsData.sessionTimeLeft - sessionTimeLeft) > 40) //tolerance of 40ms 
	{
		//then session has been reset
		std::cout << "Reset best" << std::endl;
		ResetSamples();
		ResetBest();
	}
	sessionTimeLeft = graphicsData.sessionTimeLeft;
	
	
	//Change in complete laps so we have started a lap/reset session etc //
	if (graphicsData.completedLaps > lapsComplete)
	{
		//Clear the reset counter - we have handled the 'car jump' here
		ClearResetCounter();

		//Reset pointers
		bestLapArrayPointer = 0;
		currentLapArrayPointer = 0;

		//best lap is 0 when new session occurs
		if (bestLap == 0)
		{
			bestLap = graphicsData.iLastTime;
			for (int ct = 0; ct < 20000; ct++)
			{
				bestLapSamples[ct].progress = currentLapSamples[ct].progress;
				bestLapSamples[ct].lapTime = currentLapSamples[ct].lapTime;
			}
		}
		//Improved lap time
		else if (graphicsData.iLastTime < bestLap)
		{
			bestLap = graphicsData.iLastTime;

			//copy arrays
			for (int ct = 0; ct < 20000; ct++)
			{
				bestLapSamples[ct].progress = currentLapSamples[ct].progress;
				bestLapSamples[ct].lapTime = currentLapSamples[ct].lapTime;
			}
		}
		else
		{
			//otherwise zero array
			for (int ct = 0; ct < 20000; ct++)
			{
				currentLapSamples[ct].lapTime = 0;
				currentLapSamples[ct].progress = 0;
			}
		}

		/*auto bestTime = graphicsData.lastTime;
		std::wstring ws(bestTime);
		std::string str(ws.begin(), ws.end());
		std::cout << str << std::endl;*/
		lapsComplete = graphicsData.completedLaps;
	}

	//if a change of more than a quater of the lap length then we can assume the car was reset
	else if ((previousPos - graphicsData.normalizedCarPosition) > 0.1)
	{
		//we enable the reset counter, the counter gives UPDATES_BEFORE_RESETS calls to update before 
		//the lap timer will be reset and samples ignored
		if (!lapResetCounter.resetEnabled)
		{
			EnableResetCounter();
		}
	}

	if (lapResetCounter.resetEnabled)
	{
		//Laptime must be reset//
		lapResetCounter.resetCounter++;
		if (lapResetCounter.resetCounter >= UPDATES_BEFORE_RESET)
		{
			ResetSamples();
			ClearResetCounter();
		}
	}

	previousPos = graphicsData.normalizedCarPosition;
	previousCurrentLaptime = graphicsData.iCurrentTime;

	//Record data point
	if (currentLapArrayPointer < 20000) //&& recordingLap
	{
		//below ensures that no unnecessary data points are recorded and that the timer is running prior to recording any data points
		if (graphicsData.iCurrentTime > 10 && (currentLapArrayPointer == 0 || graphicsData.normalizedCarPosition - currentLapSamples[currentLapArrayPointer - 1].progress >= 0.0004))//ms
		{
			currentLapSamples[currentLapArrayPointer].progress = graphicsData.normalizedCarPosition;
			currentLapSamples[currentLapArrayPointer].lapTime = graphicsData.iCurrentTime;

			currentLapArrayPointer++;
		}

		//need to check the rest of this

		bool noValAvail = false;
		//Then attempt the comparison method
		//Increase the best lap time pointer until we find a position in front of ours then go back one
		while (bestLapSamples[bestLapArrayPointer].progress <= graphicsData.normalizedCarPosition)
		{
			bestLapArrayPointer++;
			if (bestLapArrayPointer >= 20000)
			{
				noValAvail = true;
				//reset to avoid getting stuck in a valley
				bestLapArrayPointer = 0;
				break;
			}
		}


		//Delta is the difference between this datapoint and our current laptime
		if (!noValAvail) {
			float delta;
			//currently best lap array pointer points to a position value in front of our current position 
			//Therefore we make an approximation (if data points allow)
			if (bestLapArrayPointer >= 1)
			{

				float bias = (graphicsData.normalizedCarPosition - bestLapSamples[bestLapArrayPointer - 1].progress) / (bestLapSamples[bestLapArrayPointer].progress - bestLapSamples[bestLapArrayPointer - 1].progress);
				float approxBestLapTimeF = bias * static_cast<float>(bestLapSamples[bestLapArrayPointer].lapTime - bestLapSamples[bestLapArrayPointer - 1].lapTime) + bestLapSamples[bestLapArrayPointer - 1].lapTime;

				delta = (static_cast<float>(graphicsData.iCurrentTime) - approxBestLapTimeF) / 1000;
			}
			else
			{
				delta = static_cast<float>(graphicsData.iCurrentTime - bestLapSamples[bestLapArrayPointer].lapTime) / 1000;

			}
			return delta;

		}
	}
	return 0;
}

