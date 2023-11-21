
#include <EEPROM.h>
#include <ADC.h>
#include <ADC_util.h>

#define DEBUGMODE 1

// optical
#define opticalNb 4 // nb optical button : must be multiple of 2 for the two ADC
#define tvMax 10 // max points to calculate the optical slope
struct T_optical // structure of an optical button
{
  uint8_t nr ;
  uint8_t pin ; // analog pin to read 3.3 max
  uint8_t v ; // value of the analog read
  uint8_t state ; // state of the optical button
  elapsedMicros since ; // timer to measure the slope
  float ft[tvMax] , fv[tvMax]; // points of the slope v=f(t) for velocity calculation
  uint8_t tvNr ; // pointer for the points of the slope
} ;
T_optical optical[opticalNb]; // optical buttons
uint16_t tvNb[tvMax] ; // stat for slope collect
float slopeMax , slopeMin ; // min-max of the slope for optical measure
uint8_t triggerMin, triggerMax ; // min max of the trigger to measure 
float vMin, vMax ; // min max of note-on velocity 
uint8_t opticalPin[opticalNb] = { A0 , A1 , A2 , A3 } ; // analog pins for the optical button
bool opticalHappen ; // true if an optical event has bee trigerred
#define opticalPitch 1 // offset for noteOn
void opticalAdcPreset(T_optical *o) ;

// buttons
#define buttonNb 3 // nb button
struct T_button
{
  uint8_t nr ;
  uint8_t pin ; // logical pin to read
  uint8_t state ; // state of the button
  elapsedMillis since ; 
} ;
T_button button[buttonNb] ;
uint8_t buttonPin[buttonNb] = { 18 , 19 , 20  }; // digital pin for buttons
#define  buttonPitch 16 // offset for noteOn

// led
#define ledNb 3 // nb led
uint8_t ledPin[ledNb] = { 10 , 11 , 12 } ; // digital pins for the led
uint8_t ledChannelOn ; // reminder of the ledChannel which switched on the leds
uint8_t ledValue , ledFormerValue ; // value of the led bargraph

// potar
uint8_t potarPin0 = A4 ;
uint8_t potarPin1 = A5 ;
uint8_t potarV0 , potarV1 ;

uint8_t adcState ; // state of presets in the ADC
enum ADCSTATE { adcNothing , adcPotar , adcOptical } ; // ends with adcOptical !!

// magic number to detect first use in EEPROM
#define magic 23478

// Midi messages for S2
uint8_t midiBuf[5] ;
uint8_t midiType, midiLen ;

// analog to digital converter
ADC *adc = new ADC(); 

// time elapsed
elapsedMillis since;

// caibration
uint8_t calibrationState ;
bool ConfOk ;

// configuration memory
///////////////////////
void confSave()
{
  // write the conf on EEPROM, starting with the magic number
	uint16_t v , ad ;
	v = magic ;
	ad = 0 ;
	EEPROM.put(ad, v);
	ad += sizeof(uint16_t);
	EEPROM.put(ad, slopeMin);
	ad += sizeof(float);
	EEPROM.put(ad, slopeMax) ;
	ad += sizeof(float);
	EEPROM.put(ad, triggerMin) ;
	ad += sizeof(uint8_t);
	EEPROM.put(ad, triggerMax) ;
}
bool confRead()
{
  // read the conf fromm EEPROM. 
  // Return false if no conf stored (no magic at the beginning )
	uint16_t v , ad ;
	ad = 0 ;
	EEPROM.get(ad, v) ;
	if (v == magic)
	{
		ad += sizeof(uint16_t);
		EEPROM.get(ad, slopeMin) ;
		ad += sizeof(float);
		EEPROM.get(ad, slopeMax) ;
    ad += sizeof(float);
    EEPROM.get(ad, triggerMin) ;
    ad += sizeof(uint8_t);
    EEPROM.get(ad, triggerMax) ;
    return true ;
	}
	else
	{
		slopeMin = 1.0 ; // bits/msec
		slopeMax = 100.0; // bits/msec
    triggerMin = 50 ;
    triggerMax = 200 ;
    return false ;
	}
	
	vMax = 127.0 ;
	vMin = 1.0 ;
}

// MIDI-thru USB => S2 
//////////////////////
void s2Process()
{
  // forward MIDI-in/USB to MIDI/S2
  if ( usbMIDI.read() )
  {
    midiType = usbMIDI.getType() ;
    midiLen = 3 ;
    switch(midiType)
    {
      case usbMIDI.SystemExclusive : // sysex 
        Serial1.write(usbMIDI.getSysExArray(), usbMIDI.getSysExArrayLength());
        break ;
      case usbMIDI.ProgramChange : // PROG is only two bytes
        midiLen = 2;
      default :
        // all Midi messages are sent on the S2-midi-expander
        midiBuf[0] = ( midiType << 4 ) | usbMIDI.getChannel() ;
        midiBuf[1] = usbMIDI.getData1() ;
        midiBuf[2] = usbMIDI.getData2() ;
        Serial1.write(midiBuf, midiLen);
        break ;
    }
  }
}
void s2Init()
{
	Serial1.begin(31250);
}

// Led
//////
void ledSet(uint8_t v , uint8_t ledChannel)
{
	// Set the status of the led  
	if (v == 0) 
	{
		if ((ledChannel!= ledChannelOn) || (ledChannel != 0))
			ledValue = 0 ;
		return ;
	}
	ledValue = v ;
	ledChannelOn = ledChannel ;
}
void ledShow()
{
	// show the status of the leds with a bargraph for  0,[1..127]
	if ledFormerValue == ledValue )
		return ;
	ledFormerValue = ledValue ;
	uint8_t nr ;
	float fv = (float)(v)/127.0 ;
	float fn = (float)(ledNb) ;
	float fi , fl ;
	int il ;
	#define flmin 5.0 ;
	#define flmax 256.0 ;
	for(nr = 0 , fi = 0.0 ; nr < ledNb ; nr ++ , fi += 1.0 )
	{
		il = 0 ;
		if ((fv > ((fi -1)/fn)) && (fv < ((fi + 1)/fn)))
		{
			if (fv < (fi/fn) )
				fl = flmin + (flmax - flmin )*(pow( fv* fn - fi + 1.0 , 2));
			else
				fl = flmax + (flmin - flmax )*(pow( fv* fn - fi  , 0.5));
			il = (int)(fl) ;
		}
		if (il < 0) il = 0 ;
		if (il > 255 ) il = 255 ;
		analogWrite( ledPin[nr] , il );
	}
}
void ledInit()
{
  uint8_t nr ;
	ledChannelOn = 0 ;
	ledFormerValue = 1;
	ledValue = 0 ;
	for(nr = 0  ; nr < ledNb ; nr ++ )
		analogWrite( ledPin[nr] , 0 );
}

void midiNote(uint8_t p , uint8_t v)
{
	// send msg MIDI
	if ( calibrationState != 0) // calibration in progress, no MIDI-out
		return ;
	
	if ( v == 0)
		usbMIDI.sendNoteOff(p, 0, 1);
	else
		usbMIDI.sendNoteOn(p, v, 1);
	
	usbMIDI.send_now();
	
	ledSet(v,p) ;
}

// buttons
//////////
void buttonInit()
{
  uint8_t i ;
  T_button *b ;
  for(i = 0 , b = button ; i < buttonNb ; i ++ , b ++ )
  {
    b->nr = i ;
    b->pin = buttonPin[i];
    pinMode(b->pin, INPUT_PULLUP);
    b->state = 0 ;
  }
}
void buttonProcess()
{
  uint8_t nr ;
  T_button *b ;
  for(nr = 0 , b = button ; nr < buttonNb; nr ++ , b ++)
  {
    switch(b->state)
    {
      case 0 :
        if (digitalRead(b->pin) == LOW )
        {
          b->state = 1;
          midiNote(nr+buttonPitch,127);
          b->since = 0 ;
        }
        break ;
      case 1 :
        if (b->since > 50) // ms
          b->state = 2;
        break ;
      case 2 :
        if (digitalRead(b->pin) == HIGH)
        {
          b->state = 3;
          midiNote(nr + buttonPitch,0);
          b->since = 0 ;
        }
        break ;
      case 3 :
        if (b->since > 50) // ms
          b->state = 0 ;
        break ;
      default :
        // state zombi
        b->state = 0 ;	
        Serial.println("Error state button");
        break ;
    }
  }
}

//////////
// potar
//////////
void potarInit()
{
  pinMode( potarPin0, INPUT_DISABLE );
  pinMode( potarPin1, INPUT_DISABLE );
  potarV0 = 64 ;
  potarV1 = 64 ;
}
void potarSet()
{
	// calculate velocity range [0 < Vmin < Vmax < 128] according to volume and range cursor
	float  volume, range , dv ;
	volume = (float)(potarV0) ;
	range = (float)(potarV1) ;
	if ( volume > 64.0 )
		dv = (range * (127.0 - volume))  / 127.0 ;
	else
		dv = (range * volume)  / 127.0 ;
	vMin = volume - dv  ;
	vMax = volume + dv ;
	if ( vMin < 1.0 ) vMin = 1.0 ;
	if ( vMax < 1.0 ) vMax = 1.0 ;
	if ( vMin > 127.0 ) vMin = 127.0 ;
	if ( vMax > 127.0 ) vMax = 127.0 ;
}
void potarAdcPreset()
{
  // preset adc for the two potars
  adc->adc0->startSingleRead(potarPin0);
  adc->adc1->startSingleRead(potarPin1);
  adcState = adcPotar ;
}
void potarProcess()
{
	// read potars [1..127], and set range according to
	uint8_t v0 , v1  ;

  if ( adcState != adcPotar )
  {
    potarAdcPreset() ; // error in preset ADC ...!!!
    Serial.println("Error potarAdcPreset");
  }
  v0 = (adc->adc0->readSingle()) >> 1 ;  // 8bits => 7 bits
  v1 = (adc->adc1->readSingle()) >> 1 ;  // 8bits => 7 bits
  opticalAdcPreset(optical);

	if (( abs( v0 - potarV0) > 4) || ( abs( v1 - potarV1 > 4)))
	{
		// new potar values
 		potarV0 = (v0 ==0)?1:v0 ;
		potarV1 = (v1 ==0)?1:v0  ;
		potarSet();
	}
}

////////////
// optical
////////////
void opticalInit()
{
  // initialization of optical buttons
  uint8_t nr ;
  T_optical *o  ;
  for(nr = 0 , o = optical ; nr < opticalNb ; nr ++ , o ++ )
  {
    o->nr = nr ;
    o->pin = opticalPin[nr] ;
    pinMode( o->pin, INPUT_DISABLE );  // ? 
    o->state = 0 ;
  }
  for(nr = 0 ; nr < tvMax ; nr ++)
    tvNb[nr] = 0 ; // statistic of number of points collected v=f(t)
}
void opticalAdcPreset(T_optical *o)
{
  // prepare two analog convertions
  adc->adc0->startSingleRead(o->pin);
  adc->adc1->startSingleRead((o+1)->pin);
  adcState = adcOptical + o->nr ;
}
void opticalAdcRead(T_optical *o)
{
  if ( adcState != (adcOptical + o->nr) )
  {
    opticalAdcPreset(o) ; // error in preset ADC ...!!!
	  #ifdef DEBUGMODE
    Serial.println("Error opticalAdcPreset");
	  #endif
  }
  o->v = adc->adc0->readSingle() ;
  (o+1)->v = adc->adc1->readSingle() ;
  opticalAdcPreset(((o->nr)==0)?(optical+2):(optical)) ; // prepare next two adc
}
float regressionSlope(float* v, float* t, uint8_t nb /* , float* lrCoef,*/ )
{
  // pass t and v arrays (pointers of v=f(t)), nb is length of the t and v arrays.
  // return the abs(slope ) 

  float tbar=0;
  float vbar=0;
  float tvbar=0;
  float tsqbar=0;
  float fnb = (float)nb;
  uint8_t nr ;

  if (nb < 2)
  {
    // not enough point captured : maximal slope
    (tvNb[1]) ++ ;
	  #ifdef DEBUGMODE
    Serial.println("regressionSlope : not enough measurement");
	  #endif
    return( 999999999.0 ) ; 
  }
  (tvNb[nb]) ++ ; // statistic of number of points
	#if DEBUGMODE > 1
	for(uint8_t i = 0 ; i < tvMax ; i ++ )
		{
			Serial.print("statsNbTv[");
			Serial.print(i);
			Serial.print("]=");
			Serial.print(tvNb[i]);
		}
	#endif

  // linear regression on points {x,y}
  for (nr=0; nr < nb; nr++)
  {
    tbar += t[nr];
    vbar += v[nr];
    tvbar += t[nr]*t[nr];
    tsqbar += v[nr]*v[nr];
  }

  tbar /= fnb;
  vbar /= fnb;
  tvbar /= fnb;
  tsqbar /= fnb;
  
  // return abs(slope)
  return(abs((tvbar-tbar*vbar)/(tsqbar-tbar*tbar)));
  /*
  slope=(tvbar-tbar*vbar)/(tsqbar-tbar*tbar);
  y0=vbar-lrCoef[0]*tbar;
  */
}
uint8_t velo(float slope)
{
	// calculate velocity [1..127] according to slope and ranges of velocity 
  // 0 < Vmin < velocity=f(slope) < Vmax < 128
  int v ;
  v = vMin + (slope - slopeMin )* (vMax - vMin)/( slopeMax - slopeMin)  ;
  if ( v > 127 ) v = 127 ;
  if ( v < 1 ) v = 1 ;
  return v ;
}
void opticalMsgOn(T_optical *o)
{
	float slope ;
  // calculation of the slope v=f(t) : abs(f'(t))
  slope = regressionSlope(o->ft, o->fv, o->tvNr ) ; 
	if (calibrationState != 0)
	{
		// calibration min/max of slope
		if ( slope < slopeMin )
			slopeMin = slope ;
		if ( slope > slopeMax )
			slopeMax = slope ;
	}
	else
	{
    // send notOn with calculated velocity
    midiNote(o->nr + opticalPitch, velo(slope));
	}  
}
void opticalMsgOff(T_optical *o)
{
  // send noteOff
  midiNote(o->nr + opticalPitch,0);
}
bool opticalProcess()
{
  // scan optical buttons
  uint8_t nr ;
  bool modulo2 ;
  bool opticalSleeping ;
  T_optical *o  ;
  opticalSleeping = true ;
  for(nr = 0 , o = optical , modulo2 = true; nr < opticalNb ; nr ++ , o ++ , modulo2 = ! modulo2 )
  {
    if (modulo2) 
      opticalAdcRead(o) ; // read adc0 and adc1, and prepare next ones. Unsigned 8 bits [0..2^8]
    switch(o->state)
    {
      case 0 :
        if (o->v < triggerMax)
        { // start of the slope  
          o->since = 0 ;
          o->state = 1;
          // first point within the slope v=f(t)
          o->ft[0] = 0.0 ;
          o->fv[0] = (float)(o->v) ;
          o->tvNr = 1 ;
          opticalSleeping = false ;	
        }
        break ;
      case 1 :
        if (o->v < triggerMin)
        { // end of slope v=f(t)
          o->since = 0 ;
          o->state = 2;
          opticalMsgOn(o);
          opticalHappen = true ;
       }
        else
        {
          if (o->tvNr < tvMax)
          { // one more point for the slope v=f(t)
            o->ft[o->tvNr] = (float)(o->since) ;
            o->fv[o->tvNr] = (float)(o->v) ;
            (o->tvNr) ++ ;
            opticalSleeping = false ;
          }
          if (o->since > 200*1000) // microsec 
          { // timeout
            o->state = 3 ;
          }
        }
        break ;
      case 2 :
        // stabilisation on
        if (o->since > 5*1000) // µs
          o->state = 3 ;
        break ;
      case 3 :
        if (o->v > triggerMax) 
        {
          // come back to original position
          o->since = 0 ;
          o->state = 4;
          opticalMsgOff(o);
        }
        break ;
      case 4 :
        // stabilisation off
        if (o->since > 5*1000) // µs 
          o->state = 0 ;
        break ;
      default :
        // state zombi
        o->state = 0 ;	
        Serial.println("Error state optical");
        break ;
    }
  }
  if (opticalSleeping)
    potarAdcPreset();

  return opticalSleeping ;
}

////////
// ADC
////////
void adcInit()
{
  adc->adc[0]->setResolution(8); // unsigned 8 bits [0..2^8]
  adc->adc[0]->setAveraging(0);
  adc->adc[0]->setConversionSpeed(ADC_CONVERSION_SPEED::HIGH_SPEED);
  adc->adc[0]->setSamplingSpeed(ADC_SAMPLING_SPEED::HIGH_SPEED);
  adc->adc[1]->setResolution(8);
  adc->adc[1]->setAveraging(0);
  adc->adc[1]->setConversionSpeed(ADC_CONVERSION_SPEED::HIGH_SPEED);
  adc->adc[1]->setSamplingSpeed(ADC_SAMPLING_SPEED::HIGH_SPEED);
  adcState = adcNothing ;

}
bool calibration(bool confOk)
{
  // return true : continue in scanning optical button
  // calibrationState = 0 : end of calibration
	switch (calibrationState )
	{	
		case 1: // test to enter in calibration mode (conf not OK, or button-optical pressed)
      opticalAdcRead(optical) ;
      opticalAdcPreset(optical) ;
      if ((! confOk) || ((optical[0].v) < 100))
      {
        calibrationState = 2 ;
        triggerMin = (2^8) - 1 ;
        triggerMax = 0 ;
        since = 0 ;
        digitalWrite(ledPin[1],LOW) ;
        return true ; // don't continue in opticalButtoon read
      }
      else
      {
        calibrationState = 0 ;
        ledSet(0,0);
        return false ; // nothing to configure, lets conitnue
      }
    case 2 : case 3 : case 4 : case 5 : case 6 : case 7 : case 8 : case 9 : case 10 :
      // read min max of optical button v
      opticalAdcRead(optical) ;
      opticalAdcPreset(optical) ;
      if (optical[0].v < triggerMin) triggerMin = optical[0].v ;
      if (optical[0].v > triggerMax) triggerMax = optical[0].v ;
			if (since > 500) // ms
			{
				digitalToggle(ledPin[0]) ;
				calibrationState ++ ;
				since = 0 ;
			}
			return true ; // don't continue in opticalButtoon read
    case 11 :
      // set trigger of v in range [1/4 .. 3/4]
      triggerMin += (triggerMax - triggerMin) / 4 ;
      triggerMax -= (triggerMax - triggerMin) / 4 ;
      slopeMin = 9999999.0 ;
      slopeMax = -99999999.0 ;
      calibrationState = 12 ;
      optical[0].state = 3 ;
      opticalHappen = false ;
      return false ; // continue in opticalButtoon read
    case 12 : case 13 : case 14 : case 15 : case 16 : case 17 : case 18 : case 19 : case 20 : case 21 :
 			// underground calibration of slope (v=f(t) ) 
      if (since > 500) // ms
			{
				digitalToggle(ledPin[2]) ;
				calibrationState ++ ;
				since = 0 ;
			}
      return false ; // continue in opticalButtoon read 
		case 22 :
			// save calibration
      if (opticalHappen)
  			confSave();
      calibrationState = 0 ;
      ledSet(0,0);
      return false ; // continue in opticalButtoon read
		default :
      return false ; // continue in opticalButtoon read
	}
}

////////////
void setup()
////////////
{
 	ConfOk = confRead() ; 

  adcInit();
  potarInit();
  opticalInit();
	buttonInit();
  ledInit();
	s2Init() ;

  opticalAdcPreset(optical) ;

	calibrationState = 1 ;
	since = 0 ;	// ms
}

///////////
void loop()
///////////
{
	bool opticalSleeping  ;

  if ((calibrationState != 0) && (calibration(ConfOk)))
    return ; // calibration intrusive in progress

  // process optical buttons
  opticalSleeping = opticalProcess() ; 
	
	if ( opticalSleeping) // no optical measurement in progress. 
	{
    // process buttons
    buttonProcess();
	// show the bargraph on the leds
	ledShow() ;
    // process potar (volume/range)
    potarProcess();
		// transmit Midi-in => S2-expander
		s2Process() ;
	}
	
}
