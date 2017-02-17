#include "BATTERY_MGR.h"

#ifndef disable_debug
  #ifdef software_SIM
    BATTERY_MGR::BATTERY_MGR(HardwareSerial *serial,SIM* s1,S_EEPROM* e1)
    {
      _NSerial = serial;
      _NSerial->begin(19200);
      anotherConstructor(s1,e1);
    }
  #else
    BATTERY_MGR::BATTERY_MGR(SoftwareSerial *serial,SIM* s1,S_EEPROM* e1)
    {
      _NSerial = serial;
      _NSerial->begin(19200);
      anotherConstructor(s1,e1);
    }
  #endif
#else
  BATTERY_MGR::BATTERY_MGR(SIM* s1,S_EEPROM* e1)
  {
    anotherConstructor(s1,e1);
  }
#endif

void BATTERY_MGR::anotherConstructor(SIM* s1,S_EEPROM* e1)
{
	pinMode(PIN_BAT_SENSOR_PWR,OUTPUT);
	stopCharging();
	readingWaitTime=250;
	signalState=false;
	reportedLowBattery=false;
	lowBattery=false;	

	sim1=s1;
	eeprom1=e1;
}


void BATTERY_MGR::turnBatSignalOn()
{
	digitalWrite(PIN_BAT_SENSOR_PWR,HIGH);
	signalState=true;
}

void BATTERY_MGR::turnBatSignalOff()
{
	digitalWrite(PIN_BAT_SENSOR_PWR,LOW);
	signalState=false;
	readingWaitTemp=millis();
}

bool BATTERY_MGR::readingElligible()
{
	return(!signalState && millis()-readingWaitTemp>=(readingWaitTime*100));
}

float BATTERY_MGR::detectBatLevel()
{
	turnBatSignalOn();
	analogRead(PIN_BAT_SENSOR);
	float data=analogRead(PIN_BAT_SENSOR);
	turnBatSignalOff();
	data=data*VOLTAGERATIO;
	data=data*5.0/1024;
	return data;
}

bool BATTERY_MGR::startCharging()
{
	initiateCharging=true;
	if(eeprom1->ACPowerState())		//AC power On
	{
		digitalWrite(PIN_BAT_SENSOR_PWR,HIGH);
		chargeState=true;
	}
	else							//AC power off
		chargeState=false;

	return chargeState;
}

void BATTERY_MGR::stopCharging()
{
	digitalWrite(PIN_BAT_SENSOR_PWR,LOW);
	chargeState=false;
	initiateCharging=false;
}

void BATTERY_MGR::reportLowBattery()
{
	reportedLowBattery=sim1->registerEvent('B',true,false);
}

void BATTERY_MGR::actOnBatLevel(float batLevel)
{
	if(!chargeState)	//not charging
	{
		if(batLevel<=6.40)
		{
			if(startCharging())
				return;
			if(!lowBattery && batLevel<=6.20)
			{
				reportedLowBattery=false;
				lowBattery=true;
			}
			else if(batLevel>=6.25)
			{
				lowBattery=false;
				reportedLowBattery=false;
			}
		}
	}
	else	//charging
	{
		if(lowBattery)
		{
			lowBattery=false;
			reportedLowBattery=false;
		}

		if(batLevel>=7.10)
		{
			stopCharging();
		}
	}
}

void BATTERY_MGR::gotACPower()
{
	if(initiateCharging && !chargeState)
	{
		startCharging();
	}
}

void BATTERY_MGR::lostACPower()
{
	if(chargeState)	
	{
		stopCharging();
		initiateCharging=true;
	}
}

void BATTERY_MGR::update()
{
	if(lowBattery && !reportedLowBattery)
		reportLowBattery();

	if(readingElligible())
		actOnBatLevel(detectBatLevel());
}