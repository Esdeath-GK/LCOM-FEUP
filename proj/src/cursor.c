#include <lcom/lcf.h>
#include <machine/int86.h>

// Standard Libraries:
#include "cursor.h"
#include "proj_essentials.h"
#include "videocard_essentials.h"
#include "mouse_essentials.h"
#include "kbd_essentials.h"

// Images and additional '.h' inclusions:
#include "xpm_variables.c"

// Global and Extern variables.
extern uint8_t* secondBuffer;               // [proj_essentials.c - l.##] Pointer used to store space for double buffering.
extern uint8_t* Game_background;            // [proj_essentials.c - l.##] Pointer used to store space for the game's background.
extern uint8_t* Chips_background;           // [proj_essentials.c - l.##] Pointer used to store space for the chips.
extern int cur_xy[2];                       // [proj_essentials.c - l.##] Cursor coordinates. Always begins at (x,y) = (0,61) */



// ---------------- [ESSENTIALS] ---------------- 

void updateCursorPos(){
    struct packet mousepath = getMousePackage();
    vbe_mode_info_t vbe_info = getVbeModeInfo();
    cur_xy[0] += mousepath.delta_x;
    cur_xy[1] += (-1)*mousepath.delta_y;

    cur_xy[0] = (cur_xy[0] >= (vbe_info.XResolution - cursor_img.width)) ? (vbe_info.XResolution - cursor_img.width) : cur_xy[0];
    cur_xy[1] = (cur_xy[1] >= (vbe_info.YResolution - cursor_img.height))  ? (vbe_info.YResolution - cursor_img.height) : cur_xy[1];
}

int isInZone(int* col){

    *col = -1;

    if( cur_xy[1] <= (60 - down_arrow_img.height) || cur_xy[1] >= (514 - down_arrow_img.height)){
        return 0;
    }
    else{ 

        for(int i = 0; i < 7; i++){
            int initial_position = INITIAL_COLUMN_PIXEL + i* (COLUMN_WIDTH + COLUMN_SPACING);
            if( (cur_xy[0] >= initial_position && cur_xy[0] <= initial_position + COLUMN_WIDTH) ){
                *col = i;
            }
        }
        
        if(*col >= 0 && *col <= 6){
            return 1;
        }
    }

    return 0;
}


void deleteCursor(int type){
    vbe_mode_info_t vbe_info = getVbeModeInfo();
    printf("deleteCursor was called!\n");

    if(type <= 0 || type > 5){
        printf("[ERROR NOTIFICATION - (deleteCursor)] --> Invalid input. 'type' variable higher than 3 or equal to or less than 0.\n");
        return;
    }

    int x, y, width, height;
    int limit = 1;
    if(type == 3){
        limit = 2;
    }

    x = cur_xy[0];
    y = cur_xy[1];
    width = 30;
    height = 40;

    if( x < 0 || y < 0){
        x = 0;
        y = 61;
    }

    if( (y+height) >= vbe_info.YResolution){
        height = (vbe_info.YResolution - 1) - y;
    }

    if( (x+width) >= vbe_info.XResolution){
        width = (vbe_info.XResolution - 1) - x;
    }

    int bytesPP = ceil(vbe_info.BitsPerPixel / (float)8);

    // Begin attribution:
    int current = 0;

    while(current < limit){
        uint8_t* origin = secondBuffer;
        uint8_t* origin_backup;

        if(current == 0 && type == 1){
            origin_backup = Game_background;
        }
        else if(current == 0 && type == 2){
            origin_backup = Chips_background;
        }
        else if(current == 0 && type == 3){
            origin_backup = Chips_background;
        }
        else if(current == 1 && type == 3){
            origin_backup = Game_background;
        }
        else if(current == 0 && type == 4){
            origin_backup = XPM_blue_victorious;
        }
        else if(current == 0 && type == 5){
            origin_backup = XPM_red_victorious;
        }
        else{
            origin_backup = NULL;
            return;
        }
        

        // The mode is direct-color-based
            // secondBuffer - auxiliary
            uint8_t* aux;

            // backup - aux;
            uint8_t* aux_backup;

            int jumper = (type == 4 || type == 5) ? 4 : 3;
            int diff = (type == 4 || type == 5) ? 1 : 0;


        // Second step --> Begin attribution of colors
            for(int i = 0; i < height; i++){
                aux = (origin+((y+i)*vbe_info.XResolution+x)*bytesPP);
                aux_backup = (origin_backup+((y+i)*vbe_info.XResolution+x)*(bytesPP+diff));
                for(int j = 0; j < width; j++){
                    if( (*(aux_backup+0) != 0) || (*(aux_backup+1) != 0) || (*(aux_backup+2) != 0) ){
                        *(aux+0) = *(aux_backup+0);
                        *(aux+1) = *(aux_backup+1);
                        *(aux+2) = *(aux_backup+2);
                    }
                    aux += 3;
                    aux_backup += jumper;
                }
            }

        current++;
    }

    return;
}

int isONRestart(int precise){

    int it_is = 0;

    if(precise == 1){
        if( cur_xy[0] >= (REBOOT_X) && cur_xy[0] <= (REBOOT_X + REBOOT_W) ){
            if( cur_xy[1] >= (REBOOT_Y) && cur_xy[1] <= (REBOOT_Y + REBOOT_H) ){
                it_is = 1;
            }
        }
    }
    else{
        if( cur_xy[0] >= (REBOOT_X - 30) && cur_xy[0] <= (REBOOT_X + REBOOT_W) ){
            if( cur_xy[1] >= (REBOOT_Y - 40) && cur_xy[1] <= (REBOOT_Y + REBOOT_H) ){
                it_is = 1;
            }
        }
    }

    return it_is;
}

int isONTimer(){

    if( cur_xy[0] >= (TIMER_REGION_X - CUR_W) && cur_xy[0] <= (TIMER_REGION_X + TIMER_REGION_WIDTH + CUR_W) ){
        if( cur_xy[1] >= (TIMER_REGION_Y - CUR_H) && cur_xy[1] <= (TIMER_REGION_Y + TIMER_REGION_HEIGHT + CUR_H) ){
            return 1;
        }
    }

    return 0;
}

int onTopClose_EG(){

    int it_is = 0;

    if( (cur_xy[0] >= (SD_X - CUR_W)) && (cur_xy[0] <= (SD_X + SD_WIDTH)) ){
        if( (cur_xy[1] >= (SD_Y - CUR_H)) && (cur_xy[1] <= (SD_Y + SD_HEIGHT)) ){
            it_is = 1;
        }
    }

    return it_is;
}

int onTopRestart_EG(int precise){

    int it_is = 0;

    if(precise == 1){
        if( cur_xy[0] >= (RESTART_EG_X) && cur_xy[0] <= (RESTART_EG_X + REBOOT_W) ){
            if( cur_xy[1] >= (RESTART_EG_Y) && cur_xy[1] <= (RESTART_EG_Y + REBOOT_H) ){
                it_is = 1;
            }
        }
    }
    else{
        if( cur_xy[0] >= (RESTART_EG_X - CUR_W) && cur_xy[0] <= (RESTART_EG_X + REBOOT_W) ){
            if( cur_xy[1] >= (RESTART_EG_Y - CUR_H) && cur_xy[1] <= (RESTART_EG_Y + REBOOT_H) ){
                it_is = 1;
            }
        }
    }

    return it_is;
}
