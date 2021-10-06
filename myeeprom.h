#ifndef MY_EEPROM_AA
#define MY_EEPROM_AA

#include <Arduino.h>

//==========================

void          eeprom_setZeroPosition(unsigned int zero_position);  // value 0 .. xx
unsigned int  eeprom_getZeroPosition();



boolean eeprom_init();
boolean eeprom_write();
boolean eeprom_clear();
void    eeprom_serial();

#endif
