#include "HardwareSerial.h"
#include "pins_arduino.h"
#include "MainBoardPinout.h"
#include "MainBoard.h"
#include <SPI.h>
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_ST7796S.h>
#include <cstdint>
#include <sys/types.h>

#define NUMPIXELS 16

#define HALL1 0x48
#define HALL2 0x49

#define M_PI 3.14159265358979323846

#define INPUT_THRESHOLD 20.0

#define OLED_ADDRESS 0x3C
#define OLED_WIDTH 128
#define OLED_HEIGHT 64

#define PUZZLE_COUNT 6

typedef struct{

    char chem1[9];
    char chem2[9];
    char chem3[9];

    uint8_t amount1;
    uint8_t amount2;
    uint8_t amount3;

    char outputChemical[21];

}Puzzle;

Puzzle puzzleCreate(const char* c1, const char* c2, const char* c3, uint8_t r1, uint8_t r2, uint8_t r3, const char* out){
    Puzzle p;
    strcpy(p.chem1, c1);
    strcpy(p.chem2, c2);
    strcpy(p.chem3, c3);
    p.amount1 = r1;
    p.amount2 = r2;
    p.amount3 = r3;
    strcpy(p.outputChemical, out);
    return p;
}

int16_t hall[8] = {0};
uint16_t hallCalibration[8] = {0};

uint8_t hallPin[8] = {0x48,0x49,0x47,0x46,0x3f,0x3e,0x3c,0x3d};

uint8_t litLeds[3] = {0};

float angles[4] = {0};
float fieldStrength[4] = {0};

bool present[4] = {false};
bool inserted[4] = {false};
bool removed[4] = {false};

bool buttons[4] = {false};
bool buttonsPressed[4] = {false};

bool locked[4] = {false};

char chemicals[3][9]={{"H"},{"C"},{"O"}};
uint8_t ratios[3] = {12,6,6};
char target[21] = "";
float threshold = 0.8;

Puzzle allPuzzles[PUZZLE_COUNT];
uint8_t currentPuzzle = -1;

Adafruit_NeoPixel pixels(NUMPIXELS*3, MAIN_BOARD_WS2812_PIN, NEO_RGB + NEO_KHZ800);
Adafruit_SSD1306 display(128, 64, &Wire);
Adafruit_ST7796S bigDisplay(&mainBoardSpi, MAIN_BOARD_LCD_1_CS, MAIN_BOARD_LCD_DC, MAIN_BOARD_LCD_RESET);

void fillPuzzles(){
    allPuzzles[0] = puzzleCreate("H", "O", "", 2, 1, 0, "H2O");
    allPuzzles[1] = puzzleCreate("H", "C", "O", 12, 6, 6, "C6H12O6");
    allPuzzles[2] = puzzleCreate("H", "C", "", 8, 2, 0, "2CH4");
    allPuzzles[3] = puzzleCreate("CaCO3", "CO2", "H2O", 6, 6, 6, "6Ca(HCO3)2");
    allPuzzles[4] = puzzleCreate("SO2","O2","H2O", 2, 1, 2, "?H2SO4");
    allPuzzles[5] = puzzleCreate("Fe", "O2", "H2O", 4, 3, 6, "?Fe(OH)3");
}

void nextPuzzle(){

    currentPuzzle++;
    if (currentPuzzle > PUZZLE_COUNT - 1){
        currentPuzzle = 0;
    }

    Puzzle puzzle = allPuzzles[currentPuzzle];
    
    strcpy(chemicals[0], puzzle.chem1);
    strcpy(chemicals[1], puzzle.chem2);
    strcpy(chemicals[2], puzzle.chem3);

    strcpy(target, puzzle.outputChemical);

    ratios[0] = puzzle.amount1;
    ratios[1] = puzzle.amount2;
    ratios[2] = puzzle.amount3;

    litLeds[0] = 0; litLeds[1] = 0; litLeds[2] = 0;

    bigDisplay.setTextSize(3);
    bigDisplay.fillScreen(0xffff);
    bigDisplay.setTextColor(0,0xffff);
    bigDisplay.setCursor(50,100);
    bigDisplay.println("Make a chemical:");
    bigDisplay.setCursor(70,150);
    bigDisplay.println(target);

    pixels.fill(0,0,NUMPIXELS*3);
    pixels.show();

    litLeds[0] = 0;
    litLeds[1] = 0;
    litLeds[2] = 0;
    locked[0] = 0;
    locked[1] = 0;
    locked[2] = 0;
    
}

void validatePuzzle(){

    if(litLeds[0] == ratios[0] && litLeds[1] == ratios[1] && litLeds[2] == ratios[2]){

        bigDisplay.fillScreen(bigDisplay.color565(0, 255, 255));
        bigDisplay.setTextColor(0xffff,0);
        bigDisplay.print("Congrats!");

        for(uint8_t i = 0; i < NUMPIXELS*3; i++){
            pixels.setPixelColor(i, pixels.Color(120,0,0));
        }
        pixels.show();

        delay(3000);

        nextPuzzle();

    }
}

void setup(){
    MainBoardStart(false);

    mainBoardSpi.setFrequency(10000000);

    Wire.begin();

    Serial.begin(115200);
    pixels.begin();
    pixels.clear();
    pixels.show();

    delay(50);

    display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS);

    for(uint8_t i = 0; i < 3; i++){
        mainBoardSetI2CBus(i);
        display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
        display.clearDisplay();
        display.setFont();
        display.setTextColor(1);
        display.setCursor(0, 0);
        display.print("init ");
        display.println(i);
        display.display();
    }


    for(uint8_t i = 0; i < 8; i++){
        hallCalibration[i] = mainBoardGetAnalogMux(hallPin[i]);
        Serial.print(hallCalibration[i]);
        Serial.print(", ");
    }
    Serial.println();

    Serial.println("Calibration complete");


    analogWrite(MAIN_BOARD_LCD_BRIGHTNESS, 0xFF);

    bigDisplay.init(320, 480, 0, 0, ST7796S_BGR);
    bigDisplay.setRotation(3);
    bigDisplay.invertDisplay(true);
    bigDisplay.fillScreen(0);

    delay(100);

    fillPuzzles();
    nextPuzzle();
    
}

void loop(){
    Serial.print("Halls");
    for (uint8_t i = 0; i < 8; i++){
        hall[i] = mainBoardGetAnalogMux(hallPin[i]) - hallCalibration[i];
        Serial.print(", ");
        Serial.print(hall[i]);
    }
    Serial.println();

    


    for(uint8_t i = 0; i < 4; i++){
        int h1 = hall[2*i];
        int h2 = hall[2*i+1];
        fieldStrength[i] = std::sqrt(h1*h1 + h2*h2);

        bool button = !mainBoardGetDigitalInput(i);

        buttonsPressed[i] = false;

        if(button){

            if(!buttons[i]){
                buttonsPressed[i] = true;
            }

        }

        buttons[i] = button;

        inserted[i] = false;
        removed[i] = false;

        if(fieldStrength[i] > INPUT_THRESHOLD){

            if(buttonsPressed[i] && !locked[i]){
                locked[i] = true;
            }else if(buttonsPressed[i]){
                locked[i] = false;
            }

            if(!locked[i]){
                angles[i] = std::atan2(hall[2*i], hall[2*i+1]) + M_PI;
            }

            if(!present[i]){
                inserted[i] = true;
                locked[i] = false;
            }
            present[i] = true;
        }else{
            if(present[i]){
                removed[i] = true;
            }
            present[i] = false;
        }

        Serial.print("A");
        Serial.print(i);
        Serial.print(": ");
        Serial.print(angles[i]);
        Serial.print(",   ");
        
    }
    Serial.println();

    

    for(uint8_t q = 0; q < 3; q++){

        litLeds[q] = std::min(static_cast<int>((angles[q] * NUMPIXELS / (2 * M_PI))) + 1, NUMPIXELS);

        if(fieldStrength[q] > INPUT_THRESHOLD){
            for(uint8_t i = NUMPIXELS * q; i < NUMPIXELS * q + litLeds[q]; i++){
                pixels.setPixelColor(i, pixels.Color(0,125,0));
            }

            for(uint8_t i = NUMPIXELS * (q) + litLeds[q]; i < NUMPIXELS * (q + 1); i++){
                pixels.setPixelColor(i, pixels.Color(0,0,0));
            }
            pixels.show();
        }else{
            if(locked[q]){
                for(uint8_t i = NUMPIXELS * q; i < NUMPIXELS * q + litLeds[q]; i++){
                    pixels.setPixelColor(i, pixels.Color(0,125,0));
                }

                for(uint8_t i = NUMPIXELS * (q) + litLeds[q]; i < NUMPIXELS * (q + 1); i++){
                    pixels.setPixelColor(i, pixels.Color(0,0,0));
                }
            }else{
                for(uint8_t i = NUMPIXELS * q; i < NUMPIXELS * (q+1); i++){
                    pixels.setPixelColor(i, pixels.Color(0,0,0));
                }
                litLeds[q] = 0;
            }
            pixels.show();

            
        }


        mainBoardSetI2CBus(q);
        display.clearDisplay();
        display.setTextSize(1);
        display.setCursor(0, 0);
        display.print("Concentration: ");
        display.println(litLeds[q]);
        display.setTextSize(4);
        display.println(chemicals[q]);
        display.setTextSize(1);
        display.print("Lock: ");
        display.print(locked[q]);
        display.display();


    }

    Serial.print("Present: ");
    Serial.print(present[3]);
    Serial.print(", Button: ");
    Serial.println(buttonsPressed[3]);

    if(buttonsPressed[3] && present[3] && angles[3] > 1.5){

        validatePuzzle();

        Serial.println("Validating!");

    }

}