#pragma once
#include "hidapi.h"
#include "DisplayDataPacket.h"
#include <stdio.h>

#define VID 0x03EB
#define PID 0x2043
#define MAX_STR 255

class DisplayController {
public :
	DisplayController();
	int InitUsbDevice();
	int SendData(const ACData &acData);
	int SendData(const ACTyreData &tyreData);
	~DisplayController();
private:
	hid_device *hidDevice;
};