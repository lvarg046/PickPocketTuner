#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>

#define TFT_DC 5 // **D1**
#define TFT_RST 2 // **D4**
#define TFT_CS 12 // **NOT CONNECTED**
#define BTN_LEFT 4 // **D2**
#define BTN_RIGHT 9 // **SD2**
#define BTN_CENTER 10 // **SD3**

// create display object
Adafruit_ST7789 display = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

// screen controller
int screen_current = 0;

void setup() {
  // initialize SPI connection to display
  display.init(240, 240, SPI_MODE2);

  // attach interrupts
  attachInterrupt(digitalPinToInterrupt(BTN_LEFT), left_button, HIGH);
  attachInterrupt(digitalPinToInterrupt(BTN_RIGHT), right_button, HIGH);
  attachInterrupt(digitalPinToInterrupt(BTN_CENTER), center_button, HIGH);

  // intro test screen
  display.fillScreen(ST77XX_BLACK);
  display.drawCircle(120, 120, 80, ST77XX_WHITE);
}

void loop() {
  
}

void left_button() {
  display.fillScreen(ST77XX_BLACK);
  display.drawCircle(100, 120, 80, ST77XX_WHITE);
}

void right_button() {
  display.fillScreen(ST77XX_BLACK);
  display.drawCircle(140, 120, 80, ST77XX_WHITE);
}

void center_button() {
  display.fillScreen(ST77XX_BLACK);
  display.drawCircle(120, 120, 80, ST77XX_WHITE);
}