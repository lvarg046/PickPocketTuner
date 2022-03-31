#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoOTA.h>


/* GRAPHICS/DISPLAY LIBRARIES */
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <Fonts/FreeSerif9pt7b.h>

/* FFT LIBRARY */
#include "arduinoFFT.h"

/* ADDITIONAL LIBRARIES */
#include <math.h>
// BOARD MANAGER : NODEMCU 1.0

/* WIFI CONFIGURATIONS */

// Configuration for fallback access point 
// if Wi-Fi connection fails.
const char * AP_ssid = "ESP8266_fallback_AP";
const char * AP_password = "SuperSecretPassword";
IPAddress AP_IP = IPAddress(10,1,1,1);
IPAddress AP_subnet = IPAddress(255,255,255,0);

// Wi-Fi connection parameters.
// It will be read from the flash during setup.
struct WifiConf {
  char wifi_ssid[50];
  char wifi_password[50];
  // Make sure that there is a 0 
  // that terminatnes the c string
  // if memory is not initalized yet.
  char cstr_terminator = 0; // makse sure
};
WifiConf wifiConf;

// Web server for editing configuration.
// 80 is the default http port.
ESP8266WebServer server(80);

/* PIN DEFINITIONS */

/* DISPLAY */
#define TFT_DC 5 // D1
#define TFT_RST 2 // D4
#define TFT_CS 12

/* BUTTONS */
#define BUTTON_LEFT 9
#define BUTTON_CENTER 10
#define BUTTON_RIGHT 4

/* ANALOG DEVICES */
#define ANALOG_INPUT A0
#define BUZZ 0

/* MOTOR */
#define DIR_WIRE 1 // ORANGE
#define PWM_WIRE 15 // BLUE
#define FG_WIRE 16 // GREEN

// Removed Knight Image bitmap for readability

/* GLOBAL VARIABLES */
int motor_speed;
bool motor_flag;
bool button_flags[4] = {false, false, false, false}; // [MAIN FLAG, RIGHT BUTTON, CENTER BUTTON, LEFT BUTTON]

int test_value = 0;

int screen_value = 0;
int mode_selected = 0;
int string_selected = 0;
int library_selected = 0;

/* GENERAL TUNING TABLES */
float tuning_base [12][2] = { // This is to use to find the octave we're in
    {415.30, 1.89}, // Ab/G#
    {440, 2.00}, // A 
    {466.16, 2.12}, // A#/Bb
    {493.88, 2.25}, // B 
    {523.25, 2.38}, // C
    {554.37, 2.52}, // C#/Db
    {587.33, 2.67}, // D
    {622.25, 2.83}, // D#/Eb
    {659.25, 3.00}, // E
    {698.46, 3.17}, // F
    {739.99, 3.37}, // F#/Gb
    {783.99, 3.56} // G
};


float tuning_array [12][7] = { // Based on A - 440 Standard
    {12.98, 25.96, 51.91, 103.83, 207.65, 415.30, 830.61 }, // Ab/G# - Octaves
    {13.75, 27.50, 55.00, 110.00, 220.00, 440.00, 880.00 }, // A Octaves
    {14.57, 29.14, 58.27, 116.54, 233.08, 466.16, 932.33 }, // A#/Bb - Octaves 
    {15.46, 30.87, 61.74, 123.47, 246.94, 493.88, 987.77 }, // B Octaves
    {16.35, 32.70, 65.41, 130.81, 261.63, 523.25, 1046.50}, // C Octaves 
    {17.32, 34.65, 69.30, 138.59, 277.18, 554.37, 1108.73}, // C#/Db - Octaves
    {18.35, 36.71, 73.42, 146.83, 293.66, 587.33, 1174.66}, // D Octaves
    {19.45, 38.89, 77.78, 155.56, 311.13, 622.25, 1244.51}, // D#/Eb - Octaves
    {20.60, 41.20, 82.41, 164.81, 329.63, 659.25, 1318.51}, // E Octaves
    {21.83, 43.65, 87.31, 174.61, 349.23, 698.46, 1396.91}, // F Octaves
    {23.12, 46.25, 92.50, 185.00, 369.99, 739.99, 1479.98}, // F#/Gb - Octaves
    {24.50, 49.00, 98.00, 196.00, 392.00, 783.99, 1567.98} // G Octaves 
};

/*
*   Tuning library is ordered from lowest string to highest
*   Lowest string is thickest string, highest string is thinnest
*   Order is 6, 5, 4, 3, 2, 1
*/

float premade_tuning_lib [11][6] = { // Based on A - 440 Standard
    {82.41, 110.00, 146.83, 196.00, 246.94, 329.63},// E - Standard  E-A-D-G-B-E
    {73.42, 98.00, 130.81, 174.61, 220.00, 293.66}, // D - Standard  D-G-C-F-A-D
    {77.78, 103.83, 138.57, 185.00, 233.08, 311.13},// Eb - Standard Eb-Ab-Db-Gb-Bb-Eb
    {73.42, 110.00, 146.83, 196.00, 246.94, 329.63},// Drop D        D-A-D-G-B-E
    {65.41, 98.00, 130.81, 174.61, 220.00, 293.66}, // Drop C        C-G-C-F-A-D
    {61.74, 92.50, 123.47, 164.81, 207.65, 277.18}, // Drop B        B-Gb-B-E-Ab-Db
    {55.00, 82.51, 110.00, 146.83, 185.00, 246.94}, // Drop A        A-E-A-D-F#-B
    {77.78, 110.00, 146.83, 186.00, 220.00, 293.66},// Open D        D-A-D-F#-A-D
    {77.78, 98.00, 146.83, 196.00, 246.94, 293.66}, // Open G        D-G-D-G-B-D
    {65.41, 98.00, 130.81, 196.00, 261.63, 329.63}, // Open C        C-G-C-G-C-E
    {82.41, 123.47, 164.81, 207.65, 246.94, 329.63} // Open E        E-B-E-G#-B-E
};

const char *lib_name[11]= {"E Std", "D Std", "Eb Std", "Drop D", "Drop C", "Drop A", 
                            "Open D", "Open G", "Open C", "Open E"};
                            
const char *note_name[12] = {"G#/Ab", "A", "A#/Bb", "B", "C", "C#/Db", "D", "D#/Eb", 
                            "E", "F", "F#/Gb", "G"};

int freq_base_A[8] = {432, 434, 436, 438, 440, 442, 444, 446};
float adjusted_base[12][2] = {0};
float adjusted_array[12][7] = {0};
float adjusted_lib[11][6] = {0};

/* FFT DEFINITIONS & VARIABLES */
#define SAMPLES 512 //Must be a power of 2
#define SAMPLING_FREQUENCY 1000 //Hz, must be less than 10000 due to ADC
arduinoFFT FFT = arduinoFFT();
unsigned int sampling_period_us;
unsigned long microseconds;
double vReal[SAMPLES];
double vImag[SAMPLES];
int i_test;

// display object creation
Adafruit_ST7789 display = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

// function headers
void initialize_buttons();
void ICACHE_RAM_ATTR button_right_pressed();
void ICACHE_RAM_ATTR button_center_pressed();
void ICACHE_RAM_ATTR button_left_pressed();
void button_operations();
void device_operations();
void draw_test_circle();
void spinMotorSharp( int, double );
void spinMotorFlat( int, double );
void spinMotor();
void print_hertz( float );
void readWifiConf();
void writeWifiConf();
bool connectToWifi();
void setUpAccessPoint();
void setUpWebServer();
void handleWebServerRequest();
void setUpOverTheAirProgramming();
void drawIntroScreen();
void drawStringSelectionScreen();
void drawModeSelectionScreen();
void drawTuningLibrarySelectionScreen();
void drawFreeTuneScreen();
void drawPluckStringScreen();
void drawLeftTriangle();
void drawRightTriangle();
void drawPrevLeftTriangle();
double fft();
double cents_calculate( double, double );
float algo_riddim( float, int , int , float );
float base_freq_calc( float , float , int , int );
int octave_calc(float , float );

void tuning_test( double );

void setup() {
  Serial.begin(115200);
  
  // init EEPROM object 
  // to read/write wifi configuration.
  EEPROM.begin(512);
  readWifiConf();

  if (!connectToWiFi()) {
    setUpAccessPoint();
  }
  setUpWebServer();
  setUpOverTheAirProgramming();

  sampling_period_us = round(1000000*(1.0/SAMPLING_FREQUENCY));
  display.init(240, 240, SPI_MODE2);

  drawIntroScreen();

  pinMode(BUZZ, OUTPUT);
  pinMode(DIR_WIRE, OUTPUT);
  pinMode(PWM_WIRE, OUTPUT);

  digitalWrite(DIR_WIRE, HIGH);
  analogWrite(PWM_WIRE, 255);
  initialize_buttons();
}

void loop() {
  // Give processing time for ArduinoOTA.
  // This must be called regularly
  // for the Over-The-Air upload to work.
  ArduinoOTA.handle();

  // Give processing time for the webserver.
  // This must be called regularly
  // for the webserver to work.
  server.handleClient();
  
  if ( button_flags[0] ) {
    button_operations();
    device_operations();
  }
}

void drawScreen() {
  switch( screen_value ) {
    case 0:
      drawIntroScreen();
      break;
    case 1:
      drawModeSelectionScreen();
      break;
    case 2:
      drawStringSelectionScreen();
      break;
    case 3:
      drawTuningLibrarySelectionScreen();
      break;
    case 4:
      drawFreeTuneScreen();
      break;
    case 5:
      drawPluckStringScreen();
      break;
  }
}

// fillTriangle(apex x, apex y, bottom left x, bottom left y, bottom right x, bottom right y, color)
void drawLeftTriangle() {
  display.fillTriangle(10, 120, 30, 140, 30, 100, ST77XX_WHITE);
}

void drawRightTriangle() {
  display.fillTriangle(230, 120, 210, 140, 210, 100, ST77XX_WHITE);
}

void drawPrevLeftTriangle() {
  display.fillTriangle(10, 115, 30, 130, 30, 100, ST77XX_WHITE);
  display.setCursor(10, 136);
  display.setTextSize(2);
  display.setTextColor(ST77XX_WHITE);
  display.println("Prev");
}

void drawIntroScreen() {

//   display.drawRGBBitmap(0,0, knight, 240, 240);
  display.fillScreen(0x000F);
  display.setCursor(10, 100);
  display.setTextColor(ST77XX_WHITE, 0x000F);
  display.setTextSize(3);
  display.println( "Pick" );
  display.setCursor(10, 140);
  display.println( "Pocket" );
  display.setCursor(10, 180);
  display.println( "Tuner" );
  display.setTextSize(2);
  display.setCursor(10, 220);
  display.println( "Group 42" );
  display.println();
  delay(3000);

  screen_value = 1;
  drawScreen();
}

void drawModeSelectionScreen() {
  display.fillScreen(ST77XX_BLACK);
  if (mode_selected > 0)
    drawLeftTriangle();
  if (mode_selected < 2)
    drawRightTriangle();
  display.setCursor(20, 10);
  display.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
  display.setTextSize(3);
  display.println( "Select Mode" );

    display.setCursor(20, 40);
    switch (mode_selected)
    {
    case 0: // Mode 0: Auto Mode, Goes through all strings
        display.println("Auto Tuning");
        break;

    case 1: // Mode 1: Individual String Mode
        display.println("Indv. String");
        break;

    case 2: // Mode 2: Free Tuning Mode
        display.println("Free Tuning");
        break;
    }
}

// display.String# displays correct number of string in correct order.
void drawStringSelectionScreen() {
  display.fillScreen(ST77XX_BLACK);
  if (string_selected == 0)
    drawPrevLeftTriangle();
  if (string_selected > 0)
    drawLeftTriangle();
  if (string_selected < 5)
    drawRightTriangle();
  display.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
  display.setTextSize(3);
  display.setCursor(0, 10);
  display.println( "Select String" );
  display.println( "String #" + (String)(string_selected + (6 - ( (string_selected * 2 )))) ); 
}

void drawTuningLibrarySelectionScreen() {
  display.fillScreen(ST77XX_BLACK);
  if (library_selected == 0)
    drawPrevLeftTriangle();
  if (library_selected > 0)
    drawLeftTriangle();
  if (library_selected < 9)
    drawRightTriangle();
  display.setCursor(10, 10);
  display.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
  display.setTextSize(3);
  display.setCursor(30, 10);
  display.println( "Pick Tuning" );
  
  display.setCursor(70, 110);
  display.println( (String)(lib_name[library_selected]) );
}

void drawPluckStringScreen() {
  display.fillScreen(ST77XX_BLACK);
  display.setCursor(10, 10);
  display.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
  display.setTextSize(3);
  display.println( "Pluck String " + (String)(string_selected + (6 - ((string_selected * 2)))) );

  // fft
  delay(500);
  double peak = fft();
  tuning_test(peak);
  // spin the motor based off of fft reading
  // if auto mode, go to next string
  // else go back to mode selection

  //delay(1000);
  if (mode_selected == 0 && string_selected < 5) {
    string_selected += 1;
    drawPluckStringScreen();
  } else {
    screen_value = 3;
    library_selected = 0;
    drawTuningLibrarySelectionScreen();
  }
}

void drawFreeTuneScreen() {
  display.fillScreen(ST77XX_BLACK);
  drawRightTriangle();
  drawLeftTriangle();
  display.setCursor(10, 10);
  display.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
  display.setTextSize(3);
  display.println( "Free Mode" );
}

void readWifiConf() {
  // Read wifi conf from flash
  for (int i=0; i<sizeof(wifiConf); i++) {
    ((char *)(&wifiConf))[i] = char(EEPROM.read(i));
  }
  // Make sure that there is a 0 
  // that terminatnes the c string
  // if memory is not initalized yet.
  wifiConf.cstr_terminator = 0;
}

void writeWifiConf() {
  for (int i=0; i<sizeof(wifiConf); i++) {
    EEPROM.write(i, ((char *)(&wifiConf))[i]);
  }
  EEPROM.commit();
}

bool connectToWiFi() {
  Serial.printf("Connecting to '%s'\n", wifiConf.wifi_ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(wifiConf.wifi_ssid, wifiConf.wifi_password);
  if (WiFi.waitForConnectResult() == WL_CONNECTED) {
    Serial.print( "Connected. IP: " );
    Serial.println(WiFi.localIP());
    return true;
  } else {
    Serial.println( "Connection Failed!" );
    return false;
  }
}

void setUpAccessPoint() {
    Serial.println( "Setting up access point." );
    Serial.printf("SSID: %s\n", AP_ssid);
    Serial.printf("Password: %s\n", AP_password);

    WiFi.mode(WIFI_AP_STA);
    WiFi.softAPConfig(AP_IP, AP_IP, AP_subnet);
    if (WiFi.softAP(AP_ssid, AP_password)) {
      Serial.print( "Ready. Access point IP: " );
      Serial.println(WiFi.softAPIP());
    } else {
      Serial.println( "Setting up access point failed!" );
    }
}

void setUpWebServer() {
  server.on("/", handleWebServerRequest);
  server.begin();
}

void handleWebServerRequest() {
  bool save = false;

  if (server.hasArg("ssid") && server.hasArg("password")) {
    server.arg("ssid").toCharArray(
      wifiConf.wifi_ssid,
      sizeof(wifiConf.wifi_ssid));
    server.arg("password").toCharArray(
      wifiConf.wifi_password,
      sizeof(wifiConf.wifi_password));

    Serial.println(server.arg("ssid"));
    Serial.println(wifiConf.wifi_ssid);

    writeWifiConf();
    save = true;
  }

  String message = "";
  message += "<!DOCTYPE html>";
  message += "<html>";
  message += "<head>";
  message += "<title>ESP8266 conf</title>";
  message += "</head>";
  message += "<body>";
  if (save) {
    message += "<div>Saved! Rebooting...</div>";
  } else {
    message += "<h1>Wi-Fi conf</h1>";
    message += "<form action='/' method='POST'>";
    message += "<div>SSID:</div>";
    message += "<div><input type='text' name='ssid' value='" + String(wifiConf.wifi_ssid) + "'/></div>";
    message += "<div>Password:</div>";
    message += "<div><input type='password' name='password' value='" + String(wifiConf.wifi_password) + "'/></div>";
    message += "<div><input type='submit' value='Save'/></div>";
    message += "</form>";
  }
  message += "</body>";
  message += "</html>";
  server.send(200, "text/html", message);

  if (save) {
    Serial.println( "Wi-Fi conf saved. Rebooting..." );
    delay(1000);
    ESP.restart();
  }
}

void setUpOverTheAirProgramming() {

  // Change OTA port. 
  // Default: 8266
  // ArduinoOTA.setPort(8266);

  // Change the name of how it is going to 
  // show up in Arduino IDE.
  // Default: esp8266-[ChipID]
  // ArduinoOTA.setHostname("myesp8266" );

  // Re-programming passowrd. 
  // No password by default.
  // ArduinoOTA.setPassword("123" );

  ArduinoOTA.begin();
}

double fft() {

    delay(100);
    for(int i=0; i<SAMPLES; i++){
        microseconds = micros();    //Overflows after around 70 minutes!
        vReal[i] = analogRead(A0);
        vImag[i] = 0;
     
        while(micros() < (microseconds + sampling_period_us)){
        }
    }
    /*FFT*/
    FFT.Windowing(vReal, SAMPLES, FFT_WIN_TYP_TRIANGLE, FFT_FORWARD);
    FFT.Compute(vReal, vImag, SAMPLES, FFT_FORWARD);
    FFT.ComplexToMagnitude(vReal, vImag, SAMPLES);
    double peak = FFT.MajorPeak(vReal, SAMPLES, SAMPLING_FREQUENCY);
    
    return peak;
}

void spinMotorFlat(int time, double freq) {
    motor_flag = LOW;
    digitalWrite(DIR_WIRE, motor_flag);
    display.setCursor(60, 150);
    display.println(freq, 2);
    analogWrite(PWM_WIRE, 0);
    delay(time);
    analogWrite(PWM_WIRE, 255);
}

void spinMotorSharp(int time, double freq) {
    motor_flag = HIGH;
    digitalWrite(DIR_WIRE, motor_flag);
    display.setCursor(60, 150);
    display.println(freq, 2);
    analogWrite(PWM_WIRE, 0);
    delay(time);
    analogWrite(PWM_WIRE, 255);
}

void device_operations() {
  switch( test_value ) {
    case 1: // LEFT

      switch ( screen_value ) { 
        case 1: // mode selection
          if (mode_selected > 0) {
            mode_selected -= 1;
            drawScreen();
          }
          break;

        case 2: // string selection
          if (string_selected > 0) {
            string_selected -= 1;
            drawScreen();
          } else {
            library_selected = 0;
            screen_value = 3;
            drawScreen();
          }
          break;

        case 3: // tuning library selection
          if (library_selected > 0) {
            library_selected -= 1;
            drawScreen();
          } else {
            mode_selected = 0;
            screen_value = 1;
            drawScreen();
          }
          break;

        case 4: // free tuning
          digitalWrite(DIR_WIRE, LOW);
          digitalWrite(PWM_WIRE, 0);
          
          while (digitalRead(BUTTON_LEFT) == HIGH)
            wdt_reset();
          
          digitalWrite(PWM_WIRE, 255);
          break;
      }

      test_value = 0;
      break;
      
    case 2: // CENTER

      switch ( screen_value ) { 
        case 1: // mode selection

          switch ( mode_selected ) {
            case 0: // auto tuning
              screen_value = 3;
              library_selected = 0;
              drawScreen();
              break;

            case 1: // individual string tuning
              screen_value = 3;
              library_selected = 0;
              drawScreen();
              break;

            case 2: // free tuning
              screen_value = 4;
              drawScreen();
              break;
          }

          break;
        case 2: // string selection
          screen_value = 5;
          drawScreen();
          break;

        case 3: // library selection
          test_value = 0;
          if (mode_selected == 1) {
            screen_value = 2;
            string_selected = 0;
            drawScreen();
          } else {
            screen_value = 5;
            drawScreen();
          }
          break;

        case 4: // free tuning
          screen_value = 1;
          mode_selected = 0;
          drawScreen();
          break;
      }

      test_value = 0;
      break;

    case 3: // RIGHT

      switch ( screen_value ) { 
        case 1: // mode selection
          if (mode_selected < 2) {
            mode_selected += 1;
            drawScreen();
          }
          break;

        case 2: // string selection
          if (string_selected < 5) {
            string_selected += 1;
            drawScreen();
          }
          break;

        case 3: // tuning library selection
          if (library_selected < 9) {
            library_selected += 1;
            drawScreen();
          }
          break;

        case 4: // free tuning
          digitalWrite(DIR_WIRE, HIGH);
          digitalWrite(PWM_WIRE, 0);
          
          while (digitalRead(BUTTON_RIGHT) == HIGH)
            wdt_reset();
          
          digitalWrite(PWM_WIRE, 255);
          break;
      }

      test_value = 0;
      break;
  }
}

// initialize buttons to low for pull-up resistor
void initialize_buttons() {
  interrupts();

  // set button pin modes
  pinMode(BUTTON_LEFT, INPUT_PULLUP);
  pinMode(BUTTON_CENTER, INPUT_PULLUP);
  pinMode(BUTTON_RIGHT, INPUT_PULLUP);

  // button interrupts
  attachInterrupt(digitalPinToInterrupt(BUTTON_LEFT), button_left_pressed, HIGH);
  attachInterrupt(digitalPinToInterrupt(BUTTON_CENTER), button_center_pressed, HIGH);
  attachInterrupt(digitalPinToInterrupt(BUTTON_RIGHT), button_right_pressed, HIGH);
}

void ICACHE_RAM_ATTR button_left_pressed() {
  noInterrupts();
  
  button_flags[0] = true;
  button_flags[1] = true;

  interrupts();
} 

void ICACHE_RAM_ATTR button_center_pressed() {
  noInterrupts();
  
  button_flags[0] = true;
  button_flags[2] = true;

  interrupts();
}

void ICACHE_RAM_ATTR button_right_pressed() {
  noInterrupts();
  
  button_flags[0] = true;
  button_flags[3] = true;

  interrupts();
}


void button_operations() {
  button_flags[0] = false;
  noInterrupts();
  
  if ( button_flags[1] ) { // BUTTON LEFT
    test_value = 1;
    button_flags[1] = false;
  }

  if ( button_flags[2] ) { // BUTTON CENTER
    test_value = 2;
    button_flags[2] = false;
  }

  if ( button_flags[3] ) { // BUTTON RIGHT
    test_value = 3;
    button_flags[3] = false;
  }

  interrupts();
}

// Algo_riddim calculates the tuning after adjusting the A-440 value; 
// Currently the static inputs in this function are for the D# values going from A@432 to A@444
float algo_riddim(float input_fr, int std_fr_in, int std_fr_out, float table_440_const){ // Finding shift from 4XX to 4XX freq
    float output_freq;
    float base_out = base_freq_calc(tuning_base[7][0], tuning_base[7][1], freq_base_A[0], freq_base_A[6]);
    int octave = octave_calc(base_out, input_fr);;

    Serial.print( "Expect:627.91 BASE: " );
    Serial.println(base_out); 
    // int oct = octave_calc( tuning_base[4][0], tuning_array[4][1]);

    Serial.print( "Expect:4 Octave: " );
    Serial.println(octave);
    // float rebased = base_freq_calc( tuning_base[7][0], table_440_const, std_fr_in, std_fr_out);
    // octave = octave_calc(rebased, input_fr);

    if( std_fr_in > std_fr_out ){
        output_freq = ((input_fr * pow(2, octave)) - ( (abs((std_fr_in - std_fr_out)) / 2 ) * table_440_const) )/ ( pow(2, octave));
    } else {
        output_freq = ((input_fr * pow(2, octave)) + ( (abs((std_fr_in - std_fr_out)) / 2 ) * table_440_const) )/ ( pow(2, octave));
    }
    return output_freq;
}

// Calculating octaves above/below base_freq
int octave_calc( float base_freq, float input_freq ){
    int octave_out;
    float freq = base_freq/input_freq;
    octave_out = (log2f(freq)) + 0.5;
    return octave_out;
}

float base_freq_calc( float fr_table_base, float fr_table_const, int freq_std_in, int freq_std_out){
    float base_out;
    float temp_base;

    // Goes from fr_std_in to A@440
    if( freq_std_in < 440 ){ 
        temp_base = fr_table_base-( abs(440-freq_std_in)/2)*fr_table_const; 
    } else if( freq_std_in > 440 ){
        temp_base = fr_table_base + ( abs(440-freq_std_in)/2) * fr_table_const; 
    }
    // then from new temp_base to A@freq_std_out
    if( freq_std_in > freq_std_out ){
        base_out = temp_base - ( abs(freq_std_in-freq_std_out) / 2) * fr_table_const;
    } else if(freq_std_in < freq_std_out) {
        base_out = temp_base + ( abs(freq_std_in-freq_std_out) / 2) * fr_table_const;
    }
    return base_out;
}

double cents_calculate( double input_freq, double ref_freq){
    double ratio = ref_freq/input_freq;
    double cents = 1200*(log2(ratio));
    return cents;
}

void tuning_test( double input_freq ){
  double  target_freq = premade_tuning_lib[library_selected][string_selected]; // From premade_tuning_lib array
  display.println(target_freq, 2);
  double current_cents;
  double freq_diff; 
  double current_freq = input_freq;

  current_cents = cents_calculate( current_freq, target_freq );

  while( current_cents > 1.5  || current_cents < -1.5){
    freq_diff = current_freq - target_freq;
    current_freq = fft();
    current_cents = cents_calculate( current_freq, target_freq );
    // 75ms for 5 cents, 35ms for 3 cents, 15ms for less than 3 cents
    if( current_cents >= 5 || current_cents <= -5){
      if( freq_diff < 0 ){  
        spinMotorSharp(75, current_freq);
      } else if( freq_diff > 0 ){
        spinMotorFlat(75, current_freq);  
      }
    } else if( (current_cents >= 3 && current_cents < 5) || (current_cents <= -3 && current_cents > 5) ){
      if( freq_diff < 0 ){
        spinMotorSharp(35, current_freq);
      } else if ( freq_diff > 0 ){
        spinMotorFlat(35, current_freq);
      }
    } else {
      if( freq_diff < 0){
        spinMotorSharp(15, current_freq);
      } else if ( freq_diff > 0 ){
        spinMotorFlat(15, current_freq);
      }
    }
  }
  tone(BUZZ, 60);
  delay(50);
  noTone(BUZZ);
}