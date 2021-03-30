/**************************************************************/
/*****************  PROJ ESSENTIALS FUNCTIONS  ****************/
/**************************************************************/

/* Description:
    The following .c/.h module is, as the file itself states, called 'proj_essentials'. 
    It contains various essential functions to manage the project as a whole.
	
	Notes: This file contains all necessary macros.
*/

// State machine dictating the state of the program (whether it is in the menu or not)
typedef enum{
    CURRENTLY_IN_MENU,
    CURRENTLY_IN_GAME,
}program_state;

// State machine dictating the state of the game (the project's report contains the state diagram)
typedef enum{
    BLUE_PLAYING,
    RED_PLAYING,
    PUTTING_CHIP,
    REVEALING_PATH,
    RED_WON,
    BLUE_WON,
    CRITICAL_ERROR,
}game_state;


// Utility Macros:
#define FRAME_RATE 60                       // Frame Rate applied to the screen while the chip is falling.
#define DEFAULT_SECONDS 15                  // Time limit for the user to play. If he doesn't, his turn is skipped.
#define PAUSE 1                             // Determines whether or not the clock disappears for a brief while when the timer expires.
#define TICKS_VICTORIOUS 30                 // Ticks between frames to show the victory path.
#define ROWS 6                              // [DON'T CHANGE] Number of Rows in the table.
#define COLUMNS 7                           // [DON'T CHANGE] Number of Columns in the table.


// (Keyboard) Key codes
#define ARROW_UP 0x48                       // Up Arrow (makecode)
#define ARROW_DOWN 0x50                     // Down Arrow (makecode)
#define RIGHT_ARROW_SC 0x4d                 // Right Arrow scancode (makecode)
#define LEFT_ARROW_SC 0x4b                  // Left Arrow scancode (makecode)
#define A_SCANCODE 0x1e                     // 'A' key scancode (makecode)
#define D_SCANCODE 0x20                     // 'D' key scancode (makecode)
#define R_SCANCODE 0x13                     // 'R' key scancode (makecode)
#define Q_SCANCODE 0x10                     // 'Q' key scancode (makecode)
#define ENTER_SCANCODE 0x1c                 // ENTER scancode (makecode)
#define SPACE_SCANDCODE 0x39                // SPACE scancode (makecode)


// Distance values (don't modify any)
#define DISTANCE_LEFT 93                    // Leftmost hole's distances to the left corner of the screen.
#define DISTANCE_TOP 23                     // Topmost hole's distances to the header box indicating the turns.
#define DISTANCE_HOLESX 31                  // Distance between each hole in the x axis.
#define DISTANCE_HOLESY 13                  // Distance between each hole in the y axis.
#define CHIP_DIAMETER 61                    // Chip's diameter
#define HEADER_SIZE 60                      // Height of the header (banner that indicates the turn).

#define ERR_MARGIN_X 10                     // [END-SCREEN] Error margin in x axis used based on photoshop's given distances (image blank space)
#define ERR_MARGIN_Y 4                      // [END-SCREEN] Error margin in y axis used based on photoshop's given distances (image blank space)
#define BUTTONS_X 307 - ERR_MARGIN_X        // [END-SCREEN] Button's position in the X axis.
#define BUTTON_PLAY_Y 179 - ERR_MARGIN_Y    // [END-SCREEN] Play button's position in the Y axis
#define BUTTON_EXIT_Y 281 - ERR_MARGIN_Y    // [END-SCREEN] Exit button's position in the Y axis

#define STARTING_DATE_X 120                 // [Menu] Beginning of the date in the X coordinate.
#define STARTING_DATE_Y 520                 // [Menu] Beginning of the date in the Y coordinate.
#define STARTING_DATE2_X 145                // [Menu] Beginning of the time in the X coordinate.
#define STARTING_DATE2_Y 565                // [Menu] Beginning of the time in the Y coordinate.
#define DATE_SPACING_HORIZONTAL 1           // [Menu] Spacing between each number in the date and time.
#define DATE_NUMBER_WIDTH 22                // [Menu] Width of the smaller numbers.

#define INITIAL_COLUMN_PIXEL 93             // Position of the first column (x direction)
#define COLUMN_WIDTH 61                     // Width of the columns
#define COLUMN_SPACING 31                   // Spacing between each column.

#define REBOOT_X (81 + 570)                 // Restart button's location in the X direction.
#define REBOOT_Y (526+5)                    // Restart button's location in the Y direction.
#define REBOOT_W 60                         // Restart button's width
#define REBOOT_H 60                         // Restart button's height

#define CUR_W 30                            // Cursor approximate width
#define CUR_H 40                            // Cursor approximate height
#define CUR_W_REAL 21                       // Cursor real width
#define CUR_H_REAL 37                       // Cursor real height

#define TIMER_REGION_X 315                  // Timer Region's coordinates in the x direction
#define TIMER_REGION_Y 549                  // Timer Region's coordinates in the y direction
#define SPACING 0                           // Spacing between the timer.
#define TIMER_REGION_WIDTH (30 + 30 + (SPACING))    // Timer Region Width
#define TIMER_REGION_HEIGHT 40              // Timer Region Height
#define NUMBER_WIDTH 25                     // Width of each number

#define SD_X 95                             // Ending screen's SHUT DOWN X coordinate.
#define SD_Y 100                            // Ending screen's SHUT DOWN Y coordinate
#define SD_WIDTH 60                         // Ending screen's SHUT DOWN Width.
#define SD_HEIGHT 60                        // Ending screen's SHUT DOWN Height.

#define RESTART_EG_X 635                    // Ending screen's Restart button X coordinate.
#define RESTART_EG_Y 100                    // Ending screen's Restart button Y coordinate.




/* *************************************************************************************************************************  */
/* ************************************************ ESSENTIAL FUNCTIONS **********************************<+****************  */
/* *************************************************************************************************************************  */



/**
 * @brief - Floor function.
 * 
 * @param value - Value.
 * 
 * @return - Integer floor.
 */
int floor(float value);

/**
 * @brief - Calculates the necessary ticks for the given frame_rate.
 * 
 * @param frame_rate - The frame_rate.
 * 
 * @return - The ticket count.
*/
uint32_t ticks_fr(float frame_rate);

/**
 * @brief - Begins the game, making full use of the mouse + keyboard + timer interruptions.
 * 
 * @return - No return. Terminates when the user presses the ESC key, or when one of the player wins the game.
 */
void project_core(void *address);

/**
 * @brief - Processes the victory of the game, showing the path to victory 
 * 
 * @return - None.
*/
void victory_process();

/**
 * @brief - Change current player
 * 
 * @return - None.
*/
void changePlayer();

/**
 * @brief - Set starter player
 * 
 * @return - None.
*/
void startPlayer();

/**
 * @brief - Get the current player 
 * 
 * @return - The player
*/
int getCurrentPlayer();

/**
 * @brief - Set ChangeTurn
 * 
 * @param - Value to be set on ChangeTurn
 * 
 * @return - None.
*/
void setChangeTurn(int valor);

/**
 * @brief - Get the ChangeTurn
 * 
 * @return - The value of ChangeTurn
*/
int getChangeTurn();

/**
 * @brief - Set Remaining Seconds with param
 * @param - Value to set remainingSeconds
 * @return - void
*/
void setRemainingSeconds(int valor);

/**
 * @brief - Get value of RemainingSeconds
 * @return - RemainingSeconds
*/
int getRemainingSeconds();
