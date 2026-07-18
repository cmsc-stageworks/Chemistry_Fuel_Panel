#include "HardwareSerial.h"
#include "pins_arduino.h"
#include "MainBoardPinout.h"
#include <MainBoard.h>
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_ST7796S.h>

#define NUMPIXELS 16

#define HALL1 0x47
#define HALL2 0x46

#define M_PI 3.14159265358979323846

#define INPUT_THRESHOLD 20.0


int hall1 = 0;
int hall2 = 0;

int hall1Calibration = 0;
int hall2Calibration = 0;

float fieldStrength = 0;

int numLeds = 0;

Adafruit_NeoPixel pixels(NUMPIXELS, MAIN_BOARD_WS2812_PIN, NEO_RGB + NEO_KHZ800);
Adafruit_SSD1306 display(128, 64, &Wire);
Adafruit_ST7796S bigDisplay(MAIN_BOARD_LCD_1_CS, MAIN_BOARD_LCD_DC, MAIN_BOARD_LCD_RESET);

void show_shapes() {
  // Draw outlined and filled shapes. This demonstrates:
  // - Enclosed shapes supported by GFX (points & lines are shown later).
  // - Adapting to different-sized displays, and to rounded corners.

  const int16_t cx = display.width() / 2;  // Center of screen =
  const int16_t cy = display.height() / 2; // half of width, height
  int16_t minor = min(cx, cy);             // Lesser of half width or height
  // Shapes will be drawn in a square region centered on the screen. But one
  // particular screen -- rounded 240x280 ST7789 -- has VERY rounded corners
  // that would clip a couple of shapes if drawn full size. If using that
  // screen type, reduce area by a few pixels to avoid drawing in corners
  const uint8_t pad = 5;                   // Space between shapes is 2X this
  const int16_t size = minor - pad;        // Shapes are this width & height
  const int16_t half = size / 2;           // 1/2 of shape size

  bigDisplay.fillScreen(0); // Start by clearing the screen; color 0 = black

  // Draw outline version of basic shapes: rectangle, triangle, circle and
  // rounded rectangle in different colors. Rather than hardcoded numbers
  // for position and size, some arithmetic helps adapt to screen dimensions.
  bigDisplay.drawRect(cx - minor, cy - minor, size, size, 0xF800);
  bigDisplay.drawTriangle(cx + pad, cy - pad, cx + pad + half, cy - minor,
                       cx + minor - 1, cy - pad, 0x07E0);
  bigDisplay.drawCircle(cx - pad - half, cy + pad + half, half, 0x001F);
  bigDisplay.drawRoundRect(cx + pad, cy + pad, size, size, size / 5, 0xFFE0);
  delay(250);

  // Draw same shapes, same positions, but filled this time.
  bigDisplay.fillRect(cx - minor, cy - minor, size, size, 0xF800);
  bigDisplay.fillTriangle(cx + pad, cy - pad, cx + pad + half, cy - minor,
                       cx + minor - 1, cy - pad, 0x07E0);
  bigDisplay.fillCircle(cx - pad - half, cy + pad + half, half, 0x001F);
  bigDisplay.fillRoundRect(cx + pad, cy + pad, size, size, size / 5, 0xFFE0);
  delay(250);
} // END SHAPE EXAMPLE

void setup(){
    MainBoardStart(false);
    Serial.begin(115200);
    pixels.begin();

    mainBoardSetI2CBus(0);

    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    display.clearDisplay();

    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0,0);

    display.println("init");
    display.display();

    analogWrite(MAIN_BOARD_LCD_BRIGHTNESS, 0xFF);

    bigDisplay.init(320, 480, 0, 0, ST7796S_BGR);
    bigDisplay.invertDisplay(true);k
    bigDisplay.fillScreen(0);

    hall1Calibration = mainBoardGetAnalogMux(HALL1);
    hall2Calibration = mainBoardGetAnalogMux(HALL2);
}

void loop(){
    if(fieldStrength > INPUT_THRESHOLD){
        for(int i = 0; i < numLeds; i++){
            pixels.setPixelColor(i, pixels.Color(0,125,0));
        }
        for(int i = numLeds; i < NUMPIXELS; i++){
            pixels.setPixelColor(i, pixels.Color(0,0,0));
        }
        pixels.show();
    }else{
        pixels.clear();
        pixels.show();
    }


    hall1 = mainBoardGetAnalogMux(HALL1) - hall1Calibration;
    hall2 = mainBoardGetAnalogMux(HALL2) - hall2Calibration;

    float angle = std::atan2(-hall1, -hall2) + M_PI;

    fieldStrength = std::sqrt(hall1*hall1 + hall2*hall2);

    Serial.println(fieldStrength);

    Serial.print("Angle: ");
    Serial.print(angle);
    Serial.print(",  NumLeds: ");
    Serial.println(numLeds);

    show_shapes();

    numLeds = angle * NUMPIXELS / (2 * M_PI) + 1;
}