/*
 * This code was for using a stepper motor to try and get the motor to respond to the readings
 * from the analog A0 pin from the ESP8266. 
 * The GPIO Pins used for the motor are: 5, 4, 14, 12. These GPIO correspond to physical pins: D1, D2, D5, D6
 * Analog input pin: A0, used to connect the Piezosensor
 */

#include <Stepper.h> // Stepper motor library

const int stepsPerRevolution = 1028;  // change this to fit the number of steps per revolution for motor
Stepper myStepper(stepsPerRevolution, 5, 4, 14, 12); // GPIO Pins in code, corresponds to pins D1 D2 D5 D6 on board
int stepCount = 0;  // number of steps the motor has taken
void setup() {
  // Baud rate for plotter
  Serial.begin(9600); // Change to match baud rate of motor
}

void loop() {
  // read the sensor value from piezo:
  int sensorReading = analogRead(A0); // Reads from pin A0
  // map it to a range from 0 to 100:
  int motorSpeed = map(sensorReading, 0, 1023, 0, 100);
  // set the motor speed:
//  while(sensorReading< 100)
  Serial.println(sensorReading); // Offsetting to start at 0
  if(sensorReading > 10){
     if (motorSpeed > 0) {
      myStepper.setSpeed(motorSpeed);
      // step 1/100 of a revolution:
      myStepper.step(stepsPerRevolution / 2);
      }
    }
}
