#include "i2c_display.h"

void draw_element(Adafruit_SSD1306* display, String element, int num_element, String atomic_weight){
    display->clearDisplay();
    //Serial.println("Clear Done");
    delay(1000);
    display->setTextColor(SSD1306_WHITE); // Draw white text
    display->cp437(true);         // Use full 256 char 'Code Page 437' font

    //Atomic Number:
    display->setTextSize(1);      // Normal 1:1 pixel scale
    display->setCursor(0, 0);     // Start at top-left corner
    display->write(num_element);

    //Atomic Weight
    display->setCursor(50, 55);     // Start at top-left corner
    display->write(atomic_weight.c_str());

    //Atomic Symbol
    display->setTextSize(5);
    display->setCursor(35, 13);  
    display->write(element.c_str());
    display->display();

}