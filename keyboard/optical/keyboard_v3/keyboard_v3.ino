
#include <EEPROM.h>
#include <ADC.h>
#include <ADC_util.h>

// optical
#define opticalNb 4 // nb optical button
#define tvMax 10 // max points to calculate the optical slope
struct T_optical // structure of an optical button
{
  uint8_t nr ;
  uint8_t pin ; // analog pin to read
  uint8_t state ; // state of the optical button
  elapsedMicros since ; // timer to measeure the slope
  float t[tvMax] , v[tvMax]; // points of the slope v=f(t) for velocity calculation
  uint8_t tvNr ; // pointer for the points of the slope
} ;
T_optical optical[opticalNb]; // optical buttons
uint16_t tvNb[tvMax] ; // stat 
float slopeMax , slopeMin ; // min-max of the slope for optical measure
uint8_t triggerMin, triggerMax ; // min max of the trigger to measure 
float vMin, vMax ; // min max of note-on velocity 
bool opticalSleeping  ; // optical not in activity
#define opticalPitch 1 // offset for noteOn
uint8_t opticalPin[opticalNb] = { A0 , A1 , A2 , A3 } ; // pins for the led
bool opticalHappen ;

// buttons
#define buttonNb 3 // nb button
struct T_button
{
  uint8_t nr ;
  uint8_t pin ; // analog pin to read
  uint8_t state ; // state of the optical button
  elapsedMillis since ; 
} ;
T_button button[buttonNb] ;
uint8_t buttonPin[buttonNb] = { 18 , 19 , 20  }; // digital pin for buttons
#define  buttonPitch 16 // offset for noteOn

// led
#define ledNb 3 // bb led
uint8_t ledPin[ledNb] = { 10 , 11 , 12 } ; // pins for the led
uint8_t ledOn ;

// potar
#define potarNb 2 // nb potar
uint8_t potarPin[potarNb] = { A4 , A5 } ; // pins for the potar
float potarValue[potarNb] = { 64.0 , 64.0 } ;

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
bool forceCalibration ;

// configuration memory
///////////////////////
void confSave()
{
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
    return false ;
	}
	else
	{
		slopeMin = 1.0 ; // bits/msec
		slopeMax = 100.0; // bits/msec
    triggerMin = 56 ;
    triggerMax = 200 ;
    return true ;
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
void ledSet(uint8_t v , uint8_t ledC)
{
	// affiche un bargraph de v 0,[1..127]
  // sur le "canal" led ledC
  // pour effacer, mettre la valeur 0 sur le canal qui a allume
  // pour forcer l'effacage, prendre le canal ledC = 0 
	uint8_t i ;
  uint16_t nrl; 
  if ((ledC != 0) && ((v==0) && (ledC!= ledOn)))
    return ;
	for(i = 0 ; i < ledNb ; i ++ )
		digitalWrite(ledPin[i],HIGH);
	if ( v == 0 )
 		return;
  ledOn = ledC ;
	nrl = ((uint16_t)(ledNb)  * (uint16_t)(v))/((uint16_t)(128)) ; 
	if (nrl < 0 ) nrl = 0 ;
	if (nrl >= ledNb) nrl = ledNb - 1 ;
	digitalWrite(ledPin[nrl],LOW);
}
void ledInit()
{
  uint8_t i ;
  for(i = 0 ; i < ledNb ; i ++)
    pinMode(ledPin[i], OUTPUT);
  ledSet(0,0);
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
    pinMode(buttonPin[i], INPUT_PULLUP);
    b->nr = i ;
    b->pin = buttonPin[i];
    b->state = 0 ;
  }
}
void buttonRead()
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
        b->state = 0 ;	
        break ;
    }
  }
}

// potar
////////
void potarSet()
{
	// regle les velocite min et velocite max en sortie en fonction du potar de moyenne et du potar de dynamique
	float  volume, range , dv ;
	volume = potarValue[0] ;
	range = potarValue[1] ;
	if ( volume > 64.0 )
		dv = (range * (127.0 - volume))  / 127.0 ;
	else
		dv = (range * volume)  / 127.0 ;
	vMin = volume - dv  ;
	vMax = volume + dv ;
	if ( vMin < 1 ) vMin = 1.0 ;
	if ( vMax < 1 ) vMax = 1.0 ;
	if ( vMin > 127 ) vMin = 127.0 ;
	if ( vMax > 127 ) vMax = 127.0 ;
}
void potarRead()
{
	// read potars [1..127], and set range according to
	float v[potarNb]  ;
  uint8_t nr ;
  for(nr = 0 ; nr < potarNb ; nr ++ )
  {
    v[nr] = (float)((adc->adc0->analogRead(potarPin[nr]) )) / 2.0 ;  // 8bits => 7 bits
    if (v[nr] < 1.0) v[nr] = 1.0 ;
    if (v[nr] > 127.0 ) v[nr] = 127.0 ;
  }

	if (( abs( v[0] - potarValue[0]) > 4.0) || ( abs( v[1] - potarValue[1]) > 4.0))
	{
		// new potar values
		potarValue[0] = v[0] ;
		potarValue[1] = v[1] ;
		potarSet();
	}
}

// optical
//////////
void opticalInit()
{
  uint8_t nr ;
  T_optical *o  ;
  for(nr = 0 , o = optical ; nr < opticalNb ; nr ++ , o ++ )
  {
    o->pin = opticalPin[nr] ;
    o->state = 0 ;
    o->nr = nr ;
  }
  for(nr = 0 ; nr < tvMax ; nr ++)
    tvNb[nr] = 0 ;
}
void opticalPinPreset(T_optical *o)
{
  adc->adc0->startSingleRead(o->pin);
}
int opticalPinRead(T_optical *o)
{
  uint8_t v ;
  if ((adc->adc0->isComplete()) or (adc->adc0->isConverting()))
    v = adc->adc0->readSingle() ;
  else
    v = adc->adc0->analogRead(o->pin);
  if (o->nr < (opticalNb - 1))
    opticalPinPreset(o+1);
  return(v);
}
float regressionSlope(float* x, float* y, int n /* , float* lrCoef,*/ )
{
  // pass x and y arrays (pointers), n is length of the x and y arrays.
  // return the slope  
  // The lrCoef array is comprised of the slope=lrCoef[0] and intercept=lrCoef[1].  
  // http://en.wikipedia.org/wiki/Simple_linear_regression

  // initialize variables
  float xbar=0;
  float ybar=0;
  float xybar=0;
  float xsqbar=0;
  
  if (n < 2)
  {
    tvNb[1] ++ ;
    return( 999999999.0 ) ; // not enough point : maximal slope
  }
  tvNb[n] ++ ;

  // calculations required for linear regression
  for (int i=0; i<n; i++){
    xbar += x[i];
    ybar += y[i];
    xybar += x[i]*y[i];
    xsqbar += x[i]*x[i];
  }
  xbar=xbar/(float)n;
  ybar=ybar/(float)n;
  xybar=xybar/(float)n;
  xsqbar=xsqbar/(float)n;
  
  // return slope
  return((xybar-xbar*ybar)/(xsqbar-xbar*xbar));
  /*
  lrCoef[0]=(xybar-xbar*ybar)/(xsqbar-xbar*xbar);
  lrCoef[1]=ybar-lrCoef[0]*xbar;
  */
}
void opticalMsgOn(T_optical *o)
{
	// calculate veolocity [1..127] accordng samples read on pin, and ranges of velocity 
	int v ;
	float slope ;
  slope = regressionSlope(o->t, o->v, o->tvNr ) ; 
	if (calibrationState != 0)
	{
		// calibration min/max of slope
		if ( slope < slopeMin )
			slopeMin = slope ;
		if ( slope > slopeMax )
			slopeMax = slope ;
		v = 64 ;
	}
	else
	{
		if ( slope < slopeMin )
			slope = slopeMin ;
		if ( slope > slopeMax )
			slope = slopeMax ;
		v = vMin + (int)(((slope - slopeMin )* (float)(vMax - vMin))/( slopeMax - slopeMin))  ;
		if ( v > 127 ) v = 127 ;
		if ( v < 1 ) v = 1 ;
	}
  midiNote(o->nr + opticalPitch, v);
}
void opticalMsgOff(T_optical *o)
{
  midiNote(o->nr + opticalPitch,0);
}
void opticalRead()
{
  uint8_t nr ;
  int v ;
  T_optical *o  ;
  opticalPinPreset(optical) ;
  for(nr = 0 , o = optical; nr < opticalNb ; nr ++ , o ++ )
  {
    v = opticalPinRead(o) ;
    switch(o->state)
    {
      case 0 :
        if (v > triggerMin)
        {          
          o->since = 0 ;
          o->state = 1;
          opticalSleeping = false ;	
          o->t[0] = 0.0 ;
          o->v[0] = (float)(v) ;
          o->tvNr = 1 ;
        }
        break ;
      case 1 :
        if (v > triggerMax)
        {          
          o->since = 0 ;
          o->state = 2;
          opticalMsgOn(o);
          opticalHappen = true ;
       }
        else
        {
          if (o->tvNr < tvMax)
          {
            o->t[o->tvNr] = (float)(o->since) ;
            o->v[o->tvNr] = (float)(v) ;
            (o->tvNr) ++ ;
            opticalSleeping = false ;
          }
          if (o->since > 500*1000) // timeout
            o->state = 3 ;
        }
        break ;
      case 2 :
        // stabilisation appuye
        if (o->since > 5*1000) // µs
          o->state = 3 ;
        break ;
      case 3 :
        if (v < triggerMin) 
        {
          o->since = 0 ;
          o->state = 4;
          opticalMsgOff(o);
        }
        break ;
      case 4 :
        if (o->since > 5*1000) // µs 
          o->state = 0 ;
        break ;
      default :
        // etat zombi
        o->state = 0 ;	
        break ;
    }
  }
}

// ADC
//////
void adcInit()
{
  adc->adc[0]->setResolution(8);
  adc->adc[0]->setAveraging(0);
  adc->adc[0]->setConversionSpeed(ADC_CONVERSION_SPEED::HIGH_SPEED);
  adc->adc[0]->setSamplingSpeed(ADC_SAMPLING_SPEED::HIGH_SPEED);
}
bool calibration(bool force)
{
  // return true : continue in scanning optical button
  // calibrationState = 0 : end of calibration
  uint8_t v ;
	switch (calibrationState )
	{	
		case 1: // test to enter in calibration mode
      if (force || (adc->adc0->analogRead(optical[0].pin) < 100))
      {
        calibrationState = 2 ;
        triggerMin = (2^8) - 1 ;
        triggerMax = 0 ;
        since = 0 ;
        digitalWrite(ledPin[1],LOW) ;
      }
      return true ; // don't continue in opticalButtoon read
      break ;
    case 2 : case 3 : case 4 : case 5 : case 6 : case 7 : case 8 : case 9 : case 10 :
      v = adc->adc0->analogRead(optical[0].pin) ;
      if (v < triggerMin) triggerMin = v ;
      if (v > triggerMax) triggerMax = v ;
			if (since > 500) // ms
			{
				digitalToggle(ledPin[0]) ;
				calibrationState ++ ;
				since = 0 ;
			}
			return true ; // don't continue in opticalButtoon read
    case 11 :
      triggerMin += (triggerMax - triggerMin) / 4 ;
      triggerMax -= (triggerMax - triggerMin) / 4 ;
      slopeMin = 9999999.0 ;
      slopeMax = -99999999.0 ;
      calibrationState = 12 ;
      optical[0].state = 3 ;
      opticalHappen = false ;
      return true ;
    case 12 : case 13 : case 14 : case 15 : case 16 : case 17 : case 18 : case 19 : case 20 : case 21 :
 			if (since > 500) // ms
			{
				digitalToggle(ledPin[2]) ;
				calibrationState ++ ;
				since = 0 ;
			}
      opticalSleeping = false ;
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
void setup()
{
	forceCalibration = confRead(); 
	buttonInit();
  ledInit();
	s2Init() ;
  adcInit();

	calibrationState = 1 ;
	since = 0 ;	// ms
}

void loop()
{
	opticalSleeping = true ;

  if (calibration(forceCalibration))
    return ; // calibration intrusive in progress

  opticalRead() ;
	
	if ( opticalSleeping) // no optical measurement in progress. 
	{
    // process the buttons
    buttonRead();
		// transmit Midi-in => S2-expander
		s2Process() ;
		if (since > 1000) // ms
		{
			// process current potar (volume/range)
			potarRead();
			since = 0 ;
		}
	}
	
}
