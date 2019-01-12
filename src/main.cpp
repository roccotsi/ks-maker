#include <Arduino.h>

// define Arduino ports
const byte INPUT_CHANGE_VALUE = 2;    // Arduino port D2
const byte INPUT_NEXT = 3;            // Arduino port D3
const byte OUTPUT_POWER = 4;          // Arduino port D4
const byte INPUT_VOLTAGE = 1;         // Arduino port A1

// define modes
const byte MODE_SETTING_PPM = 0;
const byte MODE_SETTING_VOLUME = 1;
const byte MODE_ASKING_START = 2;
const byte MODE_RUNNING = 3;
const byte MODE_FINISHED = 4;

// other constants
const byte MAX_PPM = 100;
const byte MIN_PPM = 5;
const unsigned int MAX_VOLUME_ML = 1000;
const unsigned int MIN_VOLUME_ML = 50;
const byte STEP_PPM = 5;
const unsigned int STEP_VOLUME = 50;
const unsigned long INTERVAL_MEASURE_CURRENT_MILLIS = 10000;
const byte CALCULATE_PPM_AFTER_NUMBER_CURRENT_MEASUREMENTS = 6;
const unsigned int RESISTOR_MEASUREMENT_OHM = 1000;
const float REF_VOLTAGE = 4.66;

// variables
byte valButtonChangeValue = 0;
byte valButtonNext = 0;
byte mode = MODE_SETTING_PPM;
byte ppmTarget = MIN_PPM;
unsigned int volumeMlTarget = MIN_VOLUME_ML;
unsigned long lastMillisMeasuredCurrent = 0;
byte numberCurrentMeasurements = 0;
float ppmSum = 0;
float mASum = 0;

void printPpm() {
  Serial.println("ppm: " + String(ppmTarget));
}

void printVolume() {
  Serial.println("ml: " + String(volumeMlTarget));
}

void printAskStart() {
  Serial.println("Press Next to start");
}

void printRunning() {
  Serial.println("RUNNING...");
}

void printCurrentPpm() {
  Serial.println("ppm: " + String(ppmSum));
}

void printFinished() {
  Serial.println("Finished!");
}

void start() {
  digitalWrite(OUTPUT_POWER, 1);
}

void finish() {
  digitalWrite(OUTPUT_POWER, 0);
  mode = MODE_FINISHED;
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
        if (ppmTarget < MAX_PPM) {
          ppmTarget = ppmTarget + STEP_PPM;
        } else {
          ppmTarget = MIN_PPM;
        }
        printPpm();
      } else if (mode == MODE_SETTING_VOLUME) {
        if (volumeMlTarget < MAX_VOLUME_ML) {
          volumeMlTarget = volumeMlTarget + STEP_VOLUME;
        } else {
          volumeMlTarget = MIN_VOLUME_ML;
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
      mode++;
      if (mode == MODE_SETTING_VOLUME) {
        printVolume();
      } else if (mode == MODE_ASKING_START) {
        printAskStart();
      } else if (mode == MODE_RUNNING) {
        start();
        printRunning();
      }
    }
  }
}

float measureMA() {
  // measure voltage on A1 and calculate current in mA
  int analogVal = analogRead(INPUT_VOLTAGE);
  float volt = analogVal * (REF_VOLTAGE / 1024.0);
  float mA = (volt / RESISTOR_MEASUREMENT_OHM) * 1000.0;
  Serial.println(String(volt) + " V");
  Serial.println(String(mA) + " mA");
  return mA;
}

void updatePpm() {
  float mAAverage = mASum / numberCurrentMeasurements;
  float minutes = (CALCULATE_PPM_AFTER_NUMBER_CURRENT_MEASUREMENTS * INTERVAL_MEASURE_CURRENT_MILLIS) / 60000.0;
  Serial.println("Minutes: " + String(minutes));
  // ppm = (Minuten * mA) : (15*Liter)
  float liter = volumeMlTarget / 1000.0;
  float ppmOfInterval = (minutes * mAAverage) / (15.0 * liter);
  Serial.println("Calculated ppm: " + String(ppmOfInterval) + " ppm");
  ppmSum = ppmSum + ppmOfInterval;
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

  if (mode == MODE_RUNNING) {
    // measure current, update ppm and refresh display
    unsigned long currentMillis = millis();
    if ((currentMillis - lastMillisMeasuredCurrent) > INTERVAL_MEASURE_CURRENT_MILLIS) {
      lastMillisMeasuredCurrent = currentMillis;
      float mA = measureMA();
      mASum = mASum + mA;
      numberCurrentMeasurements++;

      if (numberCurrentMeasurements == CALCULATE_PPM_AFTER_NUMBER_CURRENT_MEASUREMENTS) {
        // end of interval of current measurement reached -> update ppm and check if target ppm reached
        updatePpm();
        printCurrentPpm();
        numberCurrentMeasurements = 0;
        mASum = 0;
        if (ppmSum >= ppmTarget) {
          finish();
          printFinished();
        }
      }
    }
  }
}
