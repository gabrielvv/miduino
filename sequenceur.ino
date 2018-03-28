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
#define PIN_LED_ONE    11
#define PIN_LED_TWO    13
#define PIN_BUTTON_ONE 2
#define PIN_BUTTON_TWO 3
#define PIN_BUTTON_THREE 4

#define EDIT_MODE     123
#define PLAY_MODE     124

// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS      8
#define LED_STRIPES    2

const int POT_ONE = A0;
const int POT_TWO = A1;

int currentPos = 0;
int prevStep = 0;
int tempo=0;

int noteSelectorCurrent = 0;
int prevNote = 0;

int stepStates[NUMPIXELS] = {1,1,1,1,1,1,1,1};
int stepPlaying[NUMPIXELS] = {0,0,0,0,0,0,0,0};
int stepNotes[NUMPIXELS] = {1,5,2,5,1,5,2,5};
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
  /*violet */pixels.Color(102*0.2,0,153*0.2),
  /* rose */pixels.Color(255*0.2,0,127*0.2),
  /* vert*/ pixels.Color(0,255*0.2,127*0.2),
  /* rouge */ pixels.Color(255*0.2,9*0.2,33*0.2),
  /* marron */ pixels.Color(132*0.2,46*0.2,27*0.2),
  /* orange */ pixels.Color(255*0.2,127*0.2,0*0.2),
  /* bleu */ pixels.Color(44*0.2,117*0.2,255*0.2),
  /* yellow */pixels.Color(255*0.2,255*0.2,0)
};

int button_one_state = 0;         // variable for reading the pushbutton status
int button_two_state = 0;         // variable for reading the pushbutton status
int delayval = 500; // delay for half a second

void setup() {
  cleanLeds(PIN_LED_ONE);
  cleanLeds(PIN_LED_TWO);

  // initialize the pushbutton pin as an input:
  pinMode(PIN_BUTTON_ONE, INPUT_PULLUP);
  pinMode(PIN_BUTTON_TWO, INPUT_PULLUP);

  // This is for Trinket 5V 16MHz, you can remove these three lines if you are not using a Trinket
#if defined (_AVR_ATtiny85_)
  if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
#endif
  // End of trinket special code

  pixels.begin(); // This initializes the NeoPixel library.
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

int global_mode = EDIT_MODE;
int button_one_debounce_state = LOW;
int button_two_debounce_state = LOW;

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
    if(stepStates[h] == 1 && h != currentPos){
      pixels.setPixelColor(h, pitchColors[stepNotes[h]]);
    }else if(stepStates[h] == 1 && h == currentPos){
      if(global_mode == PLAY_MODE){
        pixels.setPixelColor(h, GREEN);
      }else{
        pixels.setPixelColor(h, pitchColors[stepNotes[h]]);
      }
    }
    else if(h != currentPos){
      pixels.setPixelColor(h, NO_LIGHT);
    }else if(h == currentPos){
      pixels.setPixelColor(h, GREEN);
    }
  }
  pixels.show();
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

int readButtonState(int buttonId){
  if(buttonId == PIN_BUTTON_ONE){
    button_one_state = digitalRead(PIN_BUTTON_ONE);
    if(button_one_state == HIGH && button_one_debounce_state == LOW){
      button_one_debounce_state = HIGH;
    }
    if(button_one_state == LOW && button_one_debounce_state == HIGH){
      Serial.println("button 1 pressed");
      button_one_debounce_state = LOW;
      switch(global_mode){
        case EDIT_MODE:
          cleanLeds(PIN_LED_ONE);
          Serial.println("EDIT_MODE => PLAY_MODE");
          return global_mode = PLAY_MODE;
          break;
        case PLAY_MODE:
          Serial.println("PLAY_MODE => EDIT_MODE");
          return global_mode = EDIT_MODE;
          break;
      }
    }
  }

  if(buttonId == PIN_BUTTON_TWO){
    button_two_state = digitalRead(PIN_BUTTON_TWO);
    if(button_two_state == HIGH && button_two_debounce_state == LOW){
      button_two_debounce_state = HIGH;
    }
    if(button_two_state == LOW && button_two_debounce_state == HIGH){
      Serial.println("button 2 pressed");
      button_two_debounce_state = LOW;
      return stepStates[currentPos] = stepStates[currentPos] == 0 ? 1 : 0;
    }
  }
}

int getTempo(){
  int val = analogRead(A0);
  return (((float)val / (float)1022) * (float)120) + 60;
}

int getDelay(int tempo){
  return 60000 / tempo / 4;
}

void selectNote(int step, int note){
  stepNotes[step] = note;
}

void selectStep(int step){
  cleanLeds(PIN_LED_ONE);
  pixels.setPixelColor(step, GREEN);
  pixels.show(); 
}

unsigned long previousMillis = 0;

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
  
  if(stepStates[currentPos] == 1){
    noteOn(0, notePitches[stepNotes[currentPos]], intensity);
    stepPlaying[currentPos] = 1;
    MidiUSB.flush();
  }else{
    noteOff(0, notePitches[stepNotes[currentPos]], 0);
    MidiUSB.flush();
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

  /*for(int i = 0; i < NUMPIXELS; i++){
    pixels.setPixelColor(i, pitchColors[i]);
  }
  pixels.show();
  return*/

  readButtonState(PIN_BUTTON_ONE);
  if(global_mode == PLAY_MODE){
    play();
  }
  if(global_mode == EDIT_MODE){

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
    
    readButtonState(PIN_BUTTON_TWO);
    allumerActiveSteps();
  }
}
