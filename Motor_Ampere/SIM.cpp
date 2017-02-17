//Version 2 Dated : 29052016
#include "SIM.h"
//#include <SoftwareSerial.h>
//#include "Defintions.h"
//#include "Defintions.h"
/*
  Gets the Phone Numbers From the EEPROM Class.
  Use #use_mega to use for SELF START with Serial And Serial1
  Use #disable_debug to remove data sent to Serial port in SELF START AND SWITCH OFF.
*/

  #ifndef disable_debug

    #ifdef software_SIM
      SIM::SIM(HardwareSerial* serial,SoftwareSerial* serial1)
      {
        _NSerial=serial;
        _NSerial->begin(19200);
        _SSerial=serial1;
        _SSerial->begin(19200);
        anotherConstructor();
      }
    #else
      SIM::SIM(SoftwareSerial* serial,HardwareSerial* serial1)
      {
        _NSerial=serial;
        _NSerial->begin(19200);
        _SSerial=serial1;
        _SSerial->begin(19200);
        anotherConstructor();
      }
    #endif

  #else
  SIM::SIM(HardwareSerial* serial)
  {
    _SSerial=serial;
    _SSerial->begin(19200);
    anotherConstructor();
  }
  #endif

void SIM::clearBRead()
{
  readBefore=false;
  bReadString="";
}

void SIM::bRead()
{
  if(_SSerial->available()>0)
  {
    bReadString=_SSerial->readStringUntil('\n');
    readBefore=true;
  }
}

void SIM::anotherConstructor()
{
  //adminNumber="7041196959";
  adminNumber="7698439201";
  clearBRead();
  initialized=false;

  acceptCommandsTime=200;
  commandsAccepted=false;
 
  starPresent=false;
  DTMFCommandPresent=0;

  pinMode(PIN_DTR,OUTPUT);
  rejectCommands();

  soundPlayNumber = 10;
  soundWaitTime = 5;
  soundPlayedNumber = 0;
  bplaySound = false;

  // sendAgain=false;

  actionType = 'N';
  makeResponse = false;
  responseToAction=false;

  callCutWaitTime = 580;
  nr = 0;
  responseSetting = 'C';
  currentStatus = 'N';
  currentCallStatus = 'N';
  callAccepted = false;
  immediateEvent=false;
  sendImmediateResponse=false;
  freezeIncomingCalls=false;
  obtainNewEvent=true;
}

void SIM::delAllMsg()
{
  sendCommand("AT+CMGDA=\"DEL ALL\"",true);
}

bool SIM::isNumeric(String &str)
{
  for(byte i=0;i<str.length();i++)
  {
    if( !isDigit(str.charAt(i)))
      return false;
  }
  return true;
}

void SIM::operateOnMsg(String str,bool admin=false)
{
    if((stringContains(str,"CLEARALL",8,str.length()-1)) && admin)//if (str == "CLEARALL" && admin)
    {
      eeprom1->clearNumbers(admin);
    }
    else
    {
      int data;
      if(stringContains(str,"TEMP",4,str.length()-1))
      {
        if(isNumeric(str))
        {
            data=str.toInt();
            if(data<50) data=50;
            if(data>80) data=80;
            eeprom1->saveTempSettings(data);  //Store in EEPROM the TEMP value            
        }
      }
      else if(stringContains(str,"AUTOON",6,str.length()-1))
      {
          eeprom1->saveAutoStartSettings(true);  //set AutoStart to True in EEPROM            
      }
      else if(stringContains(str,"AUTOOFF",7,str.length()-1))
      {
          eeprom1->saveAutoStartSettings(false);  //set AUtoStart to False in EEPROM            
      }
      else if(stringContains(str,"AUTOTIME",8,str.length()-1))
      {
        if(isNumeric(str))
        {
            data=str.toInt();
            if(data<0) data=0;
            if(data>480) data=480;
            eeprom1->saveAutoStartTimeSettings(data);  //Store in EEPROM the AUTO START TIME            
        }
      }
      else if(stringContains(str,"AT+CSQ",6,str.length()-1))
      {
        sendCommand("AT+CSQ",true);
        unsigned long int temp=millis();
        String str;
        while(millis()-temp>1500)
        { 
          str=readString();
          str.trim();
          if(isCSQ(str))
          {
            sendSMS(str,true);
          }
        }
      }
      
      if(stringContains(str,"M-",2,12))
      {
        if(str.length()==10 && isNumeric(str))
        {
          bool t=eeprom1->removeNumber(str);
          #ifndef disable_debug
            _NSerial->print("Remove:");
            _NSerial->println((bool)t);
          #endif
        }
      }
      else if (stringContains(str,"M+",2,12))
      {
        if (str.length() == 10 && isNumeric(str))
        {
          bool t=eeprom1->addNumber(str);
          #ifndef disable_debug
            _NSerial->print("Add:");
            _NSerial->println((bool)t);
          #endif
        }
      }
      else if(stringContains(str,"AMON",4,str.length()-1))
      {
        if(eeprom1->alterNumberPresent)
          eeprom1->saveAlterNumberSetting(true);
      }
      else if(stringContains(str,"AMOFF",5,str.length()-1))
      {
        eeprom1->saveAlterNumberSetting(false);
      }
      else if (stringContains(str,"AM+",3,13))
      {
        if (str.length() == 10 && isNumeric(str))
        {
          bool t=eeprom1->addAlternateNumber(str);
          #ifndef disable_debug
            _NSerial->print("Alter ");
            _NSerial->print("Add:");
            _NSerial->println((bool)t);
          #endif
        }
      }
    }
}

bool SIM::isCSQ(String str)
{
  str.trim();
  return(str.startsWith("+CSQ"));
}

void SIM::sendReadMsg(String str)
{
  String s;
  s="AT+CMGR=";
  s.concat(str);
  sendCommand(s,true);
}

bool SIM::isMsgBody(String &str)
{
  //+CMGR: "REC UNREAD","+917041196959","","16/08/17,14:32:04+22"
  return stringContains(str,"+CMGR:",24,34);
}

bool SIM::isAdmin(String str)
{
  if(str==adminNumber)
    return true;
  return false;
}

bool SIM::isPrimaryNumber(String str)
{
  if(eeprom1->numbersCount>0)
  {
    if(eeprom1->primaryNumber==str)
      return true; 
    if (eeprom1->alterNumberSetting && eeprom1->alterNumber==str)
      return true;
  }
  return false;
}

void SIM::gotMsgBody(String &str)
{
  bool admin=isAdmin(str);
  if(admin || isPrimaryNumber(str))
  {
    str=readString();//_SSerial->readStringUntil('\n');
    #ifndef disable_debug
      _NSerial->print("MSGRCV:");
      _NSerial->println(str);
    #endif
    operateOnMsg(str,admin);
  }
  delAllMsg();
}

bool SIM::isNewMsg(String &str)
{
  //+CMTI: "SM",1
  return stringContains(str,"+CMTI:",12,str.length()-1);
}

void SIM::setClassReference(S_EEPROM* e1,Motor_MGR* m1)
{
  eeprom1=e1;
  motor1=m1;
}

bool SIM::initialize()
{
  byte attempts=0;
  try_again:
  if (sendBlockingATCommand("AT\r\n"))
  {
    //#ifndef disable_debug
    //_NSerial->println("AT OK");
    //#endif 
    if (sendBlockingATCommand("AT+DDET=1\r\n"))
    {
      //#ifndef disable_debug
      //_NSerial->println("DTMF OK");
      //#endif
      if (sendBlockingATCommand("AT+CLIP=1\r\n"))
      {
        //#ifndef disable_debug
        //_NSerial->println("CLIP OK");
        //#endif
        if (sendBlockingATCommand("AT+CLCC=1\r\n") && sendBlockingATCommand("AT+CMGF=1\r\n") &&  sendBlockingATCommand("AT+CNMI=2,1,0,0,0\r\n") )
        {
          #ifndef disable_debug
          //_NSerial->println("CLCC OK");
          _NSerial->println("INIT OK");
          #endif
          initialized=true;
          return true;
        }
      }
    }
  }
  if(!initialized && attempts==0)
  {
    attempts++;
    goto try_again;  
  }
  return false;
}

bool SIM::isNumber(String &str)
{
//+CLCC: 1,1,4,0,0,"+917041196959",145,""
//+CLIP: "+917041196959",145,"",,"",0
return (stringContains(str, "+CLIP: \"", 11, 21));
  //return stringContains(str, "+CLCC: 1,1,4,0,0,\"", 21, 31);
}

bool SIM::checkNumber(String number)
{
  #ifndef disable_debug
    _NSerial->print("Number:");
    _NSerial->println(number);
  #endif

  if (number==adminNumber)
    return true;
  else if(eeprom1->checkExists(number)!=0xFF)
    return true;
  
  return false;
}

void SIM::acceptCommands()
{
  if(!commandsAccepted)
  {
    #ifndef disable_debug
      _NSerial->print("com");
      _NSerial->print("Acc:");
      _NSerial->println(millis());
    #endif
    digitalWrite(PIN_DTR,LOW);
    commandsAccepted=true;
    tempAcceptCommandTime=millis();  
    while(millis()-tempAcceptCommandTime<=100)
    {}
  }
  else
  {
    tempAcceptCommandTime=millis();  
  }
}

void SIM::rejectCommands()
{
  #ifndef disable_debug
    _NSerial->print("com");
    _NSerial->print("Reject");
    _NSerial->println(millis());
  #endif
  digitalWrite(PIN_DTR,HIGH);
  commandsAccepted=false;
}
void SIM::sendCommand(char cmd,bool newline=false)
{
  acceptCommands();
    if(!newline)
    _SSerial->print(cmd);
    else
    _SSerial->println(cmd);
}

void SIM::sendCommand(String cmd,bool newline=false)
{
  acceptCommands();
  if(cmd=="")
    _SSerial->println();
  else
  {
    if(!newline)
      _SSerial->print(cmd);
    else
      _SSerial->println(cmd);
  }
}

bool SIM::sendBlockingATCommand(String cmd)
{
  //_NSerial->print(cmd); 
  sendCommand(cmd);
  //_SSerial->print(cmd);
  #ifndef disable_debug
  _NSerial->print(cmd);
  #endif

  unsigned long t = millis();
  String str;
  while (millis() - t < 3000)
  {
    if (_SSerial->available() > 0)
    {
      str = readString();
      #ifndef disable_debug
      _NSerial->println(str);
      #endif

      if (matchString(str, "OK\r") == true)
        return true;
      else if (matchString(str, "ERROR\r") == true)
        return false;
    }
  }
  return false;
}

String SIM::readString()
{
  String str = "";
  if (_SSerial->available() >  0)
  {
    acceptCommands();
    str = _SSerial->readStringUntil('\n');
    #ifndef disable_debug
    _NSerial->println(str);
    #endif
  }
  return str;
}

bool SIM::matchString(String m1, String m2)
{
  return (m1 == m2);
}

bool SIM::stringContains(String &sstr, String mstr, int sstart, int sstop)
{
  if (sstr.startsWith(mstr))
  {   
        sstr = sstr.substring(sstart, sstop);
    return true;
  }
  return false;
}

bool SIM::isRinging(String str)
{
  return (str == "RING\r");
}

bool SIM::isDTMF(String &str)
{
  return stringContains(str, "+DTMF:", 6, 7);
}

bool SIM::isCut(String str)
{
  if(currentStatus=='I' && (currentCallStatus=='I' || currentCallStatus=='O'))
  {
    #ifndef disable_debug
    _NSerial->print("STR RCV : ");
    _NSerial->print(str);
    _NSerial->println("STR END");
    #endif
  }

  if(matchString(str, "NO CARRIER\r"))
  {
    //#ifndef disable_debug
    //_NSerial->println("No Carrier Detected");
    //#endif
    return true;
  }
  else if(matchString(str, "BUSY\r"))
  {
    //#ifndef disable_debug
    //_NSerial->println("Busy Detected");
    //#endif
    return true;
  }
  else if(matchString(str, "NO ANSWER\r"))
  {
    //#ifndef disable_debug
    //_NSerial->println("No Answer Detected");
    //#endif
    return true;
  }
  else if(matchString(str, "ERROR\r"))
  {
    //#ifndef disable_debug
    //_NSerial->println("ERROR Detected");
    //#endif
    return true;
  }
  return false;

  //return (matchString(str, "NO CARRIER\r") || matchString(str, "BUSY\r") || matchString(str, "NO ANSWER\r") || matchString(str, "ERROR\r"));
}

bool SIM::isSoundStop(String str)
{
  return (matchString(str, "AMR_STOP\r"));
}

char SIM::callState(String str)
{
  //0:  call accepted
  //3:  call made
  //6:  call ended
  #ifndef disable_debug
  _NSerial->print("str:");
  _NSerial->println(str);
  #endif

  if (stringContains(str, "+CLCC: 1,0,3", 11, 12))
  {
    return 'R'; //call made
  }
  else if (stringContains(str, "+CLCC: 1,0,0", 11, 12))
  {
    return 'I'; //call accepted
  }
  else if (stringContains(str, "+CLCC: 1,0,6", 11, 12))
  {
    return 'E'; //call ended
  }
  else
  {
    return 'N';
  }
}

String SIM::getActiveNumber()
{
    if(eeprom1->numbersCount>0)
    {
      if(!eeprom1->alterNumberSetting)
        return(eeprom1->primaryNumber);
      else
        return(eeprom1->alterNumber);
    }
    else
     return(adminNumber);//="AT+CMGS=\"+917698439201\"";  
}

void SIM::makeCall()
{
  acceptCommands();
  _SSerial->flush();
 
  {
    //sendBlockingATCommand("ATD+917698439201;");
    String command;
    command="ATD+91";
    command.concat(getActiveNumber());
    command.concat(";");
    sendCommand(command,true);

    double temp=millis();
    while(millis()-temp<100)
    {    }
    sendCommand("",true);
    #ifndef disable_debug
    _NSerial->println("Call Made");
    #endif
    callCutWait = millis();
    currentStatus = 'R';
    currentCallStatus = 'O';
  }
}

void SIM::endCall()
{
  nr = 0;
  _SSerial->flush();
  sendCommand("AT+CHUP",true);
  //_SSerial->println("AT+CHUP");
  double temp=millis();
  while(millis()-temp<100)
  {  }
  sendCommand("",true);
  _SSerial->flush();
  //sendBlockingATCommand("ATH");
  freezeIncomingCalls=false;
  //gotSettingTone = false;
  soundPlayedNumber = 0;
  callAccepted = false;
  responseToAction=false;
  currentStatus = 'N';
  currentCallStatus = 'N';
  
  obtainEventTimer=millis();
  obtainNewEvent=false;
  starPresent=false;
  DTMFCommandPresent=0;
  #ifndef disable_debug
  _NSerial->println("Call End");
  #endif
  
  if(immediateEvent)
    sendImmediateFeedback(true);
}

void SIM::setObtainEvent()
{
  if(!obtainNewEvent  && millis()-obtainEventTimer>1000)
  {
      obtainNewEvent=true;
  }
}

void SIM::acceptCall()
{
    callAccepted = true;
    _SSerial->flush();
    sendCommand("ATA",true);
  _SSerial->flush();
  currentStatus = 'I';
  currentCallStatus = 'I';
  //gotSettingTone = false; 
  playSound('M',true);
}

void SIM::sendSMS(String msg="",bool predefMsg=false)
{
  _SSerial->flush();
  String responseString;

  if(!predefMsg)
  {
    switch (actionType)
    {
      case 'O':
        responseString = "M OFF";
        break;
      case 'I':
        responseString = "RPM HIGH";
        break;
      case 'D':
        responseString = "RPM LOW";
        break;
    }
  }
  else
    responseString=msg;

  //delay(350);
  #ifndef disable_debug
  _NSerial->println("SMS..");
  #endif
  //if (sendBlockingATCommand("AT+CMGF=1\r"))
  //{
    String command;
    command="AT+CMGS=\"+91";
    command.concat(getActiveNumber());
    command.concat("\"");

    _SSerial->flush();
    sendCommand(command);
    _SSerial->flush();
    sendCommand(responseString,true);
    _SSerial->flush();
    _SSerial->write(0x1A);
    _SSerial->flush();
    #ifndef disable_debug
      _NSerial->println("SMS Done");
    #endif
      //sendCommand("AT+CMGF=0\r");
      //_SSerial->print("AT+CMGF=0\r"); //Normal Mode
    // }
    //_SSerial->println("AT+CMGS=\"+917698439201\""); // recipient's mobile number, in international format
  //}
  //_SSerial->println("AT+CMGF=1\r"); // AT command to send SMS message
  //_NSerial->println(response_chk_ret_str());
  //SIM.println("AT+CMGS=\"+91" + CurEPSettings.phones[0] + "\""); // recipient's mobile number, in international format
  //_NSerial->println(response_chk_ret_str());
  //delay(500);
}

void SIM::operateDTMF(String str)
{
  if(immediateEvent)
  {
    if(str=="1")    //yes do the operation
    {
      #ifndef disable_debug
      _NSerial->flush();
      #endif
      _SSerial->flush();
      sendCommand("AT+CHUP",true);
      _SSerial->flush();
      sendImmediateFeedback(true);     //turn off motor
      endCall();
    }
    else if(str=="2")   //no dont do anything
    {
      #ifndef disable_debug
      _NSerial->flush();
      #endif
      _SSerial->flush();
      sendCommand("AT+CHUP",true);
      _SSerial->flush();
      sendImmediateFeedback(false);      //dont turn off motor
      endCall();
    }
  }
  else
  {
      if(starPresent && DTMFCommandPresent!=0)
      {
        byte temp = str.toInt();

        if(temp==9)
        {
          starPresent=false;
          DTMFCommandPresent=0;
          callCutWait=millis();
          playSound('M',true);
          return;
        }

        if(temp>5) 
          temp=5;

        if(DTMFCommandPresent==4)
        {
          currentOperation='I';
          callCutWait=millis();
          // f1(temp);
        }
        else if (DTMFCommandPresent==3)
        {
          currentOperation='D';
          callCutWait=millis();
          // f2(temp);
        }
        return;
      }

      if (str == "1") //Motor On
      {
          currentOperation='S';
          // starPresent=true;
          // DTMFCommandPresent=1;
          callCutWait=millis();
          //stopSound();
          motor1->startMotor();
          // f3(true);
      }
      else if (str == "2") //Motor Off
      {
          currentOperation='O';
          // starPresent=true;
          // DTMFCommandPresent=2;
          callCutWait=millis();
          //stopSound();
          motor1->stopMotor(true);
          // f4(true);
      }
  }
}

void SIM::operateRing()
{
  nr++;
  if (nr <= 2)
  {

    if(nr==1)
    {
      sendCommand("AT+DDET=1\r\n");
      _SSerial->flush();
    }
    
    String str;
    do
    {
      str = readString();
    }
    while (isNumber(str) == false);
    
    callCutWait=millis();

    if(str.length()>=10 && isNumeric(str))
    {
      if (nr>1 && !checkNumber(str))
      {
        endCall();
      }
    }
  }
  else if(nr==3)
  {
      callCutWait = millis();
      acceptCall();     
  }
}

bool SIM::playSoundElligible()
{
  return (bplaySound == true && ((millis() - soundWait) > (soundWaitTime * 100)));
}

void SIM::triggerPlaySound()
{
  _SSerial->flush();
  sendCommand("AT+CPAMR=\"");
  sendCommand(playFile);
  sendCommand(".amr\",0\r",true);
  _SSerial->flush();
  soundPlayedNumber++;
  bplaySound = false;
}

void SIM::playSoundAgain(String str)
{
  int noOfTimeSoundPlays=soundPlayNumber;
  
  if(immediateEvent && sendImmediateResponse)
    noOfTimeSoundPlays*=2;
  else if(starPresent)
  {
    if(!responseToAction)
    {
      if(DTMFCommandPresent>2)
        noOfTimeSoundPlays*=3;
    }
    else
        noOfTimeSoundPlays*=2;   
  }

  if (isSoundStop(str))
  {
    if (soundPlayedNumber < noOfTimeSoundPlays)
    {
      if(immediateEvent && sendImmediateResponse)
      {
          if(playFile==actionType)
            playFile='A';
          else
            playFile=actionType;
      }
      else if (starPresent)
      {
        if(!responseToAction)
        {
          if (DTMFCommandPresent>2)
          {
            if (playFile=='G' || playFile=='H')
              playFile='E';
            else if(playFile=='E')
              playFile='N';
            else if(playFile=='N')
            {
                if(DTMFCommandPresent==3) playFile='G';
                else if(DTMFCommandPresent==4) playFile='H';          
            }
          }
        }
        else
        {
          if(playFile==actionType)
            playFile='N';
          else
            playFile=actionType;
        }
      }
      playSound(playFile,false,false);
    }
    else
    {
        endCall();
    }
  }
}

void SIM::playSound(char actionType,bool init,bool newAction)
{
  _SSerial->flush();
  soundWait = millis();
  bplaySound = true;
  if(newAction)
    this->actionType=actionType;
  playFile = actionType;
    if(init)
      soundPlayedNumber=0;
}

void SIM::stopSound()
{
  _SSerial->flush();
  sendCommand("AT+CPAMR\r",true);
  _SSerial->flush();
}

bool SIM::callTimerExpire()
{
  return ((millis() - callCutWait) >= (callCutWaitTime*100));
  //return (callAccepted && ((millis() - callCutWait) >= (callCutWaitTime*100)));
}

bool SIM::responseActionElligible()
{
  return (makeResponse);  //&& (millis() - responseWait) >= (responseWaitTime * 100));
}

void SIM::makeResponseAction()
{
  makeResponse = false;
  if ((immediateEvent && sendImmediateResponse) ||  responseSetting == 'A' || responseSetting == 'C')
    makeCall();
  else// if (responseSetting == 'S')
    sendSMS();
}

bool SIM::registerEvent(char eventType, bool immediate,bool getResponse)
{
  if(!initialized)
  {
    #ifndef disable_debug
    _NSerial->print("No SIM");
    #endif
    if(immediate && getResponse)
    {
     motor1->setImmediateResponse(true);
    }
    return true;
  }

  if (!immediate)
  {
    #ifndef disable_debug
    _NSerial->print("Got Event:");
    _NSerial->println(eventType);
    #endif
    if(currentStatus=='N' && currentCallStatus=='N' && obtainNewEvent)
    {
      actionType = eventType;
      makeResponse = true;
    }
  }
  else
  {
    if(currentStatus=='N' && currentCallStatus=='N' && obtainNewEvent)
    {
      freezeIncomingCalls=true;
      acceptCommands();

      #ifndef disable_debug
      _NSerial->print("Imm event:");
      _NSerial->print(eventType);
      #endif
      
      actionType = eventType;
      sendImmediateResponse=getResponse;
      immediateEvent=true;
      makeResponseAction();
    }
    else
      return false;
  }
  return true;
}

void SIM::sendImmediateFeedback(bool temp)
{  
    if(sendImmediateResponse)
      motor1->setImmediateResponse(temp);
  
  immediateEvent=false;
  sendImmediateResponse=false;
}

bool SIM::rejectCommandsElligible()
{
  return (commandsAccepted && millis()-tempAcceptCommandTime>=(acceptCommandsTime*100));
}

void SIM::checkNetwork(String str)
{
  if(str=="+CPIN: NOT READY\r")
  {
    endCall();
    networkCounterMeasures();
  }
}

void SIM::networkCounterMeasures()
{
  _SSerial->flush();
  sendCommand("AT+CFUN=0\r\n");
  _SSerial->flush();
  unsigned long t=millis();
  while(millis()-t<20)
  {}
  sendCommand("AT+CFUN=1\r\n");
  _SSerial->flush();
  while(millis()-t<20)
  {}
}

void SIM::setMotorMGRResponse(char response)
{
  if(currentOperation=='S')  //start Motor
  {
    responseToAction=true;
    if(response=='L')
      playSound('3',true);
    else if(response=='O')
      playSound('1',true);
    else if(response=='D')
      endCall();
  }
  else if(currentOperation=='O') //switch off motor
  {
    responseToAction=true;
    if(response=='L')
      playSound('4',true);
    else if(response=='O')
      playSound('1',true);
    else if(response=='D')
      endCall();
  }
}

// void SIM::sendSettingsAgain()
// {
  // sendAgain=!spi1->sendSettings(lastSettingByte,data);
// }

// void SIM::operateOnRPMSensorData()
// {
  // rpmSensorDataReceived=false;
  // String str="RPM:";
  // String str2=String(rpmSensorData);
  // str.concat(str2);
  // sendSMS(str,true);
// }

void SIM::update()
{
  // if(triggerASKRPM)
    // triggerASKRPM=!spi1->askSensorData((byte)ASK_RPM,&rpmSensorData,&rpmSensorDataReceived);
  // 
  // if(rpmSensorDataReceived)
    // operateOnRPMSensorData();
  // 
  // if(sendAgain)
      // sendSettingsAgain();

  if(rejectCommandsElligible())
  {
    rejectCommands();
  }

  if (currentStatus == 'N')
  {
    setObtainEvent();

    if (responseActionElligible())
      makeResponseAction();
  }
  else if (currentStatus == 'I' || currentStatus == 'R')
  {
    if (callTimerExpire())
      endCall();

    // if(checkSelfStatus)
        // operateOnSelfResponse();
    // if(checkMotorStatus)
        // operateOnMotorResponse();

    if (playSoundElligible())
      triggerPlaySound();
  }

  while (_SSerial->available() > 0 || readBefore)
  {
    String str;
    if(readBefore)
    {
      str=bReadString;
      readBefore=false;
    }
    else
    {
      str=readString();
    }
    
//    if(isRinging(str))
//    {
//      #ifndef disable_debug
//      _NSerial->print("currentStatus:");
//      _NSerial->println(currentStatus);
//      _NSerial->print("currentCallStatus:");
//      _NSerial->println(currentCallStatus);
//      #endif
//    }
//    else
//    {
      if(isNewMsg(str))
      {
          sendReadMsg(str);
      }
      else if(isMsgBody(str))
      {
        gotMsgBody(str);
      }
      else
        checkNetwork(str);
//    }

    if (!freezeIncomingCalls &&  (currentStatus == 'N' || currentStatus == 'R') && (currentCallStatus == 'N' || currentCallStatus == 'I')) //Ringing Incoming Call
    {
      if (isRinging(str) == true) //  chk_ringing(str) == true)
      {
        currentStatus = 'R';
        currentCallStatus = 'I';
        operateRing(); 
      }
    }
    else if (!freezeIncomingCalls && currentStatus == 'I' && currentCallStatus == 'I') //IN CALL INCOMING CALL
    {
      if (isCut(str) == true ) //chk_cut(str) == true)
      {
        endCall();
      }
      else if (isDTMF(str) == true) //chk_DTMF(str) == true)
      {
        operateDTMF(str);
      }
      else
      {
        playSoundAgain(str);
      }
    }
    else if ((currentStatus == 'N' || currentStatus == 'R') && currentCallStatus == 'O')
    {
      if (callState(str) == 'R')
      {
        callCutWait = millis();
        currentStatus = 'R';
        currentCallStatus = 'O';
      }
      else if (isCut(str) || callState(str) == 'E') //else if (isCut(str) || stringContains(str, "+CLCC: 1,0,6", 11, 12) == true)
      {
        endCall();
        // if(immediateEvent && sendImmediateResponse)
            // sendImmediateFeedback(true);
        if (responseSetting == 'A')
            sendSMS();
      }
      else if (callState(str) == 'I') //else if (stringContains(str, "+CLCC: 1,0,0", 11, 12) == true)
      {
        #ifndef disable_debug
        _NSerial->println("Call Accepted");
        #endif
        callCutWait = millis();
        currentStatus = 'I';
        currentCallStatus = 'O';
        callAccepted = true;
        playSound(actionType,true);
      }
    }
    else if (currentStatus == 'I' && currentCallStatus == 'O') //IN CALL OUTGOING CALL
    {
      if (isCut(str) || callState(str) == 'E')
        endCall();
      else if (immediateEvent && sendImmediateResponse && isDTMF(str)) //chk_DTMF(str) == true)
      {
        operateDTMF(str);
      }
      else
      {
        playSoundAgain(str);
      }
    }
  }
  clearBRead();
}