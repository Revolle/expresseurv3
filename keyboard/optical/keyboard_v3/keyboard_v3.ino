#include <EEPROM.h>
#include <ADC.h>
#include <ADC_util.h>

#define prt Serial.print
#define prtln Serial.println

//#define DEBUGMODE
#define PINLED 13


// optical
#define opticalNb 4  // nb optical button : must be multiple of 2 for the two ADC
#define DtMIN 2000 // min delay for fff
#define DtMAX 20000 // max delay for ppp
struct T_optical {
  uint8_t nr;             // incremntal number of the optical button
  uint8_t pin;            // analog pin to read ( 3.1V max )
  uint8_t v;              // value of the analog read 8 bits
  unsigned long int ftv;  // time of the analog convertion
  unsigned long int ftv0;
  unsigned long int dtMin;
  unsigned long int dtMax;
  uint8_t state;                   // state of the optical button
  uint8_t pitch;                   // pitch sent on midiOn
  uint8_t triggerMin, triggerMax;  // min max of the trigger to measure
  bool opticalHappen;              // true if an optical event has bee trigerred
};
T_optical optical[opticalNb];  // optical buttons
//uint16_t tvNb[tvMax];                                // stat for slope collect
unsigned long int vMin, vMax;                        // min max of note-on velocity
uint8_t opticalPin[opticalNb] = { A5, A4, A3, A2 };  // analog pins for the optical button
void opticalAdcPreset(T_optical *o);
unsigned long int precisionMax, precisionMin;
elapsedMicros opticalSince;  // timer to measure the slope

// buttons
#define buttonNb 3  // nb button
struct T_button {
  uint8_t nr;
  uint8_t pin;    // logical pin to read
  uint8_t state;  // state of the button
  unsigned long int ftv0;
};
T_button button[buttonNb];
uint8_t buttonNbOn;
uint8_t buttonPin[buttonNb] = { 7, 8, 6 };  // digital pin for buttons
elapsedMillis buttonSince;

// led
#define ledNb 3                         // nb led
uint8_t ledPin[ledNb] = { 10, 9, 11 };  // digital pins for the led
uint8_t ledChannelOn;                   // reminder of the ledChannel which switched on the leds
uint8_t ledValue, ledFormerValue;       // value of the led bargraph

// potar
#define potarPin0 A1
#define potarPin1 A0
uint8_t potarV0, potarV1;
bool potarDynamicOn;  // true to set the dynamic with the potar

uint8_t adcState;  // state of presets in the ADC
enum ADCSTATE { adcNothing,
                adcPotar,
                adcOptical };  // ends with adcOptical !!

// magic number to detect first use in EEPROM
#define magic 23478

// Midi messages for S2
uint8_t midiBuf[5];
uint8_t midiType, midiLen;

// analog to digital converter
ADC *adc = new ADC();

// time elapsed
elapsedMillis since;

#define MidiChannelOut 1

///////////////
// On board Led
///////////////
void ledOnboardFlash() {
  digitalWrite(PINLED, HIGH);
  since = 0;
}
void ledOnboardProcess() {
  // keep alive led
  if (since > 800)
    digitalWrite(PINLED, HIGH);
  if (since > 805) {
    digitalWrite(PINLED, LOW);
    since = 500;
  }
}
void ledOnboardInit() {
  pinMode(PINLED, OUTPUT);
}

// MIDI-thru USB => S2
//////////////////////
void s2Process() {
  // forward MIDI-in/USB to MIDI/S2
  if (usbMIDI.read()) {
    ledOnboardFlash();
    midiType = usbMIDI.getType();
    midiLen = 3;
    switch (midiType) {
      case usbMIDI.SystemExclusive:  // sysex
        Serial1.write(usbMIDI.getSysExArray(), usbMIDI.getSysExArrayLength());
        break;
      case usbMIDI.ProgramChange:  // PROG is only two bytes
        midiLen = 2;
      default:
        // all Midi messages are sent on the S2-midi-expander
        midiBuf[0] = (midiType << 4) | usbMIDI.getChannel();
        midiBuf[1] = usbMIDI.getData1();
        midiBuf[2] = usbMIDI.getData2();
        Serial1.write(midiBuf, midiLen);
        break;
    }
  }
}
void s2Init() {
  Serial1.begin(31250);
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
  uint8_t nr;
  if (ledValue == 0) {
    for (nr = 0; nr < ledNb; nr++)
      analogWrite(ledPin[nr], 0);
    return;
  }
  float fv = (float)(ledValue) / (127.0 + 127.0 / 3);
  float fn = (float)(ledNb);
  float fi, fl;
  int il;
#define flmin 1.0
#define flmax 200.0
  for (nr = 0, fi = 0.0; nr < ledNb; nr++, fi += 1.0) {
    il = 0;
    if ((fv > ((fi - 1) / fn)) && (fv < ((fi + 1) / fn))) {
      if (fv < (fi / fn))
        fl = flmin + (flmax - flmin) * (pow(fv * fn - fi + 1.0, 2));
      else
        fl = flmax + (flmin - flmax) * (pow(fv * fn - fi, 0.5));
      il = (int)(fl);
    }
    if (il < 0) il = 0;
    if (il > 255) il = 255;
    analogWrite(ledPin[nr], il);
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
    analogWrite(ledPin[2], 0);
    analogWrite(ledPin[0], 255);
    delay(d);
    digitalWrite(PINLED, LOW);
    analogWrite(ledPin[0], 0);
    analogWrite(ledPin[1], 255);
    delay(d);
    analogWrite(ledPin[1], 0);
    analogWrite(ledPin[2], 255);
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

#ifdef DEBUGMODE
  prt("MidiNote pitch="),
    prt(p);
  prt(" velo=");
  prtln(v);
#endif

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
  buttonNbOn = 0;
}
void buttonProcess() {
  uint8_t nr;
  T_button *b;
  buttonNbOn = 0;
  for (nr = 0, b = button; nr < buttonNb; nr++, b++) {
    if (b->state > 0)
      buttonNbOn++;

    switch (b->state) {
      case 0:
        if (digitalRead(b->pin) == LOW) {
          b->state = 1;
          midiNote(nr + 1, 127);
          b->ftv0 = buttonSince;
          ledOnboardFlash();
        }
        break;
      case 1:
        if ((buttonSince - b->ftv0) > 50)  // ms
          b->state = 2;
        break;
      case 2:
        if (digitalRead(b->pin) == HIGH) {
          b->state = 3;
          midiNote(nr + 1, 0);
          b->ftv0 = buttonSince;
          ledOnboardFlash();
        }
        break;
      case 3:
        if ((buttonSince - b->ftv0) > 50)  // ms
          b->state = 0;
        break;
      default:
        // state zombi
        b->state = 0;
        prtln("Error state button");
        break;
    }
  }
  if (buttonNbOn == 0)
    buttonSince = 0;
}

//////////
// potar
//////////
void potarInit() {
  pinMode(potarPin0, INPUT_DISABLE);
  pinMode(potarPin1, INPUT_DISABLE);
  potarV0 = 0;
  potarV1 = 0;
  vMin = 1;
  vMax = 127;
  potarDynamicOn = true;
}
void potarDynamicSet() {
  // calculate velocity range [0 < Vmin < Vmax < 128] according to volume and range cursor
  unsigned long int volumeMax, volumeMin, p0, p1;
  p0 = potarV0;
  p1 = potarV1;
  volumeMax = p0;
  volumeMin = p1 * p0 / 127;
  if (volumeMin < 1)
    vMin = 1;
  else if (volumeMin > 125)
    vMin = 125;
  else
    vMin = volumeMin;
  if (volumeMax < 3)
    vMax = 3;
  else if (volumeMax > 127)
    vMax = 127;
  else
    vMax = volumeMax;
#ifdef DEBUGMODE
  prt("potarDynamicSet ");
  prt(vMin);
  prt("..");
  prtln(vMax);
#endif
}
void potarSet() {
  if (buttonNbOn > 0) {
    // if a button is pressed while potar changes : potar will be used for MIDI Control
    potarDynamicOn = false;
    ledFlash(40);
  }
  if (buttonNbOn == buttonNb) {
    // if all buttons are pressed  while potar changes : potar will be used in (default) keyboard dynamic setting
    potarDynamicOn = true;
    ledFlash(40); 
  }
  if (potarDynamicOn) {
    // keyboard dynamic setting
    potarDynamicSet();
    return;
  }
  if (buttonNbOn == 0) {
    // control change without button selection
    midiControl(14, potarV0);
    midiControl(15, potarV1);
    return;
  }
  T_button *b;
  uint8_t nr;
  for (nr = 0, b = button; nr < buttonNb; nr++, b++) {
    if (b->state > 0) {
      // control change with button selection
      midiControl(14 + (nr + 1) * 2, potarV0);
      midiControl(15 + (nr + 1) * 2, potarV1);
      return;
    }
  }
}
void potarAdcPreset() {
  // preset adc for the two potars
  adc->adc0->startSingleRead(potarPin0);
  adc->adc1->startSingleRead(potarPin1);
  adcState = adcPotar;
}
void potarProcess() {
  // read potars [1..127], and set range according to
  uint8_t v0, v1;
  bool err = false;
  if (adcState != adcPotar)
    err = true;
  if (!(adc->adc0->isConverting() || adc->adc0->isComplete()))
    err = true;
  if (!(adc->adc1->isConverting() || adc->adc1->isComplete()))
    err = true;
  if (err) {
    potarAdcPreset();  // error in preset ADC ...!!!
#ifdef DEBUGMODE
    prtln("Error potarAdcPreset");
#endif
  }
  while (!adc->adc0->isComplete())
    ;
  v0 = (adc->adc0->readSingle()) >> 1;  // 8bits => 7 bits
  while (!adc->adc1->isComplete())
    ;
  v1 = (adc->adc1->readSingle()) >> 1;  // 8bits => 7 bits

  // preset the ADC for the optical buttons
  opticalAdcPreset(optical);

  if ((abs(v0 - potarV0) > 4) || (abs(v1 - potarV1) > 4)) {
    ledOnboardFlash();
    potarV0 = v0;
    potarV1 = v1;
#ifdef DEBUGMODE
    prt("potarProcess vo=");
    prt(potarV0);
    prt(" v1=");
    prtln(potarV1);
#endif
    potarSet();
  }
}

////////////
// optical
////////////
void opticalAdcPreset(T_optical *o) {
  // prepare two analog convertions
#ifdef DEBUGMODE
  if (o->state == 1) {
    unsigned long int dt = opticalSince - o->ftv;
    if ((dt < precisionMin) || (dt > precisionMax)) {
      if (dt < precisionMin) precisionMin = dt;
      if (dt > precisionMax) precisionMax = dt;
      prt("Precision : ");
      prt(precisionMin);
      prt("..");
      prtln(precisionMax);
    }
  }
#endif
  adc->adc0->startSingleRead(o->pin);
  o->ftv = opticalSince;
  adc->adc1->startSingleRead((o + 1)->pin);
  (o + 1)->ftv = opticalSince;
  adcState = adcOptical + o->nr;
}
void opticalAdcRead(T_optical *o) {
  bool err = false;
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
  if (!(adc->adc1->isConverting() || adc->adc1->isComplete())) {
    err = true;
#ifdef DEBUGMODE
    prt("Error opticalAdcPreset [");
    prt(o->nr);
    prt("] adc1 not in progress");
#endif
  }
  if (err) {
    opticalAdcPreset(o);  // error in preset ADC ...!!!
  }
  while (!adc->adc0->isComplete())
    ;
  o->v = adc->adc0->readSingle();
#if DEBUGMODE > 5
  prt("optical Read [");
  prt(o->nr);
  prt("]=");
  prtln(o->v);
#endif
  while (!adc->adc1->isComplete())
    ;
  (o + 1)->v = adc->adc1->readSingle();
#if DEBUGMODE > 5
  prt("optical Read [");
  prt((o + 1)->nr);
  prt("]=");
  prtln((o + 1)->v);
#endif

  opticalAdcPreset(((o->nr) == 0) ? (optical + 2) : (optical));  // prepare next two adc

  if ((o->v > 254) || ((o + 1)->v > 254)) {
    // saturation electrique potentiellement dangereuse sur l'entree ADC
#ifdef DEBUGMODE
    prtln("Saturation opticalAdc !!");
#endif
    ledAlert();
  }
}
void opticalInit() {
  // initialization of optical buttons
  uint8_t nr;
  unsigned long int n;
  T_optical *o;
  bool modulo2;
  unsigned long int v0, vmin, vmax;
  for (nr = 0, o = optical; nr < opticalNb; nr++, o++) {
    o->nr = nr;
    o->pin = opticalPin[nr];
    pinMode(o->pin, INPUT_DISABLE);  // ?
    o->state = 0;
    o->dtMin = DtMIN;
    o->dtMax = DtMAX;
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
    v0 = o->triggerMax;
    vmax = 80 * v0 / 100;
    vmin = 20 * v0 / 100;
    o->triggerMax = vmax;
    o->triggerMin = vmin;
#ifdef DEBUGMODE
    prt("trigger[");
    prt(nr);
    prt("] : ");
    prt(o->triggerMin);
    prt(" .. ");
    prtln(o->triggerMax);
#endif
  }
  precisionMin = 999999;
  precisionMax = 0;
  // prepare to read the two first optical sensors
  opticalAdcPreset(optical);
}
uint8_t velo(unsigned long int dt0, T_optical *o) {
  // calculate velocity [1..127] according to slope and ranges of velocity
  // 0 < Vmin < velocity=f(slope) < Vmax < 128
  unsigned long int dt, v;
  dt = dt0;
  if (dt < o->dtMin)
    dt = o->dtMin;
  if (dt > o->dtMax)
    dt = o->dtMax;
  v = vMax - (dt - o->dtMin) * (vMax - vMin) / (o->dtMax - o->dtMin);
#ifdef DEBUGMODE
  unsigned long int p;
  p = 1000 - (dt - o->dtMin) * 1000 / (o->dtMax - o->dtMin);
  prt("velo ");
  prt(p);
  prt("%o : ");
  prt(o->dtMin);
  prt("<");
  prt(dt0);
  prt("<");
  prtln(o->dtMax);
  prt("velo ");
  prt(v);
  prt(" : ");
  prt(vMin);
  prt("<");
  prt(p);
  prt("<");
  prtln(vMax);
#endif
  if (v > 127) v = 127;
  if (v < 1) v = 1;
  return v;
}
void opticalMsgOn(T_optical *o) {
  unsigned long int dt;
  dt = o->ftv - o->ftv0;
  // send notOn with calculated velocity
  o->pitch = o->nr + buttonNb + 1;
  if (buttonNbOn > 0) {
    T_button *b;
    uint8_t nr;
    for (nr = 0, b = button; nr < buttonNb; nr++, b++) {
      if (b->state > 0) {
        o->pitch += (nr + 1) * opticalNb;
        break;
      }
    }
  }
  midiNote(o->pitch, velo(dt, o));
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
  T_optical *o;
  opticalMeasure = false;
  allOff = true;
  for (nr = 0, o = optical, modulo2 = true; nr < opticalNb; nr++, o++, modulo2 = !modulo2) {

    if (modulo2)
      opticalAdcRead(o);  // read adc0 and adc1, and prepare next ones. Unsigned 8 bits [0..2^8]

    if (o->state > 0)
      allOff = false;

#if DEBUGMODE > 5
    prt("opticalProcess v[");
    prt(nr);
    prt("]=");
    prtln(o->v);
    delay(1000);
#endif
    switch (o->state) {
      case 0:
        if (o->v < o->triggerMax) {
          allOff = false;
          o->state++;
          o->ftv0 = o->ftv;
          opticalMeasure = true;
          ledOnboardFlash();
        }
        break;
      case 1:
        opticalMeasure = true;
        if (o->v < o->triggerMin) {
          o->state++;
          opticalMsgOn(o);
          o->ftv0 = opticalSince;
          break;
        }
        if (o->v > o->triggerMax) {
          o->state = 0;
          break;
        }
        break;
      case 2:
        if ((opticalSince - o->ftv0) > 50 * 1000)  // ms
          o->state++;
        break;
      case 3:
        if (o->v > o->triggerMax) {
          o->state++;
          opticalMsgOff(o);
          o->ftv0 = o->ftv;
          ledOnboardFlash();
        }
        break;
      case 4:
        if ((opticalSince - o->ftv0) > 50 * 1000)  // ms
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

  // => opticalMeasure is false : potar will be read, which reset also adcPresets after
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
void setup() {
  ledOnboardInit();
  ledInit();
  ledFlash(80);

  adcInit();
  potarInit();
  buttonInit();
  opticalInit();
  s2Init();
  since = 0;  // ms
  ledFlash(20);
}

///////////
void loop() {
  if (!opticalProcess()) {
    // no optical slope-measurement in progress
    // preset adc for the potar
    potarAdcPreset();
    // process buttons
    buttonProcess();
    // show the bargraph on the leds
    ledShow();
    // led onboard
    ledOnboardProcess();
    // process potar (volume/range)
    potarProcess();
    // transmit Midi-in => S2-expander
    s2Process();
  }
}
