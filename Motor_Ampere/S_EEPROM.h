#ifndef S_EEPROM_h
#define S_EEPROM_h

#include <Arduino.h>
//#include <String.h>
#include <EEPROM.h>
#include "Definitions.h"
//
//#define RPMAddress 0
//#define numbersCountAddress 4
//#define mobileNumberAddress 8
//
//#define EEPROM_MIN_ADDR 0
//#define EEPROM_MAX_ADDR 1023

class S_EEPROM
{
    void loadNumbers();
    void loadTempSettings();
    void loadAutoStartSettings();
    void loadAutoStartTimeSettings();
    void loadAlterNumber();
    void loadAlterNumberSettings();
    void clearLoadedNumbers();
    
    void updateNumberChanges();
    bool write_StringEE(int Addr, String input);
    String read_StringEE(int Addr, int length);
    bool eeprom_read_string(int addr, char* buffer, int bufSize);
    bool eeprom_write_string(int addr, const char* str);
    bool eeprom_is_addr_ok(int addr);
    bool eeprom_write_bytes(int startAddr, const byte* array, int numBytes);

  public:
    byte numbersCount;
    bool varMotorState;
    bool varACPowerState;

    String primaryNumber;
    String secondary[4];
    
    String alterNumber;
    byte alterNumberSetting;
    byte alterNumberPresent;
    unsigned short int HIGHTEMP;
    byte AUTOSTART;
    unsigned short int AUTOSTARTTIME;

    S_EEPROM();

    byte checkExists(String number);
    void saveAlterNumberSetting(bool);
    void saveTempSettings(unsigned short int temp);
    void saveAutoStartSettings(bool);
    void saveAutoStartTimeSettings(unsigned short int);
    void loadAllData();
    bool addNumber(String number);
    bool addAlternateNumber(String number);
    bool removeNumber(String number);
    void clearNumbers(bool admin);
    
    bool motorState();
    void motorState(bool);

    bool ACPowerState();
    void ACPowerState(bool);
};
#endif