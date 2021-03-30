/**************************************************************/
/****************  CURSOR MANAGEMENT FUNCTIONS  ***************/
/**************************************************************/

/* Description:
    The following .c/.h module is, as the file itself states, called 'cursor'. 
    Its main utility is to make available crucial functions for the management of the cursor.
    
    Four functions will be taken consideration:
        - updateCursorPos() --> This function merely modifies the global variable 'cur_xy' with the new position based on the mouse packets received.
        - isInZone() --> Checks if the cursor is on top of a column.
        - deleteCursor() --> Cleans the cursor from the screen and replaces the objects that were behind it.
        - isOnRestart() --> Verifies if the cursor is on top of the Restart button.
        - isOnTimer() --> Verifies if the cursor is on top of the Timer zone.
        - onTopCloseEG() --> Verifies if the cursor is on top of the close button in the end-game screen.
        - onTopRestart_EG() --> Verifies if the cursor is on top of the Restart button of the end-game screen.
	
	Notes: *None*

    --> ZONES
            First column: [93 - 154]
            Second column: [185 - 246]
            Third column: [277 - 338]
            Fourth column: [369 - 430]
            Fifth column: [461 - 522]
            Sixth column: [553 - 614]
            Seventh column: [645 - 706]
*/

/**
 * @brief - Updates the cursor position, taking into consideration situations where the cursor might go beyond the screen limits.
 * 
 * @return - No return (void).
*/
void updateCursorPos();

/**
 * @brief - Determine whether or not the cursor is in a "DROP CHIP" zone.
 * 
 * @param col - Pointer to an integer which will store the column in which the cursor is: [0,1,2,3,4,5,6]. (-1 if it is not in a zone)
 * 
 * @return - Returns 1 if the cursor is in range. 0 if not.
 */
int isInZone(int* col);

/**
 * @brief - Deletes the cursor and reaplies the background surrounding it. (30 x 40 zone)
 * 
 * @param type - Type of background being replaced. 1 for the table/menu background. 2 for chips, only. 3 for everything.
 *               4 for the background of the blue_winner. 5 for the red one.
 * 
 * @return - No return.
 */
void deleteCursor(int type);

/**
 * @brief - Dictates whether or not the cursor is precisely on top of the 'Restart button'.
 * 
 * @param precise - If set, it will only return 1 if and only if the cursor values (x, and y) are inside the icon.
 * 
 * @return - 1 if positive. 0 if negative.
 */
int isONRestart(int precise);

/**
 * @brief - Dictates whether or not the cursor is on top of the 'Timer' (the coordinates do not need to be on top).
 * 
 * @return - 1 if positive. 0 if negative.
 */
int isONTimer();

/**
 * @brief - Detects whether or not the cursor is on top of the Close button.
 * 
 * @return - 1 if it is on top. 0 otherwise.
*/
int onTopClose_EG();

/**
 * @brief - Detects whether or not the cursor is on top of the Restart_EG button.
 * 
 * @param precise - If set, it will only return 1 if and only if the cursor values (x, and y) are inside the icon.
 * 
 * @return - 1 if it is on top. 0 otherwise.
*/
int onTopRestart_EG(int precise);
