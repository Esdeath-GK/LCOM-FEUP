#include <lcom/lcf.h>
#include <machine/int86.h>

// Standard Libraries:
#include "game.h"
#include "proj_essentials.h"
#include "videocard_essentials.h"
#include "kbd_essentials.h"
#include "mouse_essentials.h"
#include "cursor.h"
#include "timer.h"
#include "graphic_management.h"

// Images and additional '.h/.c' inclusions:
#include "xpm_variables.c"


/* *************************************************************************************************************************  */
/* *************************************************** EXTERN VARIABLES ****************************************************  */
/* *************************************************************************************************************************  */

extern int cur_xy[2];                                               /* Current cursor coordinates {x,y} */
extern int victorious_coordinatesXY[ROWS][COLUMNS];                 /* Coordinates marking the winning chips. */
extern uint8_t* secondBuffer;                                       /* Pointer used to store space for double buffering. */
extern uint8_t* Chips_background;                                   /* BACKUP of the chips of the game (and locations). */
extern game_state stateOfGame;                                      /* State machine. Dictates the state of the game. */
extern program_state stateOfProgram;                                /* State machine. Dictatores states of the program. */
extern uint8_t game_table[ROWS][COLUMNS];                           /* Matrix indicating which spaces are used by the chips or not. */



/* *************************************************************************************************************************  */
/* ************************************************ GAME / MENU VARIABLES **************************************************  */
/* *************************************************************************************************************************  */


int markedDraw = 0; //Used to manage the time the game displays the 'DRAW' outcome.

/* ****** AUXILIARY FUNCTION *********** */
int getMarkedDraw(){
    return markedDraw;
}

/* ************************* ESSENTIAL FUNCTIONS *************************  */
int game_react(void *address, int flag){
    // If flag = 1 indicate mouse action
    // If flag = 2 indicate kbd action

    struct packet mousepath = getMousePackage();

    static int cur_location = -1;
    int total_size = getTotalSize();

    // Indicate which column mouse is hover
    static int colNumber = 0;
    // Used to determine if the cursor was previously on top of the timer.
    int prev_onTimer;
    // Determines whether or not the cursor is on top of the restart button
    static int onTop = 0;                             
    
    switch(flag){
        case 1:
            // Step 1: Move cursor
                if(mousepath.delta_x != 0 || mousepath.delta_y != 0){
                    cur_location = (cur_location != -1) ? -1 : -1;
                    // First case: The game is in progress ...
                        if(stateOfGame != BLUE_WON && stateOfGame != RED_WON){
                            deleteCursor(3);

                            // Check if it is on top of the "RESTART" button OR TIMER display.
                            if( isONRestart(0) ){
                                if( isONRestart(1) ){
                                    onTop = 1;
                                    place_restart(0,0);
                                }
                                else{
                                    place_restart(1,0);
                                    onTop = 0;
                                }
                            }
                            else if(onTop == 1){
                                onTop = 0;
                                place_restart(1,0);
                            }
                            prev_onTimer = isONTimer();
                            updateCursorPos();

                            // Block access to top box:
                                cur_xy[0] = (cur_xy[0] < 0) ? 0 : cur_xy[0];
                                cur_xy[1] = (cur_xy[1] <= 60) ? 61 : cur_xy[1];
                            
                                if(getChangeTurn() == 0){
                                    if( isONTimer() || prev_onTimer ){
                                        void clean_time_area();
                                        remaining_time(getRemainingSeconds(), address);
                                    }
                                }

                                if( isInZone(&colNumber) ){
                                    draw_XPM(XPM_down_arrow, cur_xy[0], cur_xy[1], secondBuffer, down_arrow_img, 0);
                                }
                                else{
                                    draw_XPM(XPM_cursor, cur_xy[0], cur_xy[1], secondBuffer, cursor_img, 0);
                                }

                            memcpy(address, secondBuffer, total_size);
                        }
                        // Second case: The game has finished. Cursor is now locked inside the box.
                        else{
                            if(stateOfGame == BLUE_WON)
                                deleteCursor(4);  
                            else
                                deleteCursor(5);

                            // Close button
                                place_closeButton(stateOfGame);
                                
                            // Restart button
                                if( onTopRestart_EG(0) ){
                                    if( onTopRestart_EG(1) ){
                                        onTop = 1;
                                        place_restart(0,1);
                                    }
                                    else{
                                        place_restart(1,1);
                                        onTop = 0;
                                    }
                                }
                                else if(onTop == 1){
                                    onTop = 0;
                                    place_restart(1,1);
                                }

                            updateCursorPos();

                            cur_xy[0] = (cur_xy[0] < 0) ? 0 : cur_xy[0];
                            cur_xy[1] = (cur_xy[1] < 0) ? 0 : cur_xy[1];

                            draw_XPM(XPM_cursor, cur_xy[0], cur_xy[1], secondBuffer, cursor_img, 0);
                            memcpy(address, secondBuffer, total_size);
                        }
                }
            
            // Step 2: Determine if a button was pressed on top of a column.
                if(mousepath.lb == 1 && isInZone(&colNumber) && stateOfGame != BLUE_WON && stateOfGame != RED_WON ){
                    stateOfGame = PUTTING_CHIP;
                    reset_counter();
                    return colNumber;
                }
                else if( mousepath.lb == 1 && isONRestart(1) ){
                    RESTART(1, address);
                }
                else if( mousepath.lb == 1 && onTopRestart_EG(1) ){
                    RESTART(1, address);
                }
                else if( mousepath.lb == 1 && (stateOfGame == BLUE_WON || stateOfGame == RED_WON) && onTopClose_EG() ){
                    printf("The function has reached this parameter :: CODE 501\n");
                    RESTART(0, address);
                    stateOfProgram = CURRENTLY_IN_MENU;
                    draw_menuXPM(0, address);
                }
            break;
        
        case 2:
            // Check if we have a winner, case yes dont care about kbd
            if(stateOfGame != PUTTING_CHIP){
                // Check if it is the Restart key (R), arrow keys or A / D keys.
                if( getbyteslist(0) == R_SCANCODE && getbyteslist(1) == 0 ){
                    // Reload command sent:
                        RESTART(1, address);
                        break;
                }
                else if( getbyteslist(0) == Q_SCANCODE && getbyteslist(1) == 0 ){
                    // Quit command (q) -> Goes back to menu
                        RESTART(0, address);
                        stateOfProgram = CURRENTLY_IN_MENU;
                        break;
                }
                else if( (getbyteslist(0) == ENTER_SCANCODE || getbyteslist(0) == SPACE_SCANDCODE ) && getbyteslist(1) == 0 && stateOfGame != BLUE_WON && stateOfGame != RED_WON){
                    if(cur_location >= 0 && cur_location <= 6){
                        colNumber = cur_location;
                        stateOfGame = PUTTING_CHIP;
                        return colNumber;
                    }
                }
                else if( ((getbyteslist(1) == 0 && (getbyteslist(0) == A_SCANCODE || getbyteslist(0) == D_SCANCODE))
                            || (getbyteslist(0) == TWOBYTE_SCANCODE && (getbyteslist(1) == RIGHT_ARROW_SC || getbyteslist(1) == LEFT_ARROW_SC)) ) && stateOfGame != BLUE_WON && stateOfGame != RED_WON){
                        if( getbyteslist(0) == A_SCANCODE && cur_location == -1){
                            cur_location = -1;
                        }
                        cur_location = (getbyteslist(0) == A_SCANCODE || (getbyteslist(0) == TWOBYTE_SCANCODE && getbyteslist(1) == LEFT_ARROW_SC) ) ? cur_location-1 : cur_location;
                        cur_location = (getbyteslist(0) == D_SCANCODE || (getbyteslist(0) == TWOBYTE_SCANCODE && getbyteslist(1) == RIGHT_ARROW_SC)) ? cur_location+1 : cur_location;
                        if(cur_location < 0){
                            cur_location = 0;
                        }
                        else if(cur_location > 6){
                            cur_location = 6;
                        }

                        deleteCursor(3);    

                        int aux = INITIAL_COLUMN_PIXEL + cur_location*(COLUMN_WIDTH + COLUMN_SPACING);
                        aux += INITIAL_COLUMN_PIXEL + cur_location*(COLUMN_WIDTH + COLUMN_SPACING) + COLUMN_WIDTH - 20;
                        /* X axis */            cur_xy[0] = (int) (aux/(float)2);
                        /* Y axis (fixed) */    cur_xy[1] = 61;

                        draw_XPM(XPM_down_arrow, cur_xy[0], cur_xy[1], secondBuffer, down_arrow_img, 0);
                        memcpy(address, secondBuffer, total_size);
                }
            }
            break;
    }
    return colNumber;
}

int process_fallingChip(int column, void *address){
    /* Determine if it is the first time running this function */ static int firstTime = 1;
    /* Determine the target circle which we want to fill */ static int targetCircle;
    /* The Y coordinate we want to reach */ static int targetYCoordinate;
    /* The additive value each time we want to re-draw the chip */ static int progress;
    int total_size = getTotalSize();
    int column_filled = column_filled;
    
    if(markedDraw > 0){
        markedDraw--;
        if(markedDraw == 0){
            RESTART(1, address);
        }
        return 0;
    }

    if(firstTime == 1){
        firstTime = 0;
        targetCircle = -1;
        for(int i = 5; i >= 0; i--){
            if( game_table[i][column] == 0){
                targetCircle = i;
                break;
            }
        }
        
        if(targetCircle == -1){
            // Column is filled!
            firstTime = 1;
            if(getCurrentPlayer() == 1)
                stateOfGame = BLUE_PLAYING;
            else
                stateOfGame = RED_PLAYING;
            
            column_filled = 1;
            printf("    [ERROR] --> Column marked as filled!\n");

            return column_filled;
        }
        else{
            column_filled = 0;
            progress = 0;
            targetYCoordinate = HEADER_SIZE + DISTANCE_TOP + targetCircle * DISTANCE_HOLESY + targetCircle * CHIP_DIAMETER;
        }
    }

    int currentXCoordinate = DISTANCE_LEFT + column * CHIP_DIAMETER + column * DISTANCE_HOLESX;
    int currentYCoordinate = HEADER_SIZE + DISTANCE_TOP + progress;

    if(currentYCoordinate >= targetYCoordinate){
        currentYCoordinate = targetYCoordinate;
    }
    memset(secondBuffer, 0, total_size);
    draw_allChips();
    if(getCurrentPlayer() == 1){
        draw_XPM(XPM_ficha_azul, currentXCoordinate, currentYCoordinate, secondBuffer, ficha_azul_img, 0);
    }
    else{
        draw_XPM(XPM_ficha_vermelha, currentXCoordinate, currentYCoordinate, secondBuffer, ficha_vermelha_img, 0);
    }
    draw_XPM(XPM_tabuleiro, 0, 0, secondBuffer, tabuleiro_img, 0);
    if(getCurrentPlayer() == 1){
        draw_XPM(XPM_cabecalho_azul, 0, 0, secondBuffer, cabecalho_azul_img, 0);
    }
    else{
        draw_XPM(XPM_cabecalho_vermelho, 0, 0, secondBuffer, cabecalho_vermelho_img, 0);
    }   
    memcpy(address, secondBuffer, total_size);

    progress = progress + 8;

    if(currentYCoordinate >= targetYCoordinate){
        game_table[targetCircle][column] = getCurrentPlayer();
        firstTime = 1;
        targetCircle = -1;
        progress = 0;

        int outcome = checkWin();

        if(outcome == 1 || outcome == 2){
            stateOfGame = REVEALING_PATH;
            return column_filled;
        }
        else if(outcome == 3){
            markedDraw = 3;
        }
        
        if(stateOfGame == PUTTING_CHIP && outcome != 3){
            if(getCurrentPlayer() == 1){
                stateOfGame = RED_PLAYING;
            }
            else{
                stateOfGame = BLUE_PLAYING;
            }
            changePlayer();
        }

        memset(secondBuffer, 0, total_size);
        draw_allChips();
        draw_XPM(XPM_tabuleiro, 0, 0, secondBuffer, tabuleiro_img, 0);
        if(stateOfGame != BLUE_WON && stateOfGame != RED_WON){
            if(outcome != 3){
                place_restart(1,0);
                setRemainingSeconds(DEFAULT_SECONDS);
                addREDwarning(0);
            }
            remaining_time(getRemainingSeconds(), address);
            if(outcome == 3){
                draw_XPM(XPM_drawTop, 0, 0, secondBuffer, drawTop_img, 0);
            }
            else if(getCurrentPlayer() == 1){
                draw_XPM(XPM_cabecalho_azul, 0, 0, secondBuffer, cabecalho_azul_img, 0);
            }
            else{
                draw_XPM(XPM_cabecalho_vermelho, 0, 0, secondBuffer, cabecalho_vermelho_img, 0);
            }
        }
        if(outcome != 3){
            draw_XPM(XPM_down_arrow, cur_xy[0], cur_xy[1], secondBuffer, down_arrow_img, 0);   
        }
        memcpy(address, secondBuffer, total_size);
    }

    return column_filled;
}

void draw_allChips(){

    int currentXCoordinate;
    int currentYCoordinate;

    // Set chips
    for(int i = ROWS-1; i >= 0; i--){
        for(int j = 0; j < COLUMNS; j++){
            currentXCoordinate = DISTANCE_LEFT + j * CHIP_DIAMETER + j * DISTANCE_HOLESX;
            currentYCoordinate = HEADER_SIZE + DISTANCE_TOP + i * DISTANCE_HOLESY + i * CHIP_DIAMETER;

            if(game_table[i][j] == 1){
                draw_XPM(XPM_ficha_azul, currentXCoordinate, currentYCoordinate, secondBuffer, ficha_azul_img, 0);
                draw_XPM(XPM_ficha_azul, currentXCoordinate, currentYCoordinate, Chips_background, ficha_azul_img, 0);
            }
            else if(game_table[i][j] == 2){
                draw_XPM(XPM_ficha_vermelha, currentXCoordinate, currentYCoordinate, Chips_background, ficha_vermelha_img, 0);
                draw_XPM(XPM_ficha_vermelha, currentXCoordinate, currentYCoordinate, secondBuffer, ficha_vermelha_img, 0);
            }
            else{
                // Paint black chips.
                draw_XPM(XPM_ficha_vermelha, currentXCoordinate, currentYCoordinate, Chips_background, ficha_vermelha_img, 1);
                draw_XPM(XPM_ficha_vermelha, currentXCoordinate, currentYCoordinate, secondBuffer, ficha_vermelha_img, 1);
            }
        }
    }
}

void RESTART(int print, void *address){
    int total_size = getTotalSize();

    for(int i = ROWS-1; i >= 0; i--){
        for(int j = 0; j < COLUMNS; j++){
            game_table[i][j] = 0;
        }
    }

    if(print == 1){
        memset(secondBuffer, 0, total_size);
        draw_allChips();
        draw_XPM(XPM_tabuleiro, 0, 0, secondBuffer, tabuleiro_img, 0);
        int blackColor_ = isONRestart(1) == 1 ? 0 : 1;
        place_restart(blackColor_,0);
    }
    startPlayer();

    // Time section
        setRemainingSeconds(DEFAULT_SECONDS);
        addREDwarning(0);
        reset_counter();

    if(print == 1){
        remaining_time(getRemainingSeconds(), address);
        draw_XPM(XPM_cabecalho_azul, 0, 0, secondBuffer, cabecalho_azul_img, 0);
    }

    stateOfGame = BLUE_PLAYING;

    if(print == 1){
        int aux;
        if(isInZone(&aux) == 0)
            draw_XPM(XPM_cursor, cur_xy[0], cur_xy[1], secondBuffer, cursor_img, 0);
        else
            draw_XPM(XPM_down_arrow, cur_xy[0], cur_xy[1], secondBuffer, down_arrow_img, 0);   
        memcpy(address, secondBuffer, total_size);
    }

    return;
}

int checkWin() {
    static int filledSpaces = 0;

    filledSpaces++;

    int HEIGHT = ROWS;
    int WIDTH = COLUMNS;
    int EMPTY_SLOT = 0;
    for (int r = 0; r < HEIGHT; r++) { // iterate rows, bottom to top
        for (int c = 0; c < WIDTH; c++) { // iterate columns, left to right
            int player = game_table[r][c];
            if (player == EMPTY_SLOT)
                continue; // don't check empty slots

            // look right
            if (c + 3 < WIDTH && player == game_table[r][c+1] && player == game_table[r][c+2] && player == game_table[r][c+3]){
                victorious_coordinatesXY[r][c] = player;
                victorious_coordinatesXY[r][c+1] = player;
                victorious_coordinatesXY[r][c+2] = player;
                victorious_coordinatesXY[r][c+3] = player;
                return player;
            }
            if (r + 3 < HEIGHT) {
                // Look up
                if (player == game_table[r+1][c] && player == game_table[r+2][c] && player == game_table[r+3][c]){
                    victorious_coordinatesXY[r][c] = player;
                    victorious_coordinatesXY[r+1][c] = player;
                    victorious_coordinatesXY[r+2][c] = player;
                    victorious_coordinatesXY[r+3][c] = player;
                    return player;
                }
                // look up & right
                if (c + 3 < WIDTH && player == game_table[r+1][c+1] && player == game_table[r+2][c+2] && player == game_table[r+3][c+3]){
                    victorious_coordinatesXY[r][c] = player;
                    victorious_coordinatesXY[r+1][c+1] = player;
                    victorious_coordinatesXY[r+2][c+2] = player;
                    victorious_coordinatesXY[r+3][c+3] = player;
                    return player;
                }
                // look up & left
                if (c - 3 >= 0 && player == game_table[r+1][c-1] && player == game_table[r+2][c-2] && player == game_table[r+3][c-3]){
                    victorious_coordinatesXY[r][c] = player;
                    victorious_coordinatesXY[r+1][c-1] = player;
                    victorious_coordinatesXY[r+2][c-2] = player;
                    victorious_coordinatesXY[r+3][c-3] = player;
                    return player;
                }
            }
        }
    }

    if(filledSpaces == ROWS * COLUMNS){
        filledSpaces = 0;
        setRemainingSeconds(3);
        addREDwarning(0);
        reset_counter();
        return 3; // Draw!
    }

    return EMPTY_SLOT; // no winner found
}
