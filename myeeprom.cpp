#include "myeeprom.h"

#include <Arduino.h>
#include <EEPROM.h>

//=================================
// ROLAND IMPLEMENTATION 
//=================================

typedef struct EepromMem_struct {
  byte             valid;
  unsigned int     zero_position;
  
  byte    checksum;
} EepromMem;


//extern EepromMem eeprom_mem_glb;
EepromMem eeprom_mem_glb;



// ===============
// getters setters
// ===============
void eeprom_setZeroPosition(unsigned int zero_position) {
  eeprom_mem_glb.zero_position = zero_position;
  eeprom_write();
}
unsigned int eeprom_getZeroPosition() {
  if (eeprom_mem_glb.valid) {
    return eeprom_mem_glb.zero_position;
  } else {
    return 0;
  }
}



// ===============
// HELPERS
// ===============
byte checksum(EepromMem eeprom_memo_arg) {
  return eeprom_memo_arg.zero_position ;
}


/** 
 *  Use this in debugging to reset your eeprom
 */
boolean eeprom_clear() {
  EepromMem eeprom_mem_tmp = {};
  eeprom_mem_glb = eeprom_mem_tmp;
  eeprom_write();
  
//  EEPROM.begin(sizeof(eeprom_mem_glb));
//  for (int i = 0; i < sizeof(eeprom_mem_glb); i++) {
//    EEPROM.write(i, 0);
//  }
//  EEPROM.commit();
//  EEPROM.end();

  return true;
}


// ===============
// GENERICS
// ===============
boolean eeprom_init() {
  EepromMem eeprom_mem_tmp = {};
  EEPROM.get(0, eeprom_mem_tmp);
  if (eeprom_mem_tmp.valid == 1) {
    if (eeprom_mem_tmp.checksum == checksum(eeprom_mem_tmp)) {
      eeprom_mem_glb = eeprom_mem_tmp;
    } else {
      Serial.println("eeprom checksum invalid");
      return false;
    }
  } else {
    Serial.println("eeprom read invalid");
    return false;
  }

  return true;
}

boolean eeprom_write() {
  eeprom_mem_glb.valid = 1;
  eeprom_mem_glb.checksum = checksum(eeprom_mem_glb);
  EEPROM.put(0, eeprom_mem_glb);

  return true;
}

void eeprom_serial() {
  Serial.println("--EEPROM--------");
  Serial.println("zero_position     : " + String(eeprom_mem_glb.zero_position) );
  Serial.println("----------------");
}
