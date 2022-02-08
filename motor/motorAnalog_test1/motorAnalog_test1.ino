/*
 * IN USE
 * GPIO15/D8 -> PWM Blue wire
 * GPIO1/TXPIN -> Orange Direction wire
 * GPIO16/D0 -> Green FG wire - 5k pullup resistor
 * GPIO0/D3 --> Capacitor in series for buzzer
 * 
 * What is piezoIn really inputting ot A0? Voltage or?? Use function generator to find out
 */
#define DIR_WIRE 1
#define PWM_WIRE 15
#define BUZZ 0

int motorSpeed; // 0 to 256. -> 260 stops the motor
const unsigned char Passive_buzz = 0; // GPIO0 used for buzzer
//int piezoIn; // Reading from analog input, piezo
int motorIn; // Reading from PWM pin on motor
unsigned long myTime = 0;
bool flag = HIGH;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  // New 
  pinMode(DIR_WIRE, OUTPUT); // Direction control GPIO01/TXPin Orange Direction
  pinMode(PWM_WIRE, OUTPUT); // Blue PWM Wire GPIO15/D8
  pinMode(BUZZ, OUTPUT); // Buzzer output Pin

}

void loop() {
int piezoIn = analogRead(A0);
Serial.println(piezoIn);
if( piezoIn > 15 ){ 
  flag = !flag; // Done to reverse the direction
  tone(BUZZ, 60); // Making passive buzzer buzz at input hz
  digitalWrite(DIR_WIRE, flag); // New 
  analogWrite(PWM_WIRE, 0); // New
  
//  digitalWrite(12, true); // Old
//  analogWrite(15, 0); // Old 
  delay(200);
} else {

  noTone(BUZZ); // Turn off buzzer
  analogWrite(PWM_WIRE, 260); // New  stops it
  digitalWrite(DIR_WIRE, flag); // New
  
//   analogWrite(15, 260); // Old 260 stops it 
//   digitalWrite(12, flag); // Old, direction flip

  }
//  delay(500);
//  motorIn = analogRead(12); // Old
  motorIn = analogRead(15); // New
//  Serial.println(motorIn); // Print to Serial Monitor
}
