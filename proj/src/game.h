/**************************************************************/
/*****************  GAME MANAGEMENT FUNCTIONS  ****************/
/**************************************************************/

/* Description:
    The following .c/.h module is, as the file itself states, called 'game'. 
    Its main utility is to make available crucial functions for the management of the game along with potential utility functions.
    
    Four functions will be taken consideration:
        - game_react() --> This function will call off graphic modifications based on the interrupts received from the mouse and keyboard.
        - process_fallingChip() --> Processes the animation of a chip falling in a column that is sent as an argument.
        - draw_allChips() --> Represents on the screen all the chips in their specific places. Macros are used to calculate their positions.
        - RESTART() --> Completely restarts the game and respective global variables. The cursor is not reset!
        - checkWin() --> Uses the victory rules, trying to identify a group of four chips that satisfies the criteria. If it does, it returns a specific value calling off the victor.
	
	Notes: *None*
*/

/**
 * @brief - Modifies the screen based on the interrupts.
 * 
 * @return - Which column mouse is hover.
 */
int game_react(void *address, int flag);

/**
 * @brief - Processes the animation of a falling chip on a given column. Additionally calculates which "holes" are available for the chip.
 *          If the column is full, it displays a box warning for 1 second and does not add anything into it.
 * 
 * @param column - Column where the chip is falling.
 * 
 * @return - No return.
 */
int process_fallingChip(int column, void *address);

/**
 * @brief - Draws all chips currently placed in the table.
 * 
 * @return - No return.
 */
void draw_allChips();

/**
 * @brief - Restarts the game entirely, cleaning all chips from the table.
 * 
 * @param print - If set the table will be printed as new. Otherwise, only the game-ruling variables will be reset.
 * 
 * @return - No return.
 */
void RESTART(int print, void *address);

/**
 * @brief - The function checks if any player has won, based on the chips displayed on the table.
 *          This function also contains a static variable that is incremented each time the checkWin request is sent.
 *          If such variable is equal to 6 x 7 = 42 and it finds no winners, then it declares a draw and immediately resets the game.
 * 
 * @return - Returns 0 if there is no player. 1 if it is the blue player. 2 if it is the red player.
 *           It will return 3 if it is a draw.
*/
int checkWin();

/**
 * @brief - Get value of MarkedDraw
 * @return - MarkedDraw
*/
int getMarkedDraw();
