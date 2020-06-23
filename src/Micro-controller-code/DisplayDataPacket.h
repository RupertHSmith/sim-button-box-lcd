#pragma once
/*
*	We define two sorts of packets, realtime packets and low frequency packets
*	Realtime packets contain data that needs to be sent at high frequency, low
*	frequency packets contain data that updates at a low frequency
*/
#include <stdbool.h>

#include <avr/io.h>

typedef uint8_t GAME;
#define ASSETTO_CORSA 0
#define ASSETTO_CORSA_COMPETIZIONE 1
#define EURO_TRUCK_SIMULATOR 2
#define FORZA_HORIZON_4 3

typedef uint8_t PACKET_TYPE;
#define AC_TYRE_DATA 1
#define AC_DATA 2

typedef uint8_t AC_STATUS;

#define AC_OFF 0
#define AC_REPLAY 1
#define AC_LIVE 2
#define AC_PAUSE 3

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
} TyreTemp;

typedef struct {
	uint16_t frontL;
	uint16_t frontR;
	uint16_t rearL;
	uint16_t rearR;
} TyrePressure;

typedef struct  {
	uint32_t currentTime;
	uint32_t lastTime;
	uint32_t bestTime;
	int16_t delta;
} LapTimes;

typedef struct {
	PACKET_TYPE packetType;
	TyreTemp tyreTemperature;
	TyreWear tyreWear;
	TyrePressure TyrePressure;//new addition
	char tyreCompound[20];
} ACTyreData;

typedef struct {
	PACKET_TYPE packetType;
	AC_STATUS status;
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
