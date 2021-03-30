#ifndef KBD_ESSENTIALS_H
#define KBD_ESSENTIALS_H

/* ********************************************* DEFINITIONS ********************************************* */

#define ZERO_NULL 0                                     // Null value
#define KBD_IRQ 1                                       // Keyboard's IRQ
#define ESC_BREAK 0x81                                  // ESC key's break code
#define LINGER_TIME 20000                               // Lingering time

#define OUT_FULL BIT(0)                                 // Bit stating OUT_FULL

#define ERROR_CHECK BIT(7) | BIT(6)                     // Bits related to ERRORS
#define AUX_ERROR_CHECK BIT(7) | BIT(6) | BIT(5)        // Bits related to ERRORS + Mouse
#define BREAK_MAKE BIT(7)                               // Bit defining break from make code

#define STAT_REG 0x64                                   // Stat register
#define OUT_BUF 0x60                                    // Out_buf register
#define TWOBYTE_SCANCODE 0xE0                           // Byte that begins 2-byte codes
#define IBF_BIT BIT(1)                                  // Bit related to Input buffer

/* ******************************************************************************************************* */


/* ********* Prototypes definitions - Brief description ********* */


/**
 * @brief Subscribes and enables keyboard interrupts
 *
 * @return Returns hook_id value, or -1 in case of error
 */
int keyboard_subscribe_intr(); 


/**
 * @brief Unsubscribes keyboard interrupts
 *
 * @return Returns 0 upon success or -1 in case of failure.
 */
int keyboard_unsubscribe_intr();


/**
 * @brief Reads the key and then proceeds to print the information related to it.
 *
 * @param key_obtained 8 bit variable that contains the a byte related to a given
 *         key that has been read in 'kbc_ih' already.
 * 
 * @return Always returns zero.
 */
int keyboard_key_scan(uint8_t key_obtained);

/**
 * @brief On function test_poll it reenables the keyboard interrupts
 * 
 * @return Always returns zero.
 */
int keyboard_reenable_int();

/**
 * @brief Lingers the program repeatedly until OUT_BUF is full. This function
 *          works simultaneously with 'kbc_ih'.
 * 
 * @return NULL return.
 */
void linger();

/**
 * @brief Keyboard interrupt-handler. Immediately checks the status of the STAT_REG.
 *          If it states OUT_BUF as full, extracts the key in it.
 *          Else, lingers the function and waits for a different status report.
 * 
 * @return NULL return. All variables are instead deposited in global ones.
 */
void (kbc_ih)();


/* ------------------------------------------ */

#endif /* KEYBOARD_H */

int keyboard_detect_break();
