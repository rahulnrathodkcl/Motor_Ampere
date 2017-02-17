#include "Definitions.h"
#include "Motor_MGR.h"
#include "SIM.h"
#include "S_EEPROM.h"
#include "BATTERY_MGR.h"

S_EEPROM eeprom1;

#ifndef disable_debug
  SoftwareSerial s1(5,6);
  #ifdef software_SIM
    SIM sim1(&Serial,&s1);
    BATTERY_MGR battery1(&Serial,&sim1,&eeprom1);
    Motor_MGR motor1(&Serial,&sim1,&eeprom1,&battery1);
    HardwareSerial* USART1=&Serial;
  #else
    SIM sim1(&s1,&Serial);
    BATTERY_MGR battery1(&s1,&sim1,&eeprom1);
    Motor_MGR motor1(&s1,&sim1,&eeprom1,&battery1);
    SoftwareSerial* USART1=&s1;
  #endif
#else
	SIM sim1(&Serial);
	BATTERY_MGR battery1(&sim1,&eeprom1);
  Motor_MGR motor1(&sim1,&eeprom1,&battery1);
#endif  

void setup() {
  // put your setup code here, to run once:
  #ifndef disable_debug
    USART1->begin(19200);
  #endif

	attachInterrupt(digitalPinToInterrupt(PIN_PHASE1),IVR_PHASE,CHANGE);
	attachInterrupt(digitalPinToInterrupt(PIN_PHASE2),IVR_PHASE,CHANGE);
	attachInterrupt(digitalPinToInterrupt(PIN_ACPHASE),IVR_PHASE,CHANGE);

	sim1.setClassReference(&eeprom1,&motor1);	
}

void IVR_PHASE()
{
	motor1.eventOccured=true;	
}

void loop() {
  // put your main code here, to run repeatedly:

	sim1.update();
	battery1.update();
	motor1.update();
	//eeprom1.update();
}