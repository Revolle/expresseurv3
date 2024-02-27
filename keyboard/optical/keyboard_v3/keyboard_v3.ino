#include <EEPROM.h>
#include <ADC.h>
#include <ADC_util.h>

#define prt Serial.print
#define prtln Serial.println

#define DEBUGMODE 3
#define PINLED 13

// optical button
#define opticalNb 3  // nb optical button
#define SLOPEMIN 0.0001 // slope for minimum velocity
#define SLOPEMAX 0.025 // slope for maximum velocity
struct T_optical {
  uint8_t nr;             // incremntal number of the optical button
  uint8_t pin;            // analog pin to read ( 3.1V max )
  uint8_t v;              // value of the analog read 8 bits
  uint8_t triggerMin, triggerMax;  // min max of the trigger applied on v
  unsigned long int ftv;  // time of the analog convertion
  unsigned long int ftv0; // time of the beginning of the press
  float sumt , sumv , sumtv , sumt2 , fnb , slope ; // linear regression for the slope
  uint8_t state;                   // state of the optical button
  uint8_t pitch;                   // pitch sent on midiOn
};
T_optical optical[opticalNb];  // optical buttons
uint8_t opticalPin[opticalNb] = { A4, A3, A2 };  // analog pins for the optical button
void opticalAdcPreset(T_optical *o);
elapsedMicros opticalSince;  // timer to measure the slope
float veloMin = 10.0 ; // velocity minimum ( velocity maximum = 128 - veloMin)

// mechanical button
#define buttonNb 4  // nb button
struct T_button {
  uint8_t nr;     // incremntal number of the mechanical button
  uint8_t pin;    // digital pin to read
  uint8_t state;  // state of the button
  unsigned long int ftv0; // time of the beginning of the press
};
T_button button[buttonNb]; // mechanical buttons
uint8_t buttonPin[buttonNb] = {10, 6,7  , 9 };  // digital pin for mechanical buttons
elapsedMillis buttonSince; // timer to measure the states
#define VMIN 16 // volume min
#define VMAX (128-VMIN) // volume max
#ddefine VMID (128/2) // volume medium
#define VINC 16 // increment of the volume by the mechanical buttons 2 3
int volume = VMID ; // volume tuned-sent by the two mechanical buttons 2 3

// led
#define ledNb 2                         // nb led
uint8_t ledPin[ledNb] = { 14, 15 };     // digital pins for the led
uint8_t ledChannelOn;                   // reminder of the ledChannel which switched on the leds
uint8_t ledValue, ledFormerValue;       // value of the led bargraph

uint8_t adcState;  // state of presets in the ADC
enum ADCSTATE { adcNothing,
                adcOptical };  // ends with adcOptical !!

ADC *adc = new ADC(); // analog to digital converter

elapsedMillis sinceOnboardLed; // time elapsed for the onboard led

#define MidiChannelOut 1 // channel MIDI used for output


#define CONF_MAGIC 20 // key to detect if EEPROM has been set
#define CONF_ADDR_VMIN 4 // address EEPROM for the VMIN setting

///////////////
// read conf from EEPROM
//////////////
void confRead() {
  byte value ;
  for(uint8_t i = 0 ; i < CONF_ADDR_VMIN ; i++ ) {
    value = EEPROM.read(i) ;
    if (value != CONF_MAGIC) {
      veloMin = VMIN ;
      return;
    }
  }
  veloMin = EEPROM.read(CONF_ADDR_VMIN) ;
}
void confWrite() {
  byte value ;
  for(uint8_t i = 0 ; i < CONF_ADDR_VMIN ; i++ ) 
    EEPROM.write(i,CONF_MAGIC) ;
  EEPROM.write(CONF_ADDR_VMIN,(byte)(veloMin)) ;
}

///////////////
// On board Led
///////////////
void ledOnboardFlash() {
  digitalWrite(PINLED, HIGH);
  sinceOnboardLed = 0;
}
void ledOnboardProcess() {
  // keep alive led
  if (sinceOnboardLed > 800)
    digitalWrite(PINLED, HIGH);
  if (sinceOnboardLed > 805) {
    digitalWrite(PINLED, LOW);
    sinceOnboardLed = 500;
  }
}
void ledOnboardInit() {
  pinMode(PINLED, OUTPUT);
}

// Led
//////
void ledSet(uint8_t v, uint8_t ledChannel) {
  // Set the status of the led. v [0..127]
  if (v == 0) {
    if ((ledChannel != ledChannelOn) || (ledChannel != 0))
      ledValue = 0;
    return;
  }
  ledValue = v;
  ledChannelOn = ledChannel;
}
void ledShow() {
  // show the status of the leds with a bargraph for  0,[1..127]
  if (ledFormerValue == ledValue)
    return;
  ledFormerValue = ledValue;
#ifdef DEBUGMODE
   prt("led=");
   prt(ledValue);
#endif
  uint8_t nr;
  if (ledValue == 0) {
    for (nr = 0; nr < ledNb; nr++)
      analogWrite(ledPin[nr], 0);
#ifdef DEBUGMODE
   prtln(" off");
#endif
    return;
  }
  float fv = ledValue ;
  float fled ;
  int iled ;
  float flim ;
  flim = 127.0 * 2.0 / 3.0 ;
  if (fv < flim ) {
    fled = 255.0 * pow ((fv  - flim ) / ( 127.0 - flim )  , 2 ) ;
    analogWrite(ledPin[0], fled);
#ifdef DEBUGMODE
   prt(" led#0=");
   prtln((int)fled);
#endif
  }
  else {
#ifdef DEBUGMODE
   prt("led#0=off");
#endif
    analogWrite(ledPin[0], 0);
  }
  flim = 127.0 * 1.0 / 3.0 ;
  if (fv > flim ) {
    fled = 255.0 * pow ((fv  - flim ) / ( 127.0 - flim )  , 2 ) ;
    analogWrite(ledPin[1], fled);
#ifdef DEBUGMODE
   prt(" led#1=");
   prtln((int)fled);
#endif
  }
  else {
#ifdef DEBUGMODE
   prt("led#1=off");
#endif
    analogWrite(ledPin[1], 0);
  }
}
void ledAlert() {
  // all led on
  for (uint8_t nr = 0; nr < ledNb; nr++)
    analogWrite(ledPin[nr], 200);
}
void ledFlash(uint8_t d) {
  uint8_t nr;
  for (nr = 0; nr < 10; nr++) {
    digitalWrite(PINLED, HIGH);
    analogWrite(ledPin[1], 0);
    analogWrite(ledPin[0], 255);
    delay(d);
    digitalWrite(PINLED, LOW);
    analogWrite(ledPin[0], 0);
    analogWrite(ledPin[1], 255);
    delay(d);
  }
  for (nr = 0; nr < ledNb; nr++)
    analogWrite(ledPin[nr], 0);
}
void ledInit() {
  uint8_t nr;
  ledChannelOn = 0;
  ledFormerValue = 1;
  ledValue = 0;
  for (nr = 0; nr < ledNb; nr++)
    analogWrite(ledPin[nr], 0);
}

void midiNote(uint8_t p, uint8_t v) {
  // send msg MIDI NoteOn
  if (v == 0)
    usbMIDI.sendNoteOff(p, 0, MidiChannelOut);
  else
    usbMIDI.sendNoteOn(p, v, MidiChannelOut);

  usbMIDI.send_now();

  ledSet(v, p);
}
void midiControl(uint8_t c, uint8_t v) {
  // send msg MIDI Control
  usbMIDI.sendControlChange(c, v, MidiChannelOut);
  usbMIDI.send_now();
}

// buttons
//////////
void buttonInit() {
  uint8_t i;
  T_button *b;
  for (i = 0, b = button; i < buttonNb; i++, b++) {
    b->nr = i;
    b->pin = buttonPin[i];
    pinMode(b->pin, INPUT_PULLUP);
    b->state = 0;
  }
  volume = VMID ;
}
void buttonVolume(T_button *b) {
#ifdef DEBUGMODE
   prt("button#");
   prtln(b->nr);
#endif
bool shiftButton = false ;
  switch(b->nr) {
    case 2 :
      if (button[3].state > 0) {
        veloMin += VINC ;
        shiftButton = true ;
      }
      else
        volume += VINC ;
      break ;
    case 3 :
      if (button[2].state > 0) {
        veloMin -= VINC ;
        shiftButton = true ;
      }
      else
        volume -= VINC ;
      break ;
    default : 
      return ;
      break ;
  }
  if ( shiftButton ) {
    if ( veloMin < VMIN)
        veloMin = VMIN ;
    if ( veloMin > 64.0)
        veloMin = 64.0  ;
    confWrite();
#ifdef DEBUGMODE
     prt("veloMin=");
     prtln(veloMin);
#endif
  }
  else {
    if ( volume < VMIN)
        volume = VMIN ;
    if ( volume > VMAX)
        volume = VMAX ;
    midiControl(7, volume);
#ifdef DEBUGMODE
     prt("volume=");
     prtln(volume);
#endif
  }
}
void buttonProcess() {
  uint8_t nr;
  bool allOff ;
  T_button *b;
  for (nr = 0, b = button , allOff = true ; nr < buttonNb; nr++, b++) {
    if (b->state != 0)
      allOff = false ;
    switch (b->state) {
      case 0:
        if (digitalRead(b->pin) == LOW) {
          b->state = 1;
          midiNote(nr + 1, 127);
          buttonVolume(b);
          b->ftv0 = buttonSince;
          ledOnboardFlash();
        }
        break;
      case 1:
        if ((buttonSince - b->ftv0) > 250)  // ms
          b->state = 2;
        break;
      case 2:
        if (digitalRead(b->pin) == HIGH) {
          b->state = 3;
          midiNote(nr + 1, 0);
          b->ftv0 = buttonSince;
        }
        break;
      case 3:
        if ((buttonSince - b->ftv0) > 250)  // ms
          b->state = 0;
        break;
      default:
        // state zombi
        b->state = 0;
        prtln("Error state button");
        break;
    }
  }
  if (allOff)
    buttonSince = 0;
}

////////////
// optical
////////////
void opticalAdcPreset(T_optical *o) {
  // prepare two analog convertions
  adc->adc0->startSingleRead(o->pin);
  o->ftv = opticalSince;
  if ( o->nr < (opticalNb - 1))
    adc->adc1->startSingleRead((o + 1)->pin);
  (o + 1)->ftv = opticalSince;
  adcState = adcOptical + o->nr;
}
void opticalAdcRead(T_optical *o) {
  
  bool err = false;

  // control status of adc
  if (adcState != (adcOptical + o->nr)) {
    err = true;
#ifdef DEBUGMODE
    prt("Error opticalAdcPreset [");
    prt(o->nr);
    prt("] = ");
    prtln(adcState);
#endif
  }
  if (!(adc->adc0->isConverting() || adc->adc0->isComplete())) {
    err = true;
#ifdef DEBUGMODE
    prt("Error opticalAdcPreset [");
    prt(o->nr);
    prt("] adc0 not in progress");
#endif
  }
  if ( o->nr < (opticalNb - 1)) {
    if (!(adc->adc1->isConverting() || adc->adc1->isComplete())) {
      err = true;
  #ifdef DEBUGMODE
      prt("Error opticalAdcPreset [");
      prt(o->nr);
      prt("] adc1 not in progress");
  #endif
    }
  }
  if (err) {
    opticalAdcPreset(o);  // error in preset ADC ...!!!
  }

  // read adc0
  while (!adc->adc0->isComplete())
    ;
  o->v = adc->adc0->readSingle();

  if ( o->nr < (opticalNb - 1)) {
    // read adc1
    while (!adc->adc1->isComplete())
      ;
    (o + 1)->v = adc->adc1->readSingle();
  }

  opticalAdcPreset(((o->nr) < (opticalNb -2)) ? (optical + 2) : (optical));  // prepare next two adc
}
void opticalInit() {
  // initialization of optical buttons
  uint8_t nr;
  unsigned long int n;
  T_optical *o;
  bool modulo2;
  for (nr = 0, o = optical; nr < opticalNb; nr++, o++) {
    o->nr = nr;
    o->pin = opticalPin[nr];
    pinMode(o->pin, INPUT_DISABLE);  // ?
    o->state = 0;
    o->triggerMax = 255;
  }
  opticalAdcPreset(optical);
  for (n = 0; n < 5000; n++) {
    for (nr = 0, o = optical, modulo2 = true; nr < opticalNb; nr++, o++, modulo2 = !modulo2) {
      if (modulo2)
        opticalAdcRead(o);  // read adc0 and adc1, and prepare next ones. Unsigned 8 bits [0..2^8]
      if (o->v < o->triggerMax)
        o->triggerMax = o->v;
    }
  }
  for (nr = 0, o = optical; nr < opticalNb; nr++, o++) {
    if (o->triggerMax > 254){
    // saturation electrique potentiellement dangereuse sur l'entree ADC
#ifdef DEBUGMODE
      prtln("Saturation opticalAdc !!");
#endif
      ledAlert();
    }
    o->triggerMax = 80 * o->triggerMax / 100;
    o->triggerMin = 20 * o->triggerMax / 100;
#ifdef DEBUGMODE
    prt("trigger[");
    prt(nr);
    prt("] : ");
    prt(o->triggerMin);
    prt(" .. ");
    prtln(o->triggerMax);
#endif
  }
  // prepare to read the two first optical sensors
  opticalAdcPreset(optical);
}
void opticalMsgOn(T_optical *o) {
  // send notOn with calculated velocity
  float v;
  o->pitch = o->nr + buttonNb + 1;
  v = veloMin + (o->slope - SLOPEMIN) * ((128.0 - veloMin) - veloMin) / (SLOPEMAX - SLOPEMIN);
#if DEBUGMODE > 2
  prt("slope=");
  prt(veloMin,1);
  prt("=");
  prt(SLOPEMIN,8);
  prt("..");
  prt(o->slope,8);
  prt("=");
  prt(v,1);
  prt("..");
  prt(SLOPEMAX,8);
  prt("=");
  prt((128.0 - veloMin),1);
  prt(" / ");
  prt(o->fnb);
  prtln("ADC");
#endif
#ifdef DEBUGMODE
  char s[512];
  for(uint8_t n = 0 ; n < (uint8_t)(v) ; n++ )
    s[n] = '=';
  s[v]='*';
  s[v+1] = '\0';
  prt(v);
  prtln(s);
#endif
  if (v > (128.0 - veloMin)) v = (128.0 - veloMin);
  if (v < veloMin) v = veloMin;
  midiNote(o->pitch, (uint8_t)(v));
}
void opticalMsgOff(T_optical *o) {
  // send noteOff
  if (o->pitch > 0)
    midiNote(o->pitch, 0);
}
bool opticalProcess() {
  // scan optical buttons
  // return true if an optical slope-measurement is in progress
  uint8_t nr;
  bool modulo2;
  bool opticalMeasure, allOff;
  unsigned long int dt ;
  T_optical *o;
  for (nr = 0, o = optical, modulo2 = true , allOff = true , opticalMeasure = false ; nr < opticalNb; nr++, o++, modulo2 = !modulo2) {

    if (modulo2)
      opticalAdcRead(o);  // read adc0 and adc1, and prepare next ones. Unsigned 8 bits [0..2^8]

    if (o->state > 0)
      allOff = false;

    switch (o->state) {
      case 0:
        if (o->v < o->triggerMax) {
          allOff = false;
          o->state++;
          o->ftv0 = o->ftv;
          opticalMeasure = true;
          ledOnboardFlash();
          // linear regression init
          o->sumt = 0 ;
          o->sumv = o->v ;
          o->sumtv = 0.0 ;
          o->sumt2 = 0.0  ;
          o->fnb = 1.0 ;
        }
        break;
      case 1:
        opticalMeasure = true;
        if (o->v < o->triggerMin) {
          o->state++;
          // linear regression ended
          float d = o->fnb * o->sumt2 - o->sumv * o->sumv;
          o->slope = abs (o->fnb * o->sumtv - o->sumt * o->sumv ) / d;
          opticalMsgOn(o);
          o->ftv0 = opticalSince;
          break;
        }
        if (o->v > o->triggerMax) {
          // bad alert
          o->state = 0;
          break;
        }
        // linear regression progress
        dt = o->ftv - o->ftv0 ;
        o->sumt += (float)dt;
        o->sumv += o->v ;
        o->sumtv += (float)(dt) * (float)(o->v) ;
        o->sumt2 += (float)(dt) * (float)(dt) ;
        o->fnb += 1.0 ;
        break;
      case 2:
        if ((opticalSince - o->ftv0) > 100 * 1000)  // ms
          o->state++;
        break;
      case 3:
        if (o->v > o->triggerMax) {
          o->state++;
          opticalMsgOff(o);
          o->ftv0 = o->ftv;
        }
        break;
      case 4:
        if ((opticalSince - o->ftv0) > 100 * 1000)  // ms
          o->state = 0;
        break;
      default:
        // state zombi
        o->state = 0;
        prtln("Error optical button");
        break;
    }
  }
  if (allOff) {
    // no activity on optical : reset µsecond counter
    opticalSince = 0;
  }

  // => opticalMeasure is false 
  return opticalMeasure;
}

////////
// ADC
////////
void adcInit() {
  adc->adc0->setResolution(8);  // unsigned 8 bits [0..2^8]
  adc->adc0->setAveraging(4);
  adc->adc0->setConversionSpeed(ADC_CONVERSION_SPEED::HIGH_SPEED);
  adc->adc0->setSamplingSpeed(ADC_SAMPLING_SPEED::HIGH_SPEED);
  adc->adc1->setResolution(8);
  adc->adc1->setAveraging(4);
  adc->adc1->setConversionSpeed(ADC_CONVERSION_SPEED::HIGH_SPEED);
  adc->adc1->setSamplingSpeed(ADC_SAMPLING_SPEED::HIGH_SPEED);
  adcState = adcNothing;
}
////////////
// SETUP
///////////
void setup() {
  confRead();
  ledOnboardInit();
  ledInit();
  
  adcInit();
  buttonInit();
  opticalInit();
  since = 0;  // ms
  ledFlash(20);
}

///////////
// LOOP
//////////
void loop() {
  if (!opticalProcess()) {
    // no optical slope-measurement in progress
    // process buttons
    buttonProcess();
    // show the bargraph on the leds
    ledShow();
    // led onboard
    ledOnboardProcess();
  }
}
