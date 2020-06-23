#include "DisplayController.h"

DisplayController::DisplayController()
{
	
}

int DisplayController::InitUsbDevice()
{
	int res;
	wchar_t wstr[MAX_STR];

	res = hid_init();
	hidDevice = hid_open(VID, PID, NULL);

	//get manufacturer 
	res = hid_get_manufacturer_string(hidDevice, wstr, MAX_STR);
	wprintf(L"Manufacturer String: %s\n", wstr);

	// Read the Product String
	res = hid_get_product_string(hidDevice, wstr, MAX_STR);
	wprintf(L"Product String: %s\n", wstr);
	return 0;
}

int DisplayController::SendData(const ACData &acData)
{
	int res;

	unsigned char* packetP = (unsigned char*) &acData;

	res = hid_write(hidDevice, packetP, 65);

	return res;
}

int DisplayController::SendData(const ACTyreData &tyreData)
{
	return 0;
}

DisplayController::~DisplayController()
{
	int res = hid_exit();
}