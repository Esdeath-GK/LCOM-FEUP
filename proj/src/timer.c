#include <lcom/lcf.h>
#include <lcom/timer.h>

#include <stdint.h>

#include "i8254.h"


/* CUSTOM DEFINITIONS */
#define INITIALIZATION 0x03   /* Number whose binary representation has 1(s) listed in the same position as in status-byte AFTER SHIFTING 4 bits to the right */
#define MODE_ANALYSIS 0x0E    /* Same as above, but it happens pre-shift. */
#define BASE_ANALYSIS 0x01    /* As aforementioned */
/* ****************** */

uint32_t counter = 0;
int hook_id = 2;

int (timer_subscribe_int)(uint8_t *bit_no){

    *bit_no = BIT(hook_id);

    if (sys_irqsetpolicy(TIMER0_IRQ, IRQ_REENABLE, &hook_id) != OK) {
      printf("sys_irqsetpolicy() function has failed.\n");
      return -1;
    }
  
    /*
    if (sys_irqenable(&hook_id) != OK){
      printf("sys_irqenable() function has failed.\n");
      return -1;
    }*/

    return 0;
}

int (timer_unsubscribe_int)(){

    if (sys_irqrmpolicy(&hook_id) != OK) {
      printf("sys_irqrmpolicy() function has failed.\n");
      return -1;
    }

    return 0;
}

void (timer_int_handler)(){
  counter = counter + 1;
  return;
}

void reset_counter(){
  counter = 0;
}

uint32_t getCounter(){
  return counter;
}
