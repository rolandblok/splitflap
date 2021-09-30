//http://robocraft.ru/files/datasheet/28BYJ-48.pdf
//http://robocraft.ru/files/datasheet/28BYJ-48.pdf

#include <Stepper.h>

#define STEPPER_PIN_1  9
#define STEPPER_PIN_2 10
#define STEPPER_PIN_3 11
#define STEPPER_PIN_4 12

#define LED_PIN       13
#define SENSOR_PIN     3


void setup() {
  Serial.begin(9600);
  Serial.println("Split Flap This");
  pinMode(  STEPPER_PIN_1, OUTPUT);
  pinMode(  STEPPER_PIN_2, OUTPUT);
  pinMode(  STEPPER_PIN_3, OUTPUT);
  pinMode(  STEPPER_PIN_4, OUTPUT);

  pinMode(LED_PIN, OUTPUT); //set LED pin as output
  pinMode(SENSOR_PIN, INPUT); //set sensor pin as input
  
}

void loop() {

  static int last_hall_val = LOW;
  int hall_val = digitalRead(SENSOR_PIN); //Read the sensor
  bool high_found_already = false;
  if (last_hall_val != hall_val) {
    if(hall_val == LOW) //when magnetic field is detected, turn led on
    {
      digitalWrite(LED_BUILTIN, HIGH);
      digitalWrite(LED_PIN, HIGH);
      Serial.println("switch HIGH " + String(millis()));
      delay(500);
    }
    else
    {
      digitalWrite(LED_BUILTIN, LOW);
      digitalWrite(LED_PIN, LOW);
      Serial.println("switch LOW  " + String(millis()));
    }
  }
  last_hall_val = hall_val;

  
  OneStep(false);

}

float stepsPerRevolution = 64;
float degreePerRevolution = 5.625;
int degToSteps(float deg) {
  return (int)((stepsPerRevolution / degreePerRevolution ) * deg * 0.5);
}

void OneStep(bool dir) {
  // full step
    //https://www.youtube.com/watch?v=B86nqDRskVU
  static int step_number = 0;
  if (dir) {
    switch (step_number) {
      case 0:
        digitalWrite(STEPPER_PIN_1, HIGH);
        digitalWrite(STEPPER_PIN_2, HIGH);
        digitalWrite(STEPPER_PIN_3, LOW);
        digitalWrite(STEPPER_PIN_4, LOW);
        break;
      case 1:
        digitalWrite(STEPPER_PIN_1, LOW);
        digitalWrite(STEPPER_PIN_2, HIGH);
        digitalWrite(STEPPER_PIN_3, HIGH);
        digitalWrite(STEPPER_PIN_4, LOW);
        break;
      case 2:
        digitalWrite(STEPPER_PIN_1, LOW);
        digitalWrite(STEPPER_PIN_2, LOW);
        digitalWrite(STEPPER_PIN_3, HIGH);
        digitalWrite(STEPPER_PIN_4, HIGH);
        break;
      case 3:
        digitalWrite(STEPPER_PIN_1, HIGH);
        digitalWrite(STEPPER_PIN_2, LOW);
        digitalWrite(STEPPER_PIN_3, LOW);
        digitalWrite(STEPPER_PIN_4, HIGH);
        break;
    }
  } else {
    switch (step_number) {
      case 0:
        digitalWrite(STEPPER_PIN_1, HIGH);
        digitalWrite(STEPPER_PIN_2, LOW);
        digitalWrite(STEPPER_PIN_3, LOW);
        digitalWrite(STEPPER_PIN_4, HIGH);
        break;
      case 1:
        digitalWrite(STEPPER_PIN_1, LOW);
        digitalWrite(STEPPER_PIN_2, LOW);
        digitalWrite(STEPPER_PIN_3, HIGH);
        digitalWrite(STEPPER_PIN_4, HIGH);
        break;
      case 2:
        digitalWrite(STEPPER_PIN_1, LOW);
        digitalWrite(STEPPER_PIN_2, HIGH);
        digitalWrite(STEPPER_PIN_3, HIGH);
        digitalWrite(STEPPER_PIN_4, LOW);
        break;
      case 3:
        digitalWrite(STEPPER_PIN_1, HIGH);
        digitalWrite(STEPPER_PIN_2, HIGH);
        digitalWrite(STEPPER_PIN_3, LOW);
        digitalWrite(STEPPER_PIN_4, LOW);
    }
  }
  step_number++;
  if (step_number > 3) {
    step_number = 0;
  }
  delay(3); // 2 ms has almost no torque https://www.youtube.com/watch?v=14jF8umwJLI
}
