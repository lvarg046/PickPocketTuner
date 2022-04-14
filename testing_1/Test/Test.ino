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
const char *AP_ssid = "ESP8266_fallback_AP";
const char *AP_password = "SuperSecretPassword";
IPAddress AP_IP = IPAddress(10, 1, 1, 1);
IPAddress AP_subnet = IPAddress(255, 255, 255, 0);

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
#define TFT_CS 12 // D6

/* BUTTONS */
#define BUTTON_LEFT 9 // SD2
#define BUTTON_CENTER 10 // SD3
#define BUTTON_RIGHT 4 // D2

/* ANALOG DEVICES */
#define ANALOG_INPUT A0
#define BUZZ 0 // D3

/* MOTOR */
#define DIR_WIRE 1 // ORANGE
#define PWM_WIRE 15 // BLUE
#define FG_WIRE 16 // GREEN

// Removed Knight Image bitmap for readability

// // function headers
void ICACHE_RAM_ATTR

button_left_pressed();

void ICACHE_RAM_ATTR

button_center_pressed();

void ICACHE_RAM_ATTR

button_right_pressed();

/* GLOBAL VARIABLES */
int motor_speed;
bool motor_flag;
bool button_flags[4] = {false, false, false, false}; // [MAIN FLAG, RIGHT BUTTON, CENTER BUTTON, LEFT BUTTON]

int test_value = 0;

int screen_value = 0;
int mode_selected = 0;
int string_selected = 0;
int library_selected = 0;
int new_string_mode_selected = 0;
int freq_base_selected = 0;
int curr_freq_base = 440;
int mode_settings_selected = 0;

/* GENERAL TUNING TABLES */
float tuning_base[12][2] = { // This is to use to find the octave we're in
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

float tuning_array[12][7] = { // Based on A - 440 Standard
        {12.98, 25.96, 51.91, 103.83, 207.65, 415.30, 830.61}, // Ab/G# - Octaves
        {13.75, 27.50, 55.00, 110.00, 220.00, 440.00, 880.00}, // A Octaves
        {14.57, 29.14, 58.27, 116.54, 233.08, 466.16, 932.33}, // A#/Bb - Octaves
        {15.46, 30.87, 61.74, 123.47, 246.94, 493.88, 987.77}, // B Octaves
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
float premade_tuning_lib[11][6] = { // Based on A - 440 Standard
    { tuning_array[8][2], tuning_array[1][3], tuning_array[6][3], tuning_array[11][3], tuning_array[3][4], tuning_array[8][4] },// E - Standard  E-A-D-G-B-E
    { tuning_array[7][2], tuning_array[0][3], tuning_array[5][3], tuning_array[10][3], tuning_array[2][4], tuning_array[7][4] },// Eb - Standard Eb-Ab-Db-Gb-Bb-Eb
    { tuning_array[6][2], tuning_array[11][2], tuning_array[4][3], tuning_array[9][3], tuning_array[1][4], tuning_array[6][4] }, // D - Standard  D-G-C-F-A-D
    { tuning_array[6][2], tuning_array[1][3], tuning_array[6][3], tuning_array[11][3], tuning_array[3][4], tuning_array[8][4] },// Drop D        D-A-D-G-B-E
    { tuning_array[4][2], tuning_array[11][2], tuning_array[4][3], tuning_array[9][3], tuning_array[1][4], tuning_array[6][4] }, // Drop C        C-G-C-F-A-D
    { tuning_array[3][2], tuning_array[10][2], tuning_array[3][3], tuning_array[8][3], tuning_array[0][4], tuning_array[5][4] }, // Drop B        B-Gb-B-E-Ab-Db
    { tuning_array[1][2], tuning_array[8][2], tuning_array[1][3], tuning_array[6][3], tuning_array[10][3], tuning_array[3][4] }, // Drop A        A-E-A-D-F#-B
    { tuning_array[7][2], tuning_array[1][3], tuning_array[6][3], tuning_array[10][3], tuning_array[1][4], tuning_array[6][4] },// Open D        D-A-D-F#-A-D
    { tuning_array[7][2], tuning_array[11][2], tuning_array[6][3], tuning_array[11][3], tuning_array[3][4], tuning_array[6][4] }, // Open G        D-G-D-G-B-D
    { tuning_array[4][2], tuning_array[11][2], tuning_array[4][3], tuning_array[11][3], tuning_array[4][4], tuning_array[8][4] }, // Open C        C-G-C-G-C-E
    { tuning_array[8][2], tuning_array[3][3], tuning_array[8][3], tuning_array[0][4], tuning_array[3][4], tuning_array[8][4] } // Open E        E-B-E-G#-B-E
};


const char *premade_tuning_lib_letter[11][6] = {
    {"E",  "A",  "D",  "G",  "B",  "E"},       // E - Standard  E-A-D-G-B-E
    {"Eb", "Ab", "Db", "Gb", "Bb", "Eb"}, // Eb - Standard Eb-Ab-Db-Gb-Bb-Eb
    {"D",  "G",  "C",  "F",  "A",  "D"},       // D - Standard  D-G-C-F-A-D
    {"D",  "A",  "D",  "G",  "B",  "E"},       // Drop D        D-A-D-G-B-E
    {"C",  "G",  "C",  "F",  "A",  "D"},       // Drop C        C-G-C-F-A-D
    {"B",  "Gb", "B",  "E",  "Ab", "Db"},    // Drop B        B-Gb-B-E-Ab-Db
    {"A",  "E",  "A",  "D",  "F#", "B"},      // Drop A        A-E-A-D-F#-B
    {"D",  "A",  "D",  "F#", "A",  "D"},      // Open D        D-A-D-F#-A-D
    {"D",  "G",  "D",  "G",  "B",  "D"},       // Open G        D-G-D-G-B-D
    {"C",  "G",  "C",  "G",  "C",  "E"},       // Open C        C-G-C-G-C-E
    {"E",  "B",  "E",  "G#", "B",  "E"}       // Open E        E-B-E-G#-B-E
};

const char *lib_name[12] = {"E Std", "Eb Std", "D Std", "Drop D", "Drop C", "Drop B", "Drop A",
                            "Open D", "Open G", "Open C", "Open E"};

const char *note_name[12] = {"G#/Ab", "A", "A#/Bb", "B", "C", "C#/Db", "D", "D#/Eb",
                             "E", "F", "F#/Gb", "G"};

const uint16_t rect_colors[9] = {ST77XX_RED, ST77XX_ORANGE, ST77XX_YELLOW, 0x77F0, ST77XX_GREEN, 0x77F0, ST77XX_YELLOW,
                                 ST77XX_ORANGE, ST77XX_RED};

const int cent_box_values[9] = {135, 102, 72, 36, 3, -30, -63, -96, -129};

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

void setup() {
    Serial.begin(115200);
    pinMode(DIR_WIRE, OUTPUT);
    pinMode(PWM_WIRE, OUTPUT);
    analogWrite(PWM_WIRE, 255);

    // init EEPROM object
    // to read/write wifi configuration.
    EEPROM.begin(512);
    readWifiConf();

    if (!connectToWiFi()) {
        setUpAccessPoint();
    }
    setUpWebServer();
    setUpOverTheAirProgramming();

    sampling_period_us = round(1000000 * (1.0 / SAMPLING_FREQUENCY)); // 1000ms
    display.init(240, 240, SPI_MODE2);

    drawIntroScreen();
    pinMode(BUZZ, OUTPUT);
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

    if (button_flags[0]) {
        button_operations();
        device_operations();
    }
}

void drawScreen() {
    switch (screen_value) {
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
        case 6:
            drawTuningScreen();
            break;
        case 7:
            drawNewStringSelectionScreen();
            break;
        case 8:
            drawSelectSettingsOrModeScreen();
            break;
        case 9:
            drawFreqBaseSelectionScreen();
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
    display.println("BACK");
}

void erasePrevLeftTriangle() {
    display.fillRect(10, 100, 46, 60, ST77XX_BLACK);
}

void drawSkipRightTriangle() {
    display.fillTriangle(230, 115, 210, 130, 210, 100, ST77XX_WHITE);
    display.setCursor(192, 136);
    display.setTextSize(2);
    display.setTextColor(ST77XX_WHITE);
    display.println("SKIP");
}

void eraseSkipRightTriangle() {
    display.fillRect(190, 100, 50, 60, ST77XX_BLACK);
}

// drawRect(top left x, top left y, width, height, color)
void drawCenterRectangle(char *text) {
    display.drawRect(80, 100, 80, 40, ST77XX_WHITE);
    display.setCursor(98, 113);
    display.setTextSize(2);
    display.print(text);
}

void eraseCenterRectangle() {
    display.fillRect(60, 100, 120, 40, ST77XX_BLACK);
}

// drawRoundRect(top left x, top left y, width, height, radius, color)
void drawRectangles(double cents, double inputFreq) {
    // rectangle width : 20
    // gap between rectangles : 4
    // gap from screen edge: 14
    int cursorx = 14;
    int cursory = 175;

    drawFlatSymbol();
    drawSharpSymbol();

    display.setCursor(10, 30);
    display.setTextSize(2);
    display.print(inputFreq);
    display.setCursor(23, 50);
    display.print("Hz");

    for (int i = 0; i < 9; i++) {
        uint16_t curr_color = rect_colors[i];
        if (cents > cent_box_values[i])
            curr_color = ST77XX_BLACK;
        if (cents < -3 && i == 5)
            curr_color = rect_colors[i];
        display.setCursor(cursorx, cursory);
        display.fillRoundRect(cursorx, cursory, 20, 60, 1, curr_color);
        cursorx += 24;
    }

    display.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
    display.setTextSize(4);

    if (strlen(premade_tuning_lib_letter[library_selected][string_selected]) > 1) {
        display.setCursor(102, 10);
    } else {
        display.setCursor(113, 10);
    }
    display.print(premade_tuning_lib_letter[library_selected][string_selected]);
}

void drawFlatSymbol() {
    display.setCursor(43, 140);
    display.setTextSize(3);
    display.print("b");
}

void drawSharpSymbol() {
    display.setCursor(185, 140);
    display.setTextSize(3);
    display.print("#");
}

void drawIntroScreen() {

//   display.drawRGBBitmap(0,0, knight, 240, 240);
    display.fillScreen(ST77XX_BLACK);
    display.fillRect(60, 63, 115, 120, 0xFE63);

    // Thicker lines at edges of screen
    // Top Left
    display.drawLine(0, 0, 59, 63, 0XFE63);
    display.drawLine(1, 0, 59, 63, 0XFE63);
    display.drawLine(0, 1, 59, 63, 0XFE63);
    // Top Right
    display.drawLine(175, 62, 240, 0, 0XFE63);
    display.drawLine(175, 62, 239, 0, 0XFE63);
    display.drawLine(175, 62, 240, 1, 0XFE63);
    // Bottom Left
    display.drawLine(60, 182, 0, 240, 0XFE63);
    display.drawLine(60, 182, 1, 240, 0XFE63);
    display.drawLine(60, 182, 0, 239, 0XFE63);
    // Bottom Right
    display.drawLine(174, 182, 240, 240, 0XFE63);
    display.drawLine(174, 182, 239, 240, 0XFE63);
    display.drawLine(174, 182, 240, 239, 0XFE63);
    // Top Mid
    display.drawLine(120, 63, 120, 0, 0XFE63);
    display.drawLine(121, 63, 121, 0, 0XFE63);
    display.drawLine(119, 63, 119, 0, 0XFE63);
    // Bottom Mid
    display.drawLine(120, 178, 120, 240, 0XFE63);
    display.drawLine(121, 178, 121, 240, 0XFE63);
    display.drawLine(119, 178, 119, 240, 0XFE63);

    display.setCursor(65, 68);
    display.setTextColor(ST77XX_BLACK, 0xFE63);
    display.setTextSize(3);
    display.println("Pick");
    display.setCursor(65, 98);
    display.println("Pocket");
    display.setCursor(65, 128);
    display.println("Tuner");
    display.setTextSize(2);
    display.setCursor(65, 158);
    display.println("Group 42");

    delay(3000);

    screen_value = 8;
    display.fillScreen(ST77XX_BLACK);
    drawScreen();
}

void drawModeSelectionScreen() {
    eraseCenterRectangle();
    erasePrevLeftTriangle();
    if (mode_selected == 0) {
        drawPrevLeftTriangle();
    } else {
        drawLeftTriangle();
    }
    if (mode_selected < 3) {
        drawRightTriangle();
    } else {
        eraseSkipRightTriangle();
    }

    display.setCursor(20, 10);
    display.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
    display.setTextSize(3);
    display.println("Select Mode");

    display.setCursor(20, 40);
    switch (mode_selected) {
        case 0: // Mode 0: Auto Mode, Goes through all strings
            display.println("Auto Tuning ");
            break;

        case 1: // Mode 1: Individual String Mode
            display.println("Indv. String ");
            break;
        
        case 2: // Mode 2: New String Mode
            display.println("New String   ");
            break;
        
        case 3: // Mode 3: Free Tuning Mode
            display.println("Peg Winding ");
            break;
    }
}

/*
    for row in tuning_array
        adjusted_tuning_array[i][0] = algo_riddim( input_fr = tuning_array[i][0], std_fr_in, std_fr_out = freq_base_A[freq_base_selected], float table_440_const = tuning_array[i][1])
        for col in tuning_array
            adjusted_array[i][j] = 2 * adjusted_array[i+1][j - 1]
 */

void drawFreqBaseSelectionScreen(){
    eraseCenterRectangle();
    erasePrevLeftTriangle();
    if (freq_base_selected == 0) {
        drawPrevLeftTriangle();
    } else {
        drawLeftTriangle();
    }
    if (freq_base_selected < 7) {
        drawRightTriangle();
    } else {
        eraseSkipRightTriangle();
    }
    display.setCursor(10, 10);
    display.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
    display.setTextSize(3);
    display.setCursor(30, 10);
    display.println("Pick Base");

    display.setCursor(90, 110);
    display.println((String)(freq_base_A[freq_base_selected]));
}

void drawSelectSettingsOrModeScreen() {
    eraseCenterRectangle();
    if (mode_settings_selected > 0) {
        drawLeftTriangle();
    } else {
        erasePrevLeftTriangle();
    }
    if (mode_settings_selected < 1) {
        drawRightTriangle();
    } else {
        eraseSkipRightTriangle();
    }

    display.setCursor(20, 10);
    display.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
    display.setTextSize(3);

    switch (mode_settings_selected) {
        case 0: // Mode 0: Auto Mode, Goes through all strings
            display.println("Select Mode             ");
            break;

        case 1: // Mode 1: Individual String Mode
            display.println("  Change    ");
            display.println("   Settings    ");
            break;
    }
}

void drawNewStringSelectionScreen() {
    eraseCenterRectangle();
    erasePrevLeftTriangle();
    if (new_string_mode_selected == 1) {
        drawPrevLeftTriangle();
    } else {
        drawLeftTriangle();
    }
    if (new_string_mode_selected < 2) {
        drawRightTriangle();
    } else {
        eraseSkipRightTriangle();
    }

    display.setCursor(20, 10);
    display.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
    display.setTextSize(3);
    display.println("Select Mode");

    display.setCursor(20, 40);
    switch (new_string_mode_selected) {
        case 1: // New String Mode 1: All New Strings
            display.println("All Strings ");
            break;

        case 2: // New String Mode 2: Individual String 
            display.println("Indv. String ");
            break;
    }
}

// display.String# displays correct number of string in correct order.
void drawStringSelectionScreen() {
    eraseCenterRectangle();
    erasePrevLeftTriangle();
    if (string_selected == 0) {
        drawPrevLeftTriangle();
    } else {
        drawLeftTriangle();
    }
    if (string_selected < 5) {
        drawRightTriangle();
    } else {
        eraseSkipRightTriangle();
    }

    display.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
    display.setTextSize(3);
    display.setCursor(0, 10);
    display.println("Select String");
    tone(BUZZ, 60);
    delay(100);
    noTone(BUZZ);
    display.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
    display.setTextSize(4);

    if (strlen(premade_tuning_lib_letter[library_selected][string_selected]) > 1) {
        display.setCursor(102, 110);
    } else {
        display.setCursor(113, 110);
    }
    display.print(premade_tuning_lib_letter[library_selected][string_selected]);
}

void drawTuningLibrarySelectionScreen() {
    eraseCenterRectangle();
    erasePrevLeftTriangle();
    if (library_selected == 0) {
        drawPrevLeftTriangle();
    } else {
        drawLeftTriangle();
    }
    if (library_selected < 10) {
        drawRightTriangle();
    } else {
        eraseSkipRightTriangle();
    }
    display.setCursor(10, 10);
    display.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
    display.setTextSize(3);
    display.setCursor(30, 10);
    display.println("Pick Tuning");

    display.setCursor(70, 110);
    display.println((String)(lib_name[library_selected]));
}

void drawPluckStringScreen() {
    eraseCenterRectangle();
    eraseSkipRightTriangle();
    erasePrevLeftTriangle();
    // fft
    delay(500);
    unsigned long myTime;
    double peak = fft();
    myTime = tuning_test(peak);
    display.setCursor(98, 113);
    // display.setTextSize(2);
    // display.println("TIME: " + (String)(myTime));
    display.println("DONE!");
    delay(2000);
    // spin the motor based off of fft reading
    // if auto mode, go to next string
    // else go back to library selection

    //delay(1000);
    if ((mode_selected == 0 || new_string_mode_selected == 1) && string_selected < 5) {
        string_selected += 1;
        // add "move to next string" screen
        screen_value = 6;
        drawScreen();
    } else {
        screen_value = 2;
        string_selected = 0;
        display.fillScreen(ST77XX_BLACK);
        drawScreen();
    }
}

void drawTuningScreen() {
    display.fillScreen(ST77XX_BLACK);
    display.setTextSize(4);

    if (strlen(premade_tuning_lib_letter[library_selected][string_selected]) > 1) {
        display.setCursor(102, 10);
    } else {
        display.setCursor(113, 10);
    }

    display.print(premade_tuning_lib_letter[library_selected][string_selected]);

    display.setCursor(10, 10);
    display.setTextSize(2);
    display.print("Input");

    display.setCursor(165, 10);
    display.print("Target");

    if (premade_tuning_lib[library_selected][string_selected] > 100) {
        display.setCursor(160, 30);
    } else {
        display.setCursor(180, 30);
    }

    display.setTextSize(2);
    display.print(premade_tuning_lib[library_selected][string_selected]);
    display.setCursor(183, 50);
    display.print("Hz");

    erasePrevLeftTriangle();
    if (string_selected == 0 || mode_selected == 1) {
        drawPrevLeftTriangle();
    } else {
        drawLeftTriangle();
    }
    if ((mode_selected == 0 || new_string_mode_selected == 1) && string_selected < 5) { // auto tuning
        drawSkipRightTriangle();
    } else {
        eraseSkipRightTriangle();
    }

    drawCenterRectangle("TUNE");
}

void drawFreeTuneScreen() {
    drawRightTriangle();
    drawLeftTriangle();
    display.setCursor(10, 10);
    display.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
    display.setTextSize(3);
    display.println("Peg Winding");
    drawCenterRectangle("DONE");

}

void readWifiConf() {
    // Read wifi conf from flash
    for (int i = 0; i < sizeof(wifiConf); i++) {
        ((char *) (&wifiConf))[i] = char(EEPROM.read(i));
    }
    // Make sure that there is a 0
    // that terminatnes the c string
    // if memory is not initalized yet.
    wifiConf.cstr_terminator = 0;
}

void writeWifiConf() {
    for (int i = 0; i < sizeof(wifiConf); i++) {
        EEPROM.write(i, ((char *) (&wifiConf))[i]);
    }
    EEPROM.commit();
}

bool connectToWiFi() {
    Serial.printf("Connecting to '%s'\n", wifiConf.wifi_ssid);

    WiFi.mode(WIFI_STA);
    WiFi.begin(wifiConf.wifi_ssid, wifiConf.wifi_password);
    if (WiFi.waitForConnectResult() == WL_CONNECTED) {
        Serial.print("Connected. IP: ");
        Serial.println(WiFi.localIP());
        return true;
    } else {
        Serial.println("Connection Failed!");
        return false;
    }
}

void setUpAccessPoint() {
    Serial.println("Setting up access point.");
    Serial.printf("SSID: %s\n", AP_ssid);
    Serial.printf("Password: %s\n", AP_password);

    WiFi.mode(WIFI_AP_STA);
    WiFi.softAPConfig(AP_IP, AP_IP, AP_subnet);
    if (WiFi.softAP(AP_ssid, AP_password)) {
        Serial.print("Ready. Access point IP: ");
        Serial.println(WiFi.softAPIP());
    } else {
        Serial.println("Setting up access point failed!");
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
        Serial.println("Wi-Fi conf saved. Rebooting...");
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
    for (int i = 0; i < SAMPLES; i++) {
        microseconds = micros();    //Overflows after around 70 minutes!
        vReal[i] = analogRead(A0);
        vImag[i] = 0;

        while (micros() < (microseconds + sampling_period_us)) {
        }
    }
    /*FFT*/
    FFT.Windowing(vReal, SAMPLES, FFT_WIN_TYP_TRIANGLE, FFT_FORWARD);
    FFT.Compute(vReal, vImag, SAMPLES, FFT_FORWARD);
    FFT.ComplexToMagnitude(vReal, vImag, SAMPLES);
    double peak = FFT.MajorPeak(vReal, SAMPLES, SAMPLING_FREQUENCY);

    return peak;
}

void spinMotorFlat(int time) { // Loosens tension
    motor_flag = LOW;
    digitalWrite(DIR_WIRE, motor_flag);
    analogWrite(PWM_WIRE, 0);
    delay(time);
    analogWrite(PWM_WIRE, 255);
}

void spinMotorSharp(int time) { // Increases tension
    motor_flag = HIGH;
    digitalWrite(DIR_WIRE, motor_flag);
    analogWrite(PWM_WIRE, 0);
    delay(time);
    analogWrite(PWM_WIRE, 255);
}

void device_operations() {
    switch (test_value) {
        case 1: // LEFT

            switch (screen_value) {
                case 1: // mode selection
                    if (mode_selected > 0) {
                        mode_selected -= 1;
                        drawScreen();
                    } else {
                        mode_settings_selected = 0;
                        screen_value = 8;
                        display.fillScreen(ST77XX_BLACK);
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
                        display.fillScreen(ST77XX_BLACK);
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
                        display.fillScreen(ST77XX_BLACK);
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

                case 5: // pluck string screen
                    screen_value = 2;
                    string_selected = 0;
                    display.fillScreen(ST77XX_BLACK);
                    drawScreen();
                    break;

                case 6: // tuning screen
                    if (string_selected == 0) {
                        if (mode_selected == 1) { // indiv string
                            screen_value = 2;
                            string_selected = 0;
                        } else {
                            screen_value = 3;
                            library_selected = 0;
                        }
                        display.fillScreen(ST77XX_BLACK);
                        drawScreen();
                    } else {
                        if (mode_selected == 1) { // indiv string
                            screen_value = 2;
                            string_selected = 0;
                            display.fillScreen(ST77XX_BLACK);
                        } else {
                            string_selected -= 1;
                        }
                        drawScreen();
                    }
                    break;

                case 7: // new string mode selection screen
                    if (new_string_mode_selected > 1) {
                        new_string_mode_selected -= 1;
                        drawScreen();
                    } else {
                        new_string_mode_selected = 0;
                        mode_selected = 0;
                        screen_value = 1;
                        display.fillScreen(ST77XX_BLACK);
                        drawScreen();
                    }
                    break;

                case 8: // select mode or settings screen
                    if (mode_settings_selected > 0) {
                        mode_settings_selected -= 1;
                        drawScreen();
                    }
                    break;

                case 9: // freq base selection screen
                    if (freq_base_selected > 0) {
                        freq_base_selected -= 1;
                        drawScreen();
                    } else {
                        mode_settings_selected = 0;
                        screen_value = 8;
                        display.fillScreen(ST77XX_BLACK);
                        drawScreen();
                    }
                    break;

            }

            test_value = 0;
            break;

        case 2: // CENTER

            switch (screen_value) {
                case 1: // mode selection

                    switch (mode_selected) {
                        case 0: // auto tuning

                        case 1: // individual string tuning
                            screen_value = 3;
                            library_selected = 0;
                            display.fillScreen(ST77XX_BLACK);
                            drawScreen();
                            break;

                        case 2: // new string tuning
                            screen_value = 7;
                            new_string_mode_selected = 1;
                            display.fillScreen(ST77XX_BLACK);
                            drawScreen();
                            break;

                        case 3: // free tuning
                            screen_value = 4;
                            display.fillScreen(ST77XX_BLACK);
                            drawScreen();
                            break;
                    }
                    break;

                case 2: // string selection
                    screen_value = 6;
                    display.fillScreen(ST77XX_BLACK);
                    drawScreen();
                    break;

                case 3: // library selection
                    if (mode_selected == 1 || new_string_mode_selected == 1) {
                        screen_value = 2;
                        string_selected = 0;
                        display.fillScreen(ST77XX_BLACK);
                        drawScreen();
                    } else {
                        screen_value = 6;
                        display.fillScreen(ST77XX_BLACK);
                        drawScreen();
                    }
                    break;

                case 4: // free tuning
                    screen_value = 1;
                    mode_selected = 0;
                    display.fillScreen(ST77XX_BLACK);
                    drawScreen();
                    break;

                case 6: // tuning screen
                    if (new_string_mode_selected != 0) {
                        spinMotorSharp(3000);
                    }
                    screen_value = 5;
                    drawScreen();
                    break;

                case 7: // new string mode selection
                    screen_value = 3;
                    library_selected = 0;
                    display.fillScreen(ST77XX_BLACK);
                    drawScreen();
                    break;

                case 8: // select mode or settings screen
                
                    switch (mode_settings_selected) {
                        case 0: // mode selection
                            screen_value = 1;
                            mode_selected = 0;
                            display.fillScreen(ST77XX_BLACK);
                            drawScreen();
                            break;

                        case 1: // freq base selection
                            screen_value = 9;
                            freq_base_selected = 0;
                            display.fillScreen(ST77XX_BLACK);
                            drawScreen();
                            break;
                    }
                    break;

                case 9: // freq base selection screen
                    // update tuning library function go brrr
                    changeOfBase();
                    curr_freq_base = freq_base_A[freq_base_selected];
                    screen_value = 8;
                    mode_settings_selected = 0;
                    display.fillScreen(ST77XX_BLACK);
                    drawScreen();
                    break;
            }

            test_value = 0;
            break;

        case 3: // RIGHT

            switch (screen_value) {
                case 1: // mode selection
                    if (mode_selected < 3) {
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
                    if (library_selected < 10) {
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

                case 6: // tuning screen
                    if (mode_selected == 0 && string_selected < 5) {
                        string_selected += 1;
                        drawScreen();
                    }
                    break;
                case 7: // new string mode selection screen
                    if (new_string_mode_selected < 2) {
                        new_string_mode_selected += 1;
                        drawScreen();
                    }
                    break;

                case 8: // select mode or settings screen
                    if (mode_settings_selected < 1) {
                        mode_settings_selected += 1;
                        drawScreen();
                    }
                    break;

                case 9: // freq base selection screen 
                    if (freq_base_selected < 7) {
                        freq_base_selected += 1;
                        drawScreen();
                    }
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
    delay(100);
    button_flags[0] = true;
    button_flags[1] = true;

    interrupts();
}

void ICACHE_RAM_ATTR button_center_pressed() {
    noInterrupts();
    delay(150);
    button_flags[0] = true;
    button_flags[2] = true;

    interrupts();
}

void ICACHE_RAM_ATTR button_right_pressed() {
    noInterrupts();
    delay(100);
    button_flags[0] = true;
    button_flags[3] = true;

    interrupts();
}


void button_operations() {
    button_flags[0] = false;
    noInterrupts();

    if (button_flags[1]) { // BUTTON LEFT
        test_value = 1;
        button_flags[1] = false;
    }

    if (button_flags[2]) { // BUTTON CENTER
        test_value = 2;
        button_flags[2] = false;
    }

    if (button_flags[3]) { // BUTTON RIGHT
        test_value = 3;
        button_flags[3] = false;
    }

    interrupts();
}

// Algo_riddim calculates the tuning after adjusting the A-440 value;
// Currently the static inputs in this function are for the D# values going from A@432 to A@444
float algo_riddim(float input_fr, int std_fr_in, int std_fr_out, float table_440_const) { // Finding shift from 4XX to 4XX freq
    float output_freq;
    float base_out = base_freq_calc(tuning_base[7][0], tuning_base[7][1], freq_base_A[0], freq_base_A[6]);
    double octave = octave_calc(base_out, input_fr);

    // int oct = octave_calc( tuning_base[4][0], tuning_array[4][1]);
    // float rebased = base_freq_calc( tuning_base[7][0], table_440_const, std_fr_in, std_fr_out);
    // octave = octave_calc(rebased, input_fr);

    if (std_fr_in > std_fr_out) {
        output_freq = ((input_fr * pow(2, octave)) - ((abs((std_fr_in - std_fr_out)) / 2) * table_440_const)) / (pow(2, octave));
    } else {
        output_freq = ((input_fr * pow(2, octave)) + ((abs((std_fr_in - std_fr_out)) / 2) * table_440_const)) / (pow(2, octave));
    }
    return output_freq;
}

// Calculating octaves above/below base_freq
double octave_calc(float base_freq, float input_freq) {
    double octave_out;
    double freq = base_freq / input_freq;
    octave_out = (log2f(freq));
    return octave_out;
}

float base_freq_calc(float fr_table_base, float fr_table_const, int freq_std_in, int freq_std_out) {
    float base_out;
    float temp_base;

    // Goes from fr_std_in to A@440
    if (freq_std_in < 440) {
        temp_base = fr_table_base - (abs(440 - freq_std_in) / 2) * fr_table_const;
    } else if (freq_std_in > 440) {
        temp_base = fr_table_base + (abs(440 - freq_std_in) / 2) * fr_table_const;
    }
    // then from new temp_base to A@freq_std_out
    if (freq_std_in > freq_std_out) {
        base_out = temp_base - (abs(freq_std_in - freq_std_out) / 2) * fr_table_const;
    } else if (freq_std_in < freq_std_out) {
        base_out = temp_base + (abs(freq_std_in - freq_std_out) / 2) * fr_table_const;
    }
    return base_out;
}

double cents_calculate(double input_freq, double ref_freq) {
    double ratio = ref_freq / input_freq;
    double cents = 1200 * (log2(ratio));
    return cents;
}

unsigned long tuning_test(double input_freq) {
    double target_freq = premade_tuning_lib[library_selected][string_selected]; // From premade_tuning_lib array
    double current_cents;
    double freq_diff;
    double calc_freq;
    double octave;
    double octave2;
    double current_freq = input_freq;
    unsigned long time1 = millis();
    current_cents = cents_calculate(current_freq, target_freq);

    while (current_cents > 1.75 || current_cents < -1.75) {
        current_freq = fft();

        if (current_freq < 63) { ;;
        } else {
            octave = octave_calc(target_freq, current_freq); // Calculates octave offset based on current input
            octave = round(octave);

            // Calculates current frequency by either dividing or multiplying by the 2^octave
            current_freq = current_freq * pow(2, octave);
            freq_diff = current_freq - target_freq;
            current_cents = cents_calculate(current_freq, target_freq);

            drawRectangles(current_cents, current_freq);

            if (current_freq < 60) { ;; // no spin >:(
            } else {
                if ((current_cents >= 60) || (current_cents <= -60)) {
                    if (freq_diff < 0) {
                        spinMotorSharp(120); // TO Sharp
                    } else {
                        spinMotorFlat(80); // TO Flat
                    }
                } else if ((current_cents < 60 && current_cents >= 30) ||
                           (current_cents <= -30 && current_cents > -60)) {
                    if (freq_diff < 0) {
                        spinMotorSharp(75);
                    } else {
                        spinMotorFlat(45);
                    }
                } else if ((current_cents < 30 && current_cents >= 15) ||
                           (current_cents <= -15 && current_cents > -30)) {
                    if (freq_diff < 0) {
                        spinMotorSharp(65);
                    } else {
                        spinMotorFlat(35);
                    }
                } else if ((current_cents < 15 && current_cents >= 7.5) ||
                           (current_cents <= -7.5 && current_cents > -15)) {
                    if (freq_diff < 0) {
                        spinMotorSharp(45);
                    } else {
                        spinMotorFlat(25);
                    }
                } else if ((current_cents < 7.5 && current_cents >= 3.75) ||
                           (current_cents <= -3.75 && current_cents > -7.5)) {
                    if (freq_diff < 0) {
                        spinMotorSharp(25);
                    } else {
                        spinMotorFlat(15);
                    }
                } else if (current_cents < 3.75 && current_cents > -3.75) {
                    if (freq_diff < 0) {
                        spinMotorSharp(20);
                    } else {
                        spinMotorFlat(10);
                    }
                }
            }
        }
    }
    time1 = millis() - time1;
    time1 = time1 / 1000;
    tone(BUZZ, 60);
    delay(100);
    noTone(BUZZ);
    delay(2000);
    return time1;
}

void changeOfBase(){
    int i = 0, j = 0;
    for ( i = 0; i < 12; i++ ){
        tuning_array[i][0] = algo_riddim( tuning_array[i][0], curr_freq_base, freq_base_A[freq_base_selected], tuning_base[i][1] ); 
        for( j = 1; j < 7; j++){
            tuning_array[i][j] = 2 * tuning_array[i][j-1];
        }
    }
    assignTuningLib();
} 

void assignTuningLib() {
    premade_tuning_lib[0][0] = tuning_array[8][2];
    premade_tuning_lib[0][1] = tuning_array[1][3];
    premade_tuning_lib[0][2] = tuning_array[6][3];
    premade_tuning_lib[0][3] = tuning_array[11][3];
    premade_tuning_lib[0][4] = tuning_array[3][4];
    premade_tuning_lib[0][5] = tuning_array[8][4];

    premade_tuning_lib[1][0] = tuning_array[7][2];
    premade_tuning_lib[1][1] = tuning_array[0][3];
    premade_tuning_lib[1][2] = tuning_array[5][3];
    premade_tuning_lib[1][3] = tuning_array[10][3];
    premade_tuning_lib[1][4] = tuning_array[2][4];
    premade_tuning_lib[1][5] = tuning_array[7][4];

    premade_tuning_lib[2][0] = tuning_array[6][2];
    premade_tuning_lib[2][1] = tuning_array[11][2];
    premade_tuning_lib[2][2] = tuning_array[4][3];
    premade_tuning_lib[2][3] = tuning_array[9][3];
    premade_tuning_lib[2][4] = tuning_array[1][4];
    premade_tuning_lib[2][5] = tuning_array[6][4];

    premade_tuning_lib[3][0] = tuning_array[6][2];
    premade_tuning_lib[3][1] = tuning_array[1][3];
    premade_tuning_lib[3][2] = tuning_array[6][3];
    premade_tuning_lib[3][3] = tuning_array[11][3];
    premade_tuning_lib[3][4] = tuning_array[3][4];
    premade_tuning_lib[3][5] = tuning_array[8][4];

    premade_tuning_lib[4][0] = tuning_array[4][2];
    premade_tuning_lib[4][1] = tuning_array[11][2];
    premade_tuning_lib[4][2] = tuning_array[4][3];
    premade_tuning_lib[4][3] = tuning_array[9][3];
    premade_tuning_lib[4][4] = tuning_array[1][4];
    premade_tuning_lib[4][5] = tuning_array[6][4];

    premade_tuning_lib[5][0] = tuning_array[3][2];
    premade_tuning_lib[5][1] = tuning_array[10][2];
    premade_tuning_lib[5][2] = tuning_array[3][3];
    premade_tuning_lib[5][3] = tuning_array[8][3];
    premade_tuning_lib[5][4] = tuning_array[0][4];
    premade_tuning_lib[5][5] = tuning_array[5][4];

    premade_tuning_lib[6][0] = tuning_array[1][2];
    premade_tuning_lib[6][1] = tuning_array[8][2];
    premade_tuning_lib[6][2] = tuning_array[1][3];
    premade_tuning_lib[6][3] = tuning_array[6][3];
    premade_tuning_lib[6][4] = tuning_array[10][3];
    premade_tuning_lib[6][5] = tuning_array[3][4];

    premade_tuning_lib[7][0] = tuning_array[7][2];
    premade_tuning_lib[7][1] = tuning_array[1][3];
    premade_tuning_lib[7][2] = tuning_array[6][3];
    premade_tuning_lib[7][3] = tuning_array[10][3];
    premade_tuning_lib[7][4] = tuning_array[1][4];
    premade_tuning_lib[7][5] = tuning_array[6][4];

    premade_tuning_lib[8][0] = tuning_array[7][2];
    premade_tuning_lib[8][1] = tuning_array[11][2];
    premade_tuning_lib[8][2] = tuning_array[6][3];
    premade_tuning_lib[8][3] = tuning_array[11][3];
    premade_tuning_lib[8][4] = tuning_array[3][4];
    premade_tuning_lib[8][5] = tuning_array[6][4];

    premade_tuning_lib[9][0] = tuning_array[4][2];
    premade_tuning_lib[9][1] = tuning_array[11][2];
    premade_tuning_lib[9][2] = tuning_array[4][3];
    premade_tuning_lib[9][3] = tuning_array[11][3];
    premade_tuning_lib[9][4] = tuning_array[4][4];
    premade_tuning_lib[9][5] = tuning_array[8][4];

    premade_tuning_lib[10][0] = tuning_array[8][2];
    premade_tuning_lib[10][1] = tuning_array[3][3];
    premade_tuning_lib[10][2] = tuning_array[8][3];
    premade_tuning_lib[10][3] = tuning_array[0][4];
    premade_tuning_lib[10][4] = tuning_array[3][4];
    premade_tuning_lib[10][5] = tuning_array[8][4];
}
// TODO:
/*
*   Work on Homescreen GUI -- done-ish
*   AUTO TUNE: Remove "TUNE" button selection. Just have it move to the next string automatically ?
*   Free Mode change name to Peg Winding -- Done
*   Maybe free mode display string freq/note? -- Done
*   Add New String input, winds until tight-ish? 
*   Tuning bar: only one bar moving, rather than the whole set of bars filling?
*   __________________________________________
*   Save custom tuning? i.e. custom lib?
*   change from 440 to 4XX intonation
*   Test on other stringed instrument?
*/
