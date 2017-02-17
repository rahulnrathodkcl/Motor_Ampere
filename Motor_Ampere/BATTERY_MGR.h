//To detect the battery voltage and charge the battery if needed, 
//also stop the battery charging by sensing the voltage.

#ifndef BATTERY_MGR_h
#define BATTERY_MGR_h

#include "Definitions.h"
#include "SIM.h"
#include "S_EEPROM.h"
#include <Arduino.h>

class SIM;

class BATTERY_MGR
{
	SIM* sim1;
	S_EEPROM* eeprom1;

	bool chargeState;
	byte readingWaitTime;
	unsigned long readingWaitTemp;
	byte signalState;
	bool lowBattery;
	bool reportedLowBattery;
	bool initiateCharging;

	void anotherConstructor(SIM* s1,S_EEPROM* eeprom1);
	void turnBatSignalOn();
	void turnBatSignalOff();
	bool readingElligible();
	float detectBatLevel();
	bool startCharging();
	void stopCharging();
	void reportLowBattery();
	void actOnBatLevel(float batLevel);

	#ifndef disable_debug
  		#ifdef software_SIM
		    HardwareSerial* _NSerial;
		#else
		    SoftwareSerial* _NSerial;
		#endif
	#endif

public:

	#ifndef disable_debug
  		#ifdef software_SIM
			BATTERY_MGR(HardwareSerial* s,SIM* s1,S_EEPROM* eeprom1);
		#else
			BATTERY_MGR(SoftwareSerial* s,SIM* s1,S_EEPROM* eeprom1);
		#endif
	#else
		BATTERY_MGR(SIM* s1,S_EEPROM* eeprom1);
	#endif


	void gotACPower();
	void lostACPower();
	void update();
};
#endif