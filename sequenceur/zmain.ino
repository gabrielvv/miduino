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

  Serial.begin(9600);
  while (!Serial) {
   delay(1000); // wait for serial port to connect. Needed for native USB port only
  }
  Serial.println("Initialisation...");  
  Mouse.begin();
  mouse_init(&mouse);
  Serial.println("Trackpad setup done");
  
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

  // This initializes the NeoPixel library.
  pixelsStep.begin();
  pixelsTrack.begin();
  
  //clean all leds
  pixelsStep.clear(); pixelsStep.show();
  pixelsTrack.clear(); pixelsTrack.show();
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

/**
 * @desc
 * On tente d'optimiser en lançant "Midi.flush" qu'une seule fois à la fin de la loop
 * si nécessaire
 * 
 * Lis les 2 boutons
 * Lis le capteur infrarouge
 * Lis le trackpad
 * Lis les 2 potentiomètres
 * Lis le capteur ultra-sons
 */
void loop() {
  
  static int infraState = LOW;
  static int prevDist = 0;
  static int dist = 0;
  static int flushMidi = 0;

  flushMidi = 0;

  xy coord = mouse_read(&mouse);
  xyControlChange(coord.x, coord.y);
  
  button1.check();
  button2.check();
  
  if(mode == PLAY_MODE){
    if(play()) flushMidi = 1;
    currentTrackPos = getPot1(TRACK_NUMBER);
    if(prevTrackPos != currentTrackPos){
      selecTrack(currentTrackPos);
      prevTrackPos = currentTrackPos;
    }
  }
  if(mode == EDIT_MODE && track_mode == EDIT_TRACK){
    currentPos = getPot1(STEP_NUMBER);
    if(prevStep != currentPos){
      selectStep(currentPos);
      prevStep = currentPos;
    }

    selectedNoteIndex = getPot2(NOTE_NUMBER);
    if(prevNote != selectedNoteIndex){
      trackList[currentTrackPos].stepNotes[currentPos] = notePitches[selectedNoteIndex];
      prevNote = selectedNoteIndex;
    }
    allumerActiveSteps();
  }

  if(track_mode == CHANGE_TRACK && mode == EDIT_MODE){
    currentTrackPos = getPot1(TRACK_NUMBER);
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
  
  /*dist = getDist();
  if(abs(dist - prevDist) > 1 && dist < 20){
    //Serial.println("New distance :" + String(dist, DEC));
    prevDist = dist;
    controlChange(1,2,map(dist, 2, 30, 0, MIDI_MAX_VALUE));
    flushMidi = 1;
  }*/
  
  if(flushMidi){
    MidiUSB.flush();
  }
}

