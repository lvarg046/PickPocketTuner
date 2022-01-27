// D5/GPIO 14 -> FG green wire needs 5k pullup resistor
// D6/GPIO 12 -> PWM white/blue wire
// D8/GPIO 15 -> Direction yellow/orange wire

int i = 0;
int motorSpeed = 250;
unsigned long myTime;
bool flag = HIGH;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(15, OUTPUT); // Direction control PIN GPIO15/D8 with direction wire
  pinMode(12, OUTPUT); // PWM PIN GPIO12/D6 with PWM Wire
  pinMode(14, OUTPUT); // FG PIN GPIO14/D5
}

void loop() {
  // put your main code here, to run repeatedly:
//  digitalWrite(15, HIGH);
//  analogWrite(12, 300);
//  Serial.print("HIGH\n");
//  delay(10000);
//  
//  digitalWrite(15, LOW);
//  analogWrite(12, motorSpeed);
//  analogWrite(12, 0);
//  Serial.print("LOW\n");
//  delay(10000);
  if( millis() - myTime > 5000){
    flag = !flag;
    Serial.print(flag);
    digitalWrite(15, flag);
    myTime = millis();
  }
  while( Serial.available() == 0){
  }
  int motorSpeed = Serial.parseInt(); // This doesn't work currently, must manually set speed to a val from 0 - 256
  analogWrite(12, motorSpeed ); // input speed must be int
  delay(200);
  for( int j = 0; j<8; j++){
    i += pulseIn( 14, HIGH, 500000); // SIGNAL OUTPUT PIN GPIO14/D5 with FG wire
    // Cycle = 2*i,1s = 1000000us，Signal cycle pulse number：27*2
  }

  i = i >> 3;
  Serial.print(111111 / i);
  Serial.println(" r/min");
  i = 0;
}
