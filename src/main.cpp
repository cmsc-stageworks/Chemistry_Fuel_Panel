#include "HardwareSerial.h"
#include "pins_arduino.h"
#include <Arduino.h>
#include <FastLED.h>

#define NUM_LEDS 6
#define DATA_PIN 27
CRGB leds[NUM_LEDS];

using std::atan2;

int num1 = 0;
int num2 = 0;

int selection = 0;
int numLightsOn = 0;

bool present = false;

void setup() {
  // put your setup code here, to run once:
  pinMode(2, OUTPUT);
  pinMode(12, OUTPUT);

  pinMode(35, INPUT);
  pinMode(34, INPUT);
  pinMode(32, INPUT);
  pinMode(25, INPUT);

  Serial.begin(115200);

  FastLED.addLeds<WS2812, DATA_PIN, GRB>(leds, NUM_LEDS);
  
}

void loop() {
  // put your main code here, to run repeatedly:

  int value1 = analogRead(35);
  int value2 = analogRead(34);

  bool button = digitalRead(32);

  if (button) {

    num1 = value1;
    num2 = value2;

  }

  double angle = (atan2(-(value2 - num2) , (value1 - num1))*180/PI) + 180;

  double flux = sqrt(pow(value1 - num1, 2) + pow(value2 - num2, 2));
  
  if (flux > 25){
      
    if (present == false) {
      present = true;
      for(int i = 0; i < NUM_LEDS; i++){
        leds[i] = CRGB::Green;
      }
      FastLED.show();
      delay(300);
      for(int i = 0; i < NUM_LEDS; i++){
        leds[i] = CRGB(0,0,0);
      }
      FastLED.show();
    }

  }else{

    if(present == true){
      present = false;

      for(int i = 0; i < NUM_LEDS; i++){
        leds[i] = CRGB::Red;
      }
      FastLED.show();
      delay(300);
      for(int i = 0; i < NUM_LEDS; i++){
        leds[i] = CRGB(0,0,0);
      }
      FastLED.show();

    }

  }


  if(present){

    numLightsOn = (int)(((angle*(NUM_LEDS + 1))/360.0));
    numLightsOn = numLightsOn % 7;


    for (int i = 0; i < numLightsOn; i++){

      leds[i] = CRGB::Blue;

    }
    for(int i = numLightsOn; i < NUM_LEDS; i++){

      leds[i] = CRGB(0,0,0);

    }
    FastLED.show();


    if (digitalRead(25) == HIGH){

      selection = numLightsOn;

      Serial.print("Lights: ");
      Serial.println(selection);

    }

  } else if (selection != 0){

    for (int i = 0; i < selection; i++){

      leds[i] = CRGB::Blue;

    }
    for(int i = selection; i < NUM_LEDS; i++){

      leds[i] = CRGB(0,0,0);

    }
    FastLED.show();

  }

  delay(50);


  if (button) {

    num1 = value1;
    num2 = value2;

  }

}

