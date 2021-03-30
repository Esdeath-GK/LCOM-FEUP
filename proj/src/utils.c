#include <lcom/lcf.h>
#include <stdint.h>

uint32_t sys_calls = 0;

int(util_get_LSB)(uint16_t val, uint8_t *lsb) {
  if(lsb == NULL){
    return -1;
  }
  else{
    // Extract the least significant byte:
      *lsb = val & 0xff;
    return 0;
  }
}

int(util_get_MSB)(uint16_t val, uint8_t *msb) {
  if(msb == NULL){
    return -1;
  }
  else{
    // Extract the least significant byte:
      *msb = (val >> 8) & 0xff;
    return 0;
  }
}

int (util_sys_inb)(int port, uint8_t *value) {
  if(value == NULL){
    return -1;
  }
  else{
    sys_calls++;;

    uint32_t byte;
    // Call sys_inb;
    sys_inb(port, &byte);
    
    // Extract to a byte and store it in "value";
    *value = byte & 0xff;
    
    return 0;
  }
}
