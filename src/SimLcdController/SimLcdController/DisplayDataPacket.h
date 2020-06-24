#pragma once
/*
*	We define two sorts of packets, realtime packets and low frequency packets
*	Realtime packets contain data that needs to be sent at high frequency, low
*	frequency packets contain data that updates at a low frequency
*/
#include <stdbool.h>
#include <inttypes.h>
#include "AcSharedFileOut.h"

#define PACKED
#pragma pack(push,1)

typedef uint8_t GAME;
#define ASSETTO_CORSA 0
#define ASSETTO_CORSA_COMPETIZIONE 1
#define EURO_TRUCK_SIMULATOR 2
#define FORZA_HORIZON_4 3

typedef uint8_t PACKET_TYPE;
#define AC_TYRE_DATA 1
#define AC_DATA 2

typedef struct {
	uint16_t frontL;
	uint16_t frontR;
	uint16_t rearL;
	uint16_t rearR;
} TyreWear;

typedef struct {
	uint16_t frontL;
	uint16_t frontR;
	uint16_t rearL;
	uint16_t rearR;
} TyrePressure;

typedef struct {
	uint16_t frontL;
	uint16_t frontR;
	uint16_t rearL;
	uint16_t rearR;
} TyreTemp;

typedef struct {
	uint32_t currentTime;
	uint32_t lastTime;
	uint32_t bestTime;
	int16_t delta;			// since we only have 0.00 precision, send as 8 bit signed int
} LapTimes;

typedef struct {
	uint8_t pid = 0x8; //ignored
	PACKET_TYPE packetType;
	TyreTemp tyreTemperature;
	TyreWear tyreWear;
	TyrePressure tyrePressure;
	char tyreCompound[20];
} ACTyreData;


/*re order this properly*/
typedef struct {
	uint8_t pid = 0x8; //ignored
	PACKET_TYPE packetType;
	uint8_t status = AC_OFF;
	uint8_t gear;
	uint16_t rpm;
	float speedKmh;
	uint8_t drs;
	uint8_t tc;
	uint8_t abs;
	bool pitLimiter;
	float turboBoost;
	float maxFuel;
	uint16_t maxRpm;
	float maxTurboBoost;
	LapTimes lapTimes;
	float fuel;
	uint8_t position;
} ACData;

#pragma pack(pop)
#undef PACKED