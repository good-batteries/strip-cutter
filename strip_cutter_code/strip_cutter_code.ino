#include <AccelStepper.h>
//#include "Adafruit_VL53L0X.h" // Too much memory and too slow
#include "VL53L0X_mod.h"
#include <Servo.h>
VL53L0X_mod lox;
// default pins for SDA - A4, SCL - A5

#define motorInterfaceType 1

#define welderDirPin 2 //d2
#define welderStepPin 3 //d3
AccelStepper welderStepper = AccelStepper(motorInterfaceType, welderStepPin, welderDirPin);

#define welder2DirPin 13 // d13
#define welder2StepPin 12 //d12
AccelStepper welder2Stepper = AccelStepper(motorInterfaceType, welder2StepPin, welder2DirPin);

#define stripDirPin 7
#define stripStepPin 8
AccelStepper stripStepper = AccelStepper(motorInterfaceType, stripStepPin, stripDirPin);

#define scissorsDirPin 9
#define scissorsStepPin 10
AccelStepper scissorsStepper = AccelStepper(motorInterfaceType, scissorsStepPin, scissorsDirPin);

#define buttomPin 4 // D4 (other end is GND)
#define stripSensorPin 5 // D5 (other end is GND)
#define scissorsPin 6 // D6 (other end is GND)

#define welderPressPin A2 // (other end is GND)
#define welderPressPin2 A3 // (other end is GND)
#define welderOutputPin A0 
#define welderOutputPin2 A1 


// OTHER
bool forwardDir = false;
bool btnWasPressed = false;

unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 50; 
int lastButtonState = LOW;

// Strip

// set false for stepsMode, if you need to cut preciselly at stop sensor.
// otherwise, strip length will be based on the step motor steps count - not very precise in this implementation.
// and sensor will be used as a strip jamming/running out/slippage sensor. Hence put the sensor BEFORE the needed steps count
// make sure there is no strip slippage to have precise cutting, otherwise use sensor based mode 
bool stepsMode = false;
// used only if stepsMode = true. Adjust for needed strip length. Approximatelly 10mm = 68 steps

long expectedStepsPerStrip = 500; 

// lower speed -> more precise and more torque
int stripSpeed = 900;
int throttledSpeed = 100;
float startThrottleAt = 0.85;


unsigned long stepsWhenStripStopped = 0;
long stepsPerStrip = 0; // measured
bool stripSensorTriggered = false;


const int StripStateMoving = 1;
const int StripStateWaiting = 3;
int stripState = StripStateWaiting;

// Flipper
#define servoPin 11 // D11
Servo Servo1;
// 90grad is full release.
// 0 is locked
int closed_deg = 45;
int open_deg = 230; // go slightly further to be sure

long openTimeMs = 300;
long closingTimeMs = 300;

long openTimeStartedMs = 0;
long closingTimeStartedMs = 0;

bool needFlipper = true; // false is not implemented. If not needed - then just not connect and reduce duration to minimum needed

const int FlipperStateReady = 0;
const int FlipperStateWaiting = 3;
const int FlipperStateOpening = 2;
const int FlipperStateClosing = 1;

int flipperState = FlipperStateWaiting;

// Welder
bool needWelder = true; // set false - if welding is not needed

int welderSpeed = 1200;
int stepsToWelderStepBack = 330;
int weldDelay = 50;
long weldWaitingDelay = 100;

const int WelderStateReady = 4;
const int WelderStatePauseBeforeMoving = 6;
const int WelderStateMoving = 1;
const int WelderStateWaitingSpotWeld = 5;
const int WelderStateBackingBack = 2;
const int WelderStateWaiting = 3;


long  welderTriggerDurationMs = 50;

const int WelderTriggerStateWaiting = 1;
const int WelderTriggerStateActivated = 2;
const int WelderTriggerStateDone = 3;

int welderTriggerState = WelderTriggerStateWaiting;

int welderState[2] = {WelderStateWaiting, WelderStateWaiting};
long welderWaitingMs[2] = {0, 0};

long stepsWhenWelderStopped[2] = {0,0};
unsigned long weldWaitStartTimeDelay[2] = {0,0};


// Scissors
int scissorsSpeed = 1100;
int scissorsBackSpeed = 1200;
int stepsToScissorsStepBack = 700; // CHANGE THIS TO ADJUST HOW FAR SCISSORS SHOULD STEP BACK AFTER CUT

const int ScissorsStateMoving = 1;
const int ScissorsStateBackingBack = 2;
const int ScissorsStateWaiting = 3;
const int ScissorsStateReady = 4;
int scissorsState = ScissorsStateWaiting;
int stepsWhenScissorsStopped = 0;


void setup() {
  Serial.begin(115000);
  Serial.println(F("Start Init"));

  pinMode(welderPressPin, INPUT_PULLUP);
  pinMode(welderPressPin2, INPUT_PULLUP);
  
  pinMode(welderOutputPin, OUTPUT);
  pinMode(welderOutputPin2, OUTPUT);
  digitalWrite(welderOutputPin, HIGH); // turn off relay by default
  digitalWrite(welderOutputPin2, HIGH); //

  pinMode(buttomPin, INPUT_PULLUP);
  pinMode(stripSensorPin, INPUT_PULLUP);
  pinMode(scissorsPin, INPUT_PULLUP);

  Servo1.attach(servoPin, 500,2500);
  Servo1.write(closed_deg); 
  Serial.println(F("Servo init done"));

  lox.init();
  lox.setTimeout(500);
  lox.startContinuous();
  Serial.println(F("Lox init done"));

  stripStepper.setMaxSpeed(10000);
  welderStepper.setMaxSpeed(2600);
  welder2Stepper.setMaxSpeed(2600);
  scissorsStepper.setMaxSpeed(2600);

  Serial.println(F("Init Completed"));
}


bool iteractionAllowed = false;
bool hasError = false;
unsigned long buttonPressedMillis = 0;

unsigned long changeModeBtnDelay = 1000;
bool samePress = false;
bool modeDidChange = false;

#define MODE_NORMAL 0
#define MODE_STRIP_TEST 1
#define MODE_WELD_TEST 2
#define MODES_COUNT 3

int currentMode = 0;


unsigned long prevSpeed;
unsigned long previousBeepMillis = 0;
unsigned long beepIntervalMs = 300;
unsigned long beepFreq = 4000;
bool isBeep = false;



void doError() {
  prevSpeed = stripStepper.speed(); // save prev speed, as we will be overriding it for beep signal.
  hasError = true;
  iteractionAllowed = false;
}

void clearError() {
  stripStepper.setSpeed(prevSpeed);
  hasError = false;
}


void loop() {
  int btnVal = digitalRead(buttomPin);
  if (btnVal != lastButtonState) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }
  lastButtonState = btnVal;

  if ((millis() - lastDebounceTime) > debounceDelay) {
    // we are ready to read button state

    if (!btnVal) {
      // Button currenlty pressed      

      if (!btnWasPressed) {
        // fired once after button is pressed
        Serial.println("BUTTON PRESSED");

        buttonPressedMillis = millis();
        modeDidChange = false;
        if (iteractionAllowed) {
          iteractionAllowed = false;
          samePress = true;
          Serial.println("STOPPED");
        }
        btnWasPressed = true;
      }

      // Long press - mode change
      if ((millis() - buttonPressedMillis) > changeModeBtnDelay) {
        // if pressed for some time - it is mode change
        buttonPressedMillis = millis();
        modeDidChange = true;
        currentMode = ((currentMode + 1) % MODES_COUNT);
        Serial.println("MODE CHANGED: " + String (currentMode));
        // mode mode change sound
        for (int i = 0; i < (currentMode+1); i++) {
          makeBeep();
          delay(300);
        }
        delay(1000);
      }
    } else {
      // Button currenlty released      

      buttonPressedMillis = 0;

      if (btnWasPressed) {
        // fired once after button is pressed
        Serial.println("BUTTON RELEASED");

        if (!modeDidChange) {
          // Just start/resume
          if (!iteractionAllowed && !samePress) {
            if (hasError) {
              clearError();
            }
            iteractionAllowed = true;
            Serial.println("STARTED");
          }
        }
        btnWasPressed = false;
        samePress = false;
      }
    }



  }

  if (hasError) {
    unsigned long currentMillis = millis();
    if (currentMillis - previousBeepMillis >= beepIntervalMs) {
        previousBeepMillis = currentMillis;
        if (isBeep) {
          stripStepper.setSpeed(beepFreq);
        } else {
          stripStepper.setSpeed(0);
        }
        isBeep = !isBeep;
    }
    stripStepper.runSpeed();
  }


  if (iteractionAllowed == false) {
    return;    
  }



// UNCOMMENT NEEDED TEST
// testWelder();
// testWelder2();  
// testScissors();
// testFlipper();  
// testStripStopper();
// testBothWelders();
// AND COMMENT FROM HERE >>>
  switch (currentMode) {
    case MODE_NORMAL:
    masterLoop(); 
    break;
    case MODE_STRIP_TEST:
    testStripStopper(); // Could be used for initial length calibration without cutting strip
    break;
    case MODE_WELD_TEST:
    testBothWelders(); // Could be used for initial welding 
    break;
  }
// TO HERE <<<


  stripStepper.runSpeed();
  welderStepper.runSpeed();
  welder2Stepper.runSpeed();
  scissorsStepper.runSpeed();
}

void testStripStopper() {
  if (welderState[0] == WelderStateReady) {
    welderState[0] = WelderStateWaiting;
    scissorsState = WelderStateWaiting;
    stripState = StripStateWaiting;
    scissorsState = ScissorsStateWaiting;
    flipperState = FlipperStateWaiting;

    iteractionAllowed = false;
    return;
    
    Serial.println("testStripStopper Start");
  }
  doStripMoving();
}


void testFlipper() {
  if (flipperState == FlipperStateWaiting) {
    flipperState = FlipperStateReady;
    Serial.println("testFlipper Start");
    iteractionAllowed = false;
    return;
  }
  doFlip();  
}


void testWelder() {
  if (welderState[0] == WelderStateWaiting) {
    stripState = StripStateWaiting;
    welderState[0] = WelderStateReady;
    Serial.println("testWelder Start");
  }
  doWelder(0, &welderStepper, &welder2Stepper);

  welderState[1] =  WelderStateWaitingSpotWeld;
  doWelderTrigger();
}


void testWelder2() {
  if (welderState[1] == WelderStateWaiting) {
    stripState = StripStateWaiting;
    welderState[1] = WelderStateReady;
    Serial.println("testWelder2 Start");
  }
  doWelder(1, &welderStepper, &welder2Stepper);

  welderState[0] =  WelderStateWaitingSpotWeld;
  doWelderTrigger();
}


void testBothWelders() {
  if (welderState[0] == WelderStateWaiting && welderState[1] == WelderStateWaiting) {
    stripState = StripStateWaiting;
    welderState[0] = WelderStateReady;
    iteractionAllowed = false;
    return;
  }

  doWelder(0, &welderStepper, &welder2Stepper);
  doWelder(1, &welderStepper, &welder2Stepper);
  doWelderTrigger();
}


void testScissors() {
  if (scissorsState == ScissorsStateWaiting) {
    scissorsState = ScissorsStateReady;
    Serial.println("testScissors Start");
  }
  doScissors();
}


void doStripMoving() {
  // ----------------  StripStateMoving --------------------
  if (stripState == StripStateWaiting && (!needWelder || (welderState[0] == WelderStateWaiting && welderState[1] == WelderStateWaiting)) && scissorsState == ScissorsStateWaiting && (!needFlipper || flipperState == FlipperStateWaiting)) {
    stripState = StripStateMoving;
    stripStepper.setSpeed(stripSpeed);
    Serial.println("STRIP STARTED MOVING");
    stripSensorTriggered = false;
  }
 
  if (stripState == StripStateMoving) {

    if (!stepsMode) {
      // Strip throttling at the end to have better precision
      unsigned long startAtPos = stepsWhenStripStopped + stepsPerStrip * startThrottleAt;
      unsigned long endAtPos = stepsWhenStripStopped + stepsPerStrip * 1.0;
      unsigned long lenSteps = endAtPos - startAtPos;
      
      if (stepsPerStrip == 0) {
        if (stripStepper.speed() != throttledSpeed) {
          stripStepper.setSpeed(throttledSpeed);
          Serial.println("INITIAL STRIP THROTTLE");    
        }
      }
      else if (stripStepper.currentPosition() >= startAtPos) {
        unsigned long finalSpeed = throttledSpeed;
        if (stripStepper.speed() != finalSpeed) {
          stripStepper.setSpeed(finalSpeed);
          // Serial.println("STRIP THROTTLED. finalSpeed: " +  String(finalSpeed)+ "; Pos: " + String(stripStepper.currentPosition()));    
        }
      }

      // check strip overflow
      if (stepsPerStrip != 0 && stripStepper.currentPosition()  >= (stepsWhenStripStopped + stepsPerStrip * 1.5)) {
        stepsPerStrip = 0;
        Serial.println("ERROR: STRIP OVERFLOW");    
        doError();
        return;
      }
    }
    
    uint16_t distance;
    if (lox.readRangeNoBlocking(distance)) {
        if (distance != 65535 && distance != 0) { // 65535 indicates a timeout
            // Do not log anythig if not necessity as it disrupts the motor movements
            //  Serial.print("Distance (mm): ");
            //  Serial.println(distance);
            if (distance < 60) {
              stripSensorTriggered = true;
            }
        } else {
            Serial.println("Error occurred");
        }
    }

    bool readyToCut = false;
    if (stepsMode) {
      if (stripStepper.currentPosition()  >= stepsWhenStripStopped + expectedStepsPerStrip ) {
        if (stripSensorTriggered) {
          // sensor should be triggered at this point - otherwise strip is stuck etc...
          readyToCut = true;
        } else {
          Serial.println("ERROR: SENSOR NOT TRIGGERED BEFORE CUT");    
          doError();
          return;
        }
      }
    } else {
      readyToCut = stripSensorTriggered;
    }
    


    if (readyToCut) {
      stepsPerStrip = stripStepper.currentPosition() - stepsWhenStripStopped;
      stepsWhenStripStopped = stripStepper.currentPosition();
      stripStepper.setSpeed(0);
      Serial.println("STRIP STOPPED. Pos: " + String(stripStepper.currentPosition()) + " stepsPerStrip : " + String(stepsPerStrip)  + " at speed: " + String(stripStepper.speed()) );     

      stripState = StripStateWaiting;
      if (needWelder) {
        welderState[0] = WelderStateReady;
    //      welderState[1] = WelderStateReady;
      }
      scissorsState = ScissorsStateReady;
    }
  }
}
  

void doScissors() {
  // ----------------  SCISSORS --------------------
  if (stripState == StripStateWaiting && scissorsState == ScissorsStateReady /* && (welderState[1] == WelderStateBackingBack || welderState[1] == WelderStateWaiting)*/) {
      scissorsStepper.setSpeed(scissorsSpeed);
      scissorsState = ScissorsStateMoving;
      Serial.println("SCISSORS STATED MOVING" + String(scissorsStepper.currentPosition()));
  }
  if (scissorsState == ScissorsStateMoving) {
    int scissorsSensVal = digitalRead(scissorsPin);
    if (!scissorsSensVal) {
      Serial.println("scissors STOPPED MOVING " + String(scissorsStepper.currentPosition()));
      scissorsState = ScissorsStateBackingBack;
      scissorsStepper.setSpeed(-scissorsBackSpeed);
      stepsWhenScissorsStopped = scissorsStepper.currentPosition();
    }
  }
  if (scissorsState == ScissorsStateBackingBack) {
    if (scissorsStepper.currentPosition() < stepsWhenScissorsStopped - stepsToScissorsStepBack) {
      scissorsStepper.setSpeed(0);
      scissorsState = ScissorsStateWaiting;
      if (needFlipper) {
        flipperState = FlipperStateReady;
      }
      Serial.println("scissors STOPPED BACKING BACK" + String(scissorsStepper.currentPosition()));
    }
  }  
}

// ----------------  FLIPPER --------------------
void doFlip() {
  if (flipperState == FlipperStateReady) {
    openTimeStartedMs = millis();
    flipperState = FlipperStateOpening;
  }

  if (flipperState == FlipperStateOpening) {
    Servo1.write(open_deg); 
    if (millis() - openTimeStartedMs > openTimeMs) {
      closingTimeStartedMs = millis();
      flipperState = FlipperStateClosing;
    }
  }

  if (flipperState == FlipperStateClosing) {
    Servo1.write(closed_deg); 
    if (millis() - closingTimeStartedMs > closingTimeMs) {
      flipperState = FlipperStateWaiting;
      
    }
  } 
}


void masterLoop() {
  doStripMoving();
  if (needWelder) {
    doWelder(0, &welderStepper, &welder2Stepper);
    doWelder(1, &welderStepper, &welder2Stepper);
    doWelderTrigger();
  }
  doScissors();
  doFlip();
}


unsigned long welderTriggerStartedMs = 0;
void doWelderTrigger() {
  if (welderTriggerState == WelderTriggerStateWaiting && (welderState[0] == WelderStateWaitingSpotWeld || welderState[1] == WelderStateWaitingSpotWeld)) {
    welderTriggerStartedMs = millis();
    welderTriggerState = WelderTriggerStateActivated;
    digitalWrite(welderOutputPin, LOW); 
    digitalWrite(welderOutputPin2, LOW);
    Serial.println("WELDER TRIGGER ACTIVATED");
  }

  if (welderTriggerState == WelderTriggerStateActivated) {
    if (millis() - welderTriggerStartedMs > welderTriggerDurationMs) {
        welderTriggerState = WelderTriggerStateDone;
        digitalWrite(welderOutputPin, HIGH); 
        digitalWrite(welderOutputPin2, HIGH);
        Serial.println("WELDER TRIGGER DONE");
    }
  }
}

void doWelder(int welderIndex, AccelStepper* welder0Stepper, AccelStepper* welder1Stepper) {
  AccelStepper* welderStepper;
  bool welderPressed = false;
  bool needPauseBeforeMoving = false;
  if (welderIndex == 0) {
    welderStepper = welder0Stepper;
    welderPressed = !digitalRead(welderPressPin);
//    digitalWrite(welderOutputPin, welderPressed); 
  }
  if (welderIndex == 1) {
    welderStepper = welder1Stepper;
    welderPressed = !digitalRead(welderPressPin2);
//   digitalWrite(welderOutputPin2, welderPressed);
//    needPauseBeforeMoving = true;
  }
  
  // Welder  
  if (stripState == StripStateWaiting && welderState[welderIndex] == WelderStateReady) {
      if (needPauseBeforeMoving) {
        welderState[welderIndex] = WelderStatePauseBeforeMoving;
        welderWaitingMs[welderIndex] = millis();
      } else {
        welderState[welderIndex] = WelderStateMoving;
        welderStepper->setSpeed(welderSpeed);
        Serial.println("WELDER STATED MOVING" + String(welderStepper->currentPosition()));
      }
  }
  if (welderState[welderIndex] == WelderStatePauseBeforeMoving) {
    if (millis() - welderWaitingMs[welderIndex] > weldWaitingDelay) {
        welderState[welderIndex] = WelderStateMoving;
        welderStepper->setSpeed(welderSpeed);
        Serial.println("WELDER STATED MOVING" + String(welderStepper->currentPosition()));
    }
  }

  if (welderState[welderIndex] == WelderStateMoving) {
    if (welderPressed) {
      welderState[welderIndex] = WelderStateWaitingSpotWeld;
      weldWaitStartTimeDelay[welderIndex] = millis();
      welderStepper->setSpeed(0);
      stepsWhenWelderStopped[welderIndex] = welderStepper->currentPosition();
      Serial.println("WELDER STOPPED MOVING" + String(welderStepper->currentPosition()));
    }
  }
  if (welderState[welderIndex] == WelderStateWaitingSpotWeld && welderTriggerState == WelderTriggerStateDone) {
//    if (millis() - weldWaitStartTimeDelay[welderIndex] > weldDelay) {
      welderState[welderIndex] = WelderStateBackingBack;
      welderStepper->setSpeed(-welderSpeed);
      Serial.println("WELDER START BACKING BACK" + String(welderStepper->currentPosition()));
//    }
  }
  if (welderState[welderIndex] == WelderStateBackingBack) {
    if ((welderStepper->currentPosition() < stepsWhenWelderStopped[welderIndex] - stepsToWelderStepBack)) {
      welderStepper->setSpeed(0);
      welderState[welderIndex] = WelderStateWaiting;
      welderTriggerState = WelderTriggerStateWaiting;
      if (welderIndex == 0) {
        welderState[1] = WelderStateReady;
      }
      Serial.println("WELDER STOPPED BACKING BACK" + String(welderStepper->currentPosition()));
    }
  }  
}



void makeBeep() {
    int delay = 140;
    for (int i = 0; i < 600; i ++) {
      digitalWrite(stripStepPin, HIGH);
      delayMicroseconds(delay); // Pulse width
      digitalWrite(stripStepPin, LOW);
      delayMicroseconds(delay); // Pulse width
    }
}

