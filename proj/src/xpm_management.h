/**************************************************************/
/******************  XPM MANAGEMENT FUNCTIONS  ****************/
/**************************************************************/

/* Description:
    The following .c/.h module is, as the file itself states, called 'xpm_management'. 
    Its main utility is to make available crucial functions for the management of all XPM images in the game and menu.
    
    Four functions will be taken consideration:
        - menu_setXPM() --> This function loads all XPM images related to the Menu;
        - game_setXPM() --> Similar to the aforementioned function, but instead loads all XPM images related to the Game itself;
        - menu_freeXPM() --> This function frees all XPM images loaded by 'menu_setXPM()'.
        - game_freeXPM() --> Exactly like the 'menu_freeXPM()' but applied to the 'game_setXPM()'s loaded XPMs. 
    
    Notes: No value definitions will be added to this file for they are not necessary.
*/


// Function definitions:

/**
 * @brief - Loads all XPM images related to the Menu;
 * 
 * @return - 0 if all XPMs were correctly set-up. -1 if at least one returned an error.
*/
int menu_setXPM(void *address);

/**
 * @brief - Loads all XPM images related to the Game;
 * 
 * @return - 0 if all XPMs were correctly set-up. -1 if at least one returned an error.
 */
int game_setXPM(void *address);

/**
 * @brief - Frees all XPM variables related to the Menu.
 * 
 * @return - No return.
*/
void menu_freeXPM();

/**
 * @brief - Frees all XPM variables related to the Game.
 * 
 * @return - No return.
 */
void game_freeXPM();

