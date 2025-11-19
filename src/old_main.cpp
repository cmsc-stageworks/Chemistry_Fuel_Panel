#include "HardwareSerial.h"
#include "pins_arduino.h"
#include <Arduino.h>
#include <FastLED.h>

#define NUM_LEDS 6
#define DATA_PIN 27
CRGB leds[NUM_LEDS];

using std::atan2;

#define RING_RESOLUTION 6
#define SECTION1 0

int calibration1 = 0;
int calibration2 = 0;

int selection = 0;
int canister1Value = 0;

int hallSensor1 = 0;
int hallSensor2 = 0;

float alpha = 0.08;

bool present = false;

bool button = false;

void setLeds(int section, int size, CRGB color){

  for(int i = section; i < size; i++){

    leds[i] = color;

  }

  for(int i = section + size; i < RING_RESOLUTION; i++){

    leds[i] = CRGB(0,0,0);

  }

};

void setup() {
  // put your setup code here, to run once:
  pinMode(2, OUTPUT);
  pinMode(12, OUTPUT);

  pinMode(35, INPUT);
  pinMode(34, INPUT);
  pinMode(32, INPUT_PULLUP);
  pinMode(25, INPUT_PULLUP);

  Serial.begin(115200);

  FastLED.addLeds<WS2812, DATA_PIN, GRB>(leds, NUM_LEDS);
  
}

void loop() {
  // put your main code here, to run repeatedly:

  hallSensor1 = analogRead(35);
  hallSensor2 = analogRead(34);

  button = !digitalRead(32);

  Serial.print("HE: ");
  Serial.print(hallSensor1 - calibration1);
  Serial.print(", ");
  Serial.println(hallSensor2 - calibration2);

  // Serial.print("Calibration: ");
  // Serial.print(calibration1);
  // Serial.print(", ");
  // Serial.println(calibration2);
if (button){

  float accumulator1 = hallSensor1;
  float accumulator2 = hallSensor2;

  while (button) {

      hallSensor1 = analogRead(35);
      hallSensor2 = analogRead(34);

      accumulator1 = (hallSensor1 * alpha / 5) + (1.0 - alpha / 5) * accumulator1;
      accumulator2 = (hallSensor2 * alpha / 5) + (1.0 - alpha / 5) * accumulator2;

      button = !digitalRead(32);

      delay(20);

    }

    calibration1 = accumulator1;
    calibration2 = accumulator2;

}

  int adjustedHall1 = hallSensor1 - calibration1;
  int adjustedHall2 = hallSensor2 - calibration2;

  //Serial.print("button: ");
  //Serial.println(button);

  double angle = (atan2(-(adjustedHall1) , (adjustedHall2))*180/PI) + 180;

  double fieldStrength = (sqrt(pow(adjustedHall1, 2) + pow(adjustedHall2, 2)) * alpha) + (1.0 - alpha) * fieldStrength;

  Serial.print("Field Strength: ");
  Serial.println(fieldStrength);

  Serial.print("Angle: ");
  Serial.println(angle);
  
  if (fieldStrength > 4){
      
    if (present == false) {
      present = true;
      setLeds(SECTION1, RING_RESOLUTION, CRGB::Green);
      FastLED.show();

      delay(300);
      setLeds(SECTION1, RING_RESOLUTION, CRGB(0,0,0));
      FastLED.show();
    }

  }else{

    if(present == true){
      present = false;

      setLeds(SECTION1, RING_RESOLUTION, CRGB::Red);
      FastLED.show();
      delay(300);
      setLeds(SECTION1, RING_RESOLUTION, CRGB(0,0,0));
      FastLED.show();

    }

  }


  if(present){

    canister1Value = (int)(((angle*(NUM_LEDS + 1))/360.0));
    canister1Value = canister1Value % 7;


    setLeds(SECTION1, canister1Value, CRGB::Blue);
    FastLED.show();


    if (digitalRead(25) == LOW){

      selection = canister1Value;

      Serial.print("Lights: ");
      Serial.println(selection);

    }

  } else if (selection != 0){

    setLeds(SECTION1, selection, CRGB::Blue);
    FastLED.show();

  }

  delay(200);

}