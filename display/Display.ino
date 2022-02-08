#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>

/* SCREEN */
#define TFT_DC 5          // **D1**
#define TFT_RST 2         // **D4**
#define TFT_CS 12         // **NOT USED AT ALL**

/* BUTTONS */
#define BTN_LEFT 4        // **D2**
#define BTN_RIGHT 9       // **SD2**
#define BTN_CENTER 10     // **SD3**

/* BUZZER & */
#define BUZZ 0            // **GPIO0/D3 -> PASSIVE BUZZER **


/* MOTOR */
#define DIR_WIRE 1        // **GPIO1/TX PIN -> ORANGE DIRECTION WIRE**
#define PWM_WIRE 15       // **GPIO15/D8 PIN -> BLUE PWM WIRE**
#define FG_WIRE  16       // **GPIO16/D0 PIN -> GREEN FG Signal + 5K in series**
#define motorBaud 115200  // **BAUD RATE for monitoring motor**
int motorSpeed;           // **MOTOR SPEED**
bool flag = HIGH;         // **FLAG TO SWITCH DIRECTION OF MOTOR**


// create display object
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

// screen controller
int screen_current = 0;

// Button State 
int buttonState = 0;
void setup() {
  // initialize SPI connection to display
  tft.init(240, 240, SPI_MODE2);

  //BUTTON USING INPUT BUTTON 
  pinMode(BTN_LEFT, INPUT);
  
  // The Attach interrupts doesn't work, causes the screen to freak out and to not work properly.
//  // attach interrupts
//  attachInterrupt(digitalPinToInterrupt(BTN_LEFT), left_button, HIGH);
//  attachInterrupt(digitalPinToInterrupt(BTN_RIGHT), right_button, HIGH);
//  attachInterrupt(digitalPinToInterrupt(BTN_CENTER), center_button, HIGH);

  // intro test screen
  tft.fillScreen(ST77XX_BLACK);
  tft.drawCircle(120, 120, 80, ST77XX_WHITE);
  delay(200);

  // Set up for motor monitoring
  Serial.begin(115200); 

  // PINS for  OUTPUT
  pinMode(DIR_WIRE, OUTPUT);
  pinMode(PWM_WIRE, OUTPUT);
  pinMode(BUZZ, OUTPUT);
  
}

void loop() {

  buttonState = digitalRead(BTN_LEFT); // READING LEFT BUTTON INPUT
  
  int piezoIn = analogRead(A0); // READING FROM PIEZO
  Serial.println(piezoIn);
  
  if( buttonState == 1){ // READING PIN INPUT FROM BUTTON
    mediabuttons();          // print media buttons onto screen
   tone(BUZZ, 60);        // MAKE SOME NOISE
   flag = !flag;          // BOOL FLIP FOR DIRECTION FLIP
   digitalWrite(DIR_WIRE, flag); // CHANGE DIRECTION OF MOTOR
   analogWrite(PWM_WIRE, 0); // FULL SPEED
   
//   delay(200);           
  } else if( buttonState == 0){
    noTone(BUZZ);       
    tft.fillScreen(ST77XX_BLACK);
    analogWrite(PWM_WIRE, 260);
    digitalWrite(DIR_WIRE, flag);
  }
}

void mediabuttons() {
  // play
  tft.fillScreen(ST77XX_BLACK);
  tft.fillRoundRect(25, 10, 78, 60, 8, ST77XX_WHITE);
  tft.fillTriangle(42, 20, 42, 60, 90, 40, ST77XX_RED);
  delay(500);
  // pause
  tft.fillRoundRect(25, 90, 78, 60, 8, ST77XX_WHITE);
  tft.fillRoundRect(39, 98, 20, 45, 5, ST77XX_GREEN);
  tft.fillRoundRect(69, 98, 20, 45, 5, ST77XX_GREEN);
  delay(500);
  // play color
  tft.fillTriangle(42, 20, 42, 60, 90, 40, ST77XX_BLUE);
  delay(50);
  // pause color
  tft.fillRoundRect(39, 98, 20, 45, 5, ST77XX_RED);
  tft.fillRoundRect(69, 98, 20, 45, 5, ST77XX_RED);
  // play color
  tft.fillTriangle(42, 20, 42, 60, 90, 40, ST77XX_GREEN);
}

void left_button() {
  tft.fillScreen(ST77XX_BLACK);
  mediabuttons();
}

void right_button() {
  tft.fillScreen(ST77XX_BLACK);
  tft.drawCircle(140, 120, 80, ST77XX_WHITE);
}

void center_button() {
  tft.fillScreen(ST77XX_BLACK);
  tft.drawCircle(120, 120, 80, ST77XX_WHITE);
}
