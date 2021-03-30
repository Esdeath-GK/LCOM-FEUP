// IMPORTANT: you must include the following line in all your C files
#include <lcom/lcf.h>
#include <lcom/liblm.h>
#include <lcom/proj.h>

#include <stdbool.h>
#include <stdint.h>

// Any header files included below this line should have been created by you

#include "videocard_essentials.h"
#include "kbd_essentials.h"
#include "mouse_essentials.h"
#include "proj_essentials.h"
#include "sources/images.h"
#include "xpm_management.h"
#include "xpm_variables.c"

int main(int argc, char *argv[]) {
  // sets the language of LCF messages (can be either EN-US or PT-PT)
  lcf_set_language("EN-US");

  // enables to log function invocations that are being "wrapped" by LCF
  // [comment this out if you don't want/need it]
  lcf_trace_calls("/home/lcom/labs/proj/trace.txt");

  // enables to save the output of printf function calls on a file
  // [comment this out if you don't want/need it]
  lcf_log_output("/home/lcom/labs/proj/output.txt");

  // handles control over to LCF
  // [LCF handles command line arguments and invokes the right function]
  if (lcf_start(argc, argv))
    return 1;

  // LCF clean up tasks
  // [must be the last statement before return]
  lcf_cleanup();

  return 0;
}

int(proj_main_loop)(int argc, char *argv[]){
    
    if(DEFAULT_SECONDS <= 2){
        printf("[FATAL ERROR] - 'DEFAULT_SECONDS' cannot be lower than or equal to 2.\n");
        return 0;
    }

    // Set up VRAM address
        void* address;
        address = vg_init(0x115);

        if(address == NULL){
            printf("Error setting up VRAM address ...\n");
            return -1;
        }
    // Set up XPM's
        if( game_setXPM(address) != 0){
            printf(" !ERRROR! - Failed to set up the game's XPM images ...\n\n");
            return -1;
        }
        else if( menu_setXPM(address) != 0){
            printf(" !ERRROR! - Failed to set up the menus's XPM images ...\n\n");
            return -1;
        }

    // Load Game
        project_core(address);
    
    // Terminate the program;
        game_freeXPM();
        menu_freeXPM();
        vg_exit();

    return 0;
}
