
// Manages various MIDI input/outputs :
// digital switch on/off => midi-out Note-On Off
// analog input 0..+Vcc => midi-out CTRL
// optical button => midi-out Note-On Off with velocity
// optical string => midi-out Note-On Off with velocity
// S2 dream-expander <= midi-in expanders ( up to 12 audio mono outputs )

#include <EEPROM.h>

//////////////////////////
// selection of the design
// Select one of the configs above
#define KEYBOARD_CONFIG
// #define GUITAR_CONFIG
// end of configs
//////////////////////////

//////////////////////////
// Keyboard design
//////////////////////////
#ifdef KEYBOARD_CONFIG
#define S2DREAMBLASTER1 // S2-DreamBlaster-midiexpander 1 available
#define CHANNELOUT 1
#define MINVELOCITY 5 // velocity minimum
#define VELOMAX 3 // number fof optical velocity buttons
//#define BUTTONMAX 2 // number of mechanical buttons
#define ANALOGMAX 2 // number of analog inputs
//#define STRINGMAX 0 // number of optical strings

// pins of the keyboard sensors
#ifdef VELOMAX
const int veloPin[VELOMAX] = {17,16,1};
const int veloValue[VELOMAX] = {A6,A7,A8};
#endif
#ifdef BUTTONMAX
const int buttonPin[BUTTONMAX] = {};
#endif
#ifdef ANALOGMAX
const int analogPin[ANALOGMAX] = {A9,A11};
const bool analogPullup[ANALOGMAX] = { true, true } ;
#endif
#ifdef STRINGMAX
const int stringPin[STRINGMAX] = {0} ;
#endif
//#define midiReset1 6 // pins to reset the S2-DreamBlaster-midi-expanders
#endif // KEYBOARD_CONFIG
//////////////////////////

//////////////////////////
// Guitar design
//////////////////////////
#ifdef GUITAR_CONFIG
#define CHANNELOUT 2
#define S2DREAMBLASTER1 //  S2-DreamBlaster-midiexpander 1 available
#define MINVELOCITY 5 // velocity minimum
#define BUTTONMAX 2 // number of mechanical buttons
#define ANALOGMAX 2 // number of analog inputs
#define STRINGMAX 5 // number of optical strings

// pins of the guitar sensors
#ifdef VELOMAX
const int veloPin[VELOMAX] = {0} ;
const int veloValue[VELOMAX] = {0} ;
#endif
#ifdef BUTTONMAX
const int buttonPin[BUTTONMAX] = {9,10};
#endif
#ifdef ANALOGMAX
const int analogPin[ANALOGMAX] = {A0,A1};
const bool analogPullup[ANALOGMAX] = { true, false, false } ;
#endif
#ifdef STRINGMAX
const int stringPin[STRINGMAX] = {A2,A3,A4,A5;A6};
#endif
#define midiReset1 6 // pins to reset the S2-DreamBlaster-midi-expanders
#endif // GUITAR_CONFIG
//////////////////////////

// MIDI gloval values
byte channelToAudio[16] ; // bitmap to link a midi channel to a S2-midi-expander
// Midi message received on USB/MIDI
byte midiType ;
byte midiChannel  ;
int midiData1 ;
int midiData2  ;
// Midi message to forward on S2-DreamBlaster-midi-expander
byte midiBuf[3];
byte midiLen ;
byte *midiSysexBuf;

// Led to "see" midi flow
#define ledPin 11
elapsedMillis sinceLed;
bool ledStatus ;

// EEPROM mapping
// size of records
#define VELOEEPROMSIZE 4 
#define ANALOGEEPROMSIZE 4 
#define STRINGEEPROMSIZE 4 
// offset of records
#define VELOEEPROMOFFSET 3 
#define ANALOGEEPROMOFFSET ( VELOEEPROMOFFSET + VELOEEPROMSIZE * VELOMAX ) 
#define STRINGEEPROMOFFSET ( ANALOGEEPROMOFFSET + ANALOGEEPROMSIZE * ANALOGMAX )

// velocity touch status
#ifdef VELOMAX
int veloMin[VELOMAX] , veloMax[VELOMAX];
int veloStatus[VELOMAX];
elapsedMillis veloSince[VELOMAX];
#endif

// button status
#ifdef BUTTONMAX
int buttonStatus[BUTTONMAX];
elapsedMillis buttonSince[BUTTONMAX];
#endif

// analog status
#ifdef ANALOGMAX
int analogStatus[ANALOGMAX];
int analogMin[ANALOGMAX] , analogMax[ANALOGMAX];
elapsedMillis analogSince;
#endif

//string status
#ifdef STRINGMAX
int stringStatus[STRINGMAX];
int stringMin[STRINGMAX] , stringMax[STRINGMAX];
int stringTrigger[STRINGMAX] ;
elapsedMillis stringSince[STRINGMAX];
#endif

bool calibrateStatus ;
elapsedMillis calibrateSince;

// offsets of MIDI values
#define VELOPITCHOFFSET 1
#define BUTTONPITCHOFFSET 20
#define STRINGPITCHOFFSET 40
#define ANALOGCTRLHOFFSET 64

//////////////////////////
// led 
//////////////////////////
void ledOn()
{
    digitalWrite(ledPin,HIGH);
    sinceLed = 0;
    ledStatus = true ;
  
}
void ledProcess()
{
  // switch off the led when time-out
  if (ledStatus && ( sinceLed > 500 ))
  {
    digitalWrite(ledPin,LOW);
    ledStatus = false ;
  }
}
void ledSetup()
{
  ledStatus = false ;
  pinMode(ledPin,OUTPUT);
  ledOn();
}

//////////////////////////////////
// S2 dreamblaster Midi Expanders
/////////////////////////////////
void midiSetAudio(byte midiChannel, int audioOutput )
{
  // set the midiChannel on one of the audioOutput 1..12 (using the 6 expanders, and their two audio channel R/L ) 
  if ( audioOutput == 0 )
  {
    channelToAudio[midiChannel] = 0 ; // channel not played
    return ;
  }
  // channel played on this midi mono output 1..12
  int midiOutNr = (audioOutput - 1) >> 1;
  channelToAudio[midiChannel] |= ( 0x1 << midiOutNr ) ; 
  // set pan on right or left on the midiOutNr
  midiBuf[0] = ( 3 << 4 ) | midiChannel ; // Control change 
  midiBuf[1] = 10 ; // pan
  if ( (audioOutput & 0x1) == 1)
    midiBuf[2] = 0 ; // right
  else
    midiBuf[2] = 127 ; // left    
  switch(midiOutNr + 1)
  {
#ifdef S2DREAMBLASTERMAX1
    case 1 : Serial1.write(midiBuf, 3); break ;
#endif
#ifdef S2DREAMBLASTERMAX2
    case 2 : Serial2.write(midiBuf, 3); break ;
#endif
#ifdef S2DREAMBLASTERMAX3
    case 3 : Serial3.write(midiBuf, 3); break ;
#endif
#ifdef S2DREAMBLASTERMAX4
    case 4 : Serial4.write(midiBuf, 3); break ;
#endif
#ifdef S2DREAMBLASTERMAX5
    case 5 : Serial5.write(midiBuf, 3); break ;
#endif
#ifdef S2DREAMBLASTERMAX6
    case 6 : Serial6.write(midiBuf, 3); break ;
#endif
    default : break ;
  }
}
void s2Process()
{
  // forward MIDI-in/USB to MIDI-in/S2
  if ( usbMIDI.read() )
  {
    midiType = usbMIDI.getType() ;
    midiChannel = usbMIDI.getChannel() ;
    midiData1 = usbMIDI.getData1() ;
    midiData2 = usbMIDI.getData2() ;
    midiLen = 3 ;
    byte s2bitmap = channelToAudio[midiChannel] ;
    switch(midiType)
    {
      case 7 : // sysex to dispatch to  S2-Soundblaster-midi-expander
        midiSysexBuf = usbMIDI.getSysExArray() ;
#ifdef S2DREAMBLASTER1
        Serial1.write(midiSysexBuf, midiData1);
#endif
#ifdef S2DREAMBLASTER2
        Serial2.write(midiSysexBuf, midiData1);
#endif
#ifdef S2DREAMBLASTER3
        Serial3.write(midiSysexBuf, midiData1);
#endif
#ifdef S2DREAMBLASTER4
        Serial4.write(midiSysexBuf, midiData1);
#endif
#ifdef S2DREAMBLASTER5
        Serial5.write(midiSysexBuf, midiData1);
#endif
#ifdef S2DREAMBLASTER6
        Serial6.write(midiSysexBuf, midiData1);
#endif
        break ;
      case 3 : // CTRL
        // CTRL-42 is used to connect a channel on an audio-output : 0 (not played), 1..10
        if (midiData1 == 42 )
        {
          midiSetAudio(midiChannel,midiData2);
          break ;
        }
      case 4 : // PROG is only two bytes
        midiLen = 2;
      default :
        // all Midi messages are sent on the S2-midi-expander
        midiBuf[0] = ( midiType << 4 ) | midiChannel ;
        midiBuf[1] = midiData1 ;
        midiBuf[2] = midiData2 ;
        s2bitmap = channelToAudio[midiChannel] ;
#ifdef S2DREAMBLASTER1
        if ( s2bitmap & 0x1 ) Serial1.write(midiBuf, midiLen);
#endif
#ifdef S2DREAMBLASTER2
        if ( s2bitmap & (0x1 << 1) ) Serial2.write(midiBuf, midiLen);
#endif
#ifdef S2DREAMBLASTER3
        if ( s2bitmap & (0x1 << 2) ) Serial3.write(midiBuf, midiLen);
#endif
#ifdef S2DREAMBLASTER4
        if ( s2bitmap & (0x1 << 3) ) Serial4.write(midiBuf, midiLen);
#endif
#ifdef S2DREAMBLASTER5
        if ( s2bitmap & (0x1 << 4) ) Serial5.write(midiBuf, midiLen);
#endif
#ifdef S2DREAMBLASTER6
        if ( s2bitmap & (0x1 << 5) ) Serial6.write(midiBuf, midiLen);
#endif
        // Serial.write(sinceLed); // track delay
        break ;
    }
    ledOn() ;
  }
}
void s2Setup()
{
  for(int i = 0 ; i < 16 ; i ++ )
     channelToAudio[i] = 1 ; // all MID-IN channel are played on S2 audio-output 1
#ifdef S2DREAMBLASTER1
  Serial1.begin(31250);
#ifdef midiReset1
  pinMode(midiReset1,OUTPUT);
  digitalWrite(midiReset1,LOW);
#endif
#endif
#ifdef S2DREAMBLASTER2
  Serial2.begin(31250);
#ifdef midiReset2
  pinMode(midiReset2,OUTPUT);
  digitalWrite(midiReset2,LOW);
#endif
#endif
#ifdef S2DREAMBLASTER3
  Serial3.begin(31250);
#ifdef midiReset3
  pinMode(midiReset3,OUTPUT);
  digitalWrite(midiReset3,LOW);
#endif
#endif
#ifdef S2DREAMBLASTER4
  Serial4.begin(31250);
#ifdef midiReset4
  pinMode(midiReset4,OUTPUT);
  digitalWrite(midiReset4,LOW);
#endif
#endif
#ifdef S2DREAMBLASTER5
  Serial5.begin(31250);
#ifdef midiReset5
  pinMode(midiReset5,OUTPUT);
  digitalWrite(midiReset5,LOW);
#endif
#endif
#ifdef S2DREAMBLASTER6
  Serial6.begin(31250);
#ifdef midiReset6
  pinMode(midiReset6,OUTPUT);
  digitalWrite(midiReset6,LOW);
#endif
#endif
  delay(500);
#ifdef S2DREAMBLASTER1
#ifdef midiReset1
  digitalWrite(midiReset1,HIGH);
#endif
#endif
#ifdef S2DREAMBLASTER2
#ifdef midiReset2
  digitalWrite(midiReset2,HIGH);
#endif
#endif
#ifdef S2DREAMBLASTER3
#ifdef midiReset3
  digitalWrite(midiReset3,HIGH);
#endif
#endif
#ifdef S2DREAMBLASTER4
#ifdef midiReset4
  digitalWrite(midiReset4,HIGH);
#endif
#endif
#ifdef S2DREAMBLASTER5
#ifdef midiReset5
  digitalWrite(midiReset5,HIGH);
#endif
#endif
#ifdef S2DREAMBLASTER6
#ifdef midiReset6
  digitalWrite(midiReset6,HIGH);
#endif
#endif
}

//////////////////////////
// velocity optical
//////////////////////////
void veloProcess()
{
#ifdef VELOMAX
  for(int i = 0 ; i < VELOMAX ; i ++ )
  {
    switch(veloStatus[i])
    {
      case 0 :
        if ( digitalRead(buttonPin[i]) == LOW )
        {
          int vIn = analogRead(veloValue[i]);
          int vOut = MINVELOCITY + (( vIn - veloMin[i] ) * (127-MINVELOCITY) )/(veloMax[i] - veloMin[i]);
          if ( vOut < MINVELOCITY ) vOut = MINVELOCITY;
          if ( vOut > 127 ) vOut = 127 ;
          usbMIDI.sendNoteOn(VELOPITCHOFFSET+i,vOut,CHANNELOUT);
          ledOn();
          veloStatus[i] = 1 ;
          veloSince[i] = 0 ;
        }
      break ;
      case 1 :
        if ( veloSince[i] > 50 )
        {
          veloStatus[i] = 2 ;
        }
      break ;
      case 2 :
        if ( digitalRead(buttonPin[i]) == HIGH )
        {
          usbMIDI.sendNoteOff(VELOPITCHOFFSET+i,0,CHANNELOUT);
          veloStatus[i] = 3 ;
          veloSince[i] = 0 ;
        }
      break ;
      case 3 :
        if ( veloSince[i] > 50 )
        {
          veloStatus[i] = 0 ;
        }
      break ;
      default:
        veloStatus[i] = 0 ;
      break;
    }
  }
#endif
}
void veloCalibrate()
{
#ifdef VELOMAX
    for(int i = 0 ; i < VELOMAX; i ++ )
    {
      int vIn = analogRead(veloValue[i]);
      if ( vIn < veloMin[i] )
      {
        veloMin[i] = vIn;
      }
      if ( vIn > veloMax[i] )
      {
        veloMax[i] = vIn + 1;
      }
    }
#endif
}
void veloWriteEeprom()
{
#ifdef VELOMAX
  for(int i = 0 ; i < VELOMAX; i ++ )
  {
    EEPROM.write(VELOEEPROMOFFSET+VELOEEPROMSIZE*i,(veloMin[i])&(0x0FF));
    EEPROM.write(VELOEEPROMOFFSET+VELOEEPROMSIZE*i+1,(veloMin[i])&(0x0FF00));
    EEPROM.write(VELOEEPROMOFFSET+VELOEEPROMSIZE*i+2,(veloMax[i])&(0x0FF));
    EEPROM.write(VELOEEPROMOFFSET+VELOEEPROMSIZE*i+3,(veloMax[i])&(0x0FF00));
  }
#endif
}
void veloReadEeprom()
{
#ifdef VELOMAX
  for(int i = 0 ; i < VELOMAX; i ++ )
  {
    int b0 = EEPROM.read(VELOEEPROMOFFSET+VELOEEPROMSIZE*i);
    int b1 = EEPROM.read(VELOEEPROMOFFSET+VELOEEPROMSIZE*i+1);
    int b2 = EEPROM.read(VELOEEPROMOFFSET+VELOEEPROMSIZE*i+2);
    int b3 = EEPROM.read(VELOEEPROMOFFSET+VELOEEPROMSIZE*i+3);
    veloMin[i] = b0 + ( b1 << 8 ) ;
    veloMax[i] = b2 + ( b3 << 8 ) ;
  }
#endif
}
void veloSetup()
{
#ifdef VELOMAX
  for(int i = 0 ; i < VELOMAX; i ++ )
  {
    pinMode(veloPin[i], INPUT);
    veloStatus[i] = 0 ;
    veloMin[i] = 32000;
    veloMax[i] = 0;
  } 
#endif
}

//////////////////////////
// string optical
//////////////////////////
void stringProcess()
{
#ifdef STRINGMAX
  for(int i = 0 ; i < STRINGMAX ; i ++ )
  {
    switch(stringStatus[i])
    {
      case 0 :
        if ( analogRead(stringPin[i]) > stringTrigger [i] )
        {
          stringStatus[i] = 1 ;
          stringSince[i] = 0 ;
        }
      break ;
      case 1 :
        if ( stringSince[i] > 50 )
        {
          int vIn = analogRead(stringPin[i]);
          int vOut = MINVELOCITY + (( vIn - stringMin[i] ) * (127-MINVELOCITY) )/(stringMax[i] - stringMin[i]);
          if ( vOut < MINVELOCITY ) vOut = MINVELOCITY;
          if ( vOut > 127 ) vOut = 127 ;
          usbMIDI.sendNoteOff(STRINGPITCHOFFSET+i,0,CHANNELOUT);
          usbMIDI.sendNoteOn(STRINGPITCHOFFSET+i,vOut,CHANNELOUT);
          ledOn();
          stringStatus[i] = 2 ;
        }
      break ;
      case 2 :
        if ( analogRead(stringPin[i]) <  stringTrigger[i] )
        {
          stringStatus[i] = 3 ;
          stringSince[i] = 0 ;
         }
      break ;
      case 3 :
        if ( stringSince[i] > 50 )
        {
          stringStatus[i] = 0 ;
        }
      break ;
      default:
        veloStatus[i] = 0 ;
      break;
    }
  }
#endif
}
void stringCalibrate()
{
#ifdef STRINGMAX
    for(int i = 0 ; i < STRINGMAX; i ++ )
    {
      int vIn = analogRead(stringPin[i]);
      if ( vIn < stringMin[i] )
      {
        stringMin[i] = vIn;
      }
      if ( vIn > stringMax[i] )
      {
        stringMax[i] = vIn + 1;
      }
    }
#endif
}
void stringWriteEeprom()
{
#ifdef STRINGMAX
  for(int i = 0 ; i < STRINGMAX; i ++ )
  {
    EEPROM.write(STRINGEEPROMOFFSET+STRINGEEPROMSIZE*i,(stringMin[i])&(0xFF));
    EEPROM.write(STRINGEEPROMOFFSET+STRINGEEPROMSIZE*i+1,(stringMin[i])&(0x0FF00));
    EEPROM.write(STRINGEEPROMOFFSET+STRINGEEPROMSIZE*i+2,(stringMax[i])&(0x0FF));
    EEPROM.write(STRINGEEPROMOFFSET+STRINGEEPROMSIZE*i+3,(stringMax[i])&(0x0FF00));
    stringTrigger[i] =  stringMin[i] + ( stringMax[i] - stringMin[i] ) / 10 ;
  }
#endif
}
void stringReadEeprom()
{
#ifdef STRINGMAX
  for(int i = 0 ; i < STRINGMAX; i ++ )
  {
    int b0 = EEPROM.read(STRINGEEPROMOFFSET+STRINGEEPROMSIZE*i);
    int b1 = EEPROM.read(STRINGEEPROMOFFSET+STRINGEEPROMSIZE*i+1);
    int b2 = EEPROM.read(STRINGEEPROMOFFSET+STRINGEEPROMSIZE*i+2);
    int b3 = EEPROM.read(STRINGEEPROMOFFSET+STRINGEEPROMSIZE*i+3);
    stringMin[i] = b0 + ( b1 << 8 ) ;
    stringMax[i] = b2 + ( b3 << 8 ) ;
    stringTrigger[i] =  stringMin[i] + ( stringMax[i] - stringMin[i] ) / 10 ;
  }
#endif
}
void stringSetup()
{
#ifdef STRINGMAX
  for(int i = 0 ; i < STRINGMAX; i ++ )
  {
    pinMode(stringPin[i], INPUT);
    stringStatus[i] = 0 ;
    stringMin[i] = 32000;
    stringMax[i] = 0;
  }
#endif
}

//////////////////////////
// button mechanical
//////////////////////////
void buttonProcess()
{
#ifdef BUTTONMAX
  for(int i = 0 ; i < BUTTONMAX ; i ++ )
  {
    switch(buttonStatus[i])
    {
      case 0 :
        if ( digitalRead(buttonPin[i]) == LOW )
        {
          usbMIDI.sendNoteOn(BUTTONPITCHOFFSET+i,127,CHANNELOUT);
          buttonStatus[i] = 1 ;
          buttonSince[i] = 0 ;
        }
      break ;
      case 1 :
        if ( buttonSince[i] > 50 )
        {
          buttonStatus[i] = 2 ;
        }
      break ;
      case 2 :
        if ( digitalRead(buttonPin[i]) == HIGH )
        {
          usbMIDI.sendNoteOff(BUTTONPITCHOFFSET+i,0,CHANNELOUT);
          buttonStatus[i] = 3 ;
          buttonSince[i] = 0 ;
        }
      break ;
      case 3 :
        if ( buttonSince[i] > 50 )
        {
          buttonStatus[i] = 0 ;
        }
      break ;
      default:
        buttonStatus[i] = 0 ;
      break;
    }
  }
#endif
}
void buttonSetup()
{
#ifdef BUTTONMAX
  for(int i = 0 ; i < BUTTONMAX; i ++ )
  {
    pinMode(buttonPin[i], INPUT_PULLUP);
    buttonStatus[i] = 0 ;
  }
#endif
}

//////////////////////////
// Analog input
//////////////////////////
void analogProcess()
{
#ifdef ANALOGMAX
  if ( analogSince > 100 )
  {
    analogSince = 0 ;
    for(int i = 0 ; i < ANALOGMAX; i ++ )
    {
      int vIn = analogRead(analogPin[i]);
      int vOut = (( vIn - analogMin[i] ) * 127 )/(analogMax[i] - analogMin[i]);
      if ( vOut < 0 ) vOut = 0 ;
      if ( vOut > 127 ) vOut = 127 ;
      if ( vOut != analogStatus[i] )
      {
        analogStatus[i] = vOut ;
        usbMIDI.sendControlChange(ANALOGCTRLHOFFSET+i, vOut, CHANNELOUT);
      }
    }
  }
#endif
}
void analogCalibrate()
{
#ifdef ANALOGMAX
    for(int i = 0 ; i < ANALOGMAX; i ++ )
    {
      int vIn = analogRead(analogPin[i]);
      if ( vIn < analogMin[i] )
      {
        analogMin[i] = vIn;
      }
      if ( vIn > analogMax[i] )
      {
        analogMax[i] = vIn + 1;
      }
    }
#endif
}
void analogWriteEeprom()
{
#ifdef ANALOGMAX
  for(int i = 0 ; i < ANALOGMAX; i ++ )
  {
    EEPROM.write(ANALOGEEPROMOFFSET+ANALOGEEPROMSIZE*i,(analogMin[i])&(0x0FF));
    EEPROM.write(ANALOGEEPROMOFFSET+ANALOGEEPROMSIZE*i+1,(analogMin[i])&(0x0FF00));
    EEPROM.write(ANALOGEEPROMOFFSET+ANALOGEEPROMSIZE*i+2,(analogMax[i])&(0x0FF));
    EEPROM.write(ANALOGEEPROMOFFSET+ANALOGEEPROMSIZE*i+3,(analogMax[i])&(0x0FF00));
  }
#endif
}
void analogReadEeprom()
{
#ifdef ANALOGMAX
  for(int i = 0 ; i < ANALOGMAX; i ++ )
  {
    int b0 = EEPROM.read(ANALOGEEPROMOFFSET+ANALOGEEPROMSIZE*i);
    int b1 = EEPROM.read(ANALOGEEPROMOFFSET+ANALOGEEPROMSIZE*i+1);
    int b2 = EEPROM.read(ANALOGEEPROMOFFSET+ANALOGEEPROMSIZE*i+2);
    int b3 = EEPROM.read(ANALOGEEPROMOFFSET+ANALOGEEPROMSIZE*i+3);
    analogMin[i] = b0 + ( b1 << 8 ) ;
    analogMax[i] = b2 + ( b3 << 8 ) ;
  }
#endif
}
void analogSetup()
{
#ifdef ANALOGMAX
  for(int i = 0 ; i < ANALOGMAX; i ++ )
  {
    if ( analogPullup[i] )
      pinMode(analogPin[i], INPUT_PULLUP);
    else
      pinMode(analogPin[i], INPUT);
    analogStatus[i] = 0 ;
    analogMin[i] = 32000;
    analogMax[i] = 0;
  }
#endif
}


//////////////////////////
// calibration
//////////////////////////
void calibrateProcess()
{
  if ( ! calibrateStatus )
    return;
  if ( calibrateSince > 5000 )
  {
    calibrateStatus = false ;
    // save calibration
    EEPROM.write(0,0);
    EEPROM.write(1,1);
    EEPROM.write(2,2);
#ifdef VELOMAX
    veloWriteEeprom();
#endif
#ifdef ANALOGMAX
    analogWriteEeprom();
#endif
#ifdef STRINGMAX
    stringWriteEeprom();
#endif
    return ;
  }
#ifdef VELOMAX
  veloCalibrate() ;
#endif
#ifdef ANALOGMAX
  analogCalibrate() ;
#endif
#ifdef STRINGMAX
  stringCalibrate() ;
#endif
}
void calibrateSetup()
{
  // start calibration on first use ( "magic number" 012 not available at the top of Eeprom ), or first button pressed
  ledOn();
  calibrateSince = 0 ;
  calibrateStatus= false ;
  if (( EEPROM.read(0) == 0) && ( EEPROM.read(1) == 1) && ( EEPROM.read(2) == 2))
  {
    if ( false 
#ifdef BUTTONMAX
        || ( digitalRead(buttonPin[0]) == LOW )
#endif
#ifdef VELOMAX
        ||  ( digitalRead(veloPin[0]) == LOW )
#endif
      )
    {
      calibrateStatus = true ;
    }
    else
    {
#ifdef VELOMAX
      veloReadEeprom();
#endif
#ifdef ANALOGMAX
     analogReadEeprom();
#endif
#ifdef STRINGMAX
     stringReadEeprom();
#endif
    }
  }
  else
    calibrateStatus = true ;
}

//////////////////////////
// setup
//////////////////////////
void setup() 
{
  ledSetup() ;
  s2Setup() ;
#ifdef VELOMAX
  veloSetup();
#endif
#ifdef BUTTONMAX
  buttonSetup();
#endif
#ifdef ANALOGMAX
  analogSetup();
#endif
#ifdef STRINGMAX
  stringSetup();
#endif
  calibrateSetup();
}

//////////////////////////
// loop
//////////////////////////
void loop() 
{
  if ( calibrateStatus )
  {
    calibrateProcess();
    return;
  }
  s2Process() ;
#ifdef VELOMAX
  veloProcess();
#endif
#ifdef BUTTONMAX
  buttonProcess();
#endif
#ifdef ANALOGMAX
  analogProcess();
#endif
#ifdef STRINGMAX
  stringProcess();
#endif
  ledProcess() ;
}
