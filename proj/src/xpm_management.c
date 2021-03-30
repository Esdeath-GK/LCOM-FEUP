#include <lcom/lcf.h>
#include <machine/int86.h>
#include <lcom/xpm.h>

// Standard Libraries 
#include <stdbool.h>
#include <stdint.h>
#include "videocard_essentials.h"
#include "proj_essentials.h"

// Images and additional '.h' inclusions.
#include "sources/images.h"
#include "xpm_management.h"
#include "xpm_variables.c"

int menu_setXPM(void *address){

    // SET-UP all XPMs (no drawing)
    XPM_menu = set_up_XPM(MENU_IMG, 0, 0, address, 0, NULL, 0, &menu_img); 
    XPM_starthover = set_up_XPM(STARTHOVER_IMG, 0, 0, address, 0, NULL, 0, &start_img); 
    XPM_exithover = set_up_XPM(EXITHOVER_IMG, 0, 0, address, 0, NULL, 0, &exit_img); 

    if(XPM_menu == NULL || XPM_starthover == NULL || XPM_exithover == NULL){
        printf("Error setting up Menu-XPM images ...");
        return -1;
    }

    return 0;   
}

int game_setXPM(void *address){

    // SET-UP all XPMs (no drawing)
        XPM_tabuleiro = set_up_XPM(TABULEIRO_IMG, 0, 0, address, 0, NULL, 0, &tabuleiro_img);
        XPM_cabecalho_azul = set_up_XPM(VEZDOAZUL_IMG, 0, 0, address, 0, NULL, 0, &cabecalho_azul_img);
        XPM_cabecalho_vermelho = set_up_XPM(VEZDOVEMELHO_IMG, 0, 0, address, 0, NULL, 0, &cabecalho_vermelho_img);
        XPM_ficha_azul = set_up_XPM(FICHAAZUL_IMG, 0, 0, address, 0, NULL, 0, &ficha_azul_img);
        XPM_ficha_vermelha = set_up_XPM(FICHAVERMELHA_IMG, 0, 0, address, 0, NULL, 0, &ficha_vermelha_img);
        XPM_cursor = set_up_XPM(CURSOR_IMG, 0, 0, address, 0, NULL, 0, &cursor_img);
        XPM_down_arrow = set_up_XPM(DOWN_ARROW_IMG, 0, 0, address, 0, NULL, 0, &down_arrow_img);
        XPM_reboot_black = set_up_XPM(REBOOT_BLACK_IMG, 0, 0, address, 0, NULL, 0, &reboot_black);  
        XPM_reboot_red = set_up_XPM(REBOOT_RED_IMG, 0, 0, address, 0, NULL, 0, &reboot_red);
        XPM_reboot_red = set_up_XPM(REBOOT_RED_IMG, 0, 0, address, 0, NULL, 0, &reboot_red);
        XPM_endingRing = set_up_XPM(ENDINGRING, 0, 0, address, 0, NULL, 0, &endingRing_img);
        XPM_drawTop = set_up_XPM(DRAWTOP, 0, 0, address, 0, NULL, 0, &drawTop_img);        

    // End-game
        XPM_blue_victorious = set_up_XPM(BLUE_VICTORIOUS_IMG, 0, 0, address, 0, NULL, 0, &blue_victorious);
        XPM_blueCLOSEbutton = set_up_XPM(BLUE_BUTTON_CLOSE_IMG, 0, 0, address, 0, NULL, 0, &blueCLOSE_img);
        XPM_red_victorious = set_up_XPM(RED_VICTORIOUS_IMG, 0, 0, address, 0, NULL, 0, &red_victorious);
        XPM_redCLOSEbutton = set_up_XPM(RED_BUTTON_CLOSE_IMG, 0, 0, address, 0, NULL, 0, &redCLOSE_img);       
    
    // SET-UP all NUMBERS (no drawing)

        DATEXPM_ZERO = set_up_XPM(DATEZERO_IMG, 0, 0, address, 0, NULL, 0, &datezero_img);
        DATEXPM_ONE = set_up_XPM(DATEONE_IMG, 0, 0, address, 0, NULL, 0, &dateone_img);
        DATEXPM_TWO = set_up_XPM(DATETWO_IMG, 0, 0, address, 0, NULL, 0, &datetwo_img);
        DATEXPM_THREE = set_up_XPM(DATETHREE_IMG, 0, 0, address, 0, NULL, 0, &datethree_img);
        DATEXPM_FOUR = set_up_XPM(DATEFOUR_IMG, 0, 0, address, 0, NULL, 0, &datefour_img);
        DATEXPM_FIVE = set_up_XPM(DATEFIVE_IMG, 0, 0, address, 0, NULL, 0, &datefive_img);
        DATEXPM_SIX = set_up_XPM(DATESIX_IMG, 0, 0, address, 0, NULL, 0, &datesix_img);
        DATEXPM_SEVEN = set_up_XPM(DATESEVEN_IMG, 0, 0, address, 0, NULL, 0, &dateseven_img);
        DATEXPM_EIGHT = set_up_XPM(DATEEIGHT_IMG, 0, 0, address, 0, NULL, 0, &dateeight_img);  
        DATEXPM_NINE = set_up_XPM(DATENINE_IMG, 0, 0, address, 0, NULL, 0, &datenine_img);
        SLASHXPM = set_up_XPM(SLASH_IMG, 0, 0, address, 0, NULL, 0, &slash_img); 

        SMALLXPM_ZERO = set_up_XPM(SMALLZERO_IMG, 0, 0, address, 0, NULL, 0, &small0_img);
        SMALLXPM_ONE = set_up_XPM(SMALLONE_IMG, 0, 0, address, 0, NULL, 0, &small1_img);
        SMALLXPM_TWO = set_up_XPM(SMALLTWO_IMG, 0, 0, address, 0, NULL, 0, &small2_img);
        SMALLXPM_THREE = set_up_XPM(SMALLTHREE_IMG, 0, 0, address, 0, NULL, 0, &small3_img);
        SMALLXPM_FOUR = set_up_XPM(SMALLFOUR_IMG, 0, 0, address, 0, NULL, 0, &small4_img);
        SMALLXPM_FIVE = set_up_XPM(SMALLFIVE_IMG, 0, 0, address, 0, NULL, 0, &small5_img);
        SMALLXPM_SIX = set_up_XPM(SMALLSIX_IMG, 0, 0, address, 0, NULL, 0, &small6_img);
        SMALLXPM_SEVEN = set_up_XPM(SMALLSEVEN_IMG, 0, 0, address, 0, NULL, 0, &small7_img);
        SMALLXPM_EIGHT = set_up_XPM(SMALLEIGHT_IMG, 0, 0, address, 0, NULL, 0, &small8_img);
        SMALLXPM_NINE = set_up_XPM(SMALLNINE_IMG, 0, 0, address, 0, NULL, 0, &small9_img);
        TWOPOINTXPM = set_up_XPM(TWOPOINT_IMG, 0, 0, address, 0, NULL, 0, &twopoint_img); 

    if(XPM_tabuleiro == NULL || XPM_cabecalho_azul == NULL || XPM_cabecalho_vermelho == NULL || XPM_ficha_azul == NULL 
        || XPM_ficha_vermelha == NULL || XPM_cursor == NULL || XPM_down_arrow == NULL || XPM_reboot_black == NULL || 
        XPM_reboot_red == NULL || XPM_blue_victorious == NULL || XPM_red_victorious == NULL || XPM_blueCLOSEbutton == NULL ||
        XPM_redCLOSEbutton == NULL || XPM_endingRing == NULL || XPM_drawTop == NULL){
            
            printf("Error setting up one or more Standard-XPM images ...");
            return -1;

    }

    if(DATEXPM_ZERO == NULL || DATEXPM_ONE == NULL || DATEXPM_TWO == NULL || DATEXPM_FOUR == NULL || DATEXPM_FIVE == NULL 
        || DATEXPM_SIX == NULL || DATEXPM_SEVEN == NULL || DATEXPM_EIGHT == NULL || DATEXPM_NINE == NULL || SLASHXPM == NULL){

            printf("Error setting up one or more number images ...");
            return -1;
            
    }

    if( SMALLXPM_ZERO == NULL || SMALLXPM_ONE == NULL || SMALLXPM_TWO == NULL || SMALLXPM_THREE == NULL || SMALLXPM_FOUR == NULL
        || SMALLXPM_FIVE == NULL || SMALLXPM_SIX == NULL || SMALLXPM_SEVEN == NULL || SMALLXPM_EIGHT == NULL || SMALLXPM_NINE == NULL
        || TWOPOINTXPM == NULL){

            printf("Error setting up one or more small_number images ...");
            return -1;

    }
    return 0;   
}

void menu_freeXPM(){

    // Free all XPMs:
        set_up_XPM(0, 0, 0, NULL, 0, XPM_menu, 0, &menu_img);
        set_up_XPM(0, 0, 0, NULL, 0, XPM_starthover, 0, &start_img);
        set_up_XPM(0, 0, 0, NULL, 0, XPM_exithover, 0, &exit_img);   

    return;   
}


void game_freeXPM(){

    // Free all XPMs:
        set_up_XPM(0, 0, 0, NULL, 0, XPM_tabuleiro, 0, &tabuleiro_img);
        set_up_XPM(0, 0, 0, NULL, 0, XPM_cabecalho_azul, 0, &cabecalho_azul_img);
        set_up_XPM(0, 0, 0, NULL, 0, XPM_cabecalho_vermelho, 0, &cabecalho_vermelho_img);
        set_up_XPM(0, 0, 0, NULL, 0, XPM_ficha_azul, 0, &ficha_azul_img);   
        set_up_XPM(0, 0, 0, NULL, 0, XPM_ficha_vermelha, 0, &ficha_vermelha_img);
        set_up_XPM(0, 0, 0, NULL, 0, XPM_cursor, 0, &cursor_img);
        set_up_XPM(0, 0, 0, NULL, 0, XPM_down_arrow, 0, &down_arrow_img);
        set_up_XPM(0, 0, 0, NULL, 0, XPM_reboot_black, 0, &reboot_black);
        set_up_XPM(0, 0, 0, NULL, 0, XPM_reboot_red, 0, &reboot_red);
        set_up_XPM(0, 0, 0, NULL, 0, XPM_endingRing, 0, &endingRing_img);  
        set_up_XPM(0, 0, 0, NULL, 0, XPM_drawTop, 0, &drawTop_img);

        set_up_XPM(0, 0, 0, NULL, 0, XPM_blue_victorious, 0, &blue_victorious);
        set_up_XPM(0, 0, 0, NULL, 0, XPM_red_victorious, 0, &red_victorious);
        set_up_XPM(0, 0, 0, NULL, 0, XPM_blueCLOSEbutton, 0, &blueCLOSE_img);
        set_up_XPM(0, 0, 0, NULL, 0, XPM_redCLOSEbutton, 0, &redCLOSE_img);

        set_up_XPM(0, 0, 0, NULL, 0, DATEXPM_ZERO, 0, &datezero_img);
        set_up_XPM(0, 0, 0, NULL, 0, DATEXPM_ONE, 0, &dateone_img);
        set_up_XPM(0, 0, 0, NULL, 0, DATEXPM_TWO, 0, &datetwo_img);
        set_up_XPM(0, 0, 0, NULL, 0, DATEXPM_THREE, 0, &datethree_img);
        set_up_XPM(0, 0, 0, NULL, 0, DATEXPM_FOUR, 0, &datefour_img);
        set_up_XPM(0, 0, 0, NULL, 0, DATEXPM_FIVE, 0, &datefive_img);
        set_up_XPM(0, 0, 0, NULL, 0, DATEXPM_SIX, 0, &datesix_img);
        set_up_XPM(0, 0, 0, NULL, 0, DATEXPM_SEVEN, 0, &dateseven_img);
        set_up_XPM(0, 0, 0, NULL, 0, DATEXPM_EIGHT, 0, &dateeight_img);
        set_up_XPM(0, 0, 0, NULL, 0, DATEXPM_NINE, 0, &datenine_img);
        set_up_XPM(0, 0, 0, NULL, 0, SLASHXPM, 0, &slash_img);


        set_up_XPM(0, 0, 0, NULL, 0, SMALLXPM_ZERO, 0, &small0_img);
        set_up_XPM(0, 0, 0, NULL, 0, SMALLXPM_ONE, 0, &small1_img);
        set_up_XPM(0, 0, 0, NULL, 0, SMALLXPM_TWO, 0, &small2_img);
        set_up_XPM(0, 0, 0, NULL, 0, SMALLXPM_THREE, 0, &small3_img);
        set_up_XPM(0, 0, 0, NULL, 0, SMALLXPM_FOUR, 0, &small4_img);
        set_up_XPM(0, 0, 0, NULL, 0, SMALLXPM_FIVE, 0, &small5_img);
        set_up_XPM(0, 0, 0, NULL, 0, SMALLXPM_SIX, 0, &small6_img);
        set_up_XPM(0, 0, 0, NULL, 0, SMALLXPM_SEVEN, 0, &small7_img);
        set_up_XPM(0, 0, 0, NULL, 0, SMALLXPM_EIGHT, 0, &small8_img);
        set_up_XPM(0, 0, 0, NULL, 0, SMALLXPM_NINE, 0, &small9_img);
        set_up_XPM(0, 0, 0, NULL, 0, TWOPOINTXPM, 0, &twopoint_img);


    return;   
}
