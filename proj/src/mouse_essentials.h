#ifndef MOUSE_ESSENTIALS_H
#define MOUSE_ESSENTIALS_H

/* ********************************************* DEFINITIONS ********************************************* */

#define ZERO_NULL 0                                     // Null value
#define MOUSE_IRQ 12                                    // Mouse's IRQ line
#define LINGER_TIME 20000                               // Lingering time
#define TRIES 100                                       // Number of attempts until any loop is closed
#define WRITE_MOUSE 0xD4                                // Address (sent to 0x64) to send a byte to MOUSE (arguments on 0x60)
#define SET_STREAM 0xEA                                 // Sets stream mode on mouse
#define SET_REMOTE 0xF0                                 // Sets remote mode on mouse
#define ENABLE_DATA 0xF4                                // Sets DATA REPORT for mouse
#define DISABLE_DATA 0xF5                               // Disables DATA REPORT on mouse
#define TICKS_PER_SECOND sys_hz()                       // Number of ticks per second
#define MILI 1000                                       // Multiplier from Mili
#define BACKUP_LIMIT 10                                 // Defines the number of packets with MMB pressed down required.

#define OUT_FULL BIT(0)                                 // Bit stating OUT_FULL

#define ERROR_CHECK BIT(7) | BIT(6)                     // Bits related to ERRORS
#define AUX_ERROR_CHECK BIT(7) | BIT(6) | BIT(5)        // Bits related to ERRORS + Mouse
#define BREAK_MAKE BIT(7)                               // Bit defining break from make code

#define KBC_CMD_REG 0x64                                   // Stat register
#define OUT_BUF 0x60                                    // Out_buf register
#define TWOBYTE_SCANCODE 0xE0                           // Byte that begins 2-byte codes
#define IBF_BIT BIT(1)                                  // Bit related to Input buffer

#define ACK 0xFA                                        // ACKNOWLEDGEMENT Response byte
#define NACK 0xFE                                       // NOT-ACKNOWLEDGEMENT Response byte
#define ERROR 0xFC                                      // ERROR Response byte
#define B_COUNT 3                                       // Package byte quantity

#define IBF BIT(1)
#define OBF BIT(0)
#define TO_ERR BIT(6)
#define PAR_ERR BIT(7)
#define MOUSE_EN_DATA 0xF4


#define CHECK_BIT(var,pos) (((var)>>(pos)) & 1)         // Checks BIT and returns 1 if the BIT is the one marked.
#define FIRST_NEGATIVE 0xFF00
#define FIRST_POSITIVE 0x0000

/* ******************************************************************************************************* */



typedef enum {INIT, DRAW_L, DRAW_R, COMP_L, COMP_R} state_t;
typedef enum {RDOWN, RUP, LDOWN, LUP, MOVE, MOVE_ERROR,} event_t;



/* ********* Prototypes definitions - Brief description ********* */


/**
 * @brief Subscribes and enables mouse interrupts
 *
 * @return Returns hook_id value, or -1 in case of error
 */
int mouse_subscribe_intr(); 


/**
 * @brief Unsubscribes mouse interrupts
 *
 * @return Returns 0 upon success or -1 in case of failure.
 */
int mouse_unsubscribe_intr();

/**
 * @brief Function that loops the program until the INPUT buffer is empty
 *
 * @return Returns 0 if it does not exceed the number of 'TRIES'. -1 otherwise.
 */
int IBF_full();

/**
 * @brief Mouse's Interrupt Handler
 *
 * @return Returns NULL.
 */
void (mouse_ih_proj)(uint8_t *mousemove);

/**
 * @brief Parses the package and prints it.
 *
 * @return Returns NULL.
 */
void Mpackage_parse(uint8_t vector[]);

/**
 * @brief Lingers the program repeatedly until OUT_BUF is full.
 * 
 * @return NULL return.
 */
void linger();

/**
 * @brief Manages the state machine.
 * 
 * @return NULL return.
 */
void state_machine();


/* ***** vv Mouse-management functions vv ***** */



/**
 * @brief Appropriately sends a command to the mouse and reads it.
 * 
 * @return Returns -1 in case of unsuccess, or 0 otherwise.
 */
int mouse_send_command(uint32_t send_command);

/**
 * @brief Disables or enables interruptions based on its unique argument.
 * 
 * @param enable Dictates whether or not mouse interruptions should enable. 1 for yes. 0 for no.
 * 
 * @return Returns -1 in case of unsuccess, or 0 otherwise.
 */
int mouse_interruptions(int enable);

/**
 * @brief Reads the mouse data.
 * 
 * @return Returns -1 in case of unsuccess, or 0 otherwise.
 */
int read_Mouse();

/**
 * @brief Writes an argument (or command) to mouse
 * 
 * @return Returns -1 in case of unsuccess, or 0 otherwise.
 */
int writeto_Mouse(uint8_t cmd, uint8_t port);

/**
 * @brief Writes a command to the KBC Register.
 * 
 * @return Returns -1 in case of unsuccess, or 0 otherwise.
 */
int writeto_Controller(uint8_t cmd);

/**
 * @brief Appropriately manages the command, sending it to the mouse.
 * 
 * @return Returns -1 in case of unsuccess, or 0 otherwise.
 */
int mouse_manageCommand(uint8_t cmd);

/**
 * @brief Return all package received from mouse
 * 
 * @return Mouse package.
 */
struct packet getMousePackage();


/* ------------------------------------------ */

#endif /* MOUSE_H */
