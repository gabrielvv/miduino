#include <frequencyToNote.h>
#include <MIDIUSB.h>
#include <pitchToFrequency.h>
#include <pitchToNote.h>

// NeoPixel Ring simple sketch (c) 2013 Shae Erisson
// released under the GPLv3 license to match the rest of the AdaFruit NeoPixel library

#include <Adafruit_NeoPixel.h>
#ifdef _AVR_
  #include <avr/power.h>
#endif

#define NUM_BUTTONS  7

// Which pin on the Arduino is connected to the NeoPixels?
// On a Trinket or Gemma we suggest changing this to 1
#define PIN_LED_ONE    13
#define PIN_LED_TWO    7
#define PIN_BUTTON_ONE 2
#define PIN_BUTTON_TWO 3
#define PIN_BUZZER     4

#define EDIT_MODE     123
#define PLAY_MODE     124

// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS      8
#define LED_STRIPES    2

int currentPos = 0;
int prevStep = 0;
int tempo=0;

int stepStates[NUMPIXELS] = {0,0,0,0,0,0,0,0};
int stepPlaying[NUMPIXELS] = {0,0,0,0,0,0,0,0};
const byte notePitches[NUM_BUTTONS] = {pitchC3, pitchD3, pitchE3, pitchF3, pitchG3, pitchA3, pitchB3};

uint8_t intensity = 127;

// When we setup the NeoPixel library, we tell it how many pixels, and which pin to use to send signals.
// Note that for older NeoPixel strips you might need to change the third parameter--see the strandtest
// example for more information on possible values.
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN_LED_ONE, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel pixels2 = Adafruit_NeoPixel(NUMPIXELS, PIN_LED_TWO, NEO_GRB + NEO_KHZ800);

uint32_t menuColors[3] = {pixels.Color(10,0,0), pixels.Color(0,10,0), pixels.Color(10,10,0)};

uint32_t pitchColors[NUMPIXELS] = {
  /*violet */pixels2.Color(102,0,153), 
  /* rose */pixels2.Color(255,0,127),
  /* vert*/ pixels2.Color(0,255,127),
  /* rouge */ pixels2.Color(255,9,33),
  /* marron */ pixels2.Color(132,46,27),
  /* orange */ pixels2.Color(255,127,0),
  /* bleu */ pixels2.Color(44,117,255),
};

int button_one_state = 0;         // variable for reading the pushbutton status
int button_two_state = 0;         // variable for reading the pushbutton status
int delayval = 500; // delay for half a second

void setup() {

  pinMode(PIN_BUZZER, OUTPUT);

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

void loop() {

  for(int i = 0; i < NUMPIXELS; i++){
  pixels2.setPixelColor(i, pitchColors[i]); // Moderately bright green color.
  }
  pixels2.show(); // This sends the updated pixel color to the hardware.
  
  readButtonState(PIN_BUTTON_ONE);
  Serial.println(global_mode);
  if(global_mode == PLAY_MODE){
    play();
  }else{
    noTone(PIN_BUZZER);
  }
  if(global_mode == EDIT_MODE){
    
    currentPos = getStep();
    Serial.println(currentPos);
    if(prevStep != currentPos){
      selectStep(currentPos);
      prevStep = currentPos;
    }
  
    readButtonState(PIN_BUTTON_TWO);
    allumerActiveSteps();
  }
}

void allumerActiveSteps(){
  for(int h=0; h < NUMPIXELS; h++){
    if(stepStates[h] == 1 && h != currentPos){
      pixels.setPixelColor(h, pixels.Color(10,0,0));
    }else if(stepStates[h] == 1 && h == currentPos){
      pixels.setPixelColor(h, pixels.Color(10,10,0));
    }
    else if(h != currentPos){
      pixels.setPixelColor(h, pixels.Color(0,0,0));
    }else if(h == currentPos){
      pixels.setPixelColor(h, pixels.Color(0,10,0));
    }
  }
  pixels.show();
}

int cleanLeds(){
   for(int h=0; h < NUMPIXELS; h++){
    pixels.setPixelColor(h, pixels.Color(0,0,0));
  }
  pixels.show();
}

int getStep(){
  int val = analogRead(A0);
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
          cleanLeds();
          Serial.println("EDIT_MODE => PLAY_MODE");
          return global_mode = PLAY_MODE;
          break;
        case PLAY_MODE: 
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
  return 60000 / tempo;
}

void selectStep(int step){
  cleanLeds();
  pixels.setPixelColor(step, pixels.Color(0,10,0)); // Moderately bright green color.
  pixels.show(); // This sends the updated pixel color to the hardware.
}

void enterStep(int step){
  cleanLeds();
  pixels.setPixelColor(step, pixels.Color(10,0,0)); // Moderately bright green color.
  pixels.show(); // This sends the updated pixel color to the hardware.
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
    //tone(PIN_BUZZER, 1000);
    noteOn(0, notePitches[0], intensity);
    stepPlaying[currentPos] = 1;
    MidiUSB.flush();
  }else{
    noteOff(0, notePitches[0], 0);
    MidiUSB.flush();
    //noTone(PIN_BUZZER);
  }
  allumerActiveSteps();
  pixels.show(); // This sends the updated pixel color to the hardware.
  
  

    currentPos++;
    if(currentPos == NUMPIXELS) currentPos = 0;
   }
}
