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

#define ANIM_CALIB    0
#define ANIM_LETTER   1
#define ANIM_TEST     2
#define ANIM_HELLO    3

#define ANIM_HELLO_PAUSE_MS 1000

void(*resetFunc) (void) = 0; 

static unsigned int glb_flap_positions[NO_FLAPS];
static unsigned int glb_delay_us = 2000;
static unsigned int glb_anim_type = ANIM_CALIB;
static unsigned int glb_anim_test_rondje = 0 ; // debug variable
static char         glb_anim_letter      = ' ';
static String       glb_anim_hello_string = String("hello world ");
static unsigned int glb_anim_hello_pos    = 0;

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
      // UPDATE ZERO POSITION
      long zero_read = command.substring(2).toInt();
      if (zero_read > -1) {
        eeprom_setZeroPosition((unsigned int)zero_read);
        Serial.println("zero set to : " + String(zero_read) + " steps");
        determine_flap_positions(eeprom_getZeroPosition());
        glb_anim_type = ANIM_LETTER;
        paused = false;
      } else { 
        Serial.println("zero not recognized : " + command.substring(2));
      }
    } else if (command.startsWith("d")) {
      // SET THE DELAY
      long delay_10th_ms = command.substring(2).toInt();
      if (delay_10th_ms > 0) {
        glb_delay_us = 100* delay_10th_ms;
      }
    } else if (command.startsWith("p")) {
      // (UN)PAUSE THE ANIMATION
      paused = !paused;
      Serial.println("Paused is " + String(paused));
    } else if (command.startsWith("s")) {
      // SET <S> STEPS
      paused = true;
      long pause_steps_remain_l = command.substring(2).toInt();
      pause_steps_remain = pause_steps_remain_l;
      
    } else if (command.startsWith("x")) {
        // START LETTER BY LETTER ANIMATION
        glb_anim_test_rondje = 0;
        glb_anim_type = ANIM_TEST;
        paused = false;
    } else if (command.startsWith("h")) {
        // START hello string animation
        if (command.length() <= 2) {
          glb_anim_hello_string = String("hello world ");
        } else {
          glb_anim_hello_string = String(command.substring(2));
          glb_anim_hello_string.toLowerCase();
        }
        Serial.println("text request " + String(glb_anim_hello_string));
        glb_anim_type = ANIM_HELLO;
        glb_anim_hello_pos = 0;
        paused = false;
    } else if (command.startsWith("l")) {
        // START display letter animation
        glb_anim_type = ANIM_LETTER;
        glb_anim_letter = command.charAt(2);
        Serial.println("letter request " + String(glb_anim_letter));
        paused = false;
    } else { 
        Serial.println("commands: ");
        Serial.println("  z 100  : zero position at step 100 from hall zero (-1 : no pause) ; cur:" + String(eeprom_getZeroPosition()));
        Serial.println("  d 30   : set step 10th delay [0.1 ms] ; cur " + String(glb_delay_us/100));
        Serial.println("  p      : (un)pause animation ; cur " + String(paused));
        Serial.println("  s 100  : when in paused mode : set 100 steps (negative possible)");
        Serial.println("  x      : restart test animation");
        Serial.println("  h <txt>: restart hello animation");
        Serial.println("  l <ltr>: show a letter (if available)");
        Serial.println("  restart: restart micro controller");
        eeprom_serial();
        Serial.flush();
    }
  } 

  static int last_hall_val = LOW;
  int hall_val = digitalRead(SENSOR_PIN); //Read the sensor
  if (last_hall_val != hall_val) {
    if(hall_val == LOW) //when magnetic field is detected, turn led on
    {
      if (glb_anim_type == ANIM_CALIB) {
        glb_anim_type = ANIM_LETTER;
        glb_anim_letter = ' ';
      }
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
  if (!paused) {
    if (glb_anim_type == ANIM_CALIB) {
      OneFullStep(false);
      from_zero_step++;
      
    } else if (glb_anim_type == ANIM_TEST) {
      static int huidige = 0;
      if (glb_flap_positions[huidige] == from_zero_step) {
        huidige++;
        if (glb_anim_test_rondje % 2) {
          PowerDown();
          delay(2000);
        }
        if (huidige == NO_FLAPS) {
          huidige = 0;
          glb_anim_test_rondje ++;
        }
      }
      OneFullStep(false);
      from_zero_step++;
      
    } else if (glb_anim_type == ANIM_HELLO) {
      if (from_zero_step == glb_flap_positions[flap_index(glb_anim_hello_string.charAt(glb_anim_hello_pos))]) {
        glb_anim_hello_pos ++;
        if (glb_anim_hello_pos == glb_anim_hello_string.length()) {
          glb_anim_hello_pos = 0;
        }
        PowerDown();
        delay(ANIM_HELLO_PAUSE_MS);
      } else {
        OneFullStep(false);
        from_zero_step++;
      }
    } else if (glb_anim_type == ANIM_LETTER) {
      if (from_zero_step == glb_flap_positions[flap_index(glb_anim_letter)]) {
        PowerDown();
        paused = true;
      } else {
        OneFullStep(false);
        from_zero_step++;
      }
    }

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

/**
 * return the flap index for a character.
 * It depends on the calibration and filling-configuration of the flaps.
 * */
unsigned int flap_index(char c) {
  unsigned int flap_i = 0;
  if (c == ' ') {
    flap_i = 0;
  } else if ((c >= 'a') && (c <= 'z')) {
    flap_i = 1  + c - 'a';
  } else if ((c >= '0') && (c <= '9')) {
    flap_i = 27 + c - '0';
  } else {
    // non-present flap requested
    Serial.println("non-present flap requested : " + String(c));
  }
  return flap_i;
}


// ===============
// STEPPER CONTROL
// ===============
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
