/**************************************************************/
/******************  RTC MANAGEMENT FUNCTIONS  ****************/
/**************************************************************/

/* Description:
    The following .c/.h module is, as the file itself states, called 'rtc'. 
    Its main utility is to make available crucial functions for the management of the Real-Time Clock (RTC) I/O Device.
    
    Four functions will be taken consideration:
        - rtc_subscribe_int() --> Subscribes RTC interruptions.
        - rtc_unsubscribe_int() --> Unsubscrubes RTC interruptions.
        - read_rtc() --> Reads a given register of the RTC.
        - write_rtc() --> Writes a 8-bit value to a given register of the RTC.
        - isBCD() --> Determines if the RTC writes the variables in BCD instead of Binary by reading corresponding register (B).
        - BDCtoBinary() -> Converts a BCD-written value to a Binary-written one.
        - getDate() --> Extracts the date from the respective registers of the RTC
        - getHour() --> Extracts the hour from the respective registers of the RTC
        - setup_RTC() --> Sets up the RTC to enable interrupts on update.
        - update_date() --> Updates the date structure.
	
	Notes: None.
*/


// Values definitions:

#define RTC_IRQ                 8       // RTC IRQ Line
#define RTC_SECONDS				0       // RTC seconds register
#define RTC_MINUTES				2       // RTC minutes register
#define RTC_HOURS				4       // RTC hours register
#define RTC_DAY					7       // RTC day register
#define RTC_MONTH				8       // RTC month register
#define RTC_YEAR				9       // RTC year register
#define RTC_REG_A 				0x0A    // RTC register A
#define RTC_REG_B 				0x0B    // RTC register B
#define RTC_REG_C 				0x0C    // RTC register C 
#define RTC_REG_D 				0x0D    // RTC register D
#define RTC_ADDR_REG 			0x70    // RTC address register
#define RTC_DATA_REG 			0x71    // RTC data register

#define RTC_REG_A_UIP			BIT(7)  // UIP bit of Register A (RTC)
#define RTC_REG_B_DM			BIT(2)  // DM bit of Register B (RTC)


// Struct type that will contain the date and the time's values.
typedef struct{
    // Date
        uint8_t day;
        uint8_t month;
        uint8_t year;
    
    // Seconds
        uint8_t hours;
        uint8_t minutes;
        uint8_t seconds;
}curr_date;


// ---------------- [ESSENTIALS] ---------------- 



/**
 * @brief Subscribes RTC interruptions with an inicial hook_id given by: 21.
 * 
 * @return Returns a "null" 32 bit variable with the 21th bit set.
 */
uint32_t rtc_subscribe_int();

/**
 * @brief Unsubscribes RTC interruptions.
 * 
 * @return Returns -1 in case of failure. 0 if successfull.
 */
int rtc_unsubscribe_int();

/**
 * @brief Reads the value on a given register of the RTC that is sent as argument.
 * 
 * @param reg - Register that we wish to read.
 * 
 * @return Returns the read value from the given register.
 */
uint8_t read_rtc(uint32_t reg);

/**
 * @brief Writes a given byte to a given register of the RTC, both sent as arguments.
 * 
 * @param reg - Register that we wish to write the byte to.
 * 
 * @param byte - The byte we wish to pass.
 * 
 * @return No return (void).
 */
void write_rtc(uint32_t reg, uint32_t byte);

/**
 * @brief Checks whether or not the values of the RTC are in BCD through Register B.
 * 
 * @return 1 in case the values are BCD. 0 if they are in binary.
 */
int isBCD();

/**
 * @brief Converts values from BCD to binary.
 * 
 * @param inBCD - BCD value that will be converted to binary.
 * 
 * @return The converted value (in binary).
 */
uint8_t BCDtoBinary(uint8_t* inBCD);

/**
 * @brief Extracts the current date from the Real-Time Clock. The values are given in binary.
 * 
 * @param day - unsigned 8 bit pointer to the day variable (the 'day' value will be returned here).
 * 
 * @param month - unsigned 8 bit pointer to the month variable (the 'month' value will be returned here).
 * 
 * @param year - unsigned 8 bit pointer to the year variable (the 'year' value will be returned here).
 * 
 * @return No return (void).
 */
void getDate(uint8_t* day, uint8_t* month, uint8_t* year);

/**
 * @brief Extracts the current hour from the Real-Time Clock. The values are given in binary.
 * 
 * @param hour - unsigned 8 bit pointer to the hour variable (the 'hour' value will be returned here).
 * 
 * @param minutes - unsigned 8 bit pointer to the month variable (the 'minutes' value will be returned here).
 * 
 * @param seconds - unsigned 8 bit pointer to the year variable (the 'seconds' value will be returned here).
 * 
 * @return No return (void).
 */
void getHour(uint8_t* hour, uint8_t* minutes, uint8_t* seconds);


// ---------------- [ORGANIZATION / OPTIMIZATION] ---------------- 


/**
 * @brief 	Sets up the RTC inicial status by setting the Register B's 7th, 6th and 5th bits to zero and activating the 4th bit.
 * 			In short, the function enables time-updating and sets up the 'interruption-on-update' mode.
 * 
 * @return 	No return (void).
 * 
 * [ORGANIZATION]
 */
void setup_RTC();

/**
 * @brief 	Updates the current date (and hour!). 
 * 
 * @return  Path of date.
 * 
 * [ORGANIZATION]
 */
curr_date update_date();
