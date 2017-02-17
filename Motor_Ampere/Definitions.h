#ifndef DEF_h
#define DEF_h

#include <SoftwareSerial.h>
#include <HardwareSerial.h>

//#define disable_debug
//#define software_SIM
#ifdef disable_debug
	#undef software_SIM
#endif

#define PIN_BAT_SENSOR_PWR A0
#define PIN_BAT_SENSOR A1
#define PIN_BAT_CHARGE 7
#define VOLTAGERATIO 3

#define PIN_START 4
#define PIN_STOP 5


#define PIN_PHASE1 11
#define PIN_PHASE2 10
#define PIN_ACPHASE 9

#define PIN_DTR 5

//EEPROM 
#define autoStartAddress 0
#define autoStartTimeAddress 2
#define highTempAddress 6

#define numbersCountAddress 10
#define mobileNumberAddress 14

#define alterNumberPresentAddress 108
#define alterNumberSettingAddress 112
#define alterNumberAddress 116

#define EEPROM_MIN_ADDR 0
#define EEPROM_MAX_ADDR 1023

//____________________
#endif
//-----------------------------------------------
//Constants FOR SPI communicatins
	//Receive Data From Slave in (unsigned short int (2 bytes))
	// #define ASK_RPM 0x20
	// #define ASK_TEMP 0x21	
