/*
MIDI keyboard
Optical sensors : MIDI-velocity depends on sensor slope-value. It sends a MIDI-out note message
Mechanical sensor : pullu-up buttons to Midi-out note message
MIDI-thru : USB-MidiIn => serial-midi-out 
Tuning : maintaining an optical sensor on, and clicking 5 time on another optical sensor : the next click on this last optical sensor will tune a parameter ( resp. velocity min, curve ;, nothing, .. )
On board led : flashes on various events ( onStart, MIDI-In, Midi-out , nb_click_tuning )
*/
#include <EEPROM.h>
#include <ADC.h>
#include <ADC_util.h>

#define prt Serial.print
#define prtln Serial.println

#define DEBUGMODE 3

#define ONBOARD_LED_PIN 13
bool onBoardLedOn = false ;
#define FLASH_DT 500 // ms  flash

// optical button
#define opticalNb 2      // nb optical button
#define ADC_RESOLUION 8 // bits
#define ADC_MAX 255 // max value delivered by ADC
#define ADC_GAP_MIN 40 // Min gap between min-max to start the analysis
#define ADC_TRIGGER_PERCENT 0.2 // limit ofr the riggers over min and max
#define ADC_POSITIVE true // true if the sensor value increases while pressng the optical button (else false)
#define ADC_DELAY 100 // ms to debounce states
#define MIN_CLICK_TUNING 3 // nb of click to enter in status mode (while another button is on)
struct T_optical {
  uint8_t nr;                                  // incremntal number of the optical button
  uint8_t pin;                                 // analog pin to read ( 3.1V max )
  float v , pv ;                                     // value of the analog read 8 bits
  float v_min, v_max;                          // min max of analog read v
  float v_trigger_min, v_trigger_max;          // min max of the trigger applied on v
  float slope_min, slope_max ;                 // min max of the slope
  unsigned long int ftv;                       // time of the analog convertion
  unsigned long int ftv0;                      // time of the beginning of the press
  float sumt, sumv, sumtv, sumt2, fnb, slope;  // linear regression for the slope
  uint8_t state;                               // state of the optical button
  uint8_t pitch;                               // pitch sent on midiOn
  uint8_t nb_click_tuning ;                    // count number of click while another button is maintained on
  uint8_t ccMidi ;                             // Control change 
};
T_optical optical[opticalNb];                    // optical buttons
uint8_t opticalPin[opticalNb] = { A3, A2 };      // analog pins for the optical button
void opticalAdcPreset(T_optical *o);
elapsedMicros opticalSince;  // timer to measure the slope
int veloMin = 10;        // velocity minimum ( velocity maximum = 128 - veloMin) 
#define CURVEMIN 0.4
#define CURVEMAX 1.5
float veloCurve = 0.5;  // velocity curve [CURVEMIN..CURVEMAX]
#define DV_CONTROLCHANGE 2 // minimum Dv to send a Control-Change
uint8_t controlMidi[opticalNb] = { 1, 4 };      // Control change for each optical button

// mechanical button
#define buttonNb 1  // nb button
struct T_button {
  uint8_t nr;               // incremntal number of the mechanical button
  uint8_t pin;              // digital pin to read
  uint8_t state;            // state of the button
  unsigned long int ftv0;   // time of the beginning of the press
};
T_button button[buttonNb];                      // mechanical buttons
uint8_t buttonPin[buttonNb] = { 10 };  // digital pin for mechanical buttons
elapsedMillis buttonSince;                      // timer to measure the states

uint8_t adcState;  // state of presets in the ADC
enum ADCSTATE { adcNothing,
                adcOptical };  // ends with adcOptical !!

ADC *adc = new ADC();  // analog to digital converter

elapsedMillis sinceOnboardLed;  // time elapsed for the onboard led

#define MidiChannelOut 1  // channel MIDI used for output

uint8_t midiBuf[5];
uint8_t midiType, midiLen;
uint8_t midiJingle[] = { 0 };  // { 66, 68, 71, 76, 75, 71, 73, 0 }; // midi pitch to play at the init : ends with zero !!!

#define CONF_MAGIC 20     // key to detect if EEPROM has been set
#define CONF_ADDR_VMIN 4  // address EEPROM for the VMIN setting

///////////////
// read conf from EEPROM
//////////////
void confRead() {
  byte value;
  for (uint8_t i = 0; i < CONF_ADDR_VMIN; i++) {
    value = EEPROM.read(i);
    if (value != CONF_MAGIC) {
      veloMin = 10;
      veloCurve = CURVEMIN + ( CURVEMAX - CURVEMIN ) / 2.0 ; 
      return;
    }
  }
  veloMin = EEPROM.read(CONF_ADDR_VMIN);
  EEPROM.get(CONF_ADDR_VMIN + 1, veloCurve);
}
void confWrite() {
  for (uint8_t i = 0; i < CONF_ADDR_VMIN; i++)
    EEPROM.update(i, CONF_MAGIC);
  EEPROM.update(CONF_ADDR_VMIN, (byte)(veloMin));
  EEPROM.put(CONF_ADDR_VMIN + 1, veloCurve);
}

///////////////
// On board Led
///////////////
void ledOnboardFlash(int v) {
  analogWrite(ONBOARD_LED_PIN, constrain(2*v,1,256));
  sinceOnboardLed = 0;
  onBoardLedOn = true ;
}
void ledOnboardProcess() {
  // keep alive led
  if ( ! onBoardLedOn )
    return ;
  if (sinceOnboardLed > FLASH_DT) {
    pinMode(ONBOARD_LED_PIN, OUTPUT);
    digitalWrite(ONBOARD_LED_PIN,HIGH);
    onBoardLedOn = false ;
  }
}
void ledOnboardInit() {
  pinMode(ONBOARD_LED_PIN, OUTPUT);
  digitalWrite(ONBOARD_LED_PIN,LOW);
  delay(500);
  digitalWrite(ONBOARD_LED_PIN,HIGH);
  onBoardLedOn = false ;
}

// MIDI-thru USB => S2
//////////////////////
void s2Process() {
  // forward MIDI-in/USB to MIDI/S2
  if (usbMIDI.read()) {
    ledOnboardFlash(64);
    midiType = usbMIDI.getType();
    midiLen = 3;
    midiBuf[0] = (midiType & 0xF0) | (usbMIDI.getChannel() - 1);
    midiBuf[1] = usbMIDI.getData1();
    midiBuf[2] = usbMIDI.getData2();
    switch (midiType) {
      case usbMIDI.SystemExclusive:  // sysex
        Serial1.write(usbMIDI.getSysExArray(), usbMIDI.getSysExArrayLength());
        break;
      case usbMIDI.ProgramChange:  // PROG is only two bytes
        midiLen = 2; // no break to continue on default processing
       default:
        // all Midi messages are sent on the S2-midi-expander
        midiBuf[0] = (midiType & 0xF0) | (usbMIDI.getChannel() - 1);
        midiBuf[1] = usbMIDI.getData1();
        midiBuf[2] = usbMIDI.getData2();
        Serial1.write(midiBuf, midiLen);
        break;
    }
  }
}
void s2Jingle(uint8_t *p, uint8_t dt) {
  uint8_t *n;
  n = p;
  while (*n != 0) {
    //s2Note(*n, 64);
    delay(dt);
    //s2Note(*n, 0);
    n++;
  }
}
void s2Init() {
  Serial1.begin(31250);
  s2Jingle(midiJingle, 200);
}

void midiNote(uint8_t p, uint8_t v) {
  // send msg MIDI NoteOn
  if (v == 0)
    usbMIDI.sendNoteOff(p, 0, MidiChannelOut);
  else
    usbMIDI.sendNoteOn(p, v, MidiChannelOut);

  usbMIDI.send_now();
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
}
void buttonAction(T_button *b , bool on)
{
  midiNote(b->nr + 1, on?127:0);
}
void buttonProcess() {
  uint8_t nr;
  bool allOff;
  T_button *b;
  for (nr = 0, b = button, allOff = true; nr < buttonNb; nr++, b++) {
    if (b->state != 0)
      allOff = false;
    switch (b->state) {
      case 0:
        if (digitalRead(b->pin) == LOW) {
          b->state = 1;
          buttonAction(b,true);
          b->ftv0 = buttonSince;
          ledOnboardFlash(64);
        }
        break;
      case 1:
        if ((buttonSince - b->ftv0) > 250)  // ms
          b->state = 2;
        break;
      case 2:
        if (digitalRead(b->pin) == HIGH) {
          b->state = 3;
          buttonAction(b,false);
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
#ifdef DEBUGMODE
        prtln("Error state button");
#endif
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
  if (o->nr < (opticalNb - 1))
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
  if (o->nr < (opticalNb - 1)) {
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
  o->v = ADC_POSITIVE?(adc->adc0->readSingle()):(ADC_MAX - adc->adc0->readSingle());

  if (o->nr < (opticalNb - 1)) {
    // read adc1
    while (!adc->adc1->isComplete())
      ;
    (o + 1)->v = ADC_POSITIVE?(adc->adc1->readSingle()):(ADC_MAX - adc->adc1->readSingle());
  }

  opticalAdcPreset(((o->nr) < (opticalNb - 2)) ? (optical + 2) : (optical));  // prepare next two adc
}
void opticalInit() {
  // initialization of optical buttons
  uint8_t nr;
  T_optical *o ;
  for (nr = 0, o = optical; nr < opticalNb; nr++, o++) {
    o->nr = nr;
    o->pin = opticalPin[nr];
    pinMode(o->pin, INPUT_DISABLE);  // ?
    o->state = 0;
    o->v_trigger_min = ADC_MAX;
    o->v_trigger_max = 0 ;
    o->v_min = ADC_MAX ;
    o->v_max = 0 ;
    o->slope_min = 99999.0 ;
    o->slope_max = 0.0 ;
    o->nb_click_tuning  = 0 ;
    o->ccMidi = controlMidi[nr];
  }
  // prepare to read the two first optical sensors
  opticalAdcPreset(optical);
}
void opticalMidiControl(T_optical *o) {
  // send Contorl Change according to position
  if ( o->ccmidi == 0)
    return;
  if ( abs(o->v - o->pv) > DV_CONTROLCHANGE) {
    o->pv = o->v ;
    midiControl(o->ccMidi, constrain(map(o->v,o->v_min, o->v_max,0,127),0,127)) ;
  }
}
void opticalMidiNoteOn(T_optical *o) {
  // send notOn with calculated velocity

  if ( o->slope < o->slope_min)
    o->slope_min = 0.99 * o->slope ;
  if ( o->slope > o->slope_max)
    o->slope_max = 1.01 * o->slope;
  
  int v;
  float fveloMin = (float)veloMin ;
  v = fveloMin + ((128.0 - fveloMin) - fveloMin) * pow(((o->slope - o->slope_min) / (o->slope_max - o->slope_min)), veloCurve);
#if DEBUGMODE > 2
  prt("opticalMidiNoteOn : ");
  prt("v=");
  prt(v, 1);
  prt("");
  prt(veloMin, 1);
  prt(" : ");
  prt(o->slope_min, 8);
  prt("..");
  prt(o->slope, 8);
  prt("..");
  prt(o->slope_max, 8);
  prt(" : ");
  prt((128.0 - veloMin), 1);
  prt(" / ");
  prt("%slope=");
  prt(((o->slope - o->slope_min) / (o->slope_max - o->slope_min)),3);
  prt(" / ");
  prt("pow(%slope)=");
  prt(pow(((o->slope - o->slope_min) / (o->slope_max - o->slope_min)), veloCurve),3);
  prt(" / ");
  prt("veloCurve=");
  prt(veloCurve);
  prt(" / ");
  prt(o->fnb);
  prt("ADC");
  prtln(" / ");
#endif
#ifdef DEBUGMODE
  char s[512];
  for (uint8_t n = 0; n < (uint8_t)(v); n++)
    s[n] = '=';
  s[v] = '*';
  s[v + 1] = '\0';
  prt(v);
  prtln(s);
#endif
  ledOnboardFlash(v);
  if ( o->nb_click_tuning < MIN_CLICK_TUNING ) {
    // standard mode : send MIDI note msg
    if (v > (128 - veloMin)) 
      v = (128 - veloMin);
    if (v < veloMin) 
      v = veloMin;
    o->pitch = o->nr + 48;
    midiNote(o->pitch, (uint8_t)(v));
  }
  else {
    if ( o->nb_click_tuning == MIN_CLICK_TUNING ) {
      delay(2*FLASH_DT);
      ledOnboardFlash(128);
      delay(2*FLASH_DT);
      ledOnboardFlash(128);
      delay(2*FLASH_DT);
      ledOnboardFlash(128);
    }
    else {
      // mode nb_click_tuning : take velocity as a nb_click_tuning value
      switch (o->nr) {
        case 0 :
          veloMin = constrain(map(v,1,128,10, 60),10, 60);
          confWrite() ;
          break ;
        case 1 :
          veloCurve = CURVEMIN + (CURVEMAX - CURVEMIN) * (float)(constrain(v,1, 128)) / 128.0 ;
          confWrite() ;
          break ;
        default :
          break ;
      }
    }
  }
}
void opticalMidiNoteOff(T_optical *o) {
  // send noteOff
  if (o->pitch > 0)
    midiNote(o->pitch, 0);
}
bool opticalProcess() {
  // scan optical buttons
  // return true if an optical slope-measurement is in progress
  uint8_t nr , i ;
  bool modulo2;
  bool limite_changed;
  bool opticalMeasure, allOff;
  unsigned long int dt;
  T_optical *o;
  for (nr = 0, o = optical, modulo2 = true, allOff = true, opticalMeasure = false; nr < opticalNb; nr++, o++, modulo2 = !modulo2) {

    if (modulo2)
      opticalAdcRead(o);  // read adc0 and adc1, and prepare next ones. Unsigned 8 bits [0..2^8]

    limite_changed = false ;
    if ( o->v < o->v_min )
    {
      limite_changed = true ;
      o->v_min = o->v ;
    }
    if ( o->v > o->v_max )
    {
      o->v_max = o->v ;
      limite_changed = true ;
    }
    if (limite_changed)
    {
      if ( (o->v_max - o->v_min) < ADC_GAP_MIN )
        return false ;
      o->v_trigger_min = o->v_min + ( o->v_max - o->v_min ) * ADC_TRIGGER_PERCENT ;
      o->v_trigger_max = o->v_max - ( o->v_max - o->v_min ) * ADC_TRIGGER_PERCENT ;
    }
    
    if (o->state > 0)
      allOff = false;

    switch (o->state) {
      case 0:
        if (o->v > o->v_trigger_min) {
          allOff = false;
          o->state++;
          o->ftv0 = o->ftv;
          opticalMeasure = true;
          // linear regression init
          o->sumt = 0;
          o->sumv = o->v;
          o->sumtv = 0.0;
          o->sumt2 = 0.0;
          o->fnb = 1.0;
        }
        break;
      case 1:
        opticalMeasure = true;
        if (o->v > o->v_trigger_max) {
          o->state++;
          // linear regression ended
          float d = o->fnb * o->sumt2 - o->sumv * o->sumv;
          o->slope = abs(o->fnb * o->sumtv - o->sumt * o->sumv) / d;
          opticalMidiNoteOn(o);
          o->ftv0 = opticalSince;
          break;
        }
        if (o->v < o->v_trigger_min) {
          // bad alert
          o->state = 0;
          break;
        }
        // linear regression progress
        dt = o->ftv - o->ftv0;
        o->sumt += (float)dt;
        o->sumv += o->v;
        o->sumtv += (float)(dt) * (float)(o->v);
        o->sumt2 += (float)(dt) * (float)(dt);
        o->fnb += 1.0;
        break;
      case 2:
        if ((opticalSince - o->ftv0) > ADC_DELAY * 1000)  // microsec
          o->state++;
        break;
      case 3:
        if (o->v < o->v_trigger_min) {
          o->state++;
          opticalMidiNoteOff(o);
          for(i = 0 ; i < opticalNb ; i ++ ) {
            // when a button is released, it cancels the nb_click_tuning mode of other buttons
            if ( i != nr )
              optical[i].nb_click_tuning = 0 ;
          }
          o->ftv0 = o->ftv;
        }
        break;
      case 4:
        if ((opticalSince - o->ftv0) > ADC_DELAY * 1000)  // microsec
          o->state = 0;
        break;
      default:
        // state zombi
        o->state = 0;
#ifdef DEBUGMODE
        prtln("Error optical button");
#endif
        break;
    }
  }
  if (allOff) {
    // no activity on optical : reset µsecond counter
    opticalSince = 0;
  }
  if ( ! opticalMeasure )
    // send Control Chanage on MIDI
    opticalMidiControl(o);
  }

  // => opticalMeasure is false
  return opticalMeasure;
}

////////
// ADC
////////
void adcInit() {
  adc->adc0->setResolution(ADC_RESOLUION);  // unsigned 8 bits [0..2^8]
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
  confWrite();
  confRead();
  ledOnboardInit();

  adcInit();
  // s2Init();
  buttonInit();
  opticalInit();
  sinceOnboardLed = 0;  // ms
  ledOnboardFlash(64);
}

///////////
// LOOP
//////////
void loop() {
  if (!opticalProcess()) {
    // no optical slope-measurement in progress
    // process buttons
    buttonProcess();
    // led onboard
    ledOnboardProcess();
    // transmit Midi-in => S2-expander
    // s2Process();
  }
}
