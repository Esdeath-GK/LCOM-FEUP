#include <lcom/lcf.h>
#include <machine/int86.h>

// Standard Libraries:
#include "rtc.h"
#include "proj_essentials.h"
#include "videocard_essentials.h"
#include "graphic_management.h"

// Images and additional '.h' inclusions:
#include "xpm_variables.c"

// Global and Extern variables.
int rtc_hook_id = 21;
extern uint8_t* secondBuffer;           // [proj_essentials.c - l.##] Pointer used to store space for double buffering.


uint32_t rtc_subscribe_int() {

	int hook_temp = rtc_hook_id;

	if(sys_irqsetpolicy(RTC_IRQ, IRQ_REENABLE, &rtc_hook_id) != OK){
		printf("Error setting up RTC interruptions.\n");
		return -1;
	}

	return BIT(hook_temp);
}

int rtc_unsubscribe_int(){

	if (sys_irqrmpolicy(&rtc_hook_id) != OK){
		return -1;
	}

	return 0;
}

uint8_t read_rtc(uint32_t reg){
	uint8_t temp;

	sys_outb(RTC_ADDR_REG, reg);
	util_sys_inb(RTC_DATA_REG, &temp);

	return temp;
}

void write_rtc(uint32_t reg, uint32_t byte){

	sys_outb(RTC_ADDR_REG, reg);
	sys_outb(RTC_DATA_REG, byte);

}

int isBCD() {

	uint8_t RegB = read_rtc(RTC_REG_B);

	if (!(RegB & RTC_REG_B_DM)){
		return 1;
	}
	else{
		return 0;
	}

}

uint8_t BCDtoBinary(uint8_t* inBCD) {
	unsigned long binary;

	binary = (((*inBCD) & 0xF0) >> 4) * 10 + ((*inBCD) & 0x0F);

	return binary;
}

void getDate(uint8_t* day, uint8_t* month, uint8_t* year) {

	*day = read_rtc(RTC_DAY);
	*month = read_rtc(RTC_MONTH);
	*year = read_rtc(RTC_YEAR);

	if (isBCD()) {
		(*day) = BCDtoBinary(day);
		(*month) = BCDtoBinary(month);
		(*year) = BCDtoBinary(year);
	}

	return;
}

void getHour(uint8_t* hour, uint8_t* minutes, uint8_t* seconds) {

	*hour = read_rtc(RTC_HOURS);
	*minutes = read_rtc(RTC_MINUTES);
	*seconds = read_rtc(RTC_SECONDS);

	if (isBCD()) {
		(*hour) = BCDtoBinary(hour);
		(*minutes) = BCDtoBinary(minutes);
		(*seconds) = BCDtoBinary(seconds);
	}
	
	return;
}



// ---------------- [ORGANIZATION / OPTIMIZATION] ---------------- 


void setup_RTC(){

	// Extract current configuration of the REGISTER_B [RTC]
		uint8_t regBstatus = read_rtc(RTC_REG_B);
		regBstatus = regBstatus & 0x1F;         /* Set bits 7, 6 and 5 to zero */
		regBstatus = regBstatus | BIT(4);       /* Set bit 4 to one */
		write_rtc(RTC_REG_B, regBstatus);

	read_rtc(RTC_REG_C);

	return;
}

curr_date update_date(){
	curr_date menudate; //Struct containing the current date & time.
	getDate(&menudate.day, &menudate.month, &menudate.year);
	getHour(&menudate.hours, &menudate.minutes, &menudate.seconds);
	read_rtc(RTC_REG_C);

	return menudate;
}
