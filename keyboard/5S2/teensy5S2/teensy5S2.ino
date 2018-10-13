// A teensy3.6 receives USB/MIDI
// MIDI messages are forwaded on five S2-DreamBlaster-Midi-expanders, through serial of the teensy
// each S2-DreamBlaster-Midi-expander delivers two audio-outputs
// CTRL-42 is used to set the midi-dispatch from a channel to zero or more of the 10 audio outputs
// CTRL-42-0 : no audio output
// CTRL-42-1 : add a link on audio-output#1 ( == pan-Right / S2#1 )
// CTRL-42-2 : add a link on audio-output#2 ( == pan-Left  / S2#1 )
// CTRL-42-3 : add a link on audio-output#3 ( == pan-Right / S2#2 )
// CTRL-42-4 : add a link on audio-output#4 ( == pan-Left  / S2#2 )
// etc...
//
// name.c is used to change the name of the USB-MIDI device
//
// A 6th S2-DreamBlaster-Midi-expander can be added using TX6 of the Teensy 3.6

byte channelToAudio[16] ; // bitmap to link a midi channel to a S2-midi-expander
// Midi message received
byte midiType ;
byte midiChannel  ;
int midiData1 ;
int midiData2  ;
// Midi message to forward
byte midiBuf[3];
byte midiLen ;
byte *midiSysexBuf;

// pins to reset the S2-DreamBlaster-midi-expanders
#define midiReset1 23
#define midiReset2 19
#define midiReset3 16
#define midiReset4 30
#define midiReset5 35

// Led to "see" midi flow
#define pinLed 13
elapsedMillis sinceMidi;
bool statusLed ;

// send a Midi mesg on the rights S2-midi-expanders 
void midiSend(byte midiType,byte midiChannel,int midiData1,int midiData2)
{
  // send the midi message to the ad-hoc S2-DreamBlaster-midi-expanders
  midiBuf[0] = ( midiType << 4 ) | midiChannel ;
  midiBuf[1] = midiData1 ;
  midiBuf[2] = midiData2 ;
  midiLen = (midiType == 4)?2:3 ;
  byte s2bitmap = channelToAudio[midiChannel] ;
  if ( s2bitmap & 0x1 )
    Serial1.write(midiBuf, midiLen);
  if ( s2bitmap & (0x1 << 1) )
    Serial2.write(midiBuf, midiLen);
  if ( s2bitmap & (0x1 << 2) )
    Serial3.write(midiBuf, midiLen);
  if ( s2bitmap & (0x1 << 3) )
    Serial4.write(midiBuf, midiLen);
  if ( s2bitmap & (0x1 << 4) )
    Serial5.write(midiBuf, midiLen);
 }
void midiSetAudio(byte midiChannel, int audioOutput )
{
  // set the midiChannel on this audioOutput 1..10
  if ( audioOutput == 0 )
  {
    channelToAudio[midiChannel] = 0 ; // channel not played
    return ;
  }
  // channel played on this midi mono output 1..10
  int midiOutNr = (audioOutput - 1) >> 1;
  channelToAudio[midiChannel] |= ( 0x1 << midiOutNr ) ; 
  // set pan on right or left on the midiOutNr
  midiBuf[0] = ( 3 << 4 ) | midiChannel ; // Control change 
  midiBuf[1] = 10 ; // pan
  if ( (audioOutput & 0x1) == 1)
    midiBuf[2] = 0 ; // right
  else
    midiBuf[2] = 127 ; // left    
  switch(midiOutNr)
  {
    case 0 : Serial1.write(midiBuf, 3); break ;
    case 1 : Serial2.write(midiBuf, 3); break ;
    case 2 : Serial3.write(midiBuf, 3); break ;
    case 3 : Serial4.write(midiBuf, 3); break ;
    case 4 : Serial5.write(midiBuf, 3); break ;
  }
}
void setup() // of the microcontroller
{
  pinMode(pinLed,OUTPUT);
  digitalWrite(pinLed,HIGH);
  statusLed = true ;

  for(int i = 0 ; i < 16 ; i ++ )
     channelToAudio[i] = 1 ; // all channel payed on audio-output 1
  Serial1.begin(31250);
  Serial2.begin(31250);
  Serial3.begin(31250);
  Serial4.begin(31250);
  Serial5.begin(31250);
  pinMode(midiReset1,OUTPUT);
  pinMode(midiReset2,OUTPUT);
  pinMode(midiReset3,OUTPUT);
  pinMode(midiReset4,OUTPUT);
  pinMode(midiReset5,OUTPUT);
  digitalWrite(midiReset1,LOW);
  digitalWrite(midiReset2,LOW);
  digitalWrite(midiReset3,LOW);
  digitalWrite(midiReset4,LOW);
  digitalWrite(midiReset5,LOW);
  delay(500);
  digitalWrite(midiReset1,HIGH);
  digitalWrite(midiReset2,HIGH);
  digitalWrite(midiReset3,HIGH);
  digitalWrite(midiReset4,HIGH);
  digitalWrite(midiReset5,HIGH);

  sinceMidi = 0;
}

void loop() 
{
  if ( usbMIDI.read() )
  {
    midiType = usbMIDI.getType() ;
    midiChannel = usbMIDI.getChannel() ;
    midiData1 = usbMIDI.getData1() ;
    midiData2 = usbMIDI.getData2() ;
    switch(midiType)
    {
      case 7 : // sysex to dispatch to all S2-DreamBlaster-midi-expanders
        midiSysexBuf = usbMIDI.getSysExArray() ;
        Serial1.write(midiSysexBuf, midiData1);
        Serial2.write(midiSysexBuf, midiData1);
        Serial3.write(midiSysexBuf, midiData1);
        Serial4.write(midiSysexBuf, midiData1);
        Serial5.write(midiSysexBuf, midiData1);
        break ;
      case 3 : // CTRL
        // CTRL-42 is used to connect a channel on an audio-output : 0 (not played), 1..10
        if (midiData1 == 42 )
        {
          midiSetAudio(midiChannel,midiData2);
          break ;
        }
      default :
        // all Midi messages are sent on the right S2-midi-expanders, according to Midi-channel and previous CTRL-42
        midiSend(midiType,midiChannel,midiData1,midiData2);
        break ;
    }
    digitalWrite(pinLed,HIGH);
    sinceMidi = 0;
    statusLed = true ;
  }
  if (statusLed && ( sinceMidi > 500 ))
  {
    digitalWrite(pinLed,LOW);
    statusLed = false ;
  }
}
