#include <lcom/lcf.h>
#include <machine/int86.h>
#include <lcom/xpm.h>

#include <stdbool.h>
#include <stdint.h>
#include "videocard_essentials.h"
#include "proj_essentials.h"
#include "kbd_essentials.h"
#include "mouse_essentials.h"
#include "timer.h"
#include "sources/images.h"
#include "graphic_management.h"
#include "xpm_variables.c"
#include "xpm_management.h"
#include "cursor.h"
#include "game.h"
#include "rtc.h"

/* *************************************************************************************************************************  */
/* ************************************************ GAME / MENU VARIABLES **************************************************  */
/* *************************************************************************************************************************  */

// Important NOTE:      In the matrix, the values of the slots follow the legend bellow:
//                      Blue Side = 1 || Red Side = 2 || Empty = 0;


int cur_xy[2]={0,61};                                       /* Current cursor coordinates {x,y} */
int victorious_coordinatesXY[ROWS][COLUMNS]={ {0} };        /* Coordinates marking the winning chips. */
uint8_t* secondBuffer;                                      /* Pointer used to store space for double buffering. */
uint8_t* Game_background;                                   /* BACKUP of the game's background (table). */
uint8_t* Chips_background;                                  /* BACKUP of the chips of the game (and locations). */
int remainingSeconds = DEFAULT_SECONDS;                     /* Remaining seconds on the timer at the lower end of the screen. */
game_state stateOfGame;                                     /* State machine. Dictates the state of the game. */
program_state stateOfProgram;                               /* State machine. Dictatores states of the program. */
uint8_t game_table[ROWS][COLUMNS] = { {0} };                /* Matrix indicating which spaces are used by the chips or not. */
int currentPlayer = 1;                                      /* Indicates the player of the current turn. 1 for blue, 2 for red. */
int changeTurn = 1;                                         /* Used to indicate a change of turn has been registered */




/* *************************************************************************************************************************  */
/* ************************************************ ESSENTIAL FUNCTIONS **********************************<+****************  */
/* *************************************************************************************************************************  */



int floor(float value){
    int integer = (int)value;
    return value < integer ? integer - 1 : integer;
}

uint32_t ticks_fr(float frame_rate){
    float seconds_frame = (float)1 / (float)frame_rate;
    int microSeconds_frame = ceil((float)seconds_frame * SECOND_MIC);
    return micros_to_ticks(microSeconds_frame);
}
void changePlayer(){
    currentPlayer = currentPlayer == 1 ? 2 : 1;
    return;
}
void startPlayer(){
    currentPlayer = 1;
    return;
}
int getCurrentPlayer(){
    return currentPlayer;
}
void setChangeTurn(int valor){
    changeTurn = valor;
    return;
}
int getChangeTurn(){
    return changeTurn;
}
void setRemainingSeconds(int valor){
    remainingSeconds = valor;
    return;
}
int getRemainingSeconds(){
    return remainingSeconds;
}

void project_core(void *address){
        int total_size = getTotalSize();
        int colNumber;
        int end_printed = 0;
    // Used to manage the option in which the menu currently is.
        int options_menu = 0;      
    // Set up interruption values, along with interruption IDs.
        int r;
        int irq_mouse;
        int irq_kbd;
        uint8_t irq_timer;
        uint32_t irq_rtc = BIT(21); 
        int ipc_status;
        message msg;
        uint8_t vector[B_COUNT] = {0};
        int i = 0;
        uint8_t key_scanned = 0;

    // SET-UP all auxiliary allocation values through memory allocation. They will be freed as soon as the program terminates.
        secondBuffer = (uint8_t*)calloc(total_size, sizeof(uint8_t));
        Game_background = (uint8_t*)calloc(total_size, sizeof(uint8_t));
        Chips_background = (uint8_t*)calloc(total_size, sizeof(uint8_t));
        if(secondBuffer == NULL || Game_background == NULL || Chips_background == NULL){
            game_freeXPM();
            menu_freeXPM();
            printf("Error setting up buffers ... \n");
            return;
        }

    // Set up the auxiliary game_background backup.
        draw_XPM(XPM_tabuleiro, 0, 0, Game_background, tabuleiro_img, 0);
    
    // Set up the state machines.
        stateOfProgram = CURRENTLY_IN_MENU;
        stateOfGame = BLUE_PLAYING;

    // Subscribe interruptions to all devices. These include: Mouse, Keyboard, RTC and Timer.
        irq_mouse = mouse_subscribe_intr();
        irq_kbd = keyboard_subscribe_intr();
        irq_rtc = rtc_subscribe_int();
        timer_subscribe_int(&irq_timer);

    // Enable mouse data reporting and setup the RTC.
        mouse_send_command(ENABLE_DATA);
        setup_RTC();

    // Determine the necessary ticks to fulfill the defined FRAME_RATE and define turnNearEnd's value.
        uint32_t ticks_frame = ticks_fr( (float)FRAME_RATE );
        uint8_t turnNearEnd = ceil(DEFAULT_SECONDS / (float)2);
        if(turnNearEnd == 0)
            turnNearEnd += 2;


    // Initiate the interruption cycle. Like in the labs, the cycle terminates as soon as the ESC key is released.
        while(key_scanned != ESC_BREAK){
            if(stateOfGame != BLUE_WON && stateOfGame != RED_WON){
                end_printed = 0;
            }
            
            if( (stateOfGame == BLUE_WON || stateOfGame == RED_WON) && end_printed == 0){
                // If one of the players has already won and the ending screen has not yet been printed, then it prints it. Afterwards, it locks all interrupts except the mouse.
                end_screen(address);
                end_printed = 1;
            }

            if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0) {
                    printf("driver_receive failed with: %d", r);
                    continue;
            }

            if (is_ipc_notify(ipc_status)) { /* received notification */
                switch (_ENDPOINT_P(msg.m_source)) {
                    case HARDWARE: /* hardware interrupt notification */

                        // Mouse Interruption
                            if (msg.m_notify.interrupts & irq_mouse){ /* subscribed interrupt */
                                // Interruption registered --> Call Mouse Interrupt Handler
                                    uint8_t mousemove;
                                    mouse_ih_proj(&mousemove);
                                
                                // Assign the extracted byte to the byte-vector;
                                    vector[i] = mousemove;

                                // The following conditionals are based on Lab 4's analysis of packages.
                                    if(i == 0){
                                        // This is first byte and WE WANT IT to have the 3 BIT marked as '1'.
                                            if( CHECK_BIT(mousemove, 3) ){
                                                i++;
                                            }
                                            else{
                                                // The reading is not syncronyzed. Variable 'i' suffers a reset.
                                                i = 0;
                                            }
                                    } 
                                    else{
                                        if (i == 2){
                                                // Parse the package
                                                Mpackage_parse(vector);
                                                if(stateOfProgram == CURRENTLY_IN_GAME && stateOfGame != REVEALING_PATH){
                                                    // If the program is not revealing the winning path and is still in game, then a mouse_action is registered.
                                                    if(stateOfGame != PUTTING_CHIP){
                                                        // Like before, if the game is not processing the falling chip, then the game reacts to the mouse movement.
                                                        colNumber = game_react(address, 1); 
                                                    }
                                                }
                                                i = 0;
                                        }
                                        else{
                                            i++;
                                        }
                                    }
                            }

                        // Keyboard interruption.
                            if (msg.m_notify.interrupts & irq_kbd) { /* subscribed interrupt */
                                
                                // Keyboard's interrupt handler and extraction of the scanned_byte
                                    kbc_ih_proj(&key_scanned);
                                    keyboard_key_scan(key_scanned);
                                
                                if(stateOfProgram == CURRENTLY_IN_GAME && stateOfGame != REVEALING_PATH){
                                    // If the key is not ESC_BREAK (otherwise the game finishes immediately) and there is no second byte to read, then the system registers a reaction.
                                    if(key_scanned != ESC_BREAK && getSecondbyte() == 0){
                                        colNumber = game_react(address, 2);   
                                    }
                                }
                                else if(stateOfProgram == CURRENTLY_IN_MENU){
                                    // Menu functions:
                                    if(key_scanned != ESC_BREAK && getSecondbyte() == 0 && getbyteslist(0) == TWOBYTE_SCANCODE){       
                                        if(getbyteslist(1) == ARROW_UP){
                                            options_menu --;  
                                            if(options_menu == -1){
                                                options_menu = 1;
                                            }
                                        }
                                        else if(getbyteslist(1) == ARROW_DOWN){
                                            options_menu ++;
                                            if(options_menu == 2){
                                                options_menu = 0;
                                            }
                                        }
                                    }

                                    draw_menuXPM(options_menu, address);

                                    if(key_scanned == ENTER_SCANCODE){
                                        if(options_menu == 0){
                                            // Starts the game
                                            stateOfProgram = CURRENTLY_IN_GAME;
                                            RESTART(1, address);
                                        }
                                        else if(options_menu == 1){
                                            // Sets up the scanned key as an ESC_BREAK to immediately cut off the interruption loop.
                                            key_scanned = ESC_BREAK;
                                        }
                                    }
                                }
                            }

                        // Timer Interruptions.
                            if (msg.m_notify.interrupts & irq_timer) {
                                timer_int_handler();
                                if(stateOfProgram == CURRENTLY_IN_GAME && stateOfGame != REVEALING_PATH){
                                    if(getCounter() % 60 == 0 && stateOfGame != RED_WON && stateOfGame != BLUE_WON){
                                        if(changeTurn == 1){
                                            changeTurn = 0;
                                        }
                                        remainingSeconds--;
                                        if(remainingSeconds <= turnNearEnd){
                                            if(getMarkedDraw() == 0){
                                                addREDwarning(1);
                                            }
                                        }
                                        remaining_time(remainingSeconds, address);
                                        reset_counter();
                                    }
                                    if(stateOfGame == PUTTING_CHIP && (getCounter() >= ticks_frame) && getMarkedDraw() == 0 ){
                                        int error_column_filled = process_fallingChip(colNumber, address);
                                        if(error_column_filled == 0){
                                            reset_counter();
                                        }
                                    }
                                    else if(stateOfGame == PUTTING_CHIP && (getCounter() % 60 == 0) && getMarkedDraw() > 0){
                                        process_fallingChip(colNumber, address);
                                    }
                                }
                                else if(stateOfGame == REVEALING_PATH){
                                    if(getCounter() % TICKS_VICTORIOUS == 0){
                                        victory_process(address);
                                        reset_counter();
                                    }
                                }
                                else{
                                    reset_counter();
                                }
                            }

                        // RTC Interruptions.
                            if (msg.m_notify.interrupts & irq_rtc) {  
                                update_date();
                                if(stateOfProgram == CURRENTLY_IN_MENU){
                                    draw_menuXPM(options_menu, address);
                                }
                            }
                        break;
                    default:
                        break; /* no other notifications expected: do nothing */
                }
            }
        }

    // Unsubscribe all subscribed interruptions (additionally disable mouse data reporting).
        mouse_send_command(DISABLE_DATA);
        mouse_unsubscribe_intr();
        keyboard_unsubscribe_intr();
        rtc_unsubscribe_int();
        timer_unsubscribe_int();

    // Free all auxiliary allocations
        free(secondBuffer);
        free(Game_background);
        free(Chips_background);
}

void victory_process(void *address){
    int total_size = getTotalSize();
    static int represented = 0;
    static int winner = 0;
    static int old_line = 0;
    static int old_column = 0;

    if(represented == 0){
        memset(secondBuffer, 0, total_size);
        draw_allChips();
        draw_XPM(XPM_tabuleiro, 0, 0, secondBuffer, tabuleiro_img, 0);
        if(currentPlayer == 1)
            draw_XPM(XPM_cabecalho_azul, 0, 0, secondBuffer, cabecalho_azul_img, 0);
        else
            draw_XPM(XPM_cabecalho_vermelho, 0, 0, secondBuffer, cabecalho_vermelho_img, 0);
        memcpy(address, secondBuffer, total_size);
    }
    if(represented < 4){
        while(old_line < ROWS){
            while(old_column < COLUMNS){
                if( victorious_coordinatesXY[old_line][old_column] != 0 ){
                    if( represented == 0 )
                        winner = victorious_coordinatesXY[old_line][old_column];
                    
                    int currentXCoordinate = DISTANCE_LEFT + old_column * CHIP_DIAMETER + old_column * DISTANCE_HOLESX;
                    int currentYCoordinate = HEADER_SIZE + DISTANCE_TOP + old_line * DISTANCE_HOLESY + old_line * CHIP_DIAMETER;

                    draw_XPM(XPM_endingRing, currentXCoordinate, currentYCoordinate, secondBuffer, endingRing_img, 0);
                    memcpy(address, secondBuffer, total_size);
                    represented++;
                        if( (old_column + 1) == COLUMNS){
                            old_column = 0;
                            old_line++;
                        }
                        else{
                            old_column++;
                        }

                    return;
                }

                if( (old_column + 1) == COLUMNS){
                    old_column = 0;
                    old_line++;
                }
                else{
                    old_column++;
                }
            }
            old_column = 0;
            old_line++;
        }
    }
    else{
        if(winner == 1){
            stateOfGame = BLUE_WON;
        }
        else{
            stateOfGame = RED_WON;
        }

        represented = 0;
        old_column = 0;
        old_line = 0;
        winner = 0;

        // Clean victorious_coordinatesXY vector in case the game gets restarted.
            for(int i = 0; i < ROWS; i++){
                for(int j = 0; j < COLUMNS; j++){
                    victorious_coordinatesXY[i][j] = 0;
                }
            }
    }

    return;
}

