//Version 2 Dated 29052016
#ifndef SIM_h
#define SIM_h

#include <HardwareSerial.h>
//#include <SoftwareSerial.h>
#include "Motor_MGR.h"
#include "S_EEPROM.h"
#include "Definitions.h"
#include <Arduino.h>

//#define disable_debug

class Motor_MGR;

class SIM
{
  private:
    S_EEPROM* eeprom1;
    Motor_MGR* motor1;

    // float data;
    // byte lastSettingByte;
    // bool sendAgain;

    bool responseToAction;
    String adminNumber;
    byte soundPlayNumber;
    byte soundWaitTime; //x100 = mSec
    byte soundPlayedNumber;
    unsigned long soundWait;
    bool bplaySound;
    char playFile;

    bool starPresent;
    byte DTMFCommandPresent;

    unsigned short int callCutWaitTime;  //x100 = mSec
    unsigned long callCutWait;

    bool commandsAccepted;
    byte acceptCommandsTime;
    unsigned long tempAcceptCommandTime;

    char currentStatus;
    char currentCallStatus;

    byte nr;
    char responseSetting;
    bool callAccepted;

    bool makeResponse;
    char actionType;

    bool freezeIncomingCalls;

    bool immediateEvent;
    bool sendImmediateResponse;

    char currentOperation;
    // char motorStatus;
    // bool checkMotorStatus;
    // bool checkSelfStatus;
    // char selfStatus;

    bool obtainNewEvent;
    unsigned long obtainEventTimer;

    // bool triggerASKRPM;
    // unsigned short int rpmSensorData;
    // bool rpmSensorDataReceived;

    // void (*f1)(byte);
    // void (*f2)(byte);
    // void (*f3)(bool);
    // void (*f4)(bool);
    // void (*immediateFeedback)(bool);

    void anotherConstructor();
   
    void delAllMsg();
    bool isNumeric(String &str);
    void sendReadMsg(String str);
    bool isMsgBody(String &str);
    bool isAdmin(String str);
    bool isPrimaryNumber(String str);
    void gotMsgBody(String &str);
    bool isNewMsg(String &str);

    bool isNumber(String &str);
    bool checkNumber(String);
    
    void acceptCommands();
    void rejectCommands();
    void sendCommand(char cmd,bool newline);
    void sendCommand(String cmd,bool newline);

    bool isCSQ(String);
    bool sendBlockingATCommand(String);
    String readString();
    bool matchString(String, String);
    bool stringContains(String &sstr, String mstr, int sstart, int sstop);
    bool isRinging(String);
    bool isDTMF(String &str);
    bool isCut(String);
    bool isSoundStop(String);
    char callState(String);
    String getActiveNumber();
    void makeCall();
    void endCall();
    void acceptCall();
    void sendSMS(String,bool);
    void operateDTMF(String str);
    void operateRing();
    bool playSoundElligible();
    void triggerPlaySound();
    void playSoundAgain(String);
    void playSound(char c,bool x=false,bool newAction=true);
    void stopSound();
    bool callTimerExpire();
    bool responseActionElligible();
    void makeResponseAction();
    void sendImmediateFeedback(bool);
    bool rejectCommandsElligible();
    void checkNetwork(String);//(String str);
    void networkCounterMeasures();

    // void operateOnMotorResponse();
    // void operateOnSelfResponse();
    // void operateOnRPMSensorData();

    // void sendSettingsAgain();
    void setObtainEvent();

        #ifndef disable_debug
            #ifdef software_SIM
                HardwareSerial* _NSerial;
                SoftwareSerial* _SSerial;
            #else
                SoftwareSerial* _NSerial;
                HardwareSerial* _SSerial;
            #endif
        #else
            HardwareSerial* _SSerial;
        #endif


  public:

    bool initialized;
    String bReadString;
    bool readBefore;

        #ifndef disable_debug
            #ifdef software_SIM
                SIM(HardwareSerial* serial, SoftwareSerial* serial1);
            #else
                SIM(SoftwareSerial* serial, HardwareSerial* serial1);
            #endif
        #else
            SIM(HardwareSerial* serial);
        #endif    

    void setClassReference(S_EEPROM* e1,Motor_MGR* m1);
    bool initialize();

    bool registerEvent(char eventType, bool immediate,bool getResponse);

    void operateOnMsg(String str,bool admin);

    void clearBRead();
    void bRead();
    void setCallBackFunctions(void (*ImdEvent)(bool));

    void setMotorMGRResponse(char status);
    void update();
};
#endif