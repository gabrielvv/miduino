#define EDIT_MODE 123
#define EDIT_TRACK 125
#define CHANGE_TRACK 126
#define PLAY_MODE 127

#define NUMPIXELS      8
#define LED_STRIPES    2

struct track {
  int stepStates[NUMPIXELS] = {1,0,1,0,1,0,1,0};
  int stepPlaying[NUMPIXELS] = {0,0,0,0,0,0,0,0};
  int stepNotes[NUMPIXELS] = {1,5,1,5,1,5,1,5};
} track1, track2, track3, track4, track5, track6, track7, track8;

struct track trackList[8]= {track1,track2,track3, track4, track5, track6, track7, track8};

int currentTrackPos = 0;
int prevTrackPos = 0;
int currentPos = 0;
int prevStep = 0;
int tempo=0;

int noteSelectorCurrent = 0;
int prevNote = 0;

int mode = EDIT_MODE;
int track_mode = EDIT_TRACK;

/*********************************************

      ########  #### ##    ##  ######  
      ##     ##  ##  ###   ## ##    ## 
      ##     ##  ##  ####  ## ##       
      ########   ##  ## ## ##  ######  
      ##         ##  ##  ####       ## 
      ##         ##  ##   ### ##    ## 
      ##        #### ##    ##  ######  

**********************************************/

#define PIN_LED_ONE    12
#define PIN_LED_TWO    13
#define PIN_BUTTON_ONE 3
#define PIN_BUTTON_TWO 2
#define PIN_BUTTON_THREE 4
#define PIN_INFRA 5
#define PIN_ULTRASOUND_OUT 6
#define PIN_ULTRASOUND_IN 7

// Analog
const int POT_ONE = A0;
const int POT_TWO = A1;

/********************************************************************

  ########  ##     ## ######## ########  #######  ##    ##  ######  
  ##     ## ##     ##    ##       ##    ##     ## ###   ## ##    ## 
  ##     ## ##     ##    ##       ##    ##     ## ####  ## ##       
  ########  ##     ##    ##       ##    ##     ## ## ## ##  ######  
  ##     ## ##     ##    ##       ##    ##     ## ##  ####       ## 
  ##     ## ##     ##    ##       ##    ##     ## ##   ### ##    ## 
  ########   #######     ##       ##     #######  ##    ##  ######  
 
******************************************************************* */

#include <AceButton.h>
using namespace ace_button;
#include <AdjustableButtonConfig.h>
#include <ButtonConfig.h>

AceButton button1;
AceButton button2;
ButtonConfig buttonConfig;

void handleEventButton(AceButton* button, uint8_t eventType, uint8_t buttonState) {
   if(button->getPin() == PIN_BUTTON_ONE){
      handleEventButton1(button, eventType, buttonState);
   }else if(button->getPin() == PIN_BUTTON_TWO){
      handleEventButton2(button, eventType, buttonState);
   }
}


// Change state for button 1
void handleEventButton1(AceButton* button, uint8_t eventType, uint8_t buttonState) {
  switch(eventType) {
     case AceButton::kEventDoubleClicked:
        if(mode == EDIT_MODE){
            cleanLeds(PIN_LED_ONE);
            Serial.println("EDIT_MODE => PLAY_MODE");
            mode = PLAY_MODE;
        }
        else if(mode == PLAY_MODE){
            Serial.println("PLAY_MODE => EDIT_MODE");
            mode = EDIT_MODE;
        }
      break;
      case AceButton::kEventClicked:
        if(mode == EDIT_MODE){
          if(track_mode == CHANGE_TRACK){
            Serial.println("CHANGE_TRACK => EDIT_TRACK");
            track_mode = EDIT_TRACK;
          }
          else if(track_mode == EDIT_TRACK){
             Serial.println("EDIT_TRACK => CHANGE_TRACK");
             track_mode = CHANGE_TRACK ; 
          }
        }
      break;
  }
}

// CHange state for button 2
void handleEventButton2(AceButton* button, uint8_t eventType, uint8_t buttonState) {
    switch(eventType) {
       case AceButton::kEventReleased:
         struct track *currentTrack = &(trackList[currentTrackPos]);
         currentTrack->stepStates[currentPos] = currentTrack->stepStates[currentPos] == 0 ? 1 : 0;
         break;   
    }
}

/*****************************************************************************
                  ##     ## #### ########  #### 
                  ###   ###  ##  ##     ##  ##  
                  #### ####  ##  ##     ##  ##  
                  ## ### ##  ##  ##     ##  ##  
                  ##     ##  ##  ##     ##  ##  
                  ##     ##  ##  ##     ##  ##  
                  ##     ## #### ########  #### 
 ****************************************************************************/

#include <frequencyToNote.h>
#include <MIDIUSB.h>
#include <pitchToFrequency.h>
#include <pitchToNote.h>

void noteOn(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOn = {0x09, 0x90 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOn);
}

void noteOff(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOff = {0x08, 0x80 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOff);
}

void controlChange(byte channel, byte control, byte value) {
  midiEventPacket_t event = {0x0B, 0xB0 | channel, control, value};
  MidiUSB.sendMIDI(event);
}

const byte notePitches[8] = {
  pitchB1,
  pitchC2,
  pitchD2,
  pitchE2,
  pitchF2,
  pitchG2b,
  pitchA2b, 
  pitchB2b,
};

const uint8_t MIDI_MAX_VALUE = 127;

/*********************************************************************************************
 
  ##    ## ########  #######  ########  #### ##     ## ######## ##        ######  
  ###   ## ##       ##     ## ##     ##  ##   ##   ##  ##       ##       ##    ## 
  ####  ## ##       ##     ## ##     ##  ##    ## ##   ##       ##       ##       
  ## ## ## ######   ##     ## ########   ##     ###    ######   ##        ######  
  ##  #### ##       ##     ## ##         ##    ## ##   ##       ##             ## 
  ##   ### ##       ##     ## ##         ##   ##   ##  ##       ##       ##    ## 
  ##    ## ########  #######  ##        #### ##     ## ######## ########  ######  

*********************************************************************************************/  

#include <Adafruit_NeoPixel.h>
#ifdef _AVR_
  #include <avr/power.h>
#endif

const Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN_LED_ONE, NEO_GRB + NEO_KHZ800);
const Adafruit_NeoPixel pixels2 = Adafruit_NeoPixel(NUMPIXELS, PIN_LED_TWO, NEO_GRB + NEO_KHZ800);

const uint32_t RED = pixels.Color(10,0,0);
const uint32_t GREEN = pixels.Color(0,10,0);
const uint32_t YELLOW = pixels.Color(10,10,0);
const uint32_t NO_LIGHT = pixels.Color(0,0,0);

const uint32_t pitchColors[NUMPIXELS] = {
  /*violet */pixels.Color(102*0.1,0,153*0.1),
  /* rose */pixels.Color(255*0.1,0,127*0.1),
  /* vert*/ pixels.Color(0,255*0.1,127*0.1),
  /* rouge */ pixels.Color(255*0.1,9*0.1,33*0.1),
  /* marron */ pixels.Color(132*0.1,46*0.1,27*0.1),
  /* orange */ pixels.Color(255*0.1,127*0.1,0*0.1),
  /* bleu */ pixels.Color(44*0.1,117*0.1,255*0.1),
  /* yellow */pixels.Color(255*0.1,255*0.1,0)
};

/*******************************************************
  ######  ######## ######## ##     ## ########  
##    ## ##          ##    ##     ## ##     ## 
##       ##          ##    ##     ## ##     ## 
 ######  ######      ##    ##     ## ########  
      ## ##          ##    ##     ## ##        
##    ## ##          ##    ##     ## ##        
 ######  ########    ##     #######  ##  
 **********************************************************/

void setup() {
  
  //clean all led
  cleanLeds(PIN_LED_ONE);
  cleanLeds(PIN_LED_TWO);

  // initialize the pushbutton pin as an input:
  pinMode(PIN_BUTTON_ONE, INPUT_PULLUP);
  pinMode(PIN_BUTTON_TWO, INPUT_PULLUP);
  pinMode(PIN_BUTTON_THREE, INPUT_PULLUP);
  pinMode(PIN_INFRA, INPUT); 
  pinMode(PIN_ULTRASOUND_OUT, OUTPUT);
  pinMode(PIN_ULTRASOUND_IN, INPUT);

  // button set eventHandler;
  buttonConfig.setFeature(ButtonConfig::kFeatureClick);
  buttonConfig.setFeature(ButtonConfig::kFeatureDoubleClick);
  buttonConfig.setEventHandler(handleEventButton);
  button1.setButtonConfig(&buttonConfig);
  button1.init(PIN_BUTTON_ONE);

  button2.setButtonConfig(&buttonConfig);
  button2.init(PIN_BUTTON_TWO);
    
  pixels.begin(); // This initializes the NeoPixel library.

  // This is for Trinket 5V 16MHz, you can remove these three lines if you are not using a Trinket
  #if defined (_AVR_ATtiny85_)
    if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
  #endif
}

/****************************************************************************

##     ## ##       ######## ########     ###        ######   #######  ##     ## ##    ## ########  
##     ## ##          ##    ##     ##   ## ##      ##    ## ##     ## ##     ## ###   ## ##     ## 
##     ## ##          ##    ##     ##  ##   ##     ##       ##     ## ##     ## ####  ## ##     ## 
##     ## ##          ##    ########  ##     ##     ######  ##     ## ##     ## ## ## ## ##     ## 
##     ## ##          ##    ##   ##   #########          ## ##     ## ##     ## ##  #### ##     ## 
##     ## ##          ##    ##    ##  ##     ##    ##    ## ##     ## ##     ## ##   ### ##     ## 
 #######  ########    ##    ##     ## ##     ##     ######   #######   #######  ##    ## ########  
 
 *******************************************************************************/

int getDist(){

  static int getDist_distance = 0;
  static int getDist_prevDistance = 0;
  static int getDist_currentStep = 0; 
  static unsigned long getDist_previousInfraMillis = 0;
  static unsigned long getDist_currentInfraMillis = 0;

  getDist_currentInfraMillis = millis();
  
  if(getDist_currentStep == 0){
    //Serial.println("getDist : step1");
    digitalWrite(6, LOW);
    getDist_currentStep = 1;
    getDist_previousInfraMillis = millis();
  }
  
  // The DYP-ME007 pings on the low-high flank...
  if (getDist_currentInfraMillis - getDist_previousInfraMillis >= 2 && getDist_currentStep == 1) {
    //Serial.println("getDist : step2");
    getDist_currentStep = 2;
    digitalWrite(6, HIGH);
    getDist_previousInfraMillis = millis();
  }
  
  if (getDist_currentInfraMillis - getDist_previousInfraMillis >= 10 && getDist_currentStep == 2) {
    //Serial.println("getDist : step3");
    getDist_previousInfraMillis = 1;
    getDist_currentStep = 0;
    digitalWrite(6, LOW);
    // the distance is proportional to the time interval
    // between HIGH and LOW
    getDist_distance = pulseIn(7, HIGH)/58;
    if(getDist_prevDistance != getDist_distance){
      getDist_prevDistance = getDist_distance;
    }
  }   
  return getDist_distance;                        
}


/*************************************************************
   ###    ##       ##       ##     ## ##     ## ######## ########  
  ## ##   ##       ##       ##     ## ###   ### ##       ##     ## 
 ##   ##  ##       ##       ##     ## #### #### ##       ##     ## 
##     ## ##       ##       ##     ## ## ### ## ######   ########  
######### ##       ##       ##     ## ##     ## ##       ##   ##   
##     ## ##       ##       ##     ## ##     ## ##       ##    ##  
##     ## ######## ########  #######  ##     ## ######## ##     ## 
*******************************************************************/

void allumerActiveSteps(){

  static int currentMillis = 0;
  static int prevMillis = 0;
  static int blinkState = 0;

  currentMillis = millis();
  struct track *currentTrack = &(trackList[currentTrackPos]);
  
  for(int h=0; h < NUMPIXELS; h++){
    if(currentTrack->stepStates[h] == 1 && h != currentPos){
      pixels.setPixelColor(h, pitchColors[currentTrack->stepNotes[h]]);
    }else if(currentTrack->stepStates[h] == 1 && h == currentPos){
      if(mode == PLAY_MODE){
        pixels.setPixelColor(h, GREEN);
      }else{
        if(currentMillis - prevMillis > 500){
          prevMillis = currentMillis;
          if(blinkState){
            pixels.setPixelColor(h, pitchColors[currentTrack->stepNotes[h]]);
          }else{
            pixels.setPixelColor(h, GREEN);
          }
          blinkState = !blinkState;
        }
      }
    }
    else if(h != currentPos){
      pixels.setPixelColor(h, NO_LIGHT);
    }else if(h == currentPos){
      pixels.setPixelColor(h, GREEN);
    }
  }
  pixels.show();
  pixels2.setPixelColor(currentTrackPos, GREEN);
  pixels2.show();
}

/*************************************************************
 ######  ##       ########    ###    ##    ## 
##    ## ##       ##         ## ##   ###   ## 
##       ##       ##        ##   ##  ####  ## 
##       ##       ######   ##     ## ## ## ## 
##       ##       ##       ######### ##  #### 
##    ## ##       ##       ##     ## ##   ### 
 ######  ######## ######## ##     ## ##    ## 
 ***********************************************************/

int cleanLeds(int ledMatrix){
  if(ledMatrix == PIN_LED_ONE){
    for(int h=0; h < NUMPIXELS; h++){
      pixels.setPixelColor(h, NO_LIGHT);
    }
    pixels.show();
  }
  if(ledMatrix == PIN_LED_TWO){
    for(int h=0; h < NUMPIXELS; h++){
      pixels2.setPixelColor(h, NO_LIGHT);
    }
    pixels2.show();
  }
}

int getPot1(){
  int val = analogRead(POT_ONE);
  return (float)val/(float)1022 * 8;
}


int getPot2(){
  int val = analogRead(POT_TWO);
  return (float)val/(float)1022 * 8;
}


int getTempo(){
  int val = analogRead(A0);
  return (((float)val / (float)1022) * (float)120) + 60;
}

int getDelay(int tempo){
  return 60000 / tempo;
}

void selectNote(int step, int note){
  trackList[currentTrackPos].stepNotes[step] = note;
}

void selectStep(int step){
  cleanLeds(PIN_LED_ONE);
  pixels.setPixelColor(step, GREEN);
  pixels.show(); 
}

void selecTrack(int trackStep){
  cleanLeds(PIN_LED_TWO);
  pixels2.setPixelColor(trackStep, GREEN);
  pixels2.show();
}

/*************************************
########  ##          ###    ##    ## 
##     ## ##         ## ##    ##  ##  
##     ## ##        ##   ##    ####   
########  ##       ##     ##    ##    
##        ##       #########    ##    
##        ##       ##     ##    ##    
##        ######## ##     ##    ##   
*****************************************/
int play(){

  static int delayVal = 500; // delay for half a second
  static unsigned long previousMillis = 0;
  unsigned long currentMillis = millis();
  int midiFlush = 0;

  int newTempo = getTempo();
  if(tempo != newTempo){
    tempo = newTempo;
    controlChange(1, 0, map(tempo, 0, 180, 0, 127));
    midiFlush = 1;
  }

  if (currentMillis - previousMillis >= delayVal) {
    previousMillis = currentMillis;
    delayVal = getDelay(tempo);
    
    for(int i = 0; i<8; i++){
      struct track mTrack = trackList[i];
      if(mTrack.stepStates[currentPos] == 1){
        noteOn(i, notePitches[mTrack.stepNotes[currentPos]], MIDI_MAX_VALUE);
        mTrack.stepPlaying[currentPos] = 1;
        midiFlush = 1;
      }else{
        noteOff(0, notePitches[mTrack.stepNotes[currentPos]], 0);
        midiFlush = 1;
      }
    }
    
    allumerActiveSteps();
    pixels.show(); // This sends the updated pixel color to the hardware.
    currentPos++;
    if(currentPos == NUMPIXELS) currentPos = 0;
   }

   return midiFlush;
}

/********************************************
##        #######   #######  ########  
##       ##     ## ##     ## ##     ## 
##       ##     ## ##     ## ##     ## 
##       ##     ## ##     ## ########  
##       ##     ## ##     ## ##        
##       ##     ## ##     ## ##        
########  #######   #######  ##  
*********************************************/


void loop() {
  static int infraState = LOW;
  static int prevDist = 0;
  static int dist = 0;
  static int flushMidi = 0;

  flushMidi = 0;
  button1.check();
  if(mode == PLAY_MODE){
    if(play()) flushMidi = 1;
    currentTrackPos = getPot2();
    if(prevTrackPos != currentTrackPos){
      selecTrack(currentTrackPos);
      prevTrackPos = currentTrackPos;
    }
  }
  if(mode == EDIT_MODE && track_mode == EDIT_TRACK){
    button2.check();
    currentPos = getPot1();
    if(prevStep != currentPos){
      selectStep(currentPos);
      prevStep = currentPos;
    }

    noteSelectorCurrent = getPot2();
    if(prevNote != noteSelectorCurrent){
      selectNote(currentPos, noteSelectorCurrent);
      prevNote = noteSelectorCurrent;
    }
    allumerActiveSteps();
  }

  if(track_mode == CHANGE_TRACK && mode == EDIT_MODE){
    currentTrackPos = getPot2();
    if(prevTrackPos != currentTrackPos){
      selecTrack(currentTrackPos);
      prevTrackPos = currentTrackPos;
      allumerActiveSteps();
    }
 }
  
  if(digitalRead(PIN_INFRA) == HIGH && infraState != HIGH){
    controlChange(1,1,MIDI_MAX_VALUE);
    infraState = HIGH;
    flushMidi = 1;
  }else if(digitalRead(PIN_INFRA) == LOW && infraState != LOW){
    controlChange(1,1,0);
    infraState = LOW;
    flushMidi = 1;
  }
  
  dist = getDist();
  if(abs(dist - prevDist) > 1 && dist < 20){
    //Serial.println("New distance :" + String(dist, DEC));
    prevDist = dist;
    controlChange(1,2,map(dist, 2, 13, 0, MIDI_MAX_VALUE));
    flushMidi = 1;
  }
  
  if(flushMidi){
    MidiUSB.flush();
  }
}

