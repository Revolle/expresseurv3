#define DEBUGON 1

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

void expresseur_loadFile(int fileNr);
void expresseur_play(int pitch, int velo);

typedef struct
{
  int velocity;
  int pitch;
  int trackNr;
  int willStopIndex ; // on note-off
  int stopIndex ; // on note-on
  int eventsStart[MAXEVENTSTOPSTART]; // synchronous start
  int nbEventsStart  ;
  int eventsStop[MAXEVENTSTOPSTART]; // synchronous stop
  int nbEventsStop  ;
} T_event ; // more or less a "chord" to play
typedef struct
{
  T_event events[MAXEVENT];
  int nbEvents  ;
  int trackMax;
} T_score ; // a score is a sequential list of chords
T_score score ;
int noteOffStops[MAXPITCH]; // note of to stop on onte-off

int c_nrEvent_noteOn = 0 ; // index of event for the next note-on 
int end_score = false;

///////////////////////////////////////////////////////////////////////////////
//                       Definition Orgue
///////////////////////////////////////////////////////////////////////////////
#define MAXSPLIT 2
#define VELODEFAULT 100

#define pinLed  13
#define MaxOUT 10
#define MaxIN 9
// Pins Teensy 3.6
byte pinOut[MaxOUT] = {  29, 30, 31, 5 ,  6 ,  7 ,  8,   9  , 14 , 15 } ;
byte pinIn[MaxIN]   = { 36, 35, 33, 34, 16 , 18 , 17 , 19 , 20 };
byte buttonState[MaxOUT][MaxIN];
// two last buttons of pinOUT are for the Tirette
#define O_TIRETTE 8

//etats de l'automate
enum { B_OFF ,B_WAIT_ON , B_ON , B_WAIT_OFF } ;
unsigned long buttonSince[MaxOUT][MaxIN];
elapsedMillis sinceProg;
elapsedMillis sinceLed;
elapsedMillis sinceButton;
byte lastTiretteNr = -1 ;

#define DELAY_NOISE_TIRETTE 500 // ms
#define DELAY_NOISE_KEYBOARD 250 // ms
#define DELAY_DELAY_REPROG 4000 // max ms between two progs, to change the bank

// management of volume
#define VOL_PIANO 15
#define VOL_MESOFORTE 60
#define VOL_DELTA_FORTE 30
byte vol_base = VOL_MESOFORTE ;
byte vol_delta = 0 ;

bool expresseur = false ;
bool gauche = false ;
bool gaucheOn = false;
bool droiteOn = false ;
bool split = false ;
byte psplit = 64 ;
bool octaviaGauche = false ;
bool octaviaDroite = false ;
int midiPitchNbNoteOn[MAXSPLIT][MAXPITCH];
int nbLoop = 0 ;
bool notesOpened = false ;

#define NBCHANNEL 16
#define NBTIRETTE ( 2 * MaxIN )
#define MAXBANK 5

/* 
out/in definition

0/0 Fa
7/8 Mi

8/0 0 voix celeste
8/1 1 clairon
8/2 2 expression
8/3 3 flute
8/4 4 bourdon
8/5 5 hautbois
8/6 6 percussion
8/7 7 baryton
8/8 8 forte

9/0 9 basson
9/1 10 clarinette
9/2 11 forte
9/3 12 tremolo
9/4 13 cor_anglais 
9/5 14 fifre 
9/6 15 percussion
9/7 16 musette
9/8 17 sourdine

*/
// program per tirette, progs[bank[tirette#]][tirette#]
#define DEFAULTPROG 4 
int progs[MAXBANK][NBTIRETTE]=
  {
    {53 ,57, 0,74,20,69, 18,54, 0, 71,72, 0, 0,70,75, 7,21,0},
    {100,58, 0,73,20,65, 16,55, 0, 71,72, 0, 0,70,79, 7,22,0},
    {101,59, 0,76,17,66, 16,86, 0,  0,58, 0, 0, 0,80, 9,23,0},
    {99 ,60, 0,77,17,67,  8,61, 0,  0,58, 0, 0, 0, 0,10,24,0},
    {97 ,61, 0,78,17,68,107, 0, 0,  0,58, 0, 0, 0, 0,11,22,0}
  };
// banks per tirette, banks[tirette#]
int banks[MAXBANK][NBTIRETTE]=
  {
    {  0, 0,-1, 0, 0, 0,  0, 0,-1,  0, 0,-1,-1, 0, 0, 0 ,0,-1},
    {  0, 0,-1, 0, 8, 0,  0, 0,-1,  1, 8,-1,-1, 1, 0, 8 ,0,-1},
    {  0, 0,-1, 0, 0, 0,  1, 0,-1, -1,-1,-1,-1,-1, 0, 0 ,0,-1},
    {  0, 0,-1, 0,40, 0,  0, 0,-1, -1,-1,-1,-1,-1,-1, 0 ,0,-1},
    {  0, 0,-1, 0, 2, 0,  1,-1,-1, -1,-1,-1,-1,-1,-1, 0 ,8,-1}
  };
// status of a prog on a channel  
int prog[NBCHANNEL]={-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
// index of teh selected bank on a channel
int bank[NBCHANNEL]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
bool firstProg = true;


///////////////////////////////////////////////////////////////////////////////
//                       MIDI management
///////////////////////////////////////////////////////////////////////////////

void sendMidiByte(int b)
{
  if (( b >= 0 ) && ( b <= 127))
  {
    Serial1.write((byte)b);
    return ;
  }
#ifdef DEBUGON
  Serial.println("Error byte");
#endif
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
#if DEBUGON == 2
    Serial.print("CTRL #");
    Serial.print(i);
    Serial.print(" ");
    Serial.print(n);
    Serial.print("/");
    Serial.println(v);
#endif
    sendMidiByte(0xB0 | i);
    sendMidiByte(n);
    sendMidiByte(v);
  }
}
void general_volume()
{
  /*
  sendMidiByte(0xB0);
  sendMidiByte(0x63);
  sendMidiByte(0x37);

  sendMidiByte(0xB0);
  sendMidiByte(0x62);
  sendMidiByte(0x7);

  sendMidiByte(0xB0);
  sendMidiByte(0x6);
  sendMidiByte(vol_base + vol_delta);
  */
  sendMidiCtrl(7,(vol_base + vol_delta));
}

void sendProg(boolean on,int nrprog,int nrbank)
{
  if ((nrptog <= 0) || (nrbank == -1)) return ;
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
  int bp = b*128+p; 
  for(int i = i0; i <= i1; i ++)
  {
    if (prog[i] == bp)
      prog[i] = -1 ;
  }
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
#if DEBUGON == 2
    Serial.print("PROG ch#");
    Serial.print(slot);
    Serial.print(" ");
    Serial.print(b);
    Serial.print("/");
    Serial.println(p);
#endif
    prog[slot] = bp ;
    sendMidiByte(0xB0 | slot);
    sendMidiByte(0);
    sendMidiByte(b);
    sendMidiByte(0xC0 | slot);
    sendMidiByte(p);
  }
}
void init_x2()
{
  for(int i = 0; i < MAXSPLIT; i ++)
    for(int j= 0 ; j < MAXPITCH; j ++)
      midiPitchNbNoteOn[i][j] = 0 ;
      
  Serial1.begin(31250);
  delay(200);

  for(byte i = 0; i < 16; i ++)
  {
    sendMidiByte(0xF0);
    sendMidiByte(0x41);
    sendMidiByte(0x00);
    sendMidiByte(0x42);
    sendMidiByte(0x12);
    sendMidiByte(0x40);
    sendMidiByte(0x10 | i );
    sendMidiByte(0x15);
    sendMidiByte(0x00); // sound for all parts
    sendMidiByte(0x00);
    sendMidiByte(0xF7);
  }
  //reste all controlers
  for(byte i = 0; i < 16; i ++)
  {
    sendMidiByte(0xB0 | i);
    sendMidiByte(120);
    sendMidiByte(0);
    sendMidiByte(0xB0 | i);
    sendMidiByte(121);
    sendMidiByte(0);
    sendMidiByte(0xB0 | i);
    sendMidiByte(123);
    sendMidiByte(0);
  }
#ifdef false // DEBUGON
  // test de notes
  for(byte i= 0 ; i < 16; i ++ )
  {
    sendMidiByte(0xB0 | i);
    sendMidiByte(0);
    sendMidiByte(banks[0][i]);
    sendMidiByte(0xC0 | i);
    sendMidiByte(progs[0][i]);
    Serial1.write(0x90 | i);
    Serial1.write(0x40 | i);
    Serial1.write(0x40);
    delay(500);
    Serial1.write(0x80 | i);
    Serial1.write(0x40 | i);
    Serial1.write(0x40);
  }
#endif
  general_volume() ;
  firstProg = true;
  sendProg(true, progs[DEFAULTPROG][0],banks[DEFAULTPROG][0]);
}
void sendMidiNote(int pi,int v, int pspliti)
{
  int i0, i1 ;
  int p = pi ;
  int canal = 0 ;
  if (split)
  {
    if (p < pspliti)
    {
      i0=0; i1=5;
      if ( octaviaGauche )
        p -= 12 ;
      canal = 0 ;
    }
    else
    {
      if ( octaviaDroite )
        p += 12 ;
      i0=6; i1=15;
      canal = 1 ;
    }
  }
  else
  {
    i0=0; i1=15;
  }
  if ( v == 0 )
  {
    (midiPitchNbNoteOn[canal][p]) -- ;
    if (midiPitchNbNoteOn[canal][p] <= 0 )
    {
#if DEBUGON == 2
        Serial.print("NOTE OFF ch#");
        Serial.print(i0);
        Serial.print("..");
        Serial.print(i1);
        Serial.print(" pitch=");
        Serial.println(p);
 #endif
      for(byte i = i0; i <= i1 ; i ++)
      {
        sendMidiByte(0x80 | i);
        sendMidiByte(p);
        sendMidiByte(0x0);
        midiPitchNbNoteOn[canal][p] = 0 ;
      }
    }
  }
  else
  {
    (midiPitchNbNoteOn[canal][p]) ++ ;
    if ((midiPitchNbNoteOn[canal][p] == 1 ) && (notesOpened))
    {
      for(byte i = i0; i <= i1 ; i ++)
      {
        if (prog[i] != -1)
        {
#if DEBUGON == 2
          Serial.print("NOTE ON  ch#");
          Serial.print(i);
          Serial.print(" pitch=");
          Serial.print(p);
          Serial.print(" velo=");
          Serial.println(v);
#endif
          sendMidiByte(0x90 | i);
          sendMidiByte(p);
          sendMidiByte(v);
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
  switch(tiretteNr)
  {
    case  : 17
      // sourdine
      if ( on )
      {
        vol_base = VOL_PIANO  ;
      }
      else
      {
        vol_base = VOL_MESOFORTE  ;
      }
      general_volume() ;
      break ;
    case 8 :
    case 11 :
      // forte
      if (on)
      {
        vol_delta += VOL_DELTA_FORTE ;
      }
      else
      {
        vol_delta -= VOL_DELTA_FORTE ;
      }
      general_volume() ;
      break ;
    case 2 :
      // mode Expresseur
      expresseur = on ;
      if (expresseur)
        expresseur_loadFile(0);
      break ;
    case 12 :
      // tremolo
      sendMidiCtrl(11,(on?127:0));
      break ;
    default :
      // selection instrument program
      if ( on )
      {
        if (firstProg)
        {
          firstProg = false ;
          sendProg(false,progs[DEFAULTPROG][0],banks[DEFAULTPROG][0]);
        }
        // le meme bouton de programme est selectionne plusieurs fois de suite dans un court laps de temps : on change de bank
        if ((lastTiretteNr == tiretteNr) && (sinceProg < DELAY_DELAY_REPROG))
        {
          (bank[tiretteNr]) ++ ; 
          if (( bank[tiretteNr] >= MAXBANK ) || (progs[bank[tiretteNr]][tiretteNr] == -1))
            bank[tiretteNr] = 0 ;
        }
        lastTiretteNr = tiretteNr ;
        sinceProg = 0 ;
      }
      // changement de programme
      sendProg(on,progs[bank[tiretteNr]][tiretteNr],banks[bank[tiretteNr]][tiretteNr]);
      break ;
  }
}
void ctrlNote(int pitch, bool on)
{
  switch(pitch)
  {
    case 33 : 
      gaucheOn= on ;
      if (on)
      {
        if (droiteOn)
          split = false ;
        else
        {
          if ( split )
            octaviaGauche = (! octaviaGauche) ;
          split = true; gauche = true ;
        }
      }
      break ;
    case 93 :         
      droiteOn = on ;
      if (on)
      {
        if (gaucheOn)
          split = false ;
        else
        {
          if ( split )
            octaviaDroite = (! octaviaDroite) ;
          split = true; gauche = false ;
        }
      }
      break ;
    default :
      if ( gaucheOn || droiteOn)
        psplit = pitch ;
      else
        if (expresseur)
        {
          // mode expresseur
          if (pitch >= PITCHFILE)
          {
            expresseur_loadFile(pitch - PITCHFILE);
          }
          else
          {
            expresseur_play(pitch, VELODEFAULT);
          }
        }
        else
        {
          // mode orgue
          sendMidiNote(pitch ,(on?VELODEFAULT:0),psplit);
        }
      break ;
   }
}
void ctrlButton(byte o, byte i, bool on)
{
  if ( o >= O_TIRETTE)
  {
#if DEBUGON == 1
  Serial.print("CtrlButton tirette o=");
  Serial.print(o);
  Serial.print(" i =");
  Serial.print(o);
  Serial.print(" on= ");
  Serial.println(on);
#endif
    // tirettes
    buttonSince[o][i] = sinceButton + DELAY_NOISE_TIRETTE ;
    unsigned int tiretteNr = MaxIN*(o - O_TIRETTE) + i ;
    ctrlTirette( tiretteNr,  on);
  }
  else
  {
#if DEBUGON == 1
  Serial.print("CtrlButton clavier o=");
  Serial.print(o);
  Serial.print(" i =");
  Serial.print(o);
  Serial.print(" on= ");
  Serial.println(on);
#endif

    // clavier
    buttonSince[o][i] = sinceButton + DELAY_NOISE_KEYBOARD ;
    int pitch = 32+(o*MaxIN)+i ;
    ctrlNote(pitch, on);
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
  if (track >= (int)(score.trackMax/2))
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
  outPitch(event->pitch,0,event->trackNr);
}

void startEvent(int velo_in, T_event *event , int nbEvent)
{
  // play an event
  
  if (nbEvent == 0) return;
  
  if ((event->pitch <= 0) || (event->pitch >= 127) || (event->velocity < 2))  return; 
  
  int min_velo_out = 1 ;
  int max_velo_out = 128 ;
  int velo = event->velocity;
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
  // rescale the velocity-in
  int dyn = 127;
  int min_velo_in = 64 - dyn / 2 ;
  int max_velo_in = 64 + dyn / 2 ;
  int v_in = min_velo_in + (( max_velo_in - min_velo_in ) * velo_in ) / 128  ;
  // rescale the velocity out 
  int velo_out = min_velo_out + ((max_velo_out - min_velo_out ) * v_in ) / 128 ;
  // compensation of nbEvent note within one single key-in
  int compensation = 15 ; // [0..30]
  velo_out = ((200 - (compensation * (nbEvent - 1))) * velo_out) / 200;
  // cap the velocity-out
  if (velo_out < 1 ) velo_out = 1 ;
  if (velo_out > 127)  velo_out = 127 ;

  outPitch(event->pitch,velo_out,event->trackNr);
}

void noteOn_Event(int bid , int velo_in)
{
  // the button-id ( bid ) play the current group , and go to the next one
  
  if ( end_score ) return ;

#ifdef DEBUGON
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
#ifdef DEBUGON
      //Serial.print("noteOn_Event to_stop_index=") ; 
      //Serial.print(to_stop_index);
      //Serial.print(" stop_index ");  
      //Serial.println(event_stop->eventsStop[nrEventStop]);
#endif
      stopEvent(event_to_stop);
    }
  }
  if (event->nbEventsStart > 0)
  {
    for (int nrEventStart = 0 ; nrEventStart < event->nbEventsStart ; nrEventStart ++)
    {
#ifdef DEBUGON
      //Serial.print("noteOn_Event start_index ");  Serial.println(event->eventsStart[nrEventStart]);
#endif
      T_event *event_to_start = &(score.events[event->eventsStart[nrEventStart]]);
      startEvent(velo_in , event_to_start ,event->nbEventsStart);
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
#ifdef DEBUGON
    //Serial.print("noteOff_Event toStop=");  Serial.println(toStopIndex);
#endif
    T_event *event = &(score.events[toStopIndex]);
    for (int nrEventStop = 0 ; nrEventStop < event->nbEventsStop ; nrEventStop ++)
    {
#ifdef DEBUGON
        //Serial.print("noteOff_Event #");  Serial.println(event->eventsStop[nrEventStop]);
#endif
        T_event * eventStop = &(score.events[event->eventsStop[nrEventStop]]) ;
        stopEvent(eventStop);
    }
  }
  noteOffStops[bid] = 0 ;
}

void expresseur_play(int pitch, int velo)
{
  if (score.nbEvents == 0) return ;
  
  if (velo == 0) 
  {
  // noteoff
  noteOff_Event(pitch);
  }
  else
  {
  // noteon
  noteOn_Event(pitch , velo);
  }
}

void rewind_score()
{
  c_nrEvent_noteOn = 0 ;// current index for the next note-on 
  end_score = false;
  for(int i = 0 ; i < MAXPITCH ; i ++)
    noteOffStops[i] = 0 ;
  nextGroupEvent();
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
   Serial.print("filename : ");
   Serial.println(fileName);
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
  int *ind ;
  char c ;
  int etat = -1 ;
  int minus = 1 ;
  int index = 0  ;
  while ( myFile.available())
  {
    c = myFile.read();
    switch (etat)
    {
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
           Serial.print("ERROR : event overflow. Index=");
           Serial.println(index);
#endif
            initScore(); 
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
          if (ind == NULL) { initScore(); return;}
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
        if (ind == NULL) { initScore(); return;}
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
#ifdef DEBUGON
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
     Serial.print("last read event#");
     Serial.println(score.nbEvents);
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

  expresseur_loadFile(1);
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
  init_x2();
  delay(200);
  digitalWrite(pinLed,LOW); 
  //tester();
  //return;
#endif
  for(byte i=0;i < 36; i ++)
    pinMode(i, INPUT);     
  Serial.println("init pinoOut");
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
    digitalWrite(pinOut[o],HIGH);
    for(byte i=0;i < MaxIN; i ++)
    {
      buttonState[o][i] = B_OFF;
    }
  }
  nbLoop = 0 ;
  notesOpened = false;
}

///////////////////////////////////////////////////////////////////////////////
//                              LOOP
///////////////////////////////////////////////////////////////////////////////
void loop() 
{
  // to avoid blocked keys
  if (! notesOpened) 
    if (nbLoop < 10))
      nbLoop ++ ;
    else
      notesOpened = true ;

  // scan keys
  bool waitOn = false ;
  for(byte o=0;o < MaxOUT; o ++)
  {
    digitalWrite(pinOut[o],LOW);
    delayMicroseconds(500);
    for(byte i=0;i < MaxIN; i ++)
    {
      bool vin = digitalRead(pinIn[i]) ;
      switch(buttonState[o][i])
      {
        case B_OFF : // etat repos
          if ( vin == LOW)
          {
            // debut appui 
            buttonState[o][i] = B_WAIT_ON ;
            ctrlButton(o, i, true);
          }
          break;
        case B_WAIT_ON :
          waitOn =  true ;
          if ( sinceButton > buttonSince[o][i])
            buttonState[o][i] = B_ON ;
          break ;
        case B_ON : // etat appuye
          if (vin == HIGH)
          {
            // debut relachement
            buttonState[o][i] = B_WAIT_OFF ;
            ctrlButton(o, i, false);
          }
          break;
        case B_WAIT_OFF :
          waitOn =  true ;
          if (sinceButton > buttonSince[o][i]) 
            buttonState[o][i] = B_OFF ;
          break ;
         default :
          break ;
      }
    }
    digitalWrite(pinOut[o],HIGH);
    delayMicroseconds(500);
  }

  // led management
  if ( waitOn == false)
      sinceButton = 0 ;
  if ( waitOn )
  {
    digitalWrite(pinLed,HIGH);
    sinceLed = 0 ;
  }
  // led alive
  if ( sinceLed > 5100)
  {
    digitalWrite(pinLed,LOW);
    sinceLed = 0 ;
  }
  else if (sinceLed > 5000)
    digitalWrite(pinLed,HIGH);
  else if ( waitOn == false )
      digitalWrite(pinLed,LOW); 
}
