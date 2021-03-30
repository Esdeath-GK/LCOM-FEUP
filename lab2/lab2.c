#include <lcom/lcf.h>
#include <lcom/lab2.h>

#include <stdbool.h>
#include <stdint.h>

// Invoke counter value from timer.c
extern uint32_t counter;

int main(int argc, char *argv[]) {
  // sets the language of LCF messages (can be either EN-US or PT-PT)
  lcf_set_language("EN-US");

  // enables to log function invocations that are being "wrapped" by LCF
  // [comment this out if you don't want/need it]
  lcf_trace_calls("/home/lcom/labs/lab2/trace.txt");

  // enables to save the output of printf function calls on a file
  // [comment this out if you don't want/need it]
  lcf_log_output("/home/lcom/labs/lab2/output.txt");

  // handles control over to LCF
  // [LCF handles command line arguments and invokes the right function]
  if (lcf_start(argc, argv))
    return 1;

  // LCF clean up tasks
  // [must be the last statement before return]
  lcf_cleanup();

  return 0;
}

int(timer_test_read_config)(uint8_t timer, enum timer_status_field field) {

  if(timer < 0 || timer > 2){
    printf("Invalid timer!\n");
    return -1;
  }
  else{
    uint8_t configuration;

    // Extract timer configuration:
      timer_get_conf(timer, &configuration);
    
    // Display configuration: 
      timer_display_conf(timer, configuration, field);
  }

  return 0;
}

int(timer_test_time_base)(uint8_t timer, uint32_t freq) {
  
  if(!timer_set_frequency( timer, freq)){
    printf("Returned 0\n");
    return 0;
  }
  else{
    return 1;
  }

}

int(timer_test_int)(uint8_t time){

  // Check if the time value is admissible. If positive, suspend the function.
  if(time < 0){
    printf("Invalid time input ...\n");
    return -1;
  }

  int ipc_status;
  message msg;
  int r;
  uint8_t irq_set;
  timer_subscribe_int (&irq_set);
  while( (counter / 60) < time ) { /* Restricted to 'time' iterations */
       /* Get a request message. */
      if ( (r = driver_receive(ANY, &msg, &ipc_status)) != 0 ) { 
          printf("driver_receive failed with: %d", r);
          continue;
      }
      if (is_ipc_notify(ipc_status)) { /* received notification */
          switch (_ENDPOINT_P(msg.m_source)) {
              case HARDWARE: /* hardware interrupt notification */				
                  if (msg.m_notify.interrupts & irq_set) { /* subscribed interrupt */
                    timer_int_handler();
                    if(counter % 60 == 0){
                      timer_print_elapsed_time();
                    }
                  }
                  break;
              default:
                  break; /* no other notifications expected: do nothing */	
          }
      } else { /* received a standard message, not a notification */
          /* no standard messages expected: do nothing */
      }
 }

  // Unsubscribe timer 0 interrupts:
  int outcome;

  outcome = timer_unsubscribe_int();
  
  if(outcome != 0){
    return 1;
  }

  return 0;
}
