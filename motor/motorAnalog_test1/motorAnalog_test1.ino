// D5/GPIO 14 -> FG green wire needs 5k pullup resistor
// D6/GPIO 12 -> PWM white/blue wire 
// D8/GPIO 15 -> Direction yellow/orange wire

/*
 * TEST SWITCH - IN USE
 * D6/GPIO12 -> Direction yellow/orange wire
 * D8/GPIO15 -> PWM white/blue wire
 */

int i = 0;
int motorSpeed = 250; // 0 to 256. -> 1024 stops the motor
int piezoIn; // Reading from analog input, piezo
int motorIn; // Reading from PWM pin on motor
unsigned long myTime = 0;
bool flag = HIGH;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(15, OUTPUT); // Direction control PIN GPIO15/D8 with direction wire
  pinMode(12, OUTPUT); // PWM PIN GPIO12/D6 with PWM Wire

}

void loop() {
piezoIn = analogRead(A0);
if( piezoIn > 20 ){
  digitalWrite(12, true);
  analogWrite(15, 0);
  delay(5000);
//  delay(10000);
} else {
   analogWrite(15, 1020); // 1024 stops it, 
   digitalWrite(12, false);
  }
  delay(500);
  motorIn = analogRead(12);

  Serial.println(motorIn); // Print to Serial Monitor
}
