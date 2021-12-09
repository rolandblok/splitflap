//http://robocraft.ru/files/datasheet/28BYJ-48.pdf
//http://robocraft.ru/files/datasheet/28BYJ-48.pdf

#include <Stepper.h>

#include "myeeprom.h"

#define STEPPER_PIN_1  9
#define STEPPER_PIN_2 10
#define STEPPER_PIN_3 11
#define STEPPER_PIN_4 12

#define LED_PIN       13
#define SENSOR_PIN     3

#define NO_FLAPS      40
#define MOTOR_STEPS   2048


void(*resetFunc) (void) = 0; 

static unsigned int glb_flap_positions[NO_FLAPS];
static unsigned int glb_rondje = 0 ; // debug variable
static unsigned int glb_delay_us = 2000;

void determine_flap_positions(unsigned int zero_position) {
  float steps_per_flap = MOTOR_STEPS / NO_FLAPS;
  for (int f_i = 0; f_i < NO_FLAPS; f_i++ ) {
    glb_flap_positions[f_i] = (zero_position + round(f_i * steps_per_flap)) % MOTOR_STEPS;
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("Split Flap This");
  pinMode(  STEPPER_PIN_1, OUTPUT);
  pinMode(  STEPPER_PIN_2, OUTPUT);
  pinMode(  STEPPER_PIN_3, OUTPUT);
  pinMode(  STEPPER_PIN_4, OUTPUT);

  pinMode(LED_PIN, OUTPUT); //set LED pin as output
  pinMode(SENSOR_PIN, INPUT); //set sensor pin as input

  if (eeprom_init()) {
    eeprom_serial();
  } else {
    Serial.println("Eeprom not valid");
  }
  determine_flap_positions(eeprom_getZeroPosition());
  
}

void loop() {
  static unsigned int from_zero_step = 0;
  static bool paused = false;
  static int pause_steps_remain = 0;
  // ===============
  // serial business
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil(10);
    if (command.equals("restart")) {
      Serial.end();  //clears the serial monitor  if used
      resetFunc();
      delay(1000);
    } else if (command.startsWith("z ")) {
      long zero_read = command.substring(2).toInt();
      if (zero_read > -1) {
        eeprom_setZeroPosition((unsigned int)zero_read);
        Serial.println("zero set to : " + String(zero_read) + " steps");
        determine_flap_positions(eeprom_getZeroPosition());

      } else { 
        Serial.println("zero not recognized : " + command.substring(2));
      }
    } else if (command.startsWith("d")) {
      long delay_10th_ms = command.substring(2).toInt();
      if (delay_10th_ms > 0) {
        glb_delay_us = 100* delay_10th_ms;
      }
    } else if (command.startsWith("p")) {
      paused = !paused;
      Serial.println("Paused is " + String(paused));
    } else if (command.startsWith("s")) {
      long pause_steps_remain_l = command.substring(2).toInt();
      pause_steps_remain = pause_steps_remain_l;
      
    } else if (command.startsWith("x")) {
        glb_rondje = 0;
    } else { 
        Serial.println("commands: ");
        Serial.println("  z 100  : zero position at step 100 from hall zero (-1 : no pause) ; cur:" + String(eeprom_getZeroPosition()));
        Serial.println("  d 30   : set step 10th delay [0.1 ms] ; cur " + String(glb_delay_us/100));
        Serial.println("  p      : (un)pause animation ; cur " + String(paused));
        Serial.println("  s 100  : when in paused mode : set 100 steps (negative possible)");
        Serial.println("  x      : restart animation");
        Serial.println("  restart: restart micro controller");
        eeprom_serial();
        Serial.flush();
    }
  } 

  static int last_hall_val = LOW;
  int hall_val = digitalRead(SENSOR_PIN); //Read the sensor
  bool high_found_already = false;
  if (last_hall_val != hall_val) {
    if(hall_val == LOW) //when magnetic field is detected, turn led on
    {
      digitalWrite(LED_BUILTIN, HIGH);
      digitalWrite(LED_PIN, HIGH);
      Serial.println("max step " + String(from_zero_step));
      from_zero_step = 0;
      
    }
    else
    {
      digitalWrite(LED_BUILTIN, LOW);
      digitalWrite(LED_PIN, LOW);
    }
  }
  last_hall_val = hall_val;


  // animation
  static int huidige = 0;
  if (!paused) {
    if (glb_flap_positions[huidige] == from_zero_step) {
      huidige++;
      if (glb_rondje % 2) {
        PowerDown();
        delay(2000);
      }
      if (huidige == NO_FLAPS) {
        huidige = 0;
        glb_rondje ++;
      }
    }
    OneFullStep(false);
    from_zero_step++;

  } else {
    if (pause_steps_remain > 0) {
      Serial.println(" manual step " + String(pause_steps_remain));
      OneFullStep(false);
      pause_steps_remain--;
    } else if (pause_steps_remain < 0) {
      Serial.println(" manual step " + String(pause_steps_remain));
      OneFullStep(true);
      pause_steps_remain++;
    } else {
      PowerDown();
    }
  }
  

}

void PowerDown() {
    digitalWrite(STEPPER_PIN_1, LOW);
    digitalWrite(STEPPER_PIN_2, LOW);
    digitalWrite(STEPPER_PIN_3, LOW);
    digitalWrite(STEPPER_PIN_4, LOW);
}

//float stepsPerRevolution = 64;
//float degreePerRevolution = 5.625;
//int degToSteps(float deg) {
//  return (int)((stepsPerRevolution / degreePerRevolution ) * deg * 0.5);
//}

void OneFullStep(bool dir) {
  // full step
    //https://www.youtube.com/watch?v=B86nqDRskVU
  static int step_number = 0;
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
  dir ? step_number++ : step_number--;
  if (step_number > 3) {
    step_number = 0;
  }
  if (step_number < 0 ) {
    step_number = 3;
  }
  delayMicroseconds(glb_delay_us); // 2 ms has almost no torque https://www.youtube.com/watch?v=14jF8umwJLI
  
}

void OneHalfStep(bool dir) {
  // full step
    //https://www.youtube.com/watch?v=B86nqDRskVU
  static int step_number = 0;
  switch (step_number) {
    case 0:
      digitalWrite(STEPPER_PIN_1, HIGH);
      digitalWrite(STEPPER_PIN_2, LOW);
      digitalWrite(STEPPER_PIN_3, LOW);
      digitalWrite(STEPPER_PIN_4, LOW);
      break;
    case 1:
      digitalWrite(STEPPER_PIN_1, HIGH);
      digitalWrite(STEPPER_PIN_2, HIGH);
      digitalWrite(STEPPER_PIN_3, LOW);
      digitalWrite(STEPPER_PIN_4, LOW);
      break;
    case 2:
      digitalWrite(STEPPER_PIN_1, LOW);
      digitalWrite(STEPPER_PIN_2, HIGH);
      digitalWrite(STEPPER_PIN_3, LOW);
      digitalWrite(STEPPER_PIN_4, LOW);
      break;
    case 3:
      digitalWrite(STEPPER_PIN_1, LOW);
      digitalWrite(STEPPER_PIN_2, HIGH);
      digitalWrite(STEPPER_PIN_3, HIGH);
      digitalWrite(STEPPER_PIN_4, LOW);
      break;
    case 4:
      digitalWrite(STEPPER_PIN_1, LOW);
      digitalWrite(STEPPER_PIN_2, LOW);
      digitalWrite(STEPPER_PIN_3, HIGH);
      digitalWrite(STEPPER_PIN_4, LOW);
      break;
    case 5:
      digitalWrite(STEPPER_PIN_1, LOW);
      digitalWrite(STEPPER_PIN_2, LOW);
      digitalWrite(STEPPER_PIN_3, HIGH);
      digitalWrite(STEPPER_PIN_4, HIGH);
      break;
    case 6:
      digitalWrite(STEPPER_PIN_1, LOW);
      digitalWrite(STEPPER_PIN_2, LOW);
      digitalWrite(STEPPER_PIN_3, LOW);
      digitalWrite(STEPPER_PIN_4, HIGH);
      break;
    case 7:
      digitalWrite(STEPPER_PIN_1, HIGH);
      digitalWrite(STEPPER_PIN_2, LOW);
      digitalWrite(STEPPER_PIN_3, LOW);
      digitalWrite(STEPPER_PIN_4, HIGH);
      break;
  }
  dir ? step_number++ : step_number--;
  if (step_number > 7) {
    step_number = 0;
  }
  if (step_number < 0 ) {
    step_number = 7;
  }
  delayMicroseconds(glb_delay_us); // 2 ms has almost no torque https://www.youtube.com/watch?v=14jF8umwJLI
  
}
