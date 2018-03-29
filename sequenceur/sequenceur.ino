#include <Adafruit_NeoPixel.h>

#include <Mouse.h>
#include <ps2.h>

#include <AceButton.h>
using namespace ace_button;
#include <AdjustableButtonConfig.h>
#include <ButtonConfig.h>

#include <frequencyToNote.h>
#include <MIDIUSB.h>
#include <pitchToFrequency.h>
#include <pitchToNote.h>

#define EDIT_MODE 123
#define EDIT_TRACK 125
#define CHANGE_TRACK 126
#define PLAY_MODE 127

#define NOTE_NUMBER 13

const int STEP_NUMBER = 16;
const int TRACK_NUMBER = 8;

int currentTrackPos = 0;
int prevTrackPos = 0;
int currentPos = 0;
int prevStep = 0;
int tempo=0;

int selectedNoteIndex = 0;
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
const int POT_THREE = A3;

/*********************************************************************************************
 
  ##    ## ########  #######  ########  #### ##     ## ######## ##        ######  
  ###   ## ##       ##     ## ##     ##  ##   ##   ##  ##       ##       ##    ## 
  ####  ## ##       ##     ## ##     ##  ##    ## ##   ##       ##       ##       
  ## ## ## ######   ##     ## ########   ##     ###    ######   ##        ######  
  ##  #### ##       ##     ## ##         ##    ## ##   ##       ##             ## 
  ##   ### ##       ##     ## ##         ##   ##   ##  ##       ##       ##    ## 
  ##    ## ########  #######  ##        #### ##     ## ######## ########  ######  

*********************************************************************************************/  

const Adafruit_NeoPixel pixelsStep = Adafruit_NeoPixel(STEP_NUMBER, PIN_LED_TWO, NEO_GRB + NEO_KHZ800);
const Adafruit_NeoPixel pixelsTrack = Adafruit_NeoPixel(TRACK_NUMBER, PIN_LED_ONE, NEO_GRB + NEO_KHZ800);

const uint32_t RED = pixelsStep.Color(10,0,0);
const uint32_t GREEN = pixelsStep.Color(0,10,0);
const uint32_t YELLOW = pixelsStep.Color(10,10,0);
const uint32_t NO_LIGHT = pixelsStep.Color(0,0,0);

/*****************************************************************************
                  ##     ## #### ########  #### 
                  ###   ###  ##  ##     ##  ##  
                  #### ####  ##  ##     ##  ##  
                  ## ### ##  ##  ##     ##  ##  
                  ##     ##  ##  ##     ##  ##  
                  ##     ##  ##  ##     ##  ##  
                  ##     ## #### ########  #### 
 ****************************************************************************/
const uint8_t MIDI_MAX_VALUE = 127;

/**
 * 
 */
void noteOn(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOn = {0x09, 0x90 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOn);
}

/**
 * 
 */
void noteOff(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOff = {0x08, 0x80 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOff);
}

/**
 * 
 */
void controlChange(byte channel, byte control, byte value) {
  midiEventPacket_t event = {0x0B, 0xB0 | channel, control, value};
  MidiUSB.sendMIDI(event);
}

/**
 * Attention x et y sont des valeurs relative
 * => représente la direction du mouvement sur le trackpad
 */
void xyControlChange(int x, int y){
  static int _x = 0;
  static int _y = 0;
  const static int INCR = 2;
  static int xControlValue = MIDI_MAX_VALUE;
  static int yControlValue = MIDI_MAX_VALUE;
  
  if(_x != x){
    //Serial.println("xControlChange");
    _x = x;
    // Les valeurs MIDI doivent appartenir à l'intervall 0-127
    // @see https://www.midi.org/specifications/item/table-3-control-change-messages-data-bytes-2
    xControlValue = constrain(_x > 0 ? xControlValue+INCR : xControlValue-INCR, 0, MIDI_MAX_VALUE);
    controlChange(0, 2, xControlValue);
  }

  if(_y != y){
    //Serial.println("yControlChange");
    _y = y;
    // Les valeurs MIDI doivent appartenir à l'intervall 0-127
    // @see https://www.midi.org/specifications/item/table-3-control-change-messages-data-bytes-2
    yControlValue = constrain(_y > 0 ? yControlValue+INCR : yControlValue-INCR, 0, MIDI_MAX_VALUE);
    controlChange(0, 1, yControlValue);
  }
}

const byte NOTE_OFF = -1;
const byte notePitches[NOTE_NUMBER] = {
  NOTE_OFF,
  pitchC2,
  pitchD2b,
  pitchD2,
  pitchE2b,
  pitchE2,
  pitchF2,
  pitchG2b,
  pitchG2,
  pitchA2b,
  pitchA2,
  pitchB2b,
  pitchB2,
};

/**
 * @param {byte} note - la hauteur de la note
 * @return {uint32_t} color - la couleur correspondante grâce au tableau "pitchColors"
 */
uint32_t getColorForNote(byte note){
  
  static const uint32_t pitchColors[NOTE_NUMBER] = {
    NO_LIGHT,
    RED,
    YELLOW,
    /* rose */pixelsStep.Color(255*0.1,0,127*0.1),
    /* vert*/ pixelsStep.Color(0,255*0.1,127*0.1),
    /* rouge */ pixelsStep.Color(255*0.1,9*0.1,33*0.1),
    /* marron */ pixelsStep.Color(132*0.1,46*0.1,27*0.1),
    /* orange */ pixelsStep.Color(255*0.1,127*0.1,0*0.1),
    /* bleu */ pixelsStep.Color(44*0.1,117*0.1,255*0.1),
    /* yellow */pixelsStep.Color(255*0.1,255*0.1,0),
    /* violet */pixelsStep.Color(102*0.1,0,153*0.1),
    pixelsStep.Color(0,0,100),
    pixelsStep.Color(0,100,100),
  };
  
  for(int i = 0; i < NOTE_NUMBER; i++){
    if(notePitches[i] == note){
      return pitchColors[i];
    }
  }

  return NO_LIGHT;
}

struct track {
  int stepNotes[STEP_NUMBER];
  int mute;
  int solo;
};

/* Double Croche */
track trackList[] = {
  { .stepNotes = {pitchC2, NOTE_OFF, NOTE_OFF, pitchC2, NOTE_OFF, NOTE_OFF, pitchC2, NOTE_OFF, pitchC2, NOTE_OFF, NOTE_OFF, NOTE_OFF, NOTE_OFF, pitchC2, NOTE_OFF, pitchC2}, .mute = 0, .solo = 0},
  { .stepNotes = {NOTE_OFF, NOTE_OFF, NOTE_OFF, NOTE_OFF, pitchD2, NOTE_OFF, NOTE_OFF, NOTE_OFF, NOTE_OFF, NOTE_OFF, NOTE_OFF, NOTE_OFF, pitchD2, NOTE_OFF, NOTE_OFF, NOTE_OFF}, .mute = 0, .solo = 0},
  { .stepNotes = {pitchF2, pitchF2, pitchG2b, pitchF2, pitchF2, pitchF2, pitchG2b, pitchF2, pitchF2, pitchF2, pitchG2b, NOTE_OFF, pitchF2, NOTE_OFF, pitchF2, pitchF2}, .mute = 0, .solo = 0},
  { .stepNotes = {pitchD2, pitchD2, NOTE_OFF, NOTE_OFF, NOTE_OFF, NOTE_OFF, pitchE2, NOTE_OFF, pitchD2, NOTE_OFF, NOTE_OFF, NOTE_OFF, NOTE_OFF, NOTE_OFF, NOTE_OFF, NOTE_OFF}, .mute = 0, .solo = 0},
  { .stepNotes = {pitchC2, pitchD2b, pitchD2, pitchE2b, pitchE2, pitchF2, pitchG2b, pitchG2, pitchA2b, pitchA2, pitchB2b, pitchB2, pitchC2, pitchD2b, pitchD2}, .mute = 0, .solo = 0},
  { .stepNotes = {NOTE_OFF, NOTE_OFF, NOTE_OFF, NOTE_OFF, NOTE_OFF, NOTE_OFF, NOTE_OFF, NOTE_OFF, NOTE_OFF, NOTE_OFF, NOTE_OFF, NOTE_OFF, NOTE_OFF, NOTE_OFF, NOTE_OFF, NOTE_OFF}, .mute = 0, .solo = 0},
  { .stepNotes = {NOTE_OFF, NOTE_OFF, NOTE_OFF, NOTE_OFF, NOTE_OFF, NOTE_OFF, NOTE_OFF, NOTE_OFF, NOTE_OFF, NOTE_OFF, NOTE_OFF, NOTE_OFF, NOTE_OFF, NOTE_OFF, NOTE_OFF, NOTE_OFF}, .mute = 0, .solo = 0},
  { .stepNotes = {NOTE_OFF, NOTE_OFF, NOTE_OFF, NOTE_OFF, NOTE_OFF, NOTE_OFF, NOTE_OFF, NOTE_OFF, NOTE_OFF, NOTE_OFF, NOTE_OFF, NOTE_OFF, NOTE_OFF, NOTE_OFF, NOTE_OFF, NOTE_OFF}, .mute = 0, .solo = 0},
};

void noteOffAll(){
  for(int i = 0; i < TRACK_NUMBER; i++){
    track mTrack = trackList[i];
    for(int j = 0; j < STEP_NUMBER; j++){
      byte note = mTrack.stepNotes[j];
      if(note != NOTE_OFF){
        noteOff(i, note, MIDI_MAX_VALUE);
      }
    }
  }
  MidiUSB.flush();
}

/********************************************************************

  ########  ##     ## ######## ########  #######  ##    ##  ######  
  ##     ## ##     ##    ##       ##    ##     ## ###   ## ##    ## 
  ##     ## ##     ##    ##       ##    ##     ## ####  ## ##       
  ########  ##     ##    ##       ##    ##     ## ## ## ##  ######  
  ##     ## ##     ##    ##       ##    ##     ## ##  ####       ## 
  ##     ## ##     ##    ##       ##    ##     ## ##   ### ##    ## 
  ########   #######     ##       ##     #######  ##    ##  ######  
 
******************************************************************* */
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

void handleEventButton1(AceButton* button, uint8_t eventType, uint8_t buttonState) {
  switch(eventType) {
     case AceButton::kEventDoubleClicked:
        if(mode == EDIT_MODE){
            pixelsStep.clear();
            Serial.println("EDIT_MODE => PLAY_MODE");
            mode = PLAY_MODE;
        }
        else if(mode == PLAY_MODE){
            Serial.println("PLAY_MODE => EDIT_MODE");
            mode = EDIT_MODE;
            noteOffAll();
        }
      break;
      case AceButton::kEventClicked:
        /*int solo = trackList[currentTrackPos].solo = !trackList[currentTrackPos].solo;
        controlChange(0, 202+currentTrackPos, solo*MIDI_MAX_VALUE);
        MidiUSB.flush();*/
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

/**
 * MIDI values < 64 pour OFF, et >= 64 pour ON
 * 
 */
void handleEventButton2(AceButton* button, uint8_t eventType, uint8_t buttonState) {
    
    switch(eventType) {
       case AceButton::kEventClicked:
        int mute = trackList[currentTrackPos].mute = !trackList[currentTrackPos].mute;
        controlChange(0, 102+currentTrackPos, !mute*MIDI_MAX_VALUE);
        MidiUSB.flush();
       break;
    }
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

/**
 *
 *
 * @return {int} distance - distance en cm (oscille typiquement entre 2 et 30), 0 quand champ libre
 */
int getDist(){

  static int getDist_distance = 0;
  static int getDist_prevDistance = 0;
  static int getDist_currentStep = 0; 
  static unsigned long getDist_previousInfraMillis = 0;
  static unsigned long getDist_currentInfraMillis = 0;

  getDist_currentInfraMillis = millis();
  
  if(getDist_currentStep == 0){
    //Serial.println("getDist : step1");
    digitalWrite(PIN_ULTRASOUND_OUT, LOW);
    getDist_currentStep = 1;
    getDist_previousInfraMillis = millis();
  }
  
  // The DYP-ME007 pings on the low-high flank...
  if (getDist_currentInfraMillis - getDist_previousInfraMillis >= 100 && getDist_currentStep == 1) {
    //Serial.println("getDist : step2");
    getDist_currentStep = 2;
    digitalWrite(PIN_ULTRASOUND_OUT, HIGH);
    getDist_previousInfraMillis = millis();
  }
  
  if (getDist_currentInfraMillis - getDist_previousInfraMillis >= 200 && getDist_currentStep == 2) {
    //Serial.println("getDist : step3");
    getDist_previousInfraMillis = 1;
    getDist_currentStep = 0;
    digitalWrite(PIN_ULTRASOUND_OUT, LOW);
    
    // the distance is proportional to the time interval
    // between HIGH and LOW
    // @see http://arcbotics.com/products/sparki/parts/ultrasonic-range-finder/
    getDist_distance = pulseIn(PIN_ULTRASOUND_IN, HIGH) / 58;
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
  
  for(int h=0; h < STEP_NUMBER; h++){
    byte note = currentTrack->stepNotes[h];
    if(note != NOTE_OFF && h != currentPos){
      pixelsStep.setPixelColor(h, getColorForNote(note));
    }else if(note != NOTE_OFF && h == currentPos){
      if(mode == PLAY_MODE){
        pixelsStep.setPixelColor(h, GREEN);
      }else{
        if(currentMillis - prevMillis > 200){
          prevMillis = currentMillis;
          if(blinkState){
            pixelsStep.setPixelColor(h, getColorForNote(note));
          }else{
            pixelsStep.setPixelColor(h, NO_LIGHT);
          }
          blinkState = !blinkState;
        }
      }
    }
    else if(h != currentPos){
      pixelsStep.setPixelColor(h, NO_LIGHT);
    }else if(h == currentPos){
      pixelsStep.setPixelColor(h, GREEN);
    }
  }
  pixelsStep.show();
  pixelsTrack.setPixelColor(currentTrackPos, GREEN);
  pixelsTrack.show();
}

int getPot1(int valNumber = 8){
  int val = analogRead(POT_ONE);
  return (float)val/(float)1022 * valNumber;
}

int getPot2(int valueNumber = 8){
  int val = analogRead(POT_TWO);
  return (float)val/(float)1022 * valueNumber;
}

int getTempo(){
  return getPot2(120) + 60;
}

int getDelay(int tempo){
  return 60000 / tempo / 2;
}

void selectStep(int step){
  pixelsStep.clear();
  pixelsStep.setPixelColor(step, GREEN);
  pixelsStep.show(); 
}

void selecTrack(int trackStep){
  pixelsTrack.clear();
  pixelsTrack.setPixelColor(trackStep, GREEN);
  pixelsTrack.show();
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
    
    for(int i = 0; i < TRACK_NUMBER; i++){
      track mTrack = trackList[i];
      byte note = mTrack.stepNotes[currentPos];
      byte prevNote = mTrack.stepNotes[currentPos-1 < 0 ? STEP_NUMBER-1 : currentPos-1];
      if(note != NOTE_OFF){
        noteOn(i, note, MIDI_MAX_VALUE);
        midiFlush = 1;
      }
      if(prevNote != NOTE_OFF && note != prevNote){
        noteOff(i, prevNote, MIDI_MAX_VALUE);
        midiFlush = 1;
      }
    }
    
    allumerActiveSteps();
    pixelsStep.show(); // This sends the updated pixel color to the hardware.
    currentPos++;
    if(currentPos == STEP_NUMBER) currentPos = 0;
   }

   return midiFlush;
}
