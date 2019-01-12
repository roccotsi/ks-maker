#include <Arduino.h>

// define Arduino ports
#define INPUT_CHANGE_VALUE 2    // Arduino port D2
#define INPUT_NEXT 3            // Arduino port D3
#define OUTPUT_POWER 4          // Arduino port D4
#define INPUT_VOLTAGE 1         // Arduino port A1

// define modes
#define MODE_SETTING_PPM 0
#define MODE_SETTING_VOLUME 1
#define MODE_ASKING_START 2
#define MODE_RUNNING 3
#define MODE_FINISHED 4

const byte MAX_PPM = 50;
const byte MIN_PPM = 5;
const unsigned int MAX_VOLUME_ML = 1000;
const unsigned int MIN_VOLUME_ML = 50;
const byte STEP_PPM = 5;
const unsigned int STEP_VOLUME = 50;

// variables
byte valButtonChangeValue = 0;
byte valButtonNext = 0;
unsigned int valA1 = 0;
byte mode = MODE_SETTING_PPM;
byte ppm = MIN_PPM;
unsigned int volumeMl = MIN_VOLUME_ML;

void printPpm() {
  Serial.println("ppm: " + String(ppm));
}

void printVolume() {
  Serial.println("ml: " + String(volumeMl));
}

void printAskStart() {
  Serial.println("START ?");
}

void printRunning() {
  Serial.println("RUNNING...");
}

void handleButtonChangeValue() {
  if (mode >= MODE_ASKING_START) {
    return; // do nothing
  }

  int valNew = digitalRead(INPUT_CHANGE_VALUE);
  if (valNew != valButtonChangeValue) {
    // button set state changed
    valButtonChangeValue = valNew;
    delay(10); // entprellen

    // if button is currently pressed
    if (valButtonChangeValue == 1) {
      if (mode == MODE_SETTING_PPM) {
        if (ppm < MAX_PPM) {
          ppm = ppm + STEP_PPM;
        } else {
          ppm = MIN_PPM;
        }
        printPpm();
      } else if (mode == MODE_SETTING_VOLUME) {
        if (volumeMl < MAX_VOLUME_ML) {
          volumeMl = volumeMl + STEP_VOLUME;
        } else {
          volumeMl = MIN_VOLUME_ML;
        }
        printVolume();
      }
    }
  }
}

void handleButtonNext() {
  if (mode >= MODE_RUNNING) {
    return; // do nothing
  }

  int valNew = digitalRead(INPUT_NEXT);
  if (valNew != valButtonNext) {
    // button set state changed
    valButtonNext = valNew;
    delay(10); // entprellen

    // if button is currently pressed
    if (valButtonNext == 1) {
      mode = mode + 1;
      if (mode == MODE_SETTING_VOLUME) {
        printVolume();
      } else if (mode == MODE_ASKING_START) {
        printAskStart();
      } else if (mode == MODE_RUNNING) {
        printRunning();
      }
    }
  }
}

void setup()
{
  pinMode(INPUT_CHANGE_VALUE, INPUT);
  pinMode(INPUT_NEXT, INPUT);
  pinMode(OUTPUT_POWER, OUTPUT);
  Serial.begin(9600);

  printPpm(); // print initial ppm value
}

void loop()
{
  handleButtonChangeValue();
  handleButtonNext();

  // // A1
  // int analogValNew = analogRead(A1);
  // if (analogVal != analogValNew) {
  //   delay(500);
  //   analogVal = analogValNew;
  //   float volt = analogVal * (4.66 / 1024.0);
  //   Serial.println(String(volt) + " V");
  // }
}
