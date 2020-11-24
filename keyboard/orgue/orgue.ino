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
#define MAXEVENT 4096
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

#define pinLed  11
#define MaxOUT 10
#define MaxIN 9
byte pinOut[MaxOUT] = { 12 , 13 , 14 , 15 , 16 , 4 , 5 , 6 , 9 , 10 } ;
byte pinIn[MaxIN]   = { 17 , 18 , 19 , 20 , 21 , 0 , 1 , 2 , 3 };
byte buttonState[MaxOUT][MaxIN];
unsigned long buttonSince[MaxOUT][MaxIN];
//etats de l'automate
enum { B_OFF ,B_WAIT_ON , B_ON , B_WAIT_OFF } ;

elapsedMillis sinceLed;
elapsedMillis sinceButton;
elapsedMillis sinceProg;
byte lastProg = -1 ;

#define DELAY_NOISE_BUTTON 500 // ms
#define DELAY_NOISE_KEYBOARD 250 // ms
#define DELAY_DELAY_REPROG 4000 // max ms between two progs, to change the bank
// two last buttons of pinOUT are for the CONTROL BUTTON
#define O_BUTTON_1 8
#define O_BUTTON_2 9

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

#define NBCHANNEL 16

#define MAXBANK 3
// program per channel, progs[bank[channel#]][channel#]
int progs[MAXBANK][NBCHANNEL]=
  {
    {17,17,18,20,21,53,57,58,69,71,72,74,76,89,90,99 },
    {17,17,18,20,22,54,57,59,70,71,72,73,77,83,95,100},
    {17,17,18,19,22,55,57,61,70,59,58,75,78,85,96,101}
  };
// banks per channel, banks[bank[channel#]][channel#]
byte banks[MAXBANK][NBCHANNEL]=
  {
    { 0,40, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    { 1,33, 1, 8, 0, 0, 1, 0, 0, 1, 8, 0, 0, 0, 0, 0},
    { 2, 9, 2, 0, 8, 0, 8, 0, 1, 0, 0, 0, 0, 0, 0, 0}
  };
// status of a prog on a channel  
int prog[NBCHANNEL]={-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
// index of teh selected bank on a channel
int bank[NBCHANNEL]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

///////////////////////////////////////////////////////////////////////////////
//                       MIDI management
///////////////////////////////////////////////////////////////////////////////

void sendMidiByte(byte b)
{
  Serial.write(b);
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
#ifdef DEBUGON
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
void init_x2()
{
  for(int i = 0; i < MAXSPLIT; i ++)
    for(int j= 0 ; j < MAXPITCH; j ++)
      midiPitchNbNoteOn[i][j] = 0 ;
      
  Serial1.begin(31250);
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
  general_volume() ;
}
void sendProg(boolean on,byte nrprog,byte nrbank)
{
  byte p = constrain(nrprog, 0, 127);
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
  for(int i = i0; i <= i1; i ++)
  {
    if (prog[i] == p)
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
#ifdef DEBUGON
    Serial.print("PROG #");
    Serial.print(slot);
    Serial.print(" ");
    Serial.print(p);
#endif
    prog[slot] = p ;
    sendMidiByte(0xB0 | slot);
    sendMidiByte(0);
    sendMidiByte(b);
    sendMidiByte(0xC0 | slot);
    sendMidiByte(p);
  }
}
void sendMidiNote(byte pi,byte v, byte pspliti)
{
  byte i0, i1 ;
  byte p = pi ;
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
      for(byte i = i0; i <= i1 ; i ++)
      {
#ifdef DEBUGON
        Serial.print("NOTE OFF #");
        Serial.print(i);
        Serial.print(" ");
        Serial.print(p);
        Serial.print("/");
        Serial.println(v);
#endif
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
    if (midiPitchNbNoteOn[canal][p] == 1 )
    {
      for(byte i = i0; i <= i1 ; i ++)
      {
        if (prog[i] != -1)
        {
#ifdef DEBUGON
          Serial.print("NOTE ON  #");
          Serial.print(i);
          Serial.print(" ");
          Serial.print(p);
          Serial.print("/");
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

void ctrlButton(byte o, byte i, bool on)
{
  if (( o == O_BUTTON_1 ) || ( o == O_BUTTON_2))
  {
    // tirettes
    buttonSince[o][i] = sinceButton + DELAY_NOISE_BUTTON ;

    if (( o == O_BUTTON_2 ) && ( i == 0 ))
    {
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
    }
    else if ((( o == O_BUTTON_1 ) && ( i == 0 )) || (( o == O_BUTTON_2 ) && ( i == 5 )))
    {
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
    }
    else if (( o == O_BUTTON_1 ) && ( i == 5 ))
    {
      // mode Expresseur
      expresseur = on ;
      if (expresseur)
        expresseur_loadFile(0);
    }
    else if (( o == O_BUTTON_2 ) && ( i == 6 ))
    {
      // tremolo
      sendMidiCtrl(11,(on?127:0));
    }
    else
    {
      // selection instrument program
      byte nrcontrol = constrain((o - O_BUTTON_1)*9 + i,0,NBCHANNEL - 1 ) ;
      if ( on )
      {
        // le meme bouton de programme est selectionne plusieurs fois de suite dans un court laps de temps : on change de bank
        if ((lastProg == nrcontrol) && (sinceProg < DELAY_DELAY_REPROG))
        {
          (bank[nrcontrol]) ++ ; 
          if (( bank[nrcontrol] >= MAXBANK ) || (progs[bank[nrcontrol]][nrcontrol] == -1))
            bank[nrcontrol] = 0 ;
        }
        lastProg = nrcontrol ;
        sinceProg = 0 ;
      }
      // changement de programme
      sendProg(on,progs[bank[nrcontrol]][nrcontrol],banks[bank[nrcontrol]][nrcontrol]);
    }
  }
  else
  {
    // clavier
    buttonSince[o][i] = sinceButton + DELAY_NOISE_KEYBOARD ;
    byte pitch = 32+(o*9)+i ;
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
  
  T_event *event = &(score.events[c_nrEvent_noteOn]);
  
  int to_stop_index = event->stopIndex ;
  if (to_stop_index > 0)
  {
    T_event *event_stop = &(score.events[to_stop_index]) ;
    for (int nrEvent = 0 ; nrEvent < event_stop->nbEventsStop ; nrEvent ++)
  {
      T_event *event_to_stop = &(score.events[nrEvent]);
      stopEvent(event_to_stop);
    }
  }
  if (event->nbEventsStart > 0)
  {
    for (int nrEvent = 0 ; nrEvent < event->nbEventsStart ; nrEvent ++)
  {
      T_event *event_to_start = &(score.events[nrEvent]);
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
    T_event *event = &(score.events[toStopIndex]);
    for (int nrEvent = 0 ; nrEvent < event->nbEventsStop ; nrEvent ++)
  {
        T_event * eventStop = &(score.events[nrEvent]) ;
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
  c_nrEvent_noteOn = 1 ;// current index for the next note-on 
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
  if ( !SD.exists(fileName)) return ;
  
  // open file
  File myFile = SD.open(fileName);
  if (! myFile) return ;

  // Groups of 3 lines :
  //     1st line with nt pitch, int velocity,  int trackNr , int stopIndex, int willStopIndex
  //     2nd line with int eventsStart, ..
  //     3rd line with int eventsSttop, ..
  initScore() ;
  T_event *event = &(score.events[1]);
  int *ind ;
  char c ;
  int etat = 0 ;
  while ( (c=myFile.read()) != -1)
  {
    switch (etat)
    {
      case 0 : //pitch
      if ((c >= '0') && (c <= '9'))
      {
        if (event == NULL) { initScore() ; return ;} 
        event->pitch = 10* event->pitch + (c - '0') ;
        break ;
      }
      if (c == ',')
        etat = 1 ; 
      break ;
      case 1 : //velocity
      if ((c >= '0') && (c <= '9'))
      {
        event->velocity = 10* event->velocity + (c - '0') ;
        break ;
      }
      if (c == ',')
        etat = 2 ; 
      break ;
      case 2 : //track
      if ((c >= '0') && (c <= '9'))
      {
        event->trackNr = 10* event->trackNr + (c - '0') ;
        break ;
      }
      if (c == ',')
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
      if (c == ',')
        etat = 4 ;
      break ;
      case 4 : //willStopIndex
      if ((c >= '0') && (c <= '9'))
      {
        event->willStopIndex = 10* event->willStopIndex + (c - '0') ;
        break ;
      }
      if (c == '\n')
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
      if ((c == ',') || (c == '\n'))
      {
        (event->nbEventsStart) ++ ;
        if ( event->nbEventsStart < MAXEVENTSTOPSTART)
          ind = &(event->eventsStart[event->nbEventsStart]); 
        else
          ind = NULL ;
        if (c == '\n')
        {
          etat = 11 ;
          event->nbEventsStop = 0 ;
          ind = &(event->eventsStop[0]);
        }         
      }
      break ;
      case 11 : //eventsStop
      if ((c >= '0') && (c <= '9'))
      {
        if (ind == NULL) { initScore(); return;}
        *ind = 10* (*ind) + (c - '0') ;
        break ;
      }
      if ((c == ',') || (c == '\n'))
      {
        (event->nbEventsStop) ++ ;
        if ( event->nbEventsStop < MAXEVENTSTOPSTART)
          ind = &(event->eventsStop[event->nbEventsStop]);
        else
          ind = NULL ;
        if (c == '\n')
        {
          etat = 0 ;
          (score.nbEvents) ++ ;
          if ( score.nbEvents < MAXEVENT)
            event = &(score.events[score.nbEvents]);
          else
            event = NULL ;
        }         
      }
      break ;
      default : etat = 0 ; break ;
    }
  }
  myFile.close();
  rewind_score() ;
}

///////////////////////////////////////////////////////////////////////////////
//                             SETUP
///////////////////////////////////////////////////////////////////////////////
void setup() 
{
  pinMode(pinLed, OUTPUT);
  for(byte o=0;o < MaxOUT; o ++)
    pinMode(pinOut[o], OUTPUT);
  for(byte i=0;i < MaxIN; i ++)
    pinMode(pinIn[i], INPUT_PULLUP);     
  for(byte o=0;o < MaxOUT; o ++)
  {
    digitalWrite(pinOut[o],HIGH);
    for(byte i=0;i < MaxIN; i ++)
    {
      buttonState[o][i] = B_OFF;
    }
  }
#ifdef DEBUGON
  Serial.begin(9600);
#endif
  init_x2();
}

///////////////////////////////////////////////////////////////////////////////
//                              LOOP
///////////////////////////////////////////////////////////////////////////////
void loop() 
{
  bool waitOn = false ;
  
  for(byte o=0;o < MaxOUT; o ++)
  {
    digitalWrite(pinOut[o],LOW);
    for(byte i=0;i < MaxIN; i ++)
    {
      switch(buttonState[o][i])
      {
        case B_OFF : // etat repos
          if (digitalRead(pinIn[i]) == LOW)
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
          if (digitalRead(pinIn[i]) == HIGH)
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
