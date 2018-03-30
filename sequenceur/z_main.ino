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
  // initialize the pushbutton pin as an input:
  pinMode(PIN_BUTTON_ONE, INPUT_PULLUP);
  pinMode(PIN_BUTTON_TWO, INPUT_PULLUP);
  pinMode(PIN_BUTTON_THREE, INPUT_PULLUP);

  pinMode(PIN_LED_SUBMODE, OUTPUT);
  digitalWrite(PIN_LED_SUBMODE, LOW);
  pinMode(PIN_LED_MODE, OUTPUT);
  digitalWrite(PIN_LED_MODE, LOW);

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
 * Lis les 2 potentiomètres
 */
void loop() {
  static int midiFlush = 0;
  static byte prevNote = 0;
  static byte selectedNoteIndex = 0;

  midiFlush = 0;
  
  button1.check();
  button2.check();
  
  if(mode == PLAY_MODE){
    if(play()) midiFlush = 1;

    if(track_mode == CHANGE_TRACK){
      currentTrackPos = getPot1(TRACK_NUMBER);
      if(prevTrackPos != currentTrackPos){
        selecTrack(currentTrackPos);
        prevTrackPos = currentTrackPos;
      }
    }

    if(track_mode == EDIT_TRACK){
      currentEditStepPos = getPot1(STEP_NUMBER);
      if(prevEditStep != currentEditStepPos){
        selectStep(currentEditStepPos);
        prevEditStep = currentEditStepPos;
      }
    }
  }

  if(track_mode == EDIT_TRACK){
    selectedNoteIndex = getPot2(NOTE_NUMBER);
    if(prevNote != selectedNoteIndex){
      byte note = notePitches[selectedNoteIndex];
      trackList[currentTrackPos].stepNotes[currentEditStepPos] = note;
      prevNote = selectedNoteIndex;
    }
  }
  
  if(mode == EDIT_MODE){
    if(track_mode == EDIT_TRACK){
      currentEditStepPos = getPot1(STEP_NUMBER);
      if(prevEditStep != currentEditStepPos){
        selectStep(currentEditStepPos);
        prevEditStep = currentEditStepPos;
      }
    }

    if(track_mode == CHANGE_TRACK){
      currentTrackPos = getPot1(TRACK_NUMBER);
      if(prevTrackPos != currentTrackPos){
        selecTrack(currentTrackPos);
        prevTrackPos = currentTrackPos;
      }
    }
  }

  light();
  
  if(midiFlush){
    MidiUSB.flush();
  }
}

