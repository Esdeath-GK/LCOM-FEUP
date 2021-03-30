#include <lcom/lcf.h>
#include <machine/int86.h>

// Standard Libraries:
#include "graphic_management.h"
#include "proj_essentials.h"
#include "videocard_essentials.h"
#include "mouse_essentials.h"
#include "kbd_essentials.h"
#include "cursor.h"
#include "rtc.h"

// Images and additional '.h' inclusions:
#include "xpm_variables.c"

// External values:
extern uint8_t* secondBuffer;
extern int cur_xy[2];
extern game_state stateOfGame;
extern program_state stateOfProgram;
extern uint8_t* Game_background;


// -------------- [ESSENTIALS] --------------


void place_restart(int black, int endGame){

    if(black == 1){
        if(endGame == 0){
            draw_XPM(XPM_reboot_black, REBOOT_X, REBOOT_Y, secondBuffer, reboot_black, 0);
        }
        else{
            draw_XPM(XPM_reboot_black, RESTART_EG_X, RESTART_EG_Y,secondBuffer,reboot_black, 0);
        }
    }
    else{
        if(endGame == 0){
            draw_XPM(XPM_reboot_red, REBOOT_X, REBOOT_Y, secondBuffer, reboot_red, 0);
        }
        else{
            draw_XPM(XPM_reboot_red, RESTART_EG_X, RESTART_EG_Y, secondBuffer, reboot_red, 0);
        }
    }
}

void remaining_time(int remaining, void *address){
    int total_size = getTotalSize();

    clean_time_area();
    if(getRemainingSeconds() != 0){
        setisNumber(1);
        draw_date_number(0, TIMER_REGION_X, TIMER_REGION_Y, floor(remaining/(float)10) );
        draw_date_number(0, TIMER_REGION_X+NUMBER_WIDTH+SPACING, TIMER_REGION_Y, remaining - 10 * floor(remaining/(float)10));
        setisNumber(0);
        if( isONTimer() ){
            draw_XPM(XPM_cursor, cur_xy[0], cur_xy[1], secondBuffer, cursor_img, 0);
        }
        memcpy(address, secondBuffer, total_size);
    }
    else{
        setRemainingSeconds(DEFAULT_SECONDS);
        addREDwarning(0);
        if(PAUSE){
            setisNumber(1);
            draw_date_number(0, TIMER_REGION_X, TIMER_REGION_Y, floor(getRemainingSeconds()/(float)10) );
            draw_date_number(0, TIMER_REGION_X+NUMBER_WIDTH+SPACING, TIMER_REGION_Y, getRemainingSeconds() - 10 * floor(getRemainingSeconds()/(float)10));
            setisNumber(0);
        }
        else{
            setChangeTurn(1);
            setRemainingSeconds(DEFAULT_SECONDS+1);
        }
        if( isONTimer() ){
            draw_XPM(XPM_cursor, cur_xy[0], cur_xy[1], secondBuffer, cursor_img, 0);
        }
        changePlayer();
        if(getCurrentPlayer() == 1){
            draw_XPM(XPM_cabecalho_azul, 0, 0, secondBuffer, cabecalho_azul_img, 0);
        }
        else{
            draw_XPM(XPM_cabecalho_vermelho, 0, 0, secondBuffer, cabecalho_vermelho_img, 0);
        }
        memcpy(address, secondBuffer, total_size);

        return;
    }
    
}

void clean_time_area(){
    vbe_mode_info_t vbe_info = getVbeModeInfo();
    int x, y, width, height;

    x = TIMER_REGION_X - CUR_W;
    y = TIMER_REGION_Y - CUR_H;
    width = TIMER_REGION_WIDTH + CUR_W * 2;
    height = TIMER_REGION_HEIGHT + CUR_H * 2;

    if(y + height >= vbe_info.YResolution){
        height = vbe_info.YResolution - y - 1;
    }

    int bytesPP = ceil(vbe_info.BitsPerPixel / (float)8);

    // Begin attribution:

    uint8_t* origin = secondBuffer;
    uint8_t* origin_backup;

    origin_backup = Game_background;
        
    // The mode is direct-color-based
        // secondBuffer - auxiliary
        uint8_t* aux;

        // backup - aux;
        uint8_t* aux_backup;


    // Second step --> Begin attribution of colors
        for(int i = 0; i < height; i++){
            aux = (origin+((y+i)*vbe_info.XResolution+x)*bytesPP);
            aux_backup = (origin_backup+((y+i)*vbe_info.XResolution+x)*bytesPP);
            for(int j = 0; j < width; j++){
                if( (*(aux_backup+0) != 0) || (*(aux_backup+1) != 0) || (*(aux_backup+2) != 0) ){
                    *(aux+0) = *(aux_backup+0);
                    *(aux+1) = *(aux_backup+1);
                    *(aux+2) = *(aux_backup+2);
                }
                aux += 3;
                aux_backup += 3;
            }
        }

    return;
}

void end_screen(void *address){
    vbe_mode_info_t vbe_info = getVbeModeInfo();
    int total_size = getTotalSize();

    memset(secondBuffer, 0, total_size);
    if(stateOfGame == BLUE_WON){
        draw_XPM(XPM_blue_victorious, 0, 0, secondBuffer, blue_victorious, 0);
        draw_XPM(XPM_blueCLOSEbutton, SD_X, SD_Y, secondBuffer, blueCLOSE_img, 0);
    }
    else if(stateOfGame == RED_WON){
        draw_XPM(XPM_red_victorious, 0, 0, secondBuffer, red_victorious, 0);
        draw_XPM(XPM_redCLOSEbutton, SD_X, SD_Y, secondBuffer, redCLOSE_img, 0);
    }
    draw_XPM(XPM_reboot_black, RESTART_EG_X, RESTART_EG_Y, secondBuffer, reboot_black, 0);

    cur_xy[0] += 399;
    cur_xy[1] += 299;

    cur_xy[0] = (cur_xy[0] >= (vbe_info.XResolution - cursor_img.width)) ? (vbe_info.XResolution - cursor_img.width) : cur_xy[0];
    cur_xy[1] = (cur_xy[1] >= (vbe_info.YResolution - cursor_img.height))  ? (vbe_info.YResolution - cursor_img.height) : cur_xy[1];
    draw_XPM(XPM_cursor, cur_xy[0], cur_xy[1], secondBuffer, cursor_img, 0);
    

    memcpy(address, secondBuffer, total_size);

}

void place_closeButton(game_state state){
    if(state == BLUE_WON){
        draw_XPM(XPM_blueCLOSEbutton, SD_X, SD_Y, secondBuffer,blueCLOSE_img, 0);
    }
    else{
        draw_XPM(XPM_redCLOSEbutton, SD_X, SD_Y, secondBuffer, redCLOSE_img, 0); 
    }
}

void graphic_RTC(){
    curr_date menudate = update_date();
    // Date evaluation:
        int day_check = menudate.day;
        draw_date_number(0, STARTING_DATE_X, STARTING_DATE_Y, floor(day_check/(float)10) );
        draw_date_number(0, STARTING_DATE_X + 1*DATE_NUMBER_WIDTH + 1*DATE_SPACING_HORIZONTAL, STARTING_DATE_Y, day_check - 10 * floor(day_check/(float)10));
        draw_date_number(0, STARTING_DATE_X + 2*DATE_NUMBER_WIDTH + 2*DATE_SPACING_HORIZONTAL, STARTING_DATE_Y, 10);

        int month_check = menudate.month;
        draw_date_number(0, STARTING_DATE_X + 3*DATE_NUMBER_WIDTH + 3*DATE_SPACING_HORIZONTAL, STARTING_DATE_Y, floor(month_check/(float)10) );
        draw_date_number(0, STARTING_DATE_X + 4*DATE_NUMBER_WIDTH + 4*DATE_SPACING_HORIZONTAL, STARTING_DATE_Y, month_check - 10 * floor(month_check/(float)10));
        draw_date_number(0, STARTING_DATE_X + 5*DATE_NUMBER_WIDTH + 5*DATE_SPACING_HORIZONTAL, STARTING_DATE_Y, 10);

        int year_check = menudate.year;
        draw_date_number(0, STARTING_DATE_X + 6*DATE_NUMBER_WIDTH + 6*DATE_SPACING_HORIZONTAL, STARTING_DATE_Y, 2 );
        draw_date_number(0, STARTING_DATE_X + 7*DATE_NUMBER_WIDTH + 7*DATE_SPACING_HORIZONTAL, STARTING_DATE_Y, 0 );
        draw_date_number(0, STARTING_DATE_X + 8*DATE_NUMBER_WIDTH + 8*DATE_SPACING_HORIZONTAL, STARTING_DATE_Y, floor(year_check/(float)10) );
        draw_date_number(0, STARTING_DATE_X + 9*DATE_NUMBER_WIDTH + 9*DATE_SPACING_HORIZONTAL, STARTING_DATE_Y, year_check - 10 * floor(year_check/(float)10) );


    // Hour evaluation:
        int hours_check = menudate.hours;
        draw_date_number(1, STARTING_DATE2_X, STARTING_DATE2_Y, floor(hours_check/(float)10) );
        draw_date_number(1, STARTING_DATE2_X + 1*DATE_NUMBER_WIDTH + 1*DATE_SPACING_HORIZONTAL, STARTING_DATE2_Y, hours_check - 10 * floor(hours_check/(float)10));
        draw_date_number(1, STARTING_DATE2_X + 2*DATE_NUMBER_WIDTH + 2*DATE_SPACING_HORIZONTAL, STARTING_DATE2_Y, 10);

        int minutes_check = menudate.minutes;
        draw_date_number(1, STARTING_DATE2_X + 3*DATE_NUMBER_WIDTH + 3*DATE_SPACING_HORIZONTAL, STARTING_DATE2_Y, floor(minutes_check/(float)10) );
        draw_date_number(1, STARTING_DATE2_X + 4*DATE_NUMBER_WIDTH + 4*DATE_SPACING_HORIZONTAL, STARTING_DATE2_Y, minutes_check - 10 * floor(minutes_check/(float)10));
        draw_date_number(1, STARTING_DATE2_X + 5*DATE_NUMBER_WIDTH + 5*DATE_SPACING_HORIZONTAL, STARTING_DATE2_Y, 10);

        int seconds_check = menudate.seconds;
        draw_date_number(1, STARTING_DATE2_X + 6*DATE_NUMBER_WIDTH + 6*DATE_SPACING_HORIZONTAL, STARTING_DATE2_Y, floor(seconds_check/(float)10) );
        draw_date_number(1, STARTING_DATE2_X + 7*DATE_NUMBER_WIDTH + 7*DATE_SPACING_HORIZONTAL, STARTING_DATE2_Y, seconds_check - 10 * floor(seconds_check/(float)10) );
}


void draw_menuXPM(int optionNumber, void *address){
    int total_size = getTotalSize();
    if(optionNumber > 1 || optionNumber < 0){
        stateOfGame = CRITICAL_ERROR;
    }
    memset(secondBuffer, 0, total_size);
    draw_XPM(XPM_menu, 0, 0, secondBuffer, menu_img, 0);
    switch (optionNumber){
        case 0:
            draw_XPM(XPM_starthover, BUTTONS_X, BUTTON_PLAY_Y, secondBuffer, start_img, 0);
            break;

        case 1:
            draw_XPM(XPM_exithover, BUTTONS_X, BUTTON_EXIT_Y, secondBuffer, exit_img, 0);
            break;
        default:
            break;
    }
    graphic_RTC();
    memcpy(address,secondBuffer, total_size); 
}

void draw_date_number(int small, int x, int y, int type){
    
    if(small == 0){
        switch(type){
            case 0:
                draw_XPM(DATEXPM_ZERO, x, y, secondBuffer, datezero_img, 0);
                break;
            case 1:
                draw_XPM(DATEXPM_ONE, x, y, secondBuffer, dateone_img, 0);
                break;
            case 2:
                draw_XPM(DATEXPM_TWO, x, y, secondBuffer, datetwo_img, 0);
                break;
            case 3:
                draw_XPM(DATEXPM_THREE, x, y, secondBuffer, datethree_img, 0);
                break;
            case 4:
                draw_XPM(DATEXPM_FOUR, x, y, secondBuffer, datefour_img, 0);
                break;
            case 5:
                draw_XPM(DATEXPM_FIVE, x, y, secondBuffer, datefive_img, 0);
                break;
            case 6:
                draw_XPM(DATEXPM_SIX, x, y, secondBuffer, datesix_img, 0);
                break;
            case 7:
                draw_XPM(DATEXPM_SEVEN, x, y, secondBuffer, dateseven_img, 0);
                break;
            case 8:
                draw_XPM(DATEXPM_EIGHT, x, y, secondBuffer, dateeight_img, 0);
                break;
            case 9:
                draw_XPM(DATEXPM_NINE, x, y, secondBuffer, datenine_img, 0);
                break;
            case 10:
                draw_XPM(SLASHXPM, x, y, secondBuffer, slash_img, 0);
                break;
            default:
                break;
        }
    }
    else{
        switch(type){
            case 0:
                draw_XPM(SMALLXPM_ZERO, x, y, secondBuffer, small0_img, 0);
                break;
            case 1:
                draw_XPM(SMALLXPM_ONE, x, y, secondBuffer, small1_img, 0);
                break;
            case 2:
                draw_XPM(SMALLXPM_TWO, x, y, secondBuffer, small2_img, 0);
                break;
            case 3:
                draw_XPM(SMALLXPM_THREE, x, y, secondBuffer, small3_img, 0);
                break;
            case 4:
                draw_XPM(SMALLXPM_FOUR, x, y, secondBuffer, small4_img, 0);
                break;
            case 5:
                draw_XPM(SMALLXPM_FIVE, x, y, secondBuffer, small5_img, 0);
                break;
            case 6:
                draw_XPM(SMALLXPM_SIX, x, y, secondBuffer, small6_img, 0);
                break;
            case 7:
                draw_XPM(SMALLXPM_SEVEN, x, y, secondBuffer, small7_img, 0);
                break;
            case 8:
                draw_XPM(SMALLXPM_EIGHT, x, y, secondBuffer, small8_img, 0);
                break;
            case 9:
                draw_XPM(SMALLXPM_NINE, x, y, secondBuffer, small9_img, 0);
                break;
            case 10:
                draw_XPM(TWOPOINTXPM, x, y, secondBuffer, twopoint_img, 0);
                break;
            default:
                break;
        }
    }
}
