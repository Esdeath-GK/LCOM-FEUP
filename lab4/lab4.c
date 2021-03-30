// IMPORTANT: you must include the following line in all your C files
#include <lcom/lcf.h>

#include <stdint.h>
#include <stdio.h>
#include <minix/sysutil.h>

// Any header files included below this line should have been created by you
#include "mouse_essentials.h"
#include "timer.h"
#include "i8254.h"

/* *************************************** EXTERNAL VALUES *************************************** */
extern uint32_t counter;                // Timer counter        (timer.c - Line ###)
extern uint8_t extractB;                // Mouse packet byte    (mouse_essentials.c - Line ###)
extern struct packet mousepackage;      // Packet struct        (mouse_essentials.c - Line ###)
extern int complete;                    // Machine Status       (mouse_essentials.c - Line ###)
/* *********************************************************************************************** */

int main(int argc, char *argv[]) {
  // sets the language of LCF messages (can be either EN-US or PT-PT)
  lcf_set_language("EN-US");

  // enables to log function invocations that are being "wrapped" by LCF
  // [comment this out if you don't want/need/ it]
  lcf_trace_calls("/home/lcom/labs/lab4/trace.txt");

  // enables to save the output of printf function calls on a file
  // [comment this out if you don't want/need it]
  lcf_log_output("/home/lcom/labs/lab4/output.txt");

  // handles control over to LCF
  // [LCF handles command line arguments and invokes the right function]
  if (lcf_start(argc, argv))
    return 1;

  // LCF clean up tasks
  // [must be the last statement before return]
  lcf_cleanup();

  return 0;
}


int (mouse_test_packet)(uint32_t cnt) {

    uint32_t initial_packages = cnt;
    int ipc_status;
	message msg;
	int r;
    int mouse_irq_set = mouse_subscribe_intr();
    int i = 0;
    uint8_t vector[B_COUNT]={0};

    mouse_send_command(ENABLE_DATA);

    printf("\n");
    while (cnt > 0) {

		if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0) {
			printf("driver_receive failed with: %d", r);
			continue;
		}

		if (is_ipc_notify(ipc_status)) { /* received notification */
			switch (_ENDPOINT_P(msg.m_source)) {
			case HARDWARE: /* hardware interrupt notification */
				if (msg.m_notify.interrupts & mouse_irq_set) { /* subscribed interrupt */

                    // Interruption registered --> Call Mouse Interrupt Handler
                        mouse_ih();
                    
                    // Assign the extracted byte to the byte-vector;
					    vector[i] = extractB;

					if(i == 0){
                        // This is first byte and WE WANT IT to have the 3 BIT marked as '1'.
                        if( CHECK_BIT(extractB, 3) ){
                            i++;
                        }
                        else{
                            // The reading is not syncronyzed. Variable 'i' suffers a reset.
                            i = 0;
                        }
					} 
                    else{
						if (i == 2){

                                // Parse the package and print it
                                    Mpackage_parse(vector);

								cnt = cnt - 1;
								i = 0;
							}
                        else{
							i++;
                        }
                    }
				}
				break;
			default:
				break; /* no other notifications expected: do*/
			}
		}
	}
	printf("---> Number of packages successfully delivered (%d)!\n\n", initial_packages);


    mouse_send_command(DISABLE_DATA);


	if (mouse_unsubscribe_intr() == -1){
		printf("ERROR: kbd_subscribe_int failed.\n");
		return -1;
	}

    printf("\n:: (OK!) :: - PROGRAM SUCCESSFULLY TERMINATED!\n\n");

	return 0;
}

int (mouse_test_remote)(uint16_t period, uint8_t cnt) {

    // lcf_start() already sets up the mouse / KBC in accordance to what is necessary.

    int i = 0;
    int attempts = TRIES;
    uint8_t ExtractB;
    uint8_t vector[B_COUNT] = {0};

    // Calculate microseconds;
        int microseconds = period * MILI;
    

    while(cnt > 0){
        tickdelay(micros_to_ticks(microseconds));

        // Invoque 0xEB
            mouse_send_command(0xEB);

        while(attempts > 0){
            
            ExtractB = read_Mouse();

            // Assign the extracted byte to the byte-vector;
                vector[i] = ExtractB;        

            if(i == 0){
                // This is first byte and WE WANT IT to have the 3 BIT marked as '1'.
                if( CHECK_BIT(ExtractB, 3) ){
                    i++;
                }
                else{
                    // The reading is not syncronyzed. Variable 'i' suffers a reset.
                    i = 0;
                }
            } 
            else{
                if (i == 2){

                        // Parse the package and print it
                            Mpackage_parse(vector);

                        cnt = cnt - 1;
                        i = 0;
                        break;
                    }
                else{
                    i++;
                }
            }

            attempts--;
        }
        attempts = TRIES;  
    }

    // Restoring standard information

        // --> MOUSE
            mouse_send_command(SET_STREAM);
            mouse_send_command(DISABLE_DATA);

        // --> KBC
            uint8_t defaultbyte;
            defaultbyte = minix_get_dflt_kbc_cmd_byte();
            writeto_Mouse(0x60, 0x64);
            writeto_Mouse(defaultbyte, 0x60);

    // END OF RESTORATION

    return 0;
}



int (mouse_test_async)(uint8_t idle_time) {

    int r;
	uint8_t irq_timer0;
    int irq_mouse;
    int ipc_status;
    message msg;
    int remaining = idle_time;
    uint8_t vector[B_COUNT] = {0};
    int i = 0;

    // First step --> Activate TIMER 0 interruptions
        if( timer_subscribe_int(&irq_timer0) != 0){
            printf("Error setting up timer interruptions ... \n");
            return -1;
        }
    
    // Second step --> Activate MOUSE interruptions
        irq_mouse = mouse_subscribe_intr();
        if(irq_mouse == -1){
            printf("Error setting up mouse interruptions ...\n");
            return -1;          
        }
    
    // Third step --> Set up the mouse
        mouse_send_command(ENABLE_DATA);
    
    while(counter < idle_time * TICKS_PER_SECOND){

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
                    if(counter % TICKS_PER_SECOND == 0 && counter < idle_time * TICKS_PER_SECOND){
                        remaining--;
                        printf("%d seconds remaining;\n", remaining);
                    }
                }

				if (msg.m_notify.interrupts & irq_mouse){ /* subscribed interrupt */

                    counter = 0;
                    remaining = idle_time;

                    // Interruption registered --> Call Mouse Interrupt Handler
                        mouse_ih();
                    
                    // Assign the extracted byte to the byte-vector;
					    vector[i] = extractB;

					if(i == 0){
                        // This is first byte and WE WANT IT to have the 3 BIT marked as '1'.
                        if( CHECK_BIT(extractB, 3) ){
                            i++;
                        }
                        else{
                            // The reading is not syncronyzed. Variable 'i' suffers a reset.
                            i = 0;
                        }
					} 
                    else{
						if (i == 2){

                                // Parse the package and print it
                                    Mpackage_parse(vector);

								i = 0;
							}
                        else{
							i++;
                        }
                    }
                }
				break;
			default:
				break; /* no other notifications expected: do nothing */
			}
		}
	}
    printf("Terminated!\n");

    if (timer_unsubscribe_int() != 0) {
		printf("Error unsubscribing timer ...\n");
		return -1;
	} 

    mouse_send_command(DISABLE_DATA);

	if (mouse_unsubscribe_intr() == -1){
		printf("ERROR: kbd_subscribe_int failed.\n");
		return -1;
	}

    return 0;
}


int (mouse_test_gesture)(uint8_t x_len, uint8_t tolerance){
   
    int ipc_status;
	message msg;
	int r;
    int mouse_irq_set = mouse_subscribe_intr();
    int i = 0;
    uint8_t vector[B_COUNT]={0};
    int backup = 0; /* Reaches 10 by sending 10 packages in a row with MMB pressed down. */

    mouse_send_command(ENABLE_DATA);

    while (complete != 1) {

		if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0) {
			printf("driver_receive failed with: %d", r);
			continue;
		}

		if (is_ipc_notify(ipc_status)) { /* received notification */
			switch (_ENDPOINT_P(msg.m_source)) {
			case HARDWARE: /* hardware interrupt notification */
				if (msg.m_notify.interrupts & mouse_irq_set) { /* subscribed interrupt */

                    // Interruption registered --> Call Mouse Interrupt Handler
                        mouse_ih();
                    
                    // Assign the extracted byte to the byte-vector;
					    vector[i] = extractB;

					if(i == 0){
                        // This is first byte and WE WANT IT to have the 3 BIT marked as '1'.
                        if( CHECK_BIT(extractB, 3) ){
                            i++;
                        }
                        else{
                            // The reading is not syncronyzed. Variable 'i' suffers a reset.
                            i = 0;
                        }
					} 
                    else{
						if (i == 2){

                                // Parse the package and print it
                                    Mpackage_parse(vector);
                                
                                // Gesture manage;
                                    check_event(tolerance, x_len);

                                // BACKUP PROGRAM
                                if(mousepackage.mb == true){
                                    backup++;
                                }
                                else{
                                    backup = 0;
                                }

								i = 0;
							}
                        else{
							i++;
                        }
                    }
				}
				break;
			default:
				break; /* no other notifications expected: do*/
			}
		}

        if(backup == BACKUP_LIMIT){
            break;
        }
    }

    if(backup == BACKUP_LIMIT){
        printf("\n---> Cycle terminated from backup call.\n");
    }
    else{
        printf("\n---> Cycle terminated from AND symbol.\n");
    }

    mouse_send_command(DISABLE_DATA);


	if (mouse_unsubscribe_intr() == -1){
		printf("ERROR: kbd_subscribe_int failed.\n");
		return -1;
	}

    printf("\n:: (OK!) :: - PROGRAM SUCCESSFULLY TERMINATED!\n\n");

	return 0;
}
