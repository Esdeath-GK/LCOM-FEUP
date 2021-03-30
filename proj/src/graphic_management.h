/**************************************************************/
/***************  GRAPHIC MANAGEMENT FUNCTIONS  ***************/
/**************************************************************/

/* Description:
    The following .c/.h module is, as the file itself states, called 'graphic_management'. 
    Its main utility is to make available crucial functions that will modify the graphic visuals as the game/menu goes on.
    
    Four functions will be taken consideration:
        - place_restart() --> Coloca o botão restart.
        - remaining_time() --> Atualiza o temporizador consoante o tempo restante.
        - clean_time_area() --> Apaga os números do temporizador.
        - end_screen() --> Imprime o ecrã final do jogo.
        - place_closeButton() --> Coloca o butão de fecho (end-screen)
        - graphic_RTC() --> Atualiza o menu e a data neste.
        - draw_menuXPM() --> Desenha o menu.
        - draw_date_number() --> Coloca um número numa dada posição. 
	
	Notes: *None*
*/

/**
 * @brief - Puts the restart button.
 * 
 * @param black - If set, it prints the reboot_black.xpm. Else the red one will take its place.
 * 
 * @param endGame - If set, it prints the reboot XPM on the end_game box location.
 * 
 * @return - No return.
 */
void place_restart(int black, int endGame);

/**
 * @brief - Sets up the XPM values in accordance to the time remaining.
 * 
 * @param precise - The value of the remaining time. After printing, reduces its value by one. If the value, after reduction, is equal to zero
 *                  the function prints 0 0 and resets the timer. Additionally, passes the turn to the another player.
 * 
 * 
 * @return - No return.
*/
void remaining_time(int remaining, void *address);

/**
 * @brief - Cleans the area of the time.
 * 
 * @return - No return.
*/
void clean_time_area();

/**
 * @brief - Displays the end-screen (after a winner has been declared).
 * 
 * @return - No return.
*/
void end_screen(void *address);

/**
 * @brief - Prints the closeButton.
 * 
 * @return - No return.
 */
void place_closeButton();

/**
 * @brief - Updates the Menu and updates the current date.
 * 
 * @return - No return.
*/
void graphic_RTC();

/**
 * @brief - Draws the menu.
 * 
 * @param optionNumber -        Any number between (inclusive) 0 and 2. If the current global 'optionNumber' value is different
 *                          from the optionNumber listed in the argument, then the global value is immediately modified.
 * 
 * @return - No return.
*/
void draw_menuXPM(int optionNumber, void *address);

/**
 * @brief - Prints time and date.
 * 
 * @param small - If set to 1, it will draw smaller numbers.
 * 
 * @param x - Beginning x coordinate.
 * 
 * @param y - Beginning y coordinate.
 * 
 * @param type - Number of the XPM. 
 * 					--> If the value is between 0-9, it prints the respective numbers.
 * 					--> If the value is 10 and small = 1, then it prints ':'. Else, it prints '/'.
 * 				
 * @return - No return.
*/
void draw_date_number(int small, int x, int y, int type);
