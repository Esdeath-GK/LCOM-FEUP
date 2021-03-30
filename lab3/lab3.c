#include <lcom/lcf.h>
#include <lcom/lab2.h>
#include <lcom/lab3.h>

#include <stdbool.h>
#include <stdint.h>
#include "kbd_essentials.h"
#include "timer.h"
#include "i8254.h"


/* ******************************* EXTERNAL VALUES ******************************* */
extern uint32_t counter;                 // Timer counter    (timer.c)
extern uint32_t sys_calls;               // sys_inb calls    (utils.c)
extern uint8_t universal_status;         // kbd_status;      (kbd_essentials.c)
extern uint8_t universal_key_scanned;    // key_scanned;     (kbd_essentials.c)
/* ******************************************************************************* */



int main(int argc, char *argv[]) {
  // sets the language of LCF messages (can be either EN-US or PT-PT)
  lcf_set_language("EN-US");

  // enables to log function invocations that are being "wrapped" by LCF
  // [comment this out if you don't want/need it]
  lcf_trace_calls("/home/lcom/labs/lab3/trace.txt");

  // enables to save the output of printf function calls on a file
  // [comment this out if you don't want/need it]
  lcf_log_output("/home/lcom/labs/lab3/output.txt");

  // handles control over to LCF
  // [LCF handles command line arguments and invokes the right function]
  if (lcf_start(argc, argv))
    return 1;

  // LCF clean up tasks
  // [must be the last statement before return]
  lcf_cleanup();

  return 0;
}

int(kbd_test_scan)() {

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

              keyboard_key_scan(key_scanned);
				  
          }
				break;
			default:
				break; /* no other notifications expected: do*/
			}
		}
  }
        
  keyboard_unsubscribe_intr();

  printf("Number of sys_inb() calls: %d\n", sys_calls);

  return 1;
}

int(kbd_test_poll)(){

  uint8_t status;
  uint8_t data_one;
  uint8_t key_scanned = 0;


  while( key_scanned != ESC_BREAK ){
    
    util_sys_inb(STAT_REG, &status);

    if (status & OUT_FULL) {
        if( (status & (AUX_ERROR_CHECK)) == 0){
          if(status & OUT_FULL){
            util_sys_inb(OUT_BUF, &data_one);
            // printf("Read %d\n", data_one);
            // Check for errors
            if ((status & (ERROR_CHECK)) == 0){

              key_scanned = data_one;
              keyboard_key_scan(key_scanned);

            }
            else{
              return -1;
            }
          }
        }
        else{
          printf("Error detected\n");
          return -1;
        }
		}
  }

  // After breaking the endless cycle, the objective is to reenable the interrupts.
    keyboard_reenable_int();

  printf("Number of sys_inb() calls: %d\n", sys_calls);

  return 0;
}

int(kbd_test_timed_scan)(uint8_t idle){

	int r;
	uint8_t irq_timer0 = TIMER0_IRQ;
  uint8_t irq_kbd = BIT(0);
  uint8_t key_scanned = 0;
  int ipc_status;
  message msg;
  int remaining = idle;


  if( timer_subscribe_int(&irq_timer0) != 0){
    printf("Error setting up timer interruptions ... \n");
    return -1;
  }
  if( keyboard_subscribe_intr() != BIT(0)){
    printf("Error setting up keyboard interruptions ... \n");
    return -1;
  }

  while (key_scanned != ESC_BREAK && counter < idle * 60) {

		if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0) {
			printf("driver_receive failed with: %d", r);
			continue;
		}

		if (is_ipc_notify(ipc_status)) { /* received notification */
			switch (_ENDPOINT_P(msg.m_source)) {
			case HARDWARE: /* hardware interrupt notification */
        //printf("%x & %d = %d \n", msg.m_notify.interrupts, irq_timer0, msg.m_notify.interrupts & irq_timer0);
				if (msg.m_notify.interrupts & irq_timer0){ /* subscribed interrupt */
					timer_int_handler();
          if(counter % 60 == 0 && counter < idle * 60){
            remaining--;
            printf("%d seconds remaining;\n", remaining);
          }
        }

				if (msg.m_notify.interrupts & irq_kbd){ /* subscribed interrupt */

          counter = 0;
          remaining = idle;

          kbc_ih();
          //keyboard_output_read(&key_obtained);

          key_scanned = universal_key_scanned;

          keyboard_key_scan(key_scanned);
        }
				break;
			default:
				break; /* no other notifications expected: do nothing */
			}
		}
	}

  if(counter == idle * 60){
    printf(":: PROGRAM TERMINATED :: - Time expired!\n\n");
  }
  else{
    printf(":: PROGRAM TERMINATED :: - ESC key released.\n\n");
  }

  if (keyboard_unsubscribe_intr() != 0) {
		printf("Error unsubscribing keyboard ...\n");
		return -1;
	} 
  
	if (timer_unsubscribe_int() != 0) {
		printf("Error unsubscribing timer ...\n");
		return -1;
	}  

  return 1;
}
