#include <lcom/lcf.h>
#include <lcom/lab3.h>
#include "i8254.h"

#include <stdbool.h>
#include <stdint.h>
#include "kbd_essentials.h"
#include "timer.h"


int kbd_hookid = ZERO_NULL;
int universal_count = 0;
uint8_t universal_status;
uint8_t universal_key_scanned;

int keyboard_subscribe_intr(){

    if( sys_irqsetpolicy(KBD_IRQ, IRQ_REENABLE | IRQ_EXCLUSIVE, &kbd_hookid) != OK ){
        printf("There was an error setting up sys_irqsetpolicy ...");
        return -1;
    }

    return BIT(0);

}

int keyboard_unsubscribe_intr(){

    if (sys_irqdisable(&kbd_hookid) != OK) {
		printf("There was an error running 'sys_irqdisable' ...\n");
		return -1;
	}

	if (sys_irqrmpolicy(&kbd_hookid) != OK) {
		printf("There was an error running 'sys_irqrmpolicy' ...\n");
		return -1;
	}

	return 0;
}

int keyboard_key_scan(uint8_t key_scanned){

	static int secondbyte;
	uint8_t bytes;
  	uint8_t bytes_list[2] = {0,0};
	bool is_make;

	// First step: Conclude whether or not it is 2 bytes long

	if( (key_scanned == TWOBYTE_SCANCODE) && secondbyte == 0 ){
		secondbyte = 1;
	}
	else{
		if(secondbyte == 1){
			secondbyte = 0;
			bytes_list[1] = key_scanned;
			bytes_list[0] = TWOBYTE_SCANCODE;
			bytes = 2;
		}
		else{
			bytes_list[0] = key_scanned;
			bytes = 1;
		}

		// Second step: Determine if it is a make code of not
		if( (key_scanned & BREAK_MAKE) == BREAK_MAKE ){
			is_make = FALSE;
		}
		else{
			is_make = TRUE;
		}

		// Print the value:
		//for(int i = 0 ; i < bytes; i++)
			//printf("0x%x ", bytes_list[i]);
		//printf("\n");

		kbd_print_scancode(is_make, bytes, bytes_list);
	}

	return 0;
}

void (kbc_ih)() {

	while (1) {
		util_sys_inb(STAT_REG, &universal_status);

		if (universal_status & OUT_FULL) {
			util_sys_inb(OUT_BUF, &universal_key_scanned);
			// printf("Read %d\n", data_one);
			// Check for errors
			if ((universal_status & (ERROR_CHECK)) == 0){
				return;
			}
			else{
				return;
			}
		}

		// Infinite loop until the OUTPUT is no longer empty.
		linger();
	}

}

int keyboard_reenable_int(){
	uint8_t kb_status;
	uint8_t irqenable_command;

	// First step: Read the status register and loop if INPUT Buffer is full
		while(1){
			util_sys_inb(STAT_REG, &kb_status);
			kb_status = kb_status & IBF_BIT;
			if( (kb_status >> 1) == 1 ){
				// Is full
				linger();
			}
			else{
				// Is empty
				break;
			}
		}

	// Second step: Send the command and re-activate interruptions.
		sys_outb(0x64, 0x20);
		util_sys_inb(0x60, &irqenable_command);
		sys_outb(0x64, 0x60);
		irqenable_command = irqenable_command | BIT(0);
		sys_outb(0x60, irqenable_command);
	
	return 0;
}

void linger(){
	tickdelay(micros_to_ticks(LINGER_TIME));
}

int keyboard_detect_break() {

  int irq_set = keyboard_subscribe_intr();

  if(irq_set == -1){
    // Failed to subscribe keyboard interruptions.
    return -1;
  }

  int r;
  int ipc_status;
  message msg;
  uint8_t key_scanned = 0;

  while (key_scanned != ESC_BREAK){

		if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0) {
			printf("driver_receive failed with: %d", r);
			continue;
		}

		if (is_ipc_notify(ipc_status)) { /* received notification */
			switch (_ENDPOINT_P(msg.m_source)) {
			case HARDWARE: /* hardware interrupt notification */
				if (msg.m_notify.interrupts & irq_set) { /* subscribed interrupt */

					kbc_ih();

              		key_scanned = universal_key_scanned;

          		}
				break;
			default:
				break; /* no other notifications expected: do*/
			}
		}
  }
        
  keyboard_unsubscribe_intr();

  return 0;
}
