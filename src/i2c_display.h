#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>

#define SDA_PIN 21
#define SCL_PIN 22
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

void draw_element(Adafruit_SSD1306* display, String element, int num_element, String atomic_weight);