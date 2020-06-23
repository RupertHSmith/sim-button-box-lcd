// SimLcdController.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include "ACTelemetryProcessor.h"
#include "AcSDKDataReceiver.h"
#include "windows.h"
#include <tchar.h>
#include "DisplayDataPacket.h"
#include "hidapi.h"
#include "DisplayController.h"

int main()
{


    std::cout << "Test app ----\n";
	std::unique_ptr<AcSDKDataReceiver> acSDKDataReceiver = std::make_unique<AcSDKDataReceiver>();
	{

		//Create data receiver

		ACTelemetryProcessor telemetryProcessor;
		telemetryProcessor.InitMemoryMap();
		telemetryProcessor.SetReceiver(move(acSDKDataReceiver));



		telemetryProcessor.StartUpdates();
		int x;
		std::cin >> x;

	}



	
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
