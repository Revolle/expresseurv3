/*
  pedalier MIDI, with fore sensor
*/

#define prt Serial.print
#define prtln Serial.println

//#define DEBUGMODE 1

// define pins of sensors
#define MAXPIN 1
int sensorPin[MAXPIN] = {A0};
// Max values to measure MIDI velociy
#define MAXV 500
// Trigger to start a MIID event
#define TRIGGER_ON 9 // /10
// delay to  debounce
#define MAXT 5 // ms
// delay to measure average
#define MAXTV 50 // ms

#define VELOMIN 32
#define VELOMAX 110

struct sensor
{
  int i ;
  int state ;
  int pin ;
  int von, voff, vmin ;
  int v;
  elapsedMillis t;
} ;

sensor sensors[MAXPIN] ;
int sensorTriggerOn =  9  ; // /10 to trigger on
int ledPin = 13;      // select the pin for the LED

void initSensor(sensor *s , int i )
{
  int v ;
  s->i = i ;
  s->pin = sensorPin[i];
  s->state = 0 ;
  pinMode(s->pin, INPUT_PULLUP);
  v = analogRead(s->pin) ;
  s->von = (v * 8 )/ 10  ;
  s->voff = (v * 9 )/ 10  ;
  s->vmin = v / 2 ;
}     

void readSensor(sensor *s )
{
  int v ;
  int velo ;
  v = analogRead(s->pin) ;
  if ( v < s->vmin)
    s->vmin = v ;
  switch(s->state)
  {
    case 0 :
      if ( v < s->von )
      {
        s->v = v ;
        s->state = 1 ;
        digitalWrite(ledPin, HIGH);
        s->t = 0 ;
#ifdef DEBUGMODE
        prtln("triggeron");
#endif
      }
      break ;
    case 1 :
      s->v = (s->v + v ) /2 ;
      if (s->t > MAXTV)
      {
        velo = map(s->v , s->von , s->vmin , VELOMIN , VELOMAX );
        usbMIDI.sendNoteOn(64+ s->i, velo, 1);
        usbMIDI.send_now();
        s->state = 2 ;
        s->t = 0 ;
 #ifdef DEBUGMODE
        for(int i = 0 ; i < velo ; i += 5)
          prt("*");
        prt(" NoteOn=");
        prt(velo);
        prt(" sv=");
        prt(s->v);
        prt(" svon=");
        prt(s->von);
        prt(" svmin=");
        prtln(s->vmin);
#endif
      }
      break ;
    case 2 :
      if ( s->t > MAXT)
      {
        s->state = 3 ;
        digitalWrite(ledPin, LOW);
      }
      break ;
    case 3 :
      if ( v > s->voff )
      {
#ifdef DEBUGMODE
        prtln("                             NoteOff");
#endif
        usbMIDI.sendNoteOff(64+ s->i, 64, 1);
        s->t = 0 ;
        s->state = 4 ;
        digitalWrite(ledPin, HIGH);
      }
      break ;
    case 4 :
      if ( s->t > MAXT)
      {
        s->state = 0 ;
        digitalWrite(ledPin, LOW);
      }
      break ;
    default :
      break ;
  }
}
void setup() 
{
  // declare the ledPin as an OUTPUT:
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, HIGH);
  analogReadResolution(8);
  prtln("init");
  // pullup sensorPins
  int i ;
  sensor *s ;
  for(i=0 , s = sensors; i < MAXPIN ; i ++ , s ++)
    initSensor(s, i);
  for(i=0;i< 3; i ++)
  {
    delay(500);
    digitalWrite(ledPin, HIGH);
    delay(500);
    digitalWrite(ledPin, LOW);
  }
}

void loop() 
{
  int i ;
  sensor *s ;
  for(i=0 , s = sensors; i < MAXPIN ; i ++ , s ++)
    readSensor(s);
}
