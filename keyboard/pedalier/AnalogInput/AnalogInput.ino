/*
  pedalier MIDI, with fore sensor
*/

#define prt Serial.print
#define prtln Serial.println

#define DEBUGMODE 1

// define pins of sensors
#define MAXPIN 1
int sensorPin[MAXPIN] = {A0};
// Max values to measure MIDI velociy
#define MAXV 5
// Trigger to start a MIID event
#define TRIGGER_ON 9 // /10
// delay to avoid debounce
#define MAXT 50 // ms

#define VELOMIN 32
#define VELOMAX 110

struct sensor
{
  int i ;
  int state ;
  int pin ;
  int minv ;
  int maxv ;
  int iv ;
  int sv;
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
  s->maxv = (v * TRIGGER_ON )/ 10  ;
  s->minv = v / 2 ;
}     

void readSensor(sensor *s )
{
  int v ;
  int velo ;
  v = analogRead(s->pin) << 4 ;
  if ( v < s->minv)
    s->minv = v ;
  switch(s->state)
  {
    case 0 :
      if ( v < s->maxv )
      {
        s->iv = 1 ;
        s->sv = v ;
        s->state = 1 ;
        digitalWrite(ledPin, LOW);
#ifdef DEBUGMODE
        prtln("triggeron");
#endif
      }
      break ;
    case 1 :
      s->sv += v ;
      s->iv ++ ;
      if (s->iv > MAXV)
      {
        s->sv /= s->iv ;
        velo = map(s->sv , s->maxv , s->minv , VELOMIN , VELOMAX );
        usbMIDI.sendNoteOn(64+ s->i, velo, 1);
        usbMIDI.send_now();
#ifdef DEBUGMODE
        prtln("NoteOn");
#endif
        s->state = 2 ;
        s->t = 0 ;
      }
      break ;
    case 2 :
      if ( s->t > MAXT)
      {
        s->state = 3 ;
        digitalWrite(ledPin, HIGH);
      }
      break ;
    case 3 :
      if ( v > s->maxv )
      {
#ifdef DEBUGMODE
        prtln("NoteOff");
#endif
        usbMIDI.sendNoteOff(64+ s->i, 64, 1);
        s->t = 0 ;
        s->state = 4 ;
        digitalWrite(ledPin, LOW);
      }
      break ;
    case 4 :
      if ( s->t > MAXT)
      {
        s->state = 0 ;
        digitalWrite(ledPin, HIGH);
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
  analogReadResolution(8);
  // pullup sensorPins
  int i ;
  sensor *s ;
  for(i=0 , s = sensors; i < MAXPIN ; i ++ , s ++)
    initSensor(s, i);
}

void loop() 
{
  int i ;
  sensor *s ;
  for(i=0 , s = sensors; i < MAXPIN ; i ++ , s ++)
    readSensor(s);
}
