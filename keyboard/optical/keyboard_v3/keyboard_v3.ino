/*
MIDI keyboard
Analog sensors send : 
     - a MIDI-out noteOn/Off message : MIDI-velocity depends on sensor slope-value ; 
     - a permanent MIDI-CC according to position
Logical pullup-up buttons send :
     - to Midi-out note message
MIDI-thru : USB-MidiIn => serial-midi-out 
Tuning : Receive USB CC to tune analog parameters. 
     - CC-102 : curve/aggressivity of noteOn velocity ; 
     - CC-103 : dynamic of noteOn velocity
On board led : flashes on various events
Analog pins must be compliant with dual-ADCs capabilities
*/

#include <EEPROM.h>
#include <ADC.h>
#include <ADC_util.h>

#define prt Serial.print
#define prtln Serial.println

//#define DEBUGMODE 0

#define ONBOARD_LED_PIN 13
bool onBoardLedOn = false ;
#define FLASH_DT 500 // ms  flash
long unsigned int dtled ;

// optical button
#define NB_OPTICAL 2      // nb optical button
#define ADC_RESOLUTION 12 // bits
#define ADC_MAX (1<<ADC_RESOLUTION) // max value delivered by ADC
#define ADC_GAP_MIN 40 // Min gap between min-max to start the analysis
#define ADC_TRIGGER_PERCENT_ON 0.5 // limit for the triggers min
#define ADC_TRIGGER_PERCENT_OFF 0.3 // limit for the triggers off
#define ADC_NB_MEASURE 100.0 // number of measure for linear regression
#define ADC_POSITIVE true // true if the sensor value increases while pressng the optical button (else false)
#define ADC_DELAY 100 // ms to debounce states
struct T_optical {
  uint8_t nr;                                  // incremntal number of the optical button
  uint8_t pin;                                 // analog pin to read ( 3.1V max )
  float v , pv , pvcc ;                        // value of the analog read 
  float v_min, v_max;                          // min max of analog read v
  float v_trigger_on, v_trigger_off;          // min max of the trigger applied on v
  float slope_min, slope_max ;                 // min max of the slope
  unsigned long int ftv;                       // time of the analog convertion
  unsigned long int ftv0;                      // time of the beginning of the press
  float sumt, sumv, sumtv, sumt2, fnb, slope;  // linear regression for the slope
  uint8_t state;                               // state of the optical button
  uint8_t pitch;                               // pitch sent on midiOn
  uint8_t ccMidi ;                             // MIDI-Control-Change to send when value changes 
  int nbOn ;                                    // Nb On 
};
T_optical optical[NB_OPTICAL];                    // optical buttons
uint8_t opticalPin[NB_OPTICAL] = { A2, A3 };      // analog pins for the optical button compliant with dual-ADC
void opticalAdcPreset(T_optical *o);
elapsedMicros opticalSince;  // timer to measure the slope
int veloMin = 10;        // velocity minimum ( velocity maximum = 128 - veloMin) 
#define VELOMINDEFAULT 10
#define CURVEMIN 0.2
#define CURVEMAX 2.0
#define CURVEDEFAULT 1.0
float veloCurve = CURVEDEFAULT;  // velocity curve [CURVEMIN..CURVEMAX]
#define DV_CONTROLCHANGE 2 // minimum Dv to send a Control-Change
#define DT_CONTROLCHANGE 100 // minimum ms to send a Control-Change
uint8_t controlMidi[NB_OPTICAL] = { 0, 4 };      // Midi-our-Control-Change for each optical button (0 for nothing)
// autotuning on start 
bool tuningOnStart = true ;
#define DT_TUNING 10 // secondes , delay to tune 
#define NB_TUNING 5 // nb On miniam to validate the tuning (ele read from EEPROM last tuning

// mechanical button
#define buttonNb 1  // nb button
struct T_button {
  uint8_t nr;               // incremntal number of the mechanical button
  uint8_t pin;              // digital pin to read
  uint8_t state;            // state of the button
  unsigned long int ftv0;   // time of the beginning of the press
};
T_button button[buttonNb];                      // mechanical buttons
uint8_t buttonPin[buttonNb] = { 10 };       // digital pin for mechanical buttons
elapsedMillis buttonSince;                      // timer to measure the states

// Analog Digital Converter tuning
uint8_t adcState;  // state of presets in the ADC
enum ADCSTATE { adcNothing,
                adcOptical };  // ends with adcOptical !!
ADC *adc = new ADC();  // analog to digital converter

elapsedMillis sinceOnboardLed;  // time elapsed for the onboard led
#define MidiChannelOut 1  // channel MIDI used for output
#define PITCH_OFFSET 48 // offset of optical MidiOn pitch
uint8_t midiBuf[5];
uint8_t midiType, midiLen;
uint8_t midiJingle[] = { 0 };  // { 66, 68, 71, 76, 75, 71, 73, 0 }; // midi pitch to play at the init : ends with zero !!!
bool s2Open = false ; // true when serial is opened to forward Midi-In messages

#define CONF_ADDR_VMIN 4  // number of keys in conf_magic 
uint8_t conf_magic[CONF_ADDR_VMIN] = { 20,45,10,89 } ;  // keys to detect if EEPROM has been set

///////////////
// read conf from EEPROM
//////////////
void confRead(bool all) {
  byte value;
  uint8_t addr = 0  ;
  for (addr = 0; addr < CONF_ADDR_VMIN; addr++) {
    value = EEPROM.read(addr);
    if (value != conf_magic[addr]) {
      veloMin = VELOMINDEFAULT;
      veloCurve = CURVEDEFAULT ; 
      return;
    }
  }
  EEPROM.get(addr, veloMin); addr += sizeof(veloMin) ;
  EEPROM.get(addr, veloCurve); addr += sizeof(veloCurve) ;
  if ( all ) {
       T_optical *o ;
       uint8_t i ;
       for(o = optical , i = 0 ; i < NB_OPTICAL ; o++ , i ++ ) {
            EEPROM.get(addr,*o) ; addr += sizeof(T_optical);
       }
  }
}
void confWrite(bool all) {
  uint8_t addr = 0  ;
  for (addr = 0; addr < CONF_ADDR_VMIN; addr++)
    EEPROM.write(addr, conf_magic[addr]);
  EEPROM.put(addr, veloMin); addr += sizeof(veloMin) ;
  EEPROM.put(addr, veloCurve);addr += sizeof(veloCurve) ;
  if ( all ) {
       T_optical *o ;
       uint8_t i ;
       for(o = optical , i = 0 ; i < NB_OPTICAL ; o++ , i ++ ) {
          optical[i].v = 0.0 ;
          optical[i].pv = 0.0 ;
          optical[i].pvcc = 0.0 ;
          optical[i].ftv0 = 0 ;
          optical[i].sumt = 0.0 ;
          optical[i].sumv = 0.0 ;
          optical[i].sumtv = 0 ; 
          optical[i].sumt2 = 0.0 ;
          optical[i].fnb=0.0 ; 
          optical[i].slope = 0.0 ;
          optical[i].state = 0;
          optical[i].pitch = 0;
          optical[i].nbOn = 0 ;             
          EEPROM.put(addr,*o) ; addr += sizeof(T_optical);
       }
  }
}

///////////////
// On board Led
///////////////
void ledOnboardFlash(int v) {
  ledOnboard(tuningOnStart?false:true);
  sinceOnboardLed = 0;
  dtled = v * 2 ;
}
void ledOnboardProcess() {
  // keep alive led
  if ((onBoardLedOn && tuningOnStart )  || (! onBoardLedOn && ! tuningOnStart ))
    return ;
  
  if (sinceOnboardLed > dtled) {
    ledOnboard(tuningOnStart?true:false);
  }
}
void ledOnboard(bool on) {
    digitalWrite(ONBOARD_LED_PIN,on?HIGH:LOW);
    onBoardLedOn = on ;
}
void ledOnboardInit() {
  pinMode(ONBOARD_LED_PIN, OUTPUT);
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
        if ( s2Open )
          Serial1.write(usbMIDI.getSysExArray(), usbMIDI.getSysExArrayLength());
        break;
      case usbMIDI.ProgramChange:  // PROG is only two bytes
        midiLen = 2; // no break to continue on default processing
      case usbMIDI.ControlChange: // CC 102 & 103 are trapped to tune the keyboard
        if ( usbMIDI.getData1() == 102) {
          // curve of Midi-On velocity : 0=slow reactiviy ; 127=quick reactivity for high velocity
          veloCurve = map((float)(usbMIDI.getData2()), 0.0,127.0,CURVEMIN, CURVEMAX);
          confWrite(false) ;
          break ;
        }
        if ( usbMIDI.getData1() == 103) {
          // dynamic ofMidi-On velocity : 0=fixed velocity 64 ; 127=full range velocity 1..127
          veloMin = map(usbMIDI.getData2(), 0,127,63, 1);
          confWrite(false) ;
          break ;
        }
        // CC not trapped, continue on default forward to Serial1 
      default:
        // all Midi messages are sent on the S2-midi-expander
        midiBuf[0] = (midiType & 0xF0) | (usbMIDI.getChannel() - 1);
        midiBuf[1] = usbMIDI.getData1();
        midiBuf[2] = usbMIDI.getData2();
        if ( s2Open )
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
  s2Open = true ;
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
  midiNote(b->nr + 1 , on?127:0);
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
//////////////////////////////////

  // prepare two analog convertions
  
  #if DEBUGMODE > 7
    prt("opticalAdcPreset adc0 on pin#");
    prtln(o->pin);
  #endif
  if ( ! adc->adc0->startSingleRead(o->pin) ) {
     prt("Error startSingleRead adc0 on pin#");
     prtln(o->pin);
  }
  o->ftv = opticalSince;

  if (o->nr < (NB_OPTICAL - 1)) {
    #if DEBUGMODE > 7
        prt("opticalAdcPreset adc1 on pin#");
        prtln((o+1)->pin);
    #endif
    if ( ! adc->adc1->startSingleRead((o + 1)->pin) ) {
          prt("Error startSingleRead adc1 on pin#");
          prtln((o+1)->pin);
       }
    (o + 1)->ftv = opticalSince;
  }
  adcState = adcOptical + o->nr;
}

void opticalAdcRead(T_optical *o) {
///////////////////////////////////

  long noloop=0 ;
  bool err = false;
  // control status of adc
  if (adcState != (adcOptical + o->nr)) {
    err = true;
  #ifdef DEBUGMODE
    prt("Error opticalAdcPreset [");
    prt(o->nr);
    prt("] adcstate = ");
    prtln(adcState);
  #endif
  }
  if (!(adc->adc0->isConverting() || adc->adc0->isComplete())) {
    err = true;
    #ifdef DEBUGMODE
      prt("Error opticalAdcPreset [");
      prt(o->nr);
      prtln("] adc0 not in progress");
    #endif
  }
  if (o->nr < (NB_OPTICAL - 1)) {
    if (!(adc->adc1->isConverting() || adc->adc1->isComplete())) {
      err = true;
      #ifdef DEBUGMODE
        prt("Error opticalAdcPreset [");
        prt(o->nr);
        prtln("] adc1 not in progress");
      #endif
    }
  }
  if (err) {
    opticalAdcPreset(o);  // error in preset ADC ...!!!
  }

  // read adc0
  noloop = 0 ;
  while (!adc->adc0->isComplete()) {
    if (noloop++ > 3200000)
      break ;
  }
  if ( adc->adc0->isComplete() )
    o->v = ADC_POSITIVE?(adc->adc0->readSingle()):(ADC_MAX - adc->adc0->readSingle());
  else {
    o->v = 0 ;
    #ifdef DEBUGMODE
      prtln("Timeout adc0");
    #endif
  }

  if (o->nr < (NB_OPTICAL - 1)) {
    // read adc1
    noloop = 0 ;
    while (!adc->adc1->isComplete()) {
      if (noloop++ > 3200000)
        break ;
    }
    if ( adc->adc1->isComplete() )
      (o + 1)->v = ADC_POSITIVE?(adc->adc1->readSingle()):(ADC_MAX - adc->adc1->readSingle());
    else {
      (o + 1)->v = 0 ;
      #ifdef DEBUGMODE 
        prtln("Timeout adc1");
      #endif
    }
  }

  opticalAdcPreset(((o->nr) < (NB_OPTICAL - 2)) ? (optical + 2) : (optical));  // prepare next two adc
}

void opticalInit() {
  // initialization of optical buttons
  uint8_t nr;
  T_optical *o ;
  for (nr = 0, o = optical; nr < NB_OPTICAL; nr++, o++) {
    o->nr = nr;
    o->pin = opticalPin[nr];
    pinMode(o->pin, INPUT_DISABLE);  // ?
    o->state = 0;
    o->v_trigger_on = ADC_MAX;
    o->v_trigger_off = 0 ;
    o->v_min = ADC_MAX ;
    o->v_max = 0 ;
    o->slope_min = 999999.0 ;
    o->slope_max = 0.0 ;
    o->ccMidi = controlMidi[nr];
    o->nbOn = 0 ;
  }
  // prepare to read the two first optical sensors
  opticalAdcPreset(optical);
}
void opticalMidiControl(T_optical *o) {
  // send Contorl Change according to position
  if ( o->ccMidi == 0)
    return;
  if ( abs(o->v - o->pvcc) > DV_CONTROLCHANGE) {
    o->pvcc = o->v ;
    midiControl(o->ccMidi, constrain(map(o->v,o->v_min, o->v_max,0,127),0,127)) ;
  }
}
void opticalMidiNoteOn(T_optical *o) {
  // send notOn with calculated velocity

  // switch off all current note-on , to have only one note-on active at one time
  uint8_t nr;
  T_optical *lo ;
  for (nr = 0, lo = optical; nr < NB_OPTICAL; nr++, lo++) {
       if ( lo->pitch > 0 ) {
            midiNote(lo->pitch, 0);
            lo->pitch = 0 ;
       }
  }
     
  if ( tuningOnStart) {
       if ( o->slope < o->slope_min)
         o->slope_min = o->slope ;
       if ( o->slope > o->slope_max)
         o->slope_max = o->slope;
       (o->nbOn) ++ ;
  }
  
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
  if (v > (128 - veloMin)) 
    v = (128 - veloMin);
  if (v < veloMin) 
    v = veloMin;
  o->pitch = o->nr + PITCH_OFFSET;
  midiNote(o->pitch, (uint8_t)(v));
}
void opticalMidiNoteOff(T_optical *o) {
  // send noteOff
  if (o->pitch > 0)
    midiNote(o->pitch, 0);
}
bool opticalProcess() {
  // scan optical buttons
  // return true if an optical slope-measurement is in progress
  // states of the midi-On/Off calculation :
  //   1- when value o->v is more than o->v_min, linear regression starts 
  //   2- when value o->v is more than o->v_max, note-on is sent with velocity mapped on the result of linear regression
  //   3- when value o->v is less than o->v_min, note-off is sent
  // in addition when value o->v changes, Control-Change is sent
  uint8_t nr ;
  bool modulo2;
  bool limite_changed;
  bool opticalMeasure, allOff;
  unsigned long int dt;
  T_optical *o;
  for (nr = 0, o = optical, modulo2 = true, allOff = true, opticalMeasure = false; nr < NB_OPTICAL; nr++, o++, modulo2 = !modulo2) {

    if (modulo2)
      opticalAdcRead(o);  // read adc0 and adc1, and prepare the two next ones.

    if (o->state > 0)
      allOff = false;

    #if DEBUGMODE > 6
      prt("read value #");
      prt(o->nr);
      prt(" ");
      prtln(o->v);
      delay(200);
    #endif
    if ( tuningOnStart) {
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
           o->v_trigger_on = o->v_min + ( o->v_max - o->v_min ) * ADC_TRIGGER_PERCENT_ON ;
           o->v_trigger_off = o->v_min + ( o->v_max - o->v_min ) * ADC_TRIGGER_PERCENT_OFF ;
           #if DEBUGMODE > 2
             prt("New limits optical button #");
             prt(o->nr);
             prt(" ");
             prt(o->v_min);
             prt(".. ");
             prt(o->v_trigger_off);
             prt("=>");
             prt(o->v_trigger_on);
             prt(".. ");
             prtln(o->v_max);
           #endif
         }
         if ( (o->v_max - o->v_min) < ADC_GAP_MIN )
           continue ;
    }
    
    switch (o->state) {
      case 0:
        if (o->v > o->v_trigger_on) {
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
          break ;
         }
        break;
      case 1:
        opticalMeasure = true;
        dt = o->ftv - o->ftv0;
        if ((o->fnb > ADC_NB_MEASURE) || (dt > ADC_DELAY * 1000)) {
          o->state++;
          // linear regression ended
          float d = o->fnb * o->sumt2 - o->sumv * o->sumv;
          o->slope = abs(o->fnb * o->sumtv - o->sumt * o->sumv) / d;
          opticalMidiNoteOn(o);
          o->ftv0 = opticalSince;
          break;
        }
        // linear regression progress
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
        if (o->v < o->v_trigger_off) {
          o->state++;
          opticalMidiNoteOff(o);
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
  if ( tuningOnStart ) {
       if ( opticalSince > (DT_TUNING * 1000000 )) {
            tuningOnStart = false ;
            ledOnboard(false) ;
            int i ;
            bool nbOn = true ;
            for(i = 0 ; i < NB_OPTICAL ; i ++) {
                 if ( optical[i].nbOn < NB_TUNING )
                      nbOn = false ;
            }
            if (nbOn) {
                 // tuning valide during this initialisation phase. Store these values in EEPROM
                 confWrite(true);
            }
            else {
                 // no valide tuning, retrieve tuning from EEPROM
                 confRead(true);
            }
       }
  }
  else {
       if (allOff) {
         // no activity on optical 
         if( opticalSince > DT_CONTROLCHANGE * 1000) {
            for(nr = 0, o = optical; nr < NB_OPTICAL; nr++, o++ ) {
              opticalMidiControl(o) ;
            }
            // reset µsecond counter
            opticalSince = 0;
         }
       }
  }

  // => opticalMeasure is false
  return opticalMeasure;
}

////////
// ADC
////////
void adcInit() {
  
  adc->adc0->setResolution(ADC_RESOLUTION);  // unsigned 8 bits [0..2^8]
  adc->adc0->setAveraging(2);
  adc->adc0->setConversionSpeed(ADC_CONVERSION_SPEED::LOW_SPEED);
  adc->adc0->setSamplingSpeed(ADC_SAMPLING_SPEED::LOW_SPEED);
     
  adc->adc1->setResolution(ADC_RESOLUTION);
  adc->adc1->setAveraging(2);
  adc->adc1->setConversionSpeed(ADC_CONVERSION_SPEED::LOW_SPEED);
  adc->adc1->setSamplingSpeed(ADC_SAMPLING_SPEED::LOW_SPEED);
  
  adcState = adcNothing;
}
////////////
// SETUP
///////////
void setup() {

  #ifdef DEBUGMODE
      prtln("init start");
      delay (100);
  #endif  

  confRead(false);
  
  ledOnboardInit();
  ledOnboard(true);

  adcInit();
  // s2Init(); // if serial is connected to an expander 
  // buttonInit();
  opticalInit();
  

}

///////////
// LOOP
//////////
void loop() {
  if (! opticalProcess()) {
    // no optical slope-measurement in progress
    // process buttons
    //// buttonProcess();
    // led onboard
    ledOnboardProcess();
    // transmit Midi-in => S2-expander
    s2Process();
  }
}
