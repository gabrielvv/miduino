#include <AceButton.h>
using namespace ace_button;
#include <AdjustableButtonConfig.h>
#include <ButtonConfig.h>

#include <frequencyToNote.h>
#include <MIDIUSB.h>
#include <pitchToFrequency.h>
#include <pitchToNote.h>

#include <Adafruit_NeoPixel.h>
#ifdef _AVR_
  #include <avr/power.h>
#endif

// Which pin on the Arduino is connected to the NeoPixels?
// On a Trinket or Gemma we suggest changing this to 1
#define PIN_LED_ONE    12
#define PIN_LED_TWO    13
#define PIN_BUTTON_ONE 2
#define PIN_BUTTON_TWO 3
#define PIN_BUTTON_THREE 4
#define PIN_INFRA 5

AceButton button1;
AceButton button2;
AceButton button3;

ButtonConfig button1Config;
ButtonConfig button2Config;
ButtonConfig button3Config;

#define EDIT_MODE     123
#define PLAY_MODE     124

#define CHANGE_TRACK 125
#define EDIT_TRACK   126

// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS      8
#define LED_STRIPES    2

// Analog
const int POT_ONE = A0;
const int POT_TWO = A1;

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
int infraState = LOW;

int noteSelectorCurrent = 0;
int prevNote = 0;

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

uint8_t intensity = 127;

// When we setup the NeoPixel library, we tell it how many pixels, and which pin to use to send signals.
// Note that for older NeoPixel strips you might need to change the third parameter--see the strandtest
// example for more information on possible values.
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN_LED_ONE, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel pixels2 = Adafruit_NeoPixel(NUMPIXELS, PIN_LED_TWO, NEO_GRB + NEO_KHZ800);

const uint32_t RED = pixels.Color(10,0,0);
const uint32_t GREEN = pixels.Color(0,10,0);
const uint32_t YELLOW = pixels.Color(10,10,0);
const uint32_t NO_LIGHT = pixels.Color(0,0,0);

uint32_t pitchColors[NUMPIXELS] = {
  /*violet */pixels.Color(102*0.1,0,153*0.1),
  /* rose */pixels.Color(255*0.1,0,127*0.1),
  /* vert*/ pixels.Color(0,255*0.1,127*0.1),
  /* rouge */ pixels.Color(255*0.1,9*0.1,33*0.1),
  /* marron */ pixels.Color(132*0.1,46*0.1,27*0.1),
  /* orange */ pixels.Color(255*0.1,127*0.1,0*0.1),
  /* bleu */ pixels.Color(44*0.1,117*0.1,255*0.1),
  /* yellow */pixels.Color(255*0.1,255*0.1,0)
};

int global_mode = EDIT_MODE; 
int track_mode = EDIT_TRACK;

int delayval = 500; // delay for half a second

void setup() {
  
  //clean all led
  cleanLeds(PIN_LED_ONE);
  cleanLeds(PIN_LED_TWO);

  // initialize the pushbutton pin as an input:
  pinMode(PIN_BUTTON_ONE, INPUT_PULLUP);
  pinMode(PIN_BUTTON_TWO, INPUT_PULLUP);
  pinMode(PIN_BUTTON_THREE, INPUT_PULLUP);
  pinMode(PIN_INFRA, INPUT); 

  // button set eventHandler;
  button1Config.setEventHandler(handleEventButton);
  button1.setButtonConfig(&button1Config);
  button1.init(PIN_BUTTON_ONE);

  button2Config.setEventHandler(handleEventButton);
  button2.setButtonConfig(&button2Config);
  button2.init(PIN_BUTTON_TWO);
  
  button3Config.setEventHandler(handleEventButton);
  button3.setButtonConfig(&button3Config);
  button3.init(PIN_BUTTON_THREE);
    
  pixels.begin(); // This initializes the NeoPixel library.

  // This is for Trinket 5V 16MHz, you can remove these three lines if you are not using a Trinket
  #if defined (_AVR_ATtiny85_)
    if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
  #endif
}

void handleEventButton(AceButton* button, uint8_t eventType, uint8_t buttonState) {
    Serial.println(button->getPin());
   if(button->getPin() == PIN_BUTTON_ONE){
      handleEventButton1(button, eventType, buttonState);
   }else if(button->getPin() == PIN_BUTTON_TWO){
      handleEventButton2(button, eventType, buttonState);
   }
   else if(button->getPin() == PIN_BUTTON_THREE){
      handleEventButton3(button, eventType, buttonState);
   }
}


// Change state for button 1
void handleEventButton1(AceButton* button, uint8_t eventType, uint8_t buttonState) {
  Serial.println("PRESSE BUTTON1");
  switch(eventType) {
    Serial.println("play");
    case AceButton::kEventReleased:
      if(global_mode == EDIT_MODE){
          cleanLeds(PIN_LED_ONE);
          Serial.println("EDIT_MODE => PLAY_MODE");
          global_mode = PLAY_MODE;
          track_mode = EDIT_TRACK;
      }
      else if(global_mode == PLAY_MODE){
          Serial.println("PLAY_MODE => EDIT_MODE");
          global_mode = EDIT_MODE;
      }
      break;
  }
}

// CHange state for button 2
void handleEventButton2(AceButton* button, uint8_t eventType, uint8_t buttonState) {
    switch(eventType) {
       case AceButton::kEventReleased:
         Serial.println("button 2 pressed");
         trackList[currentTrackPos].stepStates[currentPos] = trackList[currentTrackPos].stepStates[currentPos] == 0 ? 1 : 0;
         break;   
    }
}

 

// Change state for button 3 
void handleEventButton3(AceButton* button, uint8_t eventType, uint8_t buttonState) {
  switch(eventType) {
    case AceButton::kEventReleased:
       Serial.println("PRESS BUTTON 3");
      if(track_mode == CHANGE_TRACK){
        Serial.println("CHANGE_TRACK => EDIT_TRACK");
        track_mode = EDIT_TRACK;
      }
      else if(track_mode == EDIT_TRACK){
         Serial.println("EDIT_TRACK => CHANGE_TRACK");
         track_mode = CHANGE_TRACK ; 
      }
      break;
  }
}



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
  for(int h=0; h < NUMPIXELS; h++){
    if(trackList[currentTrackPos].stepStates[h] == 1 && h != currentPos){
      pixels.setPixelColor(h, pitchColors[trackList[currentTrackPos].stepNotes[h]]);
    }else if(trackList[currentTrackPos].stepStates[h] == 1 && h == currentPos){
      if(global_mode == PLAY_MODE){
        pixels.setPixelColor(h, GREEN);
      }else{
        pixels.setPixelColor(h, pitchColors[trackList[currentTrackPos].stepNotes[h]]);
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

int getStep(){
  int val = analogRead(POT_ONE);
  return (float)val/(float)1022 * 8;
}


int getNote(){
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

unsigned long previousMillis = 0;

/*************************************
########  ##          ###    ##    ## 
##     ## ##         ## ##    ##  ##  
##     ## ##        ##   ##    ####   
########  ##       ##     ##    ##    
##        ##       #########    ##    
##        ##       ##     ##    ##    
##        ######## ##     ##    ##   
*****************************************/

void play(){

  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= delayval) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;

  int newTempo = getTempo();
  if(tempo != newTempo){
    tempo = newTempo;

    controlChange(1, 0, map(tempo, 0, 180, 0, 127));
    MidiUSB.flush();
    Serial.println(tempo);
  }

  delayval = getDelay(tempo);

  for(int i = 0; i<8; i++){
    if(trackList[i].stepStates[currentPos] == 1){
      noteOn(i, notePitches[trackList[i].stepNotes[currentPos]], intensity);
      trackList[i].stepPlaying[currentPos] = 1;
      MidiUSB.flush();
    }else{
      noteOff(0, notePitches[trackList[i].stepNotes[currentPos]], 0);
      MidiUSB.flush();
    }
 }
  
  allumerActiveSteps();
  pixels.show(); // This sends the updated pixel color to the hardware.
    currentPos++;
    if(currentPos == NUMPIXELS) currentPos = 0;
   }
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

  button1.check();
  button3.check();
  //Serial.println(global_mode);
  if(global_mode == PLAY_MODE){
    play();
  }
  if(global_mode == EDIT_MODE && track_mode == EDIT_TRACK){
    button2.check();
    currentPos = getStep();
    if(prevStep != currentPos){
      selectStep(currentPos);
      prevStep = currentPos;
    }

    noteSelectorCurrent = getNote();
    if(prevNote != noteSelectorCurrent){
      selectNote(currentPos, noteSelectorCurrent);
      prevNote = noteSelectorCurrent;
    }
    allumerActiveSteps();
  }

  if(track_mode == CHANGE_TRACK && global_mode == EDIT_MODE){
    currentTrackPos = getNote();
    if(prevTrackPos != currentTrackPos){
      selecTrack(currentTrackPos);
      prevTrackPos = currentTrackPos;
      allumerActiveSteps();
    }
 }

  if(digitalRead(PIN_INFRA) == HIGH && infraState != HIGH){
    controlChange(1,1,127);
    infraState = HIGH;
    MidiUSB.flush();
  }
  if(digitalRead(PIN_INFRA) == LOW && infraState != LOW){
    controlChange(1,1,0);
    infraState = LOW;
    MidiUSB.flush();
  }
  
}
