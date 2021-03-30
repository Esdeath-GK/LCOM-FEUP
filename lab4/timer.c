#include <lcom/lcf.h>
#include "timer.h"

#include <stdint.h>

#include "i8254.h"

/* CUSTOM DEFINITIONS */
#define INITIALIZATION 0x03   /* Number whose binary representation has 1(s) listed in the same position as in status-byte AFTER SHIFTING 4 bits to the right */
#define MODE_ANALYSIS 0x0E    /* Same as above, but it happens pre-shift. */
#define BASE_ANALYSIS 0x01    /* As aforementioned */
/* ****************** */

uint32_t counter = 0;
int hook_id = 6;

int (timer_set_frequency)(uint8_t timer, uint32_t freq) {

  if(freq < 19){
    // Note ... need to analyze case 1 - 18
    printf(":: Invalid parameter detected ... Frequencies bellow 19 are invalid! ::\n");
    printf(":: Terminating program abruptly ... \n");
    return -1;
  }
  // Step 1: Extract information (read-back command) from counter
    uint8_t byte_conf;
    timer_get_conf(timer, &byte_conf);
  
  // Step 2: Set up the correct input value:
    uint16_t inputval = TIMER_FREQ / freq;
  
    uint8_t MSB_input;
    uint8_t LSB_input;

    util_get_LSB(inputval, &LSB_input);
    util_get_MSB(inputval, &MSB_input);

  // Step 3: Modify control world while preserving the last 4 bits.

    uint8_t timer_selection;  // Bits 7 and 6.

    switch(timer){
      case '0':
        timer_selection = TIMER_SEL0;
        break;
      case '1':
        timer_selection = TIMER_SEL1;
        break;
      case '2':
        timer_selection = TIMER_SEL2;
        break;
    }

    uint8_t initialization_mode = TIMER_LSB_MSB;  // Bits 5 and 4

    uint8_t last4bits = byte_conf & (BIT(0) | BIT(1) | BIT(2) | BIT(3));  // Bits 0 - 3

    uint8_t control_word = timer_selection | initialization_mode | last4bits;
  
  // Step 4: Send the command and then the LSB + MSB

    sys_outb(TIMER_CTRL, control_word);

    switch(timer){
      case 0:
        sys_outb(TIMER_0, LSB_input);
        sys_outb(TIMER_0, MSB_input);
        break;
      case 1:
        sys_outb(TIMER_1, LSB_input);
        sys_outb(TIMER_1, MSB_input);
        break;
      case 2:
        sys_outb(TIMER_2, LSB_input);
        sys_outb(TIMER_2, MSB_input);
        break;
    }

  return 0;
}

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

int (timer_get_conf)(uint8_t timer, uint8_t *st) {

  // Routine test: determine whether or not the pointer st is null. There is no need to test the timer variable, since it has been tested already.

  if(st == NULL){
    return -1;
  }
  else{
    
    // Defining a read-back command and sending it to the I/O device.

      uint8_t read_back_command = TIMER_RB_CMD | TIMER_RB_COUNT_ | TIMER_RB_SEL(timer);
      sys_outb(TIMER_CTRL, read_back_command);

    // Request the information from the I/O device. Only one timer is called.
      if(timer == 0){
        util_sys_inb(TIMER_0, st);
      }
      else if(timer == 1){
        util_sys_inb(TIMER_1, st);
      }
      else if(timer == 2){
        util_sys_inb(TIMER_2, st);
      }

      return 0;
  }

}

int (timer_display_conf)(uint8_t timer, uint8_t st, enum timer_status_field field) {
  
  // Create a union.
    union timer_status_field_val status_report;

  // The entire process is divided into four cases. Each one associated to the respective 'field' variable in the enumeration.

      switch(field){

        // First case: We are only interested in the display-status byte, therefore:
        case tsf_all:
          status_report.byte = st;
          break;

        // Second case: We are only interested in the initialization mode. The relevant bits are the bits 5 and 4.
        case tsf_initial:
          if( ((st >> 4) & INITIALIZATION ) == 0 ){
            // LSB and MSB haven't been read. Invalid outcome.
            status_report.in_mode = INVAL_val;
          }

          if( ((st >> 4) & INITIALIZATION ) == 1 ){
            // Bits 5 and 4 are listed as 01, respectively. This means only the LSB has been read.
            status_report.in_mode = LSB_only;
          }

          if( ((st >> 4) & INITIALIZATION ) == 2 ){
            // Bits 5 and 4 are listed as 10, respectively. This means only the MSB has been read.
            status_report.in_mode = MSB_only;
          }

          if( ((st >> 4) & INITIALIZATION ) == 3 ){
            // Bits 5 and 4 are listed as 11, respectively. This means both LSB and MSB have been read, in the asserted order.
            status_report.in_mode = MSB_after_LSB;
          }
        
          break;
        
        // Third case: We are only interested in the running mode. The relevant bits are the bits 3, 2 and 1.
        case tsf_mode:
          switch( (st & MODE_ANALYSIS) >> 1 ){
            case 0:
              status_report.count_mode=0;
              break;
            case 1:
              status_report.count_mode=1;
              break;
            case 2:
              status_report.count_mode=2;
              break;
            case 6:
              status_report.count_mode=2;
              break;
            case 3:
              status_report.count_mode=3;
              break;
            case 4:
              status_report.count_mode=4;
              break;
            case 5:
              status_report.count_mode=5;
              break;
          }
          break;

        // Fourth and final case: We are only interested in the counting base. The relevant bits are the bit 0.
        case tsf_base:
          if((st & BASE_ANALYSIS) == 0){
            status_report.bcd = false;
          }
          else{
            status_report.bcd = true;
          }
          break;

      }
        
      timer_print_config(timer, field, status_report);
      return 0;
}
