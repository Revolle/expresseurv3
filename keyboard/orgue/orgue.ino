#define DEBUGON 2
//#define MIDIUSB 1

///////////////////////////////////////////////////////////////////////////////
//                       Definition SD-Card
///////////////////////////////////////////////////////////////////////////////
#include <SD.h>
#include <SPI.h>
const int chipSelect = BUILTIN_SDCARD;

///////////////////////////////////////////////////////////////////////////////
//                       Definition Expresseur
///////////////////////////////////////////////////////////////////////////////
#define MAXEVENT 2500
#define MAXEVENTSTOPSTART 8
#define PITCHFILE 64
#define MAXPITCH 128
#define MAXKEYNR 72

void expresseur_loadFile(int fileNr);
void expresseur_play(int pitch, int velo);

typedef struct
{
  short unsigned int velocity;
  short unsigned int pitch;
  short unsigned int trackNr;
  short unsigned int willStopIndex ; // on note-off
  short unsigned int stopIndex ; // on note-on
  short unsigned int eventsStart[MAXEVENTSTOPSTART]; // synchronous start
  short unsigned int nbEventsStart  ;
  short unsigned int eventsStop[MAXEVENTSTOPSTART]; // synchronous stop
  short unsigned int nbEventsStop  ;
} T_event ; // more or less a "chord" to play
typedef struct
{
  T_event events[MAXEVENT];
  short unsigned int nbEvents  ;
  short unsigned int trackMax;
} T_score ; // a score is a sequential list of chords
T_score score ;
int noteOffStops[MAXPITCH]; // note of to stop on onte-off
#define DEFAULTFILE 1

int c_nrEvent_noteOn = 0 ; // index of event for the next note-on 
int end_score = false;
int noteExpresseurOn = 0 ;
bool firstErrAllNoteOff = true ;
bool newEvent = false ;

///////////////////////////////////////////////////////////////////////////////
//                       Definition Orgue
///////////////////////////////////////////////////////////////////////////////
#define MAXSPLIT 2
#define VELODEFAULT 100
#define VELOLOW 64
#define VELOHIGH 100
#define BALANCEMAX 100
#define pinLed  13
#define MaxOUT 10
#define MaxIN 9
// Pins Teensy 3.6
byte pinOut[MaxOUT] = { 29, 30, 31, 5 ,  6,  7,  8,  9, 14, 15 } ;
byte pinIn[MaxIN]   = { 36, 35, 33, 34, 16, 18, 17, 19, 20 };
byte buttonState[MaxOUT][MaxIN];
// two last buttons of pinOUT are for the Tirette
#define O_TIRETTE 8
#define OFFSETPITCH 17

//etats de l'automate
enum { B_OFF ,B_WAIT_ON , B_ON , B_WAIT_OFF , B_OUT } ;
unsigned long buttonSince[MaxOUT][MaxIN];
elapsedMillis sinceLed;
elapsedMillis sinceButton;
int lastTiretteNr = -1 ;
bool keyShift = false ;
bool splitChange = false ;

#define DELAY_NOISE_TIRETTE 500 // ms
#define DELAY_NOISE_KEYBOARD 10 // ms

// management of volume
#define VOL_PIANO 15
#define VOL_MESOFORTE 60
#define VOL_DELTA_FORTE 30
byte vol_base = VOL_MESOFORTE ;
byte vol_delta = 0 ;

// touche de "fonction"
#define KEYSHIFT 5

bool expresseur = false ;
bool gauche = false ;
bool split = false ;
byte psplit = 64 ; /* Mi-4 */
int octaviaGauche = 0 ;
int octaviaDroite = 0 ;
int balanceGaucheDroite = BALANCEMAX / 2 ;

int midiPitchNbNoteOn[MAXSPLIT][MAXPITCH];

#define NBCHANNEL 16
#define MAXTIRETTE ( 2 * MaxIN )
#define MAXBANK 5

/* 
out/in definition

0/0 Fa ... 7/8 Mi

8/0 0 voix celeste
8/1 1 clairon
8/2 2 expression
8/3 3 flute
8/4 4 bourdon
8/5 5 percussion
8/6 6 hautbois 
8/7 7 baryton
8/8 8 forte

9/0 9 basson
9/1 10 clarinette
9/2 11 forte
9/3 12 tremolo
9/4 13 cor_anglais 
9/5 14 percussion 
9/6 15 fifre percussion
9/7 16 musette
9/8 17 sourdine
*/
// program per tirette, progs[bank[tirette#]][tirette#]
#define DEFAULTPROG 4 
int progs[MAXBANK][MAXTIRETTE]=
  {
    {53 ,57, 0,74,20, 18,69,54, 0, 71,72, 0,19,70, 7,75,21,0},
    {100,58, 0,73,20, 16,65,55, 0, 71,72, 0,19,70, 7,79,22,0},
    {101,59, 0,76,17, 16,66,86, 0,  0,58, 0,19, 0, 9,80,23,0},
    {99 ,60, 0,77,17,  8,67,61, 0,  0,58, 0, 0, 0,10, 0,24,0},
    {97 ,61, 0,78,17,107,68, 0, 0,  0,58, 0, 0, 0,11, 0,22,0}
  };
// banks per tirette, banks[tirette#]
int banks[MAXBANK][MAXTIRETTE]=
  {
    {  0, 0,-1, 0, 0, 0,  0, 0,-1,  0, 0,-1, 8, 0, 0 , 0,0,-1},
    {  0, 0,-1, 0, 8, 0,  0, 0,-1,  1, 8,-1,16, 1, 8 , 0,0,-1},
    {  0, 0,-1, 0, 0, 1,  0, 0,-1, -1,-1,-1,24,-1, 0 , 0,0,-1},
    {  0, 0,-1, 0,40, 0,  0, 0,-1, -1,-1,-1,-1,-1, 0 ,-1,0,-1},
    {  0, 0,-1, 0, 2, 1,  0,-1,-1, -1,-1,-1,-1,-1, 0 ,-1,8,-1}
  };
// status of a prog on a channel  
int prog[NBCHANNEL]=        {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
int lastBankNr[MAXTIRETTE]= {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
bool firstProg = true;


///////////////////////////////////////////////////////////////////////////////
//                       MIDI management
///////////////////////////////////////////////////////////////////////////////

void sendMidiByte(int nbByte, int b1 , int b2, int b3)
{
  if (( b1 < 0 ) || ( b1 > 255) || ( b2 < 0 ) || ( b2 > 255) || ( b3 < 0 ) || ( b3 > 255) || (nbByte < 1) ||  (nbByte > 3))
  {
    #ifdef DEBUGON
    Serial.print("ERROR sendMidiByte *"); Serial.print(nbByte);   Serial.print(" ");
    Serial.print(b1,HEX);   Serial.print("/"); Serial.print(b2,HEX);  Serial.print("/"); Serial.println(b3,HEX);
    #endif
    return;
  }
  #if DEBUGON > 3
    Serial.print("sendMidiByte *"); Serial.print(nbByteX);   Serial.print(" ");
    Serial.print(b1,HEX);   Serial.print("/"); Serial.print(b2,HEX);  Serial.print("/"); Serial.println(b3,HEX);
  #endif
  switch(nbByte)
  {
    case 1 : Serial1.write((byte)b1); break ;
    case 2 : Serial1.write((byte)b1); Serial1.write((byte)b2); break ;
    case 3 : Serial1.write((byte)b1); Serial1.write((byte)b2); Serial1.write((byte)b3); break ;
    default : break ;
  }
  Serial1.flush();
  newEvent = true ;
}
void sendMidiCtrl(byte n,byte v)
{
  byte i0, i1 ;
  if (split)
  {
    if (gauche)
    {
      i0=0; i1=5;
    }
    else
    {
      i0=6; i1=15;
    }
  }
  else
  {
    i0=0; i1=15;
  }
  for(byte i = i0; i <= i1 ; i ++)
  {
    #if DEBUGON > 1
      Serial.print("CTRL #"); Serial.print(i); Serial.print(" "); Serial.print(n); Serial.print("/"); Serial.println(v);
    #endif
    sendMidiByte(3 , 0xB0 | i , n , v );
    #ifdef MIDIUSB
      usbMIDI.sendControlChange(n, v, i);
    #endif
  }
}
void general_volume()
{
  int vol = vol_base + vol_delta ;
  #if DEBUGON > 1
    Serial.print("General volume = "); Serial.println(vol);
  #endif
  /*
  sendMidiByte(3, 0xB0, 0x63 , 0x37 );
  sendMidiByte(3 , 0xB0 , 0x62 , 0x7);
  sendMidiByte(3, 0xB0 , 0x6 , vol_base + vol_delta );
  */
  sendMidiCtrl(7,vol);
}

void sendProg(boolean on,int nrprog,int nrbank)
{
  byte p = constrain(nrprog, 1, 128) - 1;
  byte b = constrain(nrbank, 0, 127);
  byte i0, i1 ;
  if (split)
  {
    if (gauche)
    {
      i0=0; i1=5;
    }
    else
    {
      i0=6; i1=15;
    }
  }
  else
  {
    i0=0; i1=15;
  }
  
  if ((nrprog <= 0) || (nrbank < 0)) 
  {
    // reset
    #if DEBUGON > 1
      Serial.println("PROG reset"); 
    #endif
    for(int i = i0; i <= i1; i ++)
         prog[i] = -1 ;
    return ;  
  }

  // off prog
  int bp = b*128+p; 
  for(int i = i0; i <= i1; i ++)
  {
    if (prog[i] == bp)
      prog[i] = -1 ;
  }
  
  // on prog
  if (on)
  {
    byte slot = 0 ;
    boolean slotfound = false ;
    for(byte i = i0; i <= i1 ; i ++)
    {
      if (prog[i] == -1)
      {
        slotfound = true ;
        slot = i ;
        break ;
      }
    }
    if ( ! slotfound)
      slot = i0;
    #if DEBUGON > 1
      Serial.print("PROG ch#"); Serial.print(slot); Serial.print(" ");
      Serial.print(b); Serial.print("/"); Serial.println(p);
    #endif
    prog[slot] = bp ;
    sendMidiByte(3 , 0xB0 | slot , 0 , b);
    sendMidiByte(2 , 0xC0 | slot , p , 0);
    #ifdef MIDIUSB
      usbMIDI.sendControlChange(0, b, slot);
      usbMIDI.sendProgramChange(p, slot);
    #endif
  }
}
void reset_x2()
{
  #if DEBUGON > 1
    Serial.println("sount note all parts");
  #endif
  for(byte i = 0; i < 16; i ++)
  {
    sendMidiByte(3, 0xF0 ,0x41, 0x00);
    sendMidiByte(3 , 0x42 , 0x12 , 0x40);
    sendMidiByte(3 , 0x10 | i , 0x15 , 0x00 ); // sound for all parts
    sendMidiByte(2, 0x00 ,0xF7 , 0 );
  }
  #if DEBUGON > 1
  Serial.println("reset all controlers");
  #endif  
  for(byte i = 0; i < 16; i ++)
  {
    sendMidiByte(3,0xB0 | i,120,0);
    sendMidiByte(3,0xB0 | i,121,0);
    sendMidiByte(3,0xB0 | i,123,0);
  }
  #if DEBUGON > 1
  Serial.println("test notes");
  for(byte i= 0 ; i < 2 /*MAXTIRETTE*/ ; i ++ )
  {
    if ( progs[0][i] > 0 )
    {
      sendMidiByte(3,0xB0,0,banks[0][i]);
      sendMidiByte(2,0xC0,progs[0][i],0);
      sendMidiByte(3,0x90,0x40 | i,0x40);
      delay(500);
      sendMidiByte(2,0x90,0x40 | i,0x40);
    }
  }
  #endif
  general_volume() ;
  firstProg = true;
  #if DEBUGON > 1
    Serial.println("default prog");
  #endif  
  sendProg(true, progs[0][DEFAULTPROG],banks[0][DEFAULTPROG]);
}
void init_x2()
{
  for(int i = 0; i < MAXSPLIT; i ++)
    for(int j= 0 ; j < MAXPITCH; j ++)
      midiPitchNbNoteOn[i][j] = 0 ;
      
  Serial1.begin(31250);
  delay(200);
  reset_x2();
}
void sendMidiNote(int pi,int v, int pspliti)
{
  int velo = v ;
  int i0, i1 ;
  int p = pi ;
  int canal = 0 ;
  if (split)
  {
    if (p < pspliti)
    {
      if (on)
      {
        splitChange = (gauche == false) ; 
        gauche = true ;
      }
      i0=0; i1=5;
      p += 12 * octaviaGauche ;
      canal = 0 ;
      velo = 127 - balanceGaucheDroite ;
    }
    else
    {
      if (on)
      {
        splitChange = (gauche == true) ; 
        gauche = false ;
      }
      p += 12 * octaviaDroite ;
      i0=6; i1=15;
      canal = 1 ;
      velo = 127 - BALANCEMAX + balanceGaucheDroite ;
    }
  }
  else
  {
    p += 12 * octaviaDroite ;
    i0=0; i1=15;
  }
  if ( v == 0 )
  {
    (midiPitchNbNoteOn[canal][p]) -- ;
    if (midiPitchNbNoteOn[canal][p] <= 0 )
    {
        midiPitchNbNoteOn[canal][p] = 0 ;
        #if DEBUGON > 1
          Serial.print("NOTE OFF ch#"); Serial.print(i0);Serial.print("..");Serial.print(i1);
          Serial.print(" canal="); Serial.print(canal);Serial.print(" pitch=");Serial.println(p);
        #endif
      for(byte i = i0; i <= i1 ; i ++)
      {
        sendMidiByte(3,0x80 | i,p,0);
        #ifdef MIDIUSB
          usbMIDI.sendNoteOff(p, 100, i);
        #endif
      }
    }
  }
  else
  {
    (midiPitchNbNoteOn[canal][p]) ++ ;
    #if DEBUGON > 1
      Serial.print("NOTE ON  ch#");Serial.print(i0);Serial.print("..");Serial.print(i1);
      Serial.print(" canal=");Serial.print(canal);Serial.print(" pitch=");Serial.print(p);
      Serial.print(" velo=");Serial.print(v);
      Serial.print(" nbNoteOn=");Serial.println(midiPitchNbNoteOn[canal][p]);
    #endif
    if (midiPitchNbNoteOn[canal][p] == 1 )
    {
      for(byte i = i0; i <= i1 ; i ++)
      {
        if (prog[i] != -1)
        {
          #if DEBUGON > 1
            Serial.print("NOTE ON  ch#");Serial.print(i);Serial.print(" pitch=");
            Serial.print(p);Serial.print(" velo=");Serial.println(v);
          #endif
          sendMidiByte(3,0x90 | i,p,velo);
          #ifdef MIDIUSB
            usbMIDI.sendNoteOn(p, velo, i);
          #endif
        }
      }
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
//                       Orgue management
///////////////////////////////////////////////////////////////////////////////
void ctrlTirette(int tiretteNr, bool on)
{
  #if DEBUGON > 1
    Serial.print("ctrlTirette #");Serial.print(tiretteNr);Serial.print(" ");Serial.println(on);
  #endif
  switch(tiretteNr)
  {
    case  17 :
      // sourdine
      if ( on )
        vol_base = VOL_PIANO  ;
      else
        vol_base = VOL_MESOFORTE  ;
      general_volume() ;
      break ;
    case 8 :
    case 11 :
      // forte
      if (on)
        vol_delta += VOL_DELTA_FORTE ;
      else
        vol_delta -= VOL_DELTA_FORTE ;
      general_volume() ;
      break ;
    case 2 :
      // mode Expresseur
      expresseur = on ;
      if (expresseur)
        expresseur_loadFile(DEFAULTFILE);
      break ;
    default :
      // selection instrument program
      if ( on )
      {
        if (firstProg)
        {
          firstProg = false ;
          sendProg(false,progs[0][DEFAULTPROG],banks[0][DEFAULTPROG]);
        }
        if ( splitChange )
        {
          sendProg(false,-1,-1);
          splitChange = false ;
        }
        lastTiretteNr = tiretteNr ;
        lastBankNr[tiretteNr] = 0 ;
      }
      else
      {
        lastTiretteNr = -1 ;
      }
      // changement de programme
      int mprog = progs[lastBankNr[tiretteNr]][tiretteNr] ;
      int mbank = banks[lastBankNr[tiretteNr]][tiretteNr] ;
      #if DEBUGON > 1
        Serial.print(on);Serial.print(" Program #");Serial.print(mprog);Serial.print(" bank#");Serial.println(mbank);
      #endif
      sendProg(on,mprog,mbank);
      break ;
  }
}
void shiftBank(int bankNr)
{
  if ((lastTiretteNr < 0 ) || (lastBankNr[lastTiretteNr] < 0 )) return ;
  int mprog = progs[bankNr][lastTiretteNr] ;
  int mbank = banks[bankNr][lastTiretteNr] ;
  if ((mprog != -1) && (mbank != -1))
  {
    // changement de programme
    #if DEBUGON > 1
      Serial.print("shiftBank");Serial.print(" Program #");Serial.print(mprog);Serial.print(" bank#");Serial.println(mbank);
    #endif
    int pprog = progs[lastBankNr[lastTiretteNr]][lastTiretteNr] ;
    int pbank = banks[lastBankNr[lastTiretteNr]][lastTiretteNr] ;
    sendProg(false,pprog,pbank);
    sendProg(true,mprog,mbank);
    lastBankNr[lastTiretteNr] = bankNr ;
  }
}
void allNoteOff()
{
  bool errAllNoteOff = false ;
  for(byte i = 0; i < 16; i ++)
    sendMidiByte(3,0xB0 | i,123,0);
  for(int i = 0 ; i < MAXSPLIT; i ++ )
    for(int p = 0 ; p < MAXPITCH ; p++)
    {
      #ifdef DEBUGON
        if (( midiPitchNbNoteOn[i][p] > 0 ) && firstErrAllNoteOff )
        {
          Serial.print("ERROR allNoteOff midiPitchNbNoteOn ");Serial.print(i);Serial.print("/");Serial.print(p);Serial.print("="); Serial.println(midiPitchNbNoteOn[i][p]);
          errAllNoteOff = true ;
        }
      #endif
      midiPitchNbNoteOn[i][p] = 0 ;
    }
  #ifdef DEBUGON
    if ( noteExpresseurOn > 0 )
    {
      Serial.print("ERROR allNoteOff noteExpresseurOn ="); Serial.println(noteExpresseurOn);
    }
  #endif
  noteExpresseurOn = 0 ;
  if ( errAllNoteOff )
    firstErrAllNoteOff = false ;
}
void blackWhite(int pitch, int *blackNr, int *whiteNr, int *octave)
{
    int pit = pitch % 12;
    *octave = pitch / 12 ;
    *blackNr = -1 ;
    *whiteNr = -1 ;
    switch(pit)
    {
      case 0 : *whiteNr = 0 ; break ;
      case 1 : *blackNr = 0 ; break ;
      case 2 : *whiteNr = 1 ; break ;
      case 3 : *blackNr = 1 ; break ;
      case 4 : *whiteNr = 2 ; break ;
      case 5 : *whiteNr = 3 ; break ;
      case 6 : *blackNr = 2 ; break ;
      case 7 : *whiteNr = 4 ; break ;
      case 8 : *blackNr = 3 ; break ;
      case 9 : *whiteNr = 5 ; break ;
      case 10 : *blackNr = 4 ; break ;
      case 11 : *whiteNr = 6 ; break ;
      default : break ; 
     }
}
void tuneSplit(int keyNr)
{
  int blackNr = 0 ;
  int whiteNr = 0 ;
  int octave = 0 ;
  blackWhite(OFFSETPITCH + keyNr, &blackNr, &whiteNr, &octave);
  if ( blackNr == -1)
  {
    // balance droite/gauche
    balanceGaucheDroite = (BALANCEMAX *(keyNr - 20 ))/ ( MAXKEYNR - 20 );
  }
  else
  {
    // split
    psplit = OFFSETPITCH + keyNr ;
  }
}
void ctrlShiftKey(int keyNr)
{
  char buf[80];
  buf[0] = '\0' ;
  switch(keyNr)
  {
      case KEYSHIFT :  allNoteOff();      break ;  
    case 6    :                                       break ;
    case 7    :                                       break ;
      case 8  : shiftBank(0)                        ; strcpy(buf," bank(0)"); break ;
    case 9    : split = false                       ; strcpy(buf," split false"); break ;
      case 10 : shiftBank(1)                        ; strcpy(buf," bank(1)"); break ;
    case 11   : split = true                        ; strcpy(buf," split true"); break ;
    case 12   : octaviaGauche = -1  ; splitChange = (gauche == false) ; gauche = true ; strcpy(buf," octavia Gauche -");break ;
      case 13 : shiftBank(2)                        ; strcpy(buf," bank(2)"); break ;
    case 14   : octaviaGauche =  0  ; splitChange = (gauche == false) ; gauche = true ; strcpy(buf," octavia Gauche 0"); break ;
      case 15 : shiftBank(3)                        ; strcpy(buf," bank(3)"); break ;
    case 16   : octaviaGauche =  1  ; splitChange = (gauche == false) ; gauche = true ; strcpy(buf," octavia Gauche +"); break ;
      case 17 : shiftBank(4)                        ; strcpy(buf," bank(4)"); break ;
    case 18   : octaviaDroite = -1  ; splitChange = (gauche == true) ; gauche = false; strcpy(buf," octavia Droite -"); break ;
    case 19   : octaviaDroite =  0  ; splitChange = (gauche == true) ; gauche = false; strcpy(buf," octavia Droite 0"); break ;
      case 20 :                                       break ;
    case 21   : octaviaDroite =  1  ; splitChange = (gauche == true) ; gauche = false; strcpy(buf," octavia Droite +"); break ;
    default   : tuneSplit(keyNr) ; strcpy(buf," split") ; break ;      
  }
  #if DEBUGON > 1
     Serial.print("ctrlShiftKey = "); Serial.print(keyNr);  Serial.print(" : "); Serial.println(buf) ;
  #endif
}
void modeExpresseur(int keyNr , bool on)
{
  int blackNr = 0 ;
  int whiteNr = 0 ;
  int octave = 0 ;
  blackWhite(OFFSETPITCH + keyNr,  &blackNr,  &whiteNr,  &octave);
  #if DEBUGON > 1
    Serial.print("modeExpresseur keyNr= "); Serial.print(keyNr); Serial.print(" blackNr= "); Serial.println(blackNr); 
  #endif
  if (blackNr != -1)
  {
    if ( on )
    {
      int nrFile = (octave - 3) * 5 + blackNr ;
      #if DEBUGON > 1
        Serial.print("modeExpresseur loadFile nrFile = "); Serial.println(nrFile); 
      #endif
      expresseur_loadFile(nrFile);
      if (score.nbEvents < 2)
      {
        #if DEBUGON > 1
          Serial.print("err / modeExpresseur loadFile default nrFile = "); Serial.println(DEFAULTFILE); 
        #endif
        expresseur_loadFile(DEFAULTFILE) ;
      }
      if (score.nbEvents < 2)
      {
        expresseur = false ;
        #if DEBUGON > 1
          Serial.println("err / modeExpresseur loadFile");  
        #endif
      }
    }
  }
  else
  {
    expresseur_play(keyNr, (on?VELODEFAULT:0));
  }
}
void ctrlNote(int keyNr , bool on)
{
  if (expresseur)
  {
    modeExpresseur(keyNr , on);
  }
  else
  {
    // mode orgue
    sendMidiNote(OFFSETPITCH + keyNr ,(on?VELODEFAULT:0),psplit);
  }
}
void ctrlButton(byte o, byte i, bool on)
{
  if ( o >= O_TIRETTE)
  {
    #if DEBUGON == 1
      Serial.print("CtrlButton tirette ");Serial.print(o);Serial.print("/");Serial.print(i);Serial.print(" on=");Serial.println(on);
    #endif
    // tirettes
    buttonSince[o][i] = sinceButton + DELAY_NOISE_TIRETTE ;
    unsigned int tiretteNr = MaxIN*(o - O_TIRETTE) + i ;
    ctrlTirette( tiretteNr,  on);
  }
  else
  {
    #if DEBUGON == 1
      Serial.print("CtrlButton clavier ");Serial.print(o);Serial.print("/");Serial.print(i);Serial.print(" on=");Serial.println(on);
    #endif

    // clavier
    buttonSince[o][i] = sinceButton + DELAY_NOISE_KEYBOARD ;
    int keyNr = (o*MaxIN)+i  ;
    if (keyNr == KEYSHIFT)
    {
      keyShift = on ;
      #if DEBUGON > 1
         Serial.print("Key-Shift = "); Serial.println(keyShift);
      #endif
    }
    if (keyShift)
    {
      if (on)
        ctrlShiftKey(keyNr);
    }
    else
      ctrlNote(keyNr, on);
  }
}

///////////////////////////////////////////////////////////////////////////////
//                               Management Expresseur
///////////////////////////////////////////////////////////////////////////////


void nextGroupEvent()
{
  // adjust the position on the next playable event
  // set these booleans :    end of events
  c_nrEvent_noteOn = c_nrEvent_noteOn + 1 ;
  if (c_nrEvent_noteOn < 1 )
  {
    c_nrEvent_noteOn = 1 ;
  }
  if ( c_nrEvent_noteOn > score.nbEvents )
  {
    c_nrEvent_noteOn = score.nbEvents ;
    end_score= true;
  }
  while (( c_nrEvent_noteOn < score.nbEvents ) &&  (score.events[c_nrEvent_noteOn].nbEventsStart ==  0 )) 
  {
    c_nrEvent_noteOn ++ ;
  }
  end_score =  ( c_nrEvent_noteOn >= score.nbEvents  ) ;
}

void outPitch(int pitch, int velo_out, int track)
{
  if (track <= (int)(score.trackMax/2))
  {
    sendMidiNote(pitch ,velo_out,pitch+1);
  }
  else
  {
    sendMidiNote(pitch ,velo_out,pitch-1);
  }
}

void stopEvent(T_event *event)
{
  // stop an event
  noteExpresseurOn -- ;
  #ifdef DEBUGON
  if (noteExpresseurOn < 0)
  {
    Serial.print("ERROR noteExpresseurOn =");  Serial.println(noteExpresseurOn);
    noteExpresseurOn = 0 ;
  }
  #endif

  outPitch(event->pitch,0,event->trackNr);
}

void startEvent(int velo_in_unused, T_event *event , int nbEvent , int bid)
{
  // play an event
  
  if (nbEvent == 0) return;
  
  noteExpresseurOn ++ ;
  
  if ((event->pitch <= 0) || (event->pitch >= 127) || (event->velocity < 2))  return; 
  
  int min_velo_out = 1 ;
  int max_velo_out = 128 ;
  int velo = event->velocity;
  int velo_in = VELOHIGH ;
  if ( score.trackMax >  1)
  {
    velo_in = VELOLOW ;
    bool paire = ((score.trackMax % 2) == 0) ;
    int nZone = score.trackMax + (paire?1:0) ;
    int lZone = MAXKEYNR /nZone ;
    int midZone = (score.trackMax - (paire?0:1)) / 2 ;
    for(int i= 0 , mBid = lZone , ntrack = score.trackMax - 1 ; i < nZone ; i ++ , mBid += lZone , ntrack -- )
    {
      if ( i == midZone)
      {
        if ( bid < mBid)
        {
          velo_in = VELOHIGH ;
          break ;
        }
        if ( paire )
          ntrack ++ ;
      }
      else
      {
        if ( bid < mBid)
        {
          if (ntrack == event->trackNr )
            velo_in = VELOHIGH ;
          break ;
        }
      }
    }
  }
  #if DEBUGON > 1
    Serial.print("equilibrage tracks : trackMAx="); Serial.print(score.trackMax);Serial.print(" trackNr="); Serial.print(event->trackNr);Serial.print(" velo="); Serial.println(velo_in);
  #endif

  if (velo < 64 )
  {
    min_velo_out = 1;
    max_velo_out = 2 * velo;
  }
  else
  {
    min_velo_out = ( velo - 63 ) ;
    max_velo_out = 128;
  }
  // rescale the velocity out 
  int velo_out = min_velo_out + ((max_velo_out - min_velo_out ) * velo_in ) / 128 ;
/*  
  // compensation of nbEvent note within one single key-in
  int compensation = 15 ; // [0..30]
  velo_out = ((200 - (compensation * (nbEvent - 1))) * velo_out) / 200;
*/
  // cap the velocity-out
  if (velo_out < 1 ) velo_out = 1 ;
  if (velo_out > 127)  velo_out = 127 ;

  outPitch(event->pitch,velo_out,event->trackNr);
}

void noteOn_Event(int bid , int velo_in)
{
  // the button-id ( bid ) play the current group , and go to the next one
  
  if ( end_score ) return ;

  #if DEBUGON > 1
    Serial.print("noteOn_Event #");  Serial.println(c_nrEvent_noteOn);
  #endif
  T_event *event = &(score.events[c_nrEvent_noteOn]);
  
  int to_stop_index = event->stopIndex ;
  if (to_stop_index > 0)
  {
    T_event *event_stop = &(score.events[to_stop_index]) ;
    for (int nrEventStop = 0 ; nrEventStop < event_stop->nbEventsStop ; nrEventStop ++)
    {
      T_event *event_to_stop = &(score.events[event_stop->eventsStop[nrEventStop]]);
      #if DEBUGON > 2
        Serial.print("noteOn_Event to_stop_index=") ;Serial.print(to_stop_index);
        Serial.print(" stop_index "); Serial.println(event_stop->eventsStop[nrEventStop]);
      #endif
      stopEvent(event_to_stop);
    }
  }
  if (event->nbEventsStart > 0)
  {
    for (int nrEventStart = 0 ; nrEventStart < event->nbEventsStart ; nrEventStart ++)
    {
      #if DEBUGON > 2
        Serial.print("noteOn_Event start_index ");  Serial.println(event->eventsStart[nrEventStart]);
      #endif
      T_event *event_to_start = &(score.events[event->eventsStart[nrEventStart]]);
      startEvent(velo_in , event_to_start ,event->nbEventsStart , bid);
    }
  }
    
  // reminder for notes to stop on note-off
  noteOffStops[bid] = event->willStopIndex;
 
  // goto nex event to play for the next noteOn_Event
  nextGroupEvent();
}

void noteOff_Event(int bid)
{  
  // stop the group of event started with the same button-id ( bid )
  int toStopIndex = noteOffStops[bid];
  if (toStopIndex > 0)
  {
    #if DEBUGON > 2
      Serial.print("noteOff_Event toStop=");  Serial.println(toStopIndex);
    #endif
    T_event *event = &(score.events[toStopIndex]);
    for (int nrEventStop = 0 ; nrEventStop < event->nbEventsStop ; nrEventStop ++)
    {
      #if DEBUGON > 2
        Serial.print("noteOff_Event #");  Serial.println(event->eventsStop[nrEventStop]);
      #endif
      T_event * eventStop = &(score.events[event->eventsStop[nrEventStop]]) ;
      stopEvent(eventStop);
    }
  }
  noteOffStops[bid] = 0 ;
}

void expresseur_play(int keyNr, int velo)
{
  if (score.nbEvents == 0) return ;
  if (velo == 0) 
    noteOff_Event(keyNr);
  else
    noteOn_Event(keyNr , velo);
}

void rewind_score()
{
  c_nrEvent_noteOn = 0 ;// current index for the next note-on 
  end_score = false;
  for(int i = 0 ; i < MAXPITCH ; i ++)
    noteOffStops[i] = 0 ;
  nextGroupEvent();
  allNoteOff();
  noteExpresseurOn = 0 ;
}

void initScore()
{
  score.nbEvents = 1 ; // index 0 is unused : [0] = nill
  score.trackMax = 0 ;
  T_event *event ;
  for(int i= 0 ; i < MAXEVENT ; i ++ )
  {
    event = &(score.events[i]);
    event->pitch = 0 ;
    event->velocity = 0 ;
    event->trackNr = 0 ;
    event->stopIndex = 0 ;
    event->willStopIndex = 0 ;
    event->nbEventsStart = 0 ;
    event->nbEventsStop = 0 ;
    event->stopIndex = 0 ;
    for(int j= 0; j < MAXEVENTSTOPSTART ; j ++)
    {
      event->eventsStart[j] = 0 ;
      event->eventsStop[j] = 0 ;
    }
  }
  rewind_score();
}

// load an expresseur-score from a txt file in SD card
void expresseur_loadFile(int fileNr)
{
  if ( !SD.begin(chipSelect) ) return ;

  // filename f<nnn>.txt
  char fileName[10] ;
  int unite = fileNr - (10*(int)(fileNr/10));
  int dizaine = (fileNr - (100*(int)(fileNr/100)) - unite)/10 ;
  int centaine = (fileNr - dizaine*10 - unite)/100 ;
  fileName[0]='f';
  fileName[1]= centaine + '0' ;
  fileName[2]= dizaine + '0' ;
  fileName[3]= unite + '0' ;
  fileName[4] = '.' ;
  fileName[5] = 't' ;
  fileName[6] = 'x' ;
  fileName[7] = 't' ;
  fileName[8] = '\0' ;
  #ifdef DEBUGON
    Serial.print("filename : ");Serial.println(fileName);
  #endif
  if ( !SD.exists(fileName)) return ;
  
  // open file
  File myFile = SD.open(fileName);
  if (! myFile) return ;
  #ifdef DEBUGON
   Serial.println("file is opened");
  #endif

  // Groups of 3 lines :
  //     1st line with nt pitch, int velocity,  int trackNr , int stopIndex, int willStopIndex
  //     2nd line with int eventsStart, ..
  //     3rd line with int eventsSttop, ..
  initScore() ;
  T_event *event = &(score.events[1]);
  short unsigned int *ind ;
  char c ;
  int etat = -1 ;
  int minus = 1 ;
  int index = 0  ;
  while ( myFile.available())
  {
    c = myFile.read();
    switch (etat)
    {
      case -2 : break ; // NOP
      case -1 : //index
        if ((c >= '0') && (c <= '9'))
        {
          index = 10* index + (c - '0') ;
          break ;
        }
        else if (c == ',')
        {
          if (index < MAXEVENT)
          {
            event = &(score.events[index]);
            if ( index > score.nbEvents )
              score.nbEvents = index + 1 ;
          }
          else 
          {
            #ifdef DEBUGON
              Serial.print("ERROR : event overflow. Index=");Serial.println(index);
            #endif
            etat = -2 ;
            // initScore(); 
            return;
          }
          etat = 0 ;
        } 
        break ;
      case 0 : //pitch
      if ((c >= '0') && (c <= '9'))
      {
        if (event == NULL) { initScore() ; return ;} 
        event->pitch = minus * (10* event->pitch + (c - '0')) ;
        minus = 1 ;
        break ;
      }
      else if (c == '-')
        minus = -1 ; 
      else if (c == ',')
        etat = 1 ; 
      break ;
      case 1 : //velocity
        if ((c >= '0') && (c <= '9'))
        {
          event->velocity = 10* event->velocity + (c - '0') ;
          break ;
        }
        else if (c == ',')
        etat = 2 ; 
      break ;
      case 2 : //track
        if ((c >= '0') && (c <= '9'))
        {
          event->trackNr = 10* event->trackNr + (c - '0') ;
          break ;
        }
        else if (c == ',')
        {
          if (event->trackNr >= score.trackMax)
            score.trackMax = event->trackNr + 1 ;
          etat = 3 ;
        }
      break ;
      case 3 : //stopIndex
        if ((c >= '0') && (c <= '9'))
        {
          event->stopIndex = 10* event->stopIndex + (c - '0') ;
          break ;
        }
        else if (c == ',')
          etat = 4 ;
        break ;
      case 4 : //willStopIndex
        if ((c >= '0') && (c <= '9'))
        {
          event->willStopIndex = 10* event->willStopIndex + (c - '0') ;
          break ;
        }
        else if (c == '\n')
        {
          etat = 10 ;
          event->nbEventsStart = 0 ;
          ind = &(event->eventsStart[0]);
        }       
        break ;
      case 10: //eventsStart
        if ((c >= '0') && (c <= '9'))
        {
          if (ind == NULL) { etat = -2 ; break ;}
          *ind = 10* (*ind) + (c - '0') ;
          break ;
        }
        else if (c == ',')
        {
          (event->nbEventsStart) ++ ;
          if ( event->nbEventsStart < MAXEVENTSTOPSTART)
            ind = &(event->eventsStart[event->nbEventsStart]); 
          else
          {
            ind = NULL ;
            #ifdef DEBUGON
              Serial.println("ERROR : eventStart overflow");
            #endif
          }
        }
        else if (c == '\n')
         {
            if ( *ind > 0 )
              (event->nbEventsStart) ++ ;
            etat = 11 ;
            event->nbEventsStop = 0 ;
            ind = &(event->eventsStop[0]);
          }        
        break ;
      case 11 : //eventsStop
      if ((c >= '0') && (c <= '9'))
      {
        if (ind == NULL) { etat = -2 ; break ;}
        *ind = 10* (*ind) + (c - '0') ;
        break ;
      }
      else if (c == ',')
      {
        (event->nbEventsStop) ++ ;
        if ( event->nbEventsStop < MAXEVENTSTOPSTART)
          ind = &(event->eventsStop[event->nbEventsStop]);
        else
        {
          ind = NULL ;
          #ifdef DEBUGON
            Serial.println("ERROR : eventStop overflow");
          #endif
        }
      }
      else if (c == '\n')
      {
        if ( *ind > 0 )
          (event->nbEventsStop) ++ ;
          #if DEBUG > 1
          if (( index == 66) || ( index == 69)) 
          {
             Serial.print("event#");
             Serial.print(index);
             Serial.print(" pitch#");
             Serial.print(event->pitch);
             if ( event->velocity != 64)
             {
               Serial.print(" velo=");
               Serial.print(event->velocity);
             }
             Serial.print(" track#");
             Serial.print(event->trackNr);
             if ( event->stopIndex > 0 )
             {
               Serial.print(" stop#");
               Serial.print(event->stopIndex);
             }
             if ( event->willStopIndex > 0 )
             {
                Serial.print(" willStop#");
                Serial.print(event->willStopIndex);
             }
             Serial.println("");
             if ( event->nbEventsStart > 0 )
             {
               Serial.print("    starts ");
               for(int i = 0 ; i < event->nbEventsStart ; i ++)
               {
                   Serial.print(" #");
                   Serial.print(event->eventsStart[i]);
                   Serial.print("/p=");
                   Serial.print(score.events[event->eventsStart[i]].pitch);
               }
               Serial.println("");
             }
             if ( event->nbEventsStop > 0 )
             {
                Serial.print("    stops ");
                for(int i = 0 ; i < event->nbEventsStop ; i ++)
                 {
                     Serial.print(" #");
                     Serial.print(event->eventsStop[i]);
                     Serial.print("/p=");
                     Serial.print(score.events[event->eventsStop[i]].pitch);
                 }
                 Serial.println("");
             }
          }
          #endif
          index = 0 ;
          etat = -1 ;
        }         
      break ;
      default : etat = -1 ; break ;
    }
  }
  myFile.close();
  #ifdef DEBUGON
     Serial.print("last read event#");Serial.println(score.nbEvents);
  #endif
  rewind_score() ;
}

void tester()
{
  /*
  sendMidiNote(64 ,100,psplit);
  delay(1000);
  sendMidiNote(68 ,100,psplit);
  delay(1000);
  sendMidiNote(64 ,0,psplit);
  sendMidiNote(68 ,0,psplit);
  delay(1000);
  sendProg(true, progs[1][0],banks[1][0]);
  delay(1000);
  sendMidiNote(68 ,100,psplit);
  delay(1000);
  sendMidiNote(68 ,0,psplit);
  delay(1000);
  sendProg(false, progs[0][0],banks[0][0]);
  delay(1000);
  sendMidiNote(68 ,100,psplit);
  delay(1000);
  sendMidiNote(68 ,0,psplit);
  */

  expresseur_loadFile(DEFAULTFILE);
  sendProg(true, progs[0][1],banks[0][1]);
  for(int i= 1 ; i < 600; i ++)
  {
    digitalWrite(pinLed,HIGH);
    expresseur_play(50,100);
    delay(10);
    expresseur_play(51,0);
    delay(100);
    expresseur_play(51,100);
    delay(10);
    digitalWrite(pinLed,LOW);
    expresseur_play(50,0);
    delay(100);
  }
}
///////////////////////////////////////////////////////////////////////////////
//                             SETUP
///////////////////////////////////////////////////////////////////////////////
void setup() 
{
  pinMode(pinLed, OUTPUT);
  
  #ifdef DEBUGON
    Serial.begin(9600);
    digitalWrite(pinLed,HIGH);
    delay(200);
    digitalWrite(pinLed,LOW); 
    Serial.println("debug");
    delay(200);
    digitalWrite(pinLed,HIGH);
    Serial.println("organ");
  #endif

  init_x2();
  delay(200);
  digitalWrite(pinLed,LOW); 

  Serial.println("init pinOut");
  for(byte o=0;o < MaxOUT; o ++)
  {
    pinMode(pinOut[o], OUTPUT);
    digitalWrite(pinOut[o],HIGH);
  }

  Serial.println("init pinIn");
  for(byte i=0;i < MaxIN; i ++)
    pinMode(pinIn[i], INPUT_PULLUP);     

  Serial.println("init buttonState");
  for(byte o=0;o < MaxOUT; o ++)
  {
    digitalWrite(pinOut[o],LOW);
    delayMicroseconds(1000);
    for(byte i=0;i < MaxIN; i ++)
    {
      if ((digitalRead(pinIn[i]) == HIGH) && (o < O_TIRETTE))
      {
        buttonState[o][i] = B_OUT; // keyboark key is out of order
        #ifdef DEBUGON
          Serial.print("Key out of order ");Serial.print(o);Serial.print("/");Serial.println(i);
        #endif
      }
      else
        buttonState[o][i] = B_OFF;
    }
    digitalWrite(pinOut[o],HIGH);
    delayMicroseconds(1000);
  }
  initScore();
}

///////////////////////////////////////////////////////////////////////////////
//                              LOOP
///////////////////////////////////////////////////////////////////////////////
void loop() 
{
  // scan keys
  bool sAllKeyStable = true ;
  bool sAllNoteOff = true ;
  
  for(byte o=0;o < MaxOUT; o ++)
  {
    digitalWrite(pinOut[o],LOW);
    delayMicroseconds(500);
    for(byte i=0;i < MaxIN; i ++)
    {
      bool vin = digitalRead(pinIn[i]) ;
      if ( o >= O_TIRETTE )
        vin = ! vin ;
      switch(buttonState[o][i])
      {
        ////////////////////////////////
        // etat repos 
        ////////////////////////////////
        case B_OFF : // etat repos
          if ( vin == HIGH)
          {
            // debut appui 
            #if DEBUGON > 3
              Serial.println("B_OFF => B_WAIT_ON ");Serial.print(o);Serial.print("/");Serial.println(i);
            #endif
            buttonState[o][i] = B_WAIT_ON ;
            ctrlButton(o, i, true);
            if ( o < O_TIRETTE) sAllNoteOff = false ;
            sAllKeyStable =  false ;
          }
          break;
        ////////////////////////////////
        // etat styabilisation appuye 
        ////////////////////////////////
        case B_WAIT_ON :
          if ( o < O_TIRETTE) sAllNoteOff = false ;
          sAllKeyStable =  false ;
          if ( sinceButton > buttonSince[o][i])
          {
            #if DEBUGON > 3
              Serial.println("B_WAIT_ON => B_ON ");Serial.print(o);Serial.print("/");Serial.println(i);
            #endif
            buttonState[o][i] = B_ON ;
          }
          break ;
        ////////////////////////////////
        // etat appuye 
        ////////////////////////////////
        case B_ON : // etat appuye
          if ( o < O_TIRETTE) sAllNoteOff = false ;
          if (vin == LOW)
          {
            // debut relachement
            #if DEBUGON > 3
              Serial.println("B_ON => B_WAIT_OFF ");Serial.print(o);Serial.print("/");Serial.println(i);
            #endif
            buttonState[o][i] = B_WAIT_OFF ;
            ctrlButton(o, i, false);
            sAllKeyStable =  false ;
          }
          break;
        ////////////////////////////////
        // etat stabilisation repos 
        ////////////////////////////////
        case B_WAIT_OFF :
          if ( o < O_TIRETTE) sAllNoteOff = false ;
          sAllKeyStable =  false ;
          if (sinceButton > buttonSince[o][i]) 
          {
            #if DEBUGON > 3
              Serial.println("B_WAIT_OFF => B_OFF ");Serial.print(o);Serial.print("/");Serial.println(i);
            #endif
            buttonState[o][i] = B_OFF ;
          }
          break ;
        ////////////////////////////////
        // etat out of order 
        ////////////////////////////////
        case B_OUT :
          break ;
        default :
            #ifdef DEBUGON
              Serial.println("ERROR etat loop ");Serial.println(buttonState[o][i]);
            #endif
          break ;
      }
    }
    digitalWrite(pinOut[o],HIGH);
    delayMicroseconds(500);
  }

  // antibug ...
  if ( sAllNoteOff && (noteExpresseurOn == 0) && ( newEvent == true) ) 
  {
    allNoteOff();
    newEvent = false ;
  }

  // reset timer button
  if (( sAllKeyStable ) && (sinceButton > 1000)) sinceButton = 0 ;
  
  // led management     
  if ( ! sAllKeyStable) // something is changing
  { 
    digitalWrite(pinLed,HIGH);
    sinceLed = 0 ;
  }
  if ( sinceLed > 5100) // keep alive Off
  { 
    digitalWrite(pinLed,LOW);
    sinceLed = 0 ;
  }
  else if (sinceLed > 5000) // keep alive On
    digitalWrite(pinLed,HIGH);
  else if ( sAllKeyStable ) // everithing is stable
      digitalWrite(pinLed,LOW); 
}
