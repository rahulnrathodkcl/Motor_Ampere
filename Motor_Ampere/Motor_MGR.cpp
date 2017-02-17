#include "Motor_MGR.h"

/*
simEventTemp[0] 	: _motor not started							N
simEventTemp[1] 	: _cannot turn off motor due to some problem	P
simEventTemp[2]		: _motor has turned off due to unknown reason.	U
simEventTemp[3]		: _single phasing has occured					F
simEventTemp[4]		: _AC power On									G
simEventTemp[5]		: _AC power off									L
simEventTemp[6]		: _motor has turned off due to power cut off.	C
simEventTemp[7]		: _motor has started  							S
simEventTemp[8]		: _motor has turned off							O
*/

#ifndef disable_debug
  #ifdef software_SIM
    Motor_MGR::Motor_MGR(HardwareSerial *serial,SIM* sim1,S_EEPROM* eeprom1,BATTERY_MGR* battery1)
    {
      	_NSerial = serial;
      	_NSerial->begin(19200);
		anotherConstructor(sim1,eeprom1,battery1);
    }
  #else
    Motor_MGR::Motor_MGR(SoftwareSerial *serial,SIM* sim1,S_EEPROM* eeprom1,BATTERY_MGR* battery1)
    {
		_NSerial = serial;
		_NSerial->begin(19200);
		anotherConstructor(sim1,eeprom1,battery1);
    }
  #endif
#else
  Motor_MGR::Motor_MGR(SIM* sim1,S_EEPROM* eeprom1, BATTERY_MGR* battery1,BATTERY_MGR* battery1)
  {
    anotherConstructor(sim1,eeprom1,battery1);
  }
#endif

void Motor_MGR::anotherConstructor(SIM* sim1,S_EEPROM* eeprom1, BATTERY_MGR* battery1)
{
	this->sim1=sim1;
	this->eeprom1=eeprom1;
	this->battery1=battery1;

	pinMode(PIN_START,OUTPUT);
	digitalWrite(PIN_START,LOW);
	pinMode(PIN_STOP,OUTPUT);
	digitalWrite(PIN_STOP,LOW);

	gotOffCommand=false;
	eventOccured=false;
	waitingForResponse=false;

	//startTimerTime=600;
	startTimerOn=false;

	stopTimerTime=600;
	stopTimerOn=false;

	singlePhasingTime=200;
	singlePhasingTimerOn=false;

	startSequenceTimerTime=5;
	startSequenceOn=false;

	stopSequenceTimerTime=5;
	stopSequenceOn=false;

	for(byte i=0;i<9;i++)
		simEventTemp[i]=true;
	
	getMotorState();
}

bool Motor_MGR::getMotorState()
{
	bool p1,p2,p3;
	readSensorState(p1,p2,p3);
	
	eeprom1->ACPowerState(p3);
	if(p3 && !phaseAC)
		battery1->gotACPower();
	else if(!p3 && phaseAC)
		battery1->lostACPower();

	eeprom1->motorState((p1 && p2 && p3));

	updateSensorState(p1,p2,p3);
	return eeprom1->motorState();
}

void Motor_MGR::readSensorState(bool &p1,bool &p2,bool &p3)
{
	noInterrupts();
	p1=digitalRead(PIN_PHASE1);
	p2=digitalRead(PIN_PHASE2);
	p3=digitalRead(PIN_ACPHASE);
	interrupts();
}

void Motor_MGR::updateSensorState(bool &p1,bool &p2,bool &p3)
{
	phase1=p1;
	phase2=p2;
	phaseAC=p3;
}

void Motor_MGR::operateOnEvent()
{
	eventOccured=false;
	bool tp1,tp2,tac;
	readSensorState(tp1,tp2,tac);

	if(eeprom1->motorState())		//motorOn
	{
		if(!tac && !tp1 && !tp2)	//acPower Cut Off
		{
			stopMotor();
			simEventTemp[6]=sim1->registerEvent('C',true,false);
			//report To SIM Motor Off due to POWER CUT OFF
			setACPowerState(false);
		}

		if((tac && phaseAC) &&
			(!tp1 && phase1)  && 
			(!tp2 && phase2))		//motor switched off due to any reason
		{
			simEventTemp[2]=sim1->registerEvent('U',true,false);
			//report to SIM Motor Off due to Unknown Reason
			stopMotor();//reportMotorStopped;
		}
		
		if(!tac && phaseAC || 			
			(!tp1 && phase1) || 
			(!tp2 && phase2))	////single phasing occured
		{
			tempSinglePhasingTimer=millis();
			singlePhasingTimerOn=true;
			if(!tac && phaseAC)			// if AC phase got off then
			{
				setACPowerState(false);
			}
		}

		if(tac && tp1 && tp2 && 
			((tac && !phaseAC) || 
			(tp1 && !phase1)	|| 
			(tp2 && !phase2))) 	//single Phasing had occured Previously but all phase are present now.
		{
			singlePhasingTimerOn=false;  //discardSinglePhasing();
			if(tac && !phaseAC)			// if AC power got back then
			{
				setACPowerState(true);
			}
		}
	}
	else				//motorOff
	{	
		if(tac && !phaseAC)					//Got AC Power
		{
			setACPowerState(true);
			if(eeprom1->AUTOSTART)			//startMotor if autoStart=ON
			{
				if(eeprom1->AUTOSTARTTIME==0)
					startMotor();
				else
				{
					startTimerOn=true;
					tempStartTimer=millis();
				}
			}
			else
				simEventTemp[4]= sim1->registerEvent('G',true,false);
				// ;//register TO SIM AC power ON
		}
		else if(!tac && phaseAC) 			//lostACPower();
		{
			setACPowerState(false);
			simEventTemp[5]= sim1->registerEvent('L',true,false);
			//register To SIM AC Power OFF
		}
	}
	updateSensorState(tp1,tp2,tac);
}

bool Motor_MGR::startMotorTimerOver()
{
	return (millis()-tempStartTimer>=(((unsigned long int)eeprom1->AUTOSTARTTIME*6000)));
}

bool Motor_MGR::stopMotorTimerOver()
{
	return (millis()-tempStopTimer>=(stopTimerTime*100));
}

void Motor_MGR::setACPowerState(bool ACPowerState)
{
	if(ACPowerState)
	{
		eeprom1->ACPowerState(true);
		battery1->gotACPower();
	}
	else
	{
		eeprom1->ACPowerState(false);
		battery1->lostACPower();
	}
}

bool Motor_MGR::singlePhasingTimerOver()
{
	return (singlePhasingTimerOn && millis()-tempSinglePhasingTimer>(singlePhasingTime*100));
}

void Motor_MGR::operateOnSinglePhasing()
{
	stopMotor();
	waitingForResponse=true;
	simEventTemp[3] = sim1->registerEvent('F',true,true);
	//reportSinglePhasing TO SIM..
}

void Motor_MGR::startMotor()
{
	startTimerOn=false;
	digitalWrite(PIN_START,HIGH);
	tempStartSequenceTimer=millis();
	startSequenceOn=true;
	eeprom1->motorState(true);
}
	
void Motor_MGR::stopMotor(bool commanded)
{
	digitalWrite(PIN_STOP,HIGH);
	tempStopSequenceTimer=millis();
	stopSequenceOn=true;
	eeprom1->motorState(false);
	gotOffCommand=commanded;
}

void Motor_MGR::terminateStopRelay()
{
	if(stopSequenceOn && millis() - tempStopSequenceTimer > (stopSequenceTimerTime*100))	
	{
		digitalWrite(PIN_STOP,LOW);
		stopSequenceOn=false;

		getMotorState();
		if(!phase1 && !phase2)
		{
			if(gotOffCommand)
			{
				simEventTemp[8] = sim1->registerEvent('O',true,false);
				gotOffCommand=false;
				//register TO SIM motor has turned off
			}
		}	
		else if(phase1 || phase2)
		{
			simEventTemp[1] = sim1->registerEvent('P',true,false);
			//register to SIM cannot turn off motor due to some problem
		}
	}
}

void Motor_MGR::terminateStartRelay()
{
	if(startSequenceOn &&  millis() - tempStartSequenceTimer > (startSequenceTimerTime*100))
	{
		digitalWrite(PIN_START,LOW);
		startSequenceOn=false;
		if(getMotorState())
			simEventTemp[7] = sim1->registerEvent('S',true,false);// ;//register To SIM Motor has started
		else
			simEventTemp[0] = sim1->registerEvent('N',true,false);//register To SIM motor not started
	}
}

void Motor_MGR::setImmediateResponse(bool response)
{
	if(waitingForResponse)
	{
		waitingForResponse=false;
		if(response)		//reponse=true So, stop Motor(do the operation).
			stopMotor();
	}
}

void Motor_MGR::SIMEventManager()
{
	if(!simEventTemp[0])
		simEventTemp[0] = sim1->registerEvent('N',true,false);

	if(!simEventTemp[1])
		simEventTemp[1] = sim1->registerEvent('P',true,false);

	if(!simEventTemp[2])
		simEventTemp[2]=sim1->registerEvent('U',true,false);
	
	if(!simEventTemp[3])
		simEventTemp[3] = sim1->registerEvent('F',true,true);

	if(!simEventTemp[4])
		simEventTemp[4]= sim1->registerEvent('G',true,false);

	if(!simEventTemp[5])
		simEventTemp[5]= sim1->registerEvent('L',true,false);

	if(!simEventTemp[6])
		simEventTemp[6]=sim1->registerEvent('C',true,false);

	if(!simEventTemp[7])
		simEventTemp[7] = sim1->registerEvent('S',true,false);

	if(!simEventTemp[8])
		simEventTemp[8] = sim1->registerEvent('O',true,false);
}

void Motor_MGR::update()
{
	if(!startSequenceOn && !stopSequenceOn && eventOccured)
		operateOnEvent();

	if(singlePhasingTimerOn)
		if(singlePhasingTimerOver())
			operateOnSinglePhasing();

	if(startTimerOn)
		if(startMotorTimerOver())
			startMotor();
	
	if(stopTimerOn)
		if(stopMotorTimerOver())
			stopMotor();

	if(startSequenceOn)
		terminateStartRelay();

	if(stopSequenceOn)
		terminateStopRelay();

	SIMEventManager();
}