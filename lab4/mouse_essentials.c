#include <lcom/lcf.h>
#include "i8254.h"

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include "utils.c"
#include "mouse_essentials.h"
#include "timer.h"

int Mhook_id = 18;
int policy_set = 0;
uint8_t extractB;
struct packet mousepackage;

int mouse_subscribe_intr(){
    uint8_t jhin = Mhook_id;

	if (sys_irqsetpolicy(MOUSE_IRQ, IRQ_REENABLE | IRQ_EXCLUSIVE, &Mhook_id) != OK) {
		printf("There was an error setting up mouse's interrupt-subscription ...\n");
		return -1;
	}

    policy_set = 1;

	return BIT(jhin);
}

int mouse_interruptions(int enable){
    if(policy_set != 1){
        printf(":: ERROR:: Mouse interruption policy has not been set yet ...\n");
        return -1;
    }

    switch(enable){
        case 0:
            if (sys_irqdisable(&Mhook_id) != OK) {
                printf("There was an error executing sys_irqrmpolicy of mouse_interruptions ...\n");
                return -1;
            }
            break;
        case 1:
            if (sys_irqenable(&Mhook_id) != OK) {
                printf("There was an error executing sys_irqenable of mouse_interruptions ....\n");
                return -1;
            }
            break;
    }

    return 0;
}

int mouse_unsubscribe_intr(){

    if (sys_irqdisable(&Mhook_id) != OK) {
		printf("There was an error executing sys_irqdisable of mouse_unsubscribe_intr ...\n");
		return -1;
	}

	if (sys_irqrmpolicy(&Mhook_id) != OK) {
		printf("There was an error executing sys_irqrmpolicy of mouse_unsubscribe_intr ...\n");
		return -1;
	}

    policy_set = 0;

	return 0;
}


void linger(){
	tickdelay(micros_to_ticks(LINGER_TIME));
}

void (mouse_ih)(){
    
    uint8_t KBC_status;

    while (1) {
		util_sys_inb(KBC_CMD_REG, &KBC_status);

		if (KBC_status & OUT_FULL) {
			util_sys_inb(OUT_BUF, &extractB);

			if ((extractB & (ERROR_CHECK)) == 0){
				return;
			}
			else{
				return;
			}
		}

		// Infinite loop until the OUTPUT is no longer empty.
		linger();
	}

}

void Mpackage_parse(uint8_t vector[]){

    // Place the bytes inside the structure in the respective order.
    for(int i = 0; i < B_COUNT; i++){
        mousepackage.bytes[i] = vector[i];
    }

    // Parse left mouse button
    if( CHECK_BIT(vector[0],0) ){
        mousepackage.lb = true;
    }
    else{
        mousepackage.lb = false;
    }

    // Parse right mouse button
    if( CHECK_BIT(vector[0],1) ){
        mousepackage.rb = true;
    }
    else{
        mousepackage.rb = false;
    }

    // Parse middle mouse button
    if( CHECK_BIT(vector[0],2) ){
        mousepackage.mb = true;
    }
    else{
        mousepackage.mb = false;
    }

    // Check for X overflow and register the info
    if( CHECK_BIT(vector[0],6) ){
        mousepackage.x_ov = true;
    }
    else{
        mousepackage.x_ov = false;
    }

    // Check for Y overflow and register the info
    if( CHECK_BIT(vector[0],7) ){
        mousepackage.y_ov = true;
    }
    else{
        mousepackage.y_ov = false;
    }

    uint16_t full_value = 0;
    uint16_t signal;

    // Analyze delta X
    if( CHECK_BIT(vector[0],4) ){
        // Delta X was marked negative

        signal = FIRST_NEGATIVE;
        full_value = signal | vector[1];
            /*
            printf("Delta X was marked negative!!\n");
            printf("  ---> ::DEBUG:: - Standard byte: 0x%x (%d);\n", vector[1], vector[1]);
            printf("  ---> ::DEBUG:: - Added byte: 0x%x (%d);\n", 0xFF00, 0xFF00);
            printf("  ---> ::DEBUG:: - Final byte: 0x%x (%d);\n", full_value, full_value);
            printf("  ---> ::DEBUG:: - DELIVERED byte: 0x%x (%d);\n", (full_value ^ 0xFFFF) + 1, -((full_value ^ 0xFFFF) + 1));
            */
        mousepackage.delta_x = -((full_value ^ 0xFFFF) + 1);
    }
    else{
        mousepackage.delta_x = vector[1];
    }

    // Analye Delta Y
    if( CHECK_BIT(vector[0],5) ){
        // Y was marked negative
        signal = FIRST_NEGATIVE;
        full_value = signal | vector[2];
        mousepackage.delta_y = -((full_value ^ 0xFFFF) + 1);
    }
    else{
        mousepackage.delta_y = vector[2];
    }

    mouse_print_packet(&mousepackage);
}

/* *************************************************************************** */
/* *************************************************************************** */
/* *************************************************************************** */
/* *****************             STATE MACHINE               ***************** */
/* *************************************************************************** */
/* *************************************************************************** */
/* *************************************************************************** */
/* *************************************************************************** */

// Crucial variables

state_t current = INIT;     /* Defines an enum for states */
event_t event_enum;         /* Defines an enum for events */
int total_x = 0;            /* Total amount of units the mouse has moved over the x (horizontal) axis */
int total_y = 0;            /* Total amount of units the mouse has moved over the y (vertical) axis */
float slope;                /* Line slope (delta_y/delta_x) */
int complete = 0;           /* Variable that determines whether or not the [state] machine has reached a completed status */
uint8_t x_lenmin;           /* Minimum x-displacement required */
uint8_t slope_tol = 5;      /* Tolerating events in a row where the mouse might slightly move just to the right or upwards. */

void state_machine(){

    switch(current){
        case INIT:
            if(event_enum == LDOWN){
                //printf("--> Program progressed to DRAW_L state\n");
                current = DRAW_L;
            }
            break;
        
        case DRAW_L:
            if(event_enum == MOVE_ERROR){
                //printf("--> Program returned to INIT state\n");
                current = INIT;
            }
            else if(event_enum == LUP){
                if(total_x >= x_lenmin){
                    //printf("--> x_min satisfied. Progressed to COMP_L\n");
                    current = COMP_L;
                    total_x = 0;
                    total_y = 0;
                }
                else{
                    //printf("--> x_min not satisfied. Moved back to INIT\n");
                    current = INIT;
                }
            }
            break;
        case COMP_L:
            if(event_enum == RDOWN){
                //printf("--> State progressed to DRAW_R\n");
                current = DRAW_R;
            }
            else if(event_enum == LDOWN){
                //printf("--> LDOWN was re-pressed in COMP_L returning to INIT.\n");
                current = DRAW_L;
            }
            break;
        case DRAW_R:
            if(event_enum == MOVE_ERROR){
                //printf("--> Program returned to INIT state !!FROM DRAW_R!!\n");
                current = INIT;
            }
            else if(event_enum == RUP){
                if(total_x >= x_lenmin){
                    //printf("--> x_min satisfied. Progressed to COMP_L.\n");
                    current = COMP_R;
                    total_x = 0;
                    total_y = 0;
                    complete = 1;
                }
                else{
                    //printf("--> x_min not satisfied. Moved back to INIT.\n");
                    current = INIT;
                }
            }
            break;
        default:
            break;
    }

}


void check_event(uint8_t tolerance, uint8_t x_len){

    x_lenmin = x_len;

    // Elemental case --> RMB or LMB simultaneously pressed OR MMB pressed OR all buttons pressed.
        if( mousepackage.mb == true || (mousepackage.lb == true && mousepackage.rb == true) ){
            //printf("--> (Elemental case) - More than one button pressed\n");
            event_enum = MOVE_ERROR;
            state_machine();
        }

/* INIT CASES */

    // Left mouse button is pressed
        else if(current == INIT && mousepackage.lb == true){
            //printf("--> (Initial state + LMB pressed)\n");
            event_enum = LDOWN;
            state_machine();
        }
        else if(current == DRAW_L && mousepackage.lb == false){
                //printf("--> (LMB released)\n");
            event_enum = LUP;
            state_machine();
        }
        else if(current == DRAW_L && mousepackage.lb == true){
            // Analizar movimento
                int abs_deltax = mousepackage.delta_x;
                int abs_deltay = mousepackage.delta_y;

                // Delta X menor que zero
                if(mousepackage.delta_x < 0){
                    abs_deltax = abs_deltax * -1;
                    // Determinar se delta_x respeita a tolerância
                    if(abs_deltax > tolerance){
                        //printf("--> Absolute value of negative delta x higher than tolerance.\n");
                        event_enum = MOVE_ERROR;
                        state_machine();
                    }
                }
                else{
                    total_x = total_x + abs_deltax;
                }

                // Delta Y menor que zero
                if(mousepackage.delta_y < 0){
                    abs_deltay = abs_deltay * -1;
                    // Determinar se delta_x respeita a tolerância
                    if(abs_deltay > tolerance){
                        //printf("--> Absolutele value of negative y higher than tolerance\n");
                        event_enum = MOVE_ERROR;
                        state_machine();
                    }
                }
                else{
                    total_y = total_y + abs_deltay;
                }
                
            // SLOPE VALUE
                if(abs_deltax != 0){
                    slope = abs_deltay / abs_deltax;
                }
                else{
                    slope = -1;
                }
                if(slope < 1){
                    if(slope_tol > 0){
                        slope_tol--;
                    }
                    else{
                        //printf("--> Slope lower than one\n");
                        complete = 1;
                        event_enum = MOVE_ERROR;
                        state_machine();
                    }
                }
                else{
                    slope_tol = 5;
                }
            
        }
        else if(current == COMP_L && mousepackage.lb == true){
            event_enum = LDOWN;
            state_machine();
        }
        else if(current == COMP_L && mousepackage.rb == true){
            //printf("--> RDOWN pressed while in COMP_L\n");
            event_enum = RDOWN;
            state_machine();
        }
        else if(current == DRAW_R && mousepackage.rb == true){
            // Analizar movimento
                int abs_deltax = mousepackage.delta_x;
                int abs_deltay = mousepackage.delta_y;

                // Delta X menor que zero
                if(mousepackage.delta_x < 0){
                    abs_deltax = abs_deltax * -1;
                    // Determinar se delta_x respeita a tolerância
                    if(abs_deltax > tolerance){
                        event_enum = MOVE_ERROR;
                        state_machine();
                    }
                }
                else{
                    total_x = total_x + abs_deltax;
                }

                // Delta Y menor que zero
                if(mousepackage.delta_y > 0){
                    // Determinar se delta_x respeita a tolerância
                    if(abs_deltay > tolerance){
                        event_enum = MOVE_ERROR;
                        state_machine();
                    }
                }
                else{
                    total_y = total_y + abs_deltay;
                }

            // SLOPE VALUE
                if(abs_deltax != 0){
                    slope = abs_deltay / abs_deltax;
                }
                else{
                    slope = 3;
                }
                if(slope > -1){
                    if(slope_tol > 0){
                        //printf("--> SLOPE NOT ACCEPTED: (%d / %d = %d)\n", abs_deltay, abs_deltax, slope);
                        //printf("--> slope tolerance reduced (%d)\n", slope_tol-1);
                        slope_tol--;
                    }
                    else{
                        //printf("--> Incorrect slope while in DRAW_R!!!\n");
                        //printf("--> SLOPE: (%d / %d = %d)\n", abs_deltay, abs_deltax, slope);
                        complete = 1;
                        event_enum = MOVE_ERROR;
                        state_machine();
                    }
                }
                else{
                    //printf("Slope accepted (%d / %d = %d)\n", abs_deltay, abs_deltax, slope);
                    slope_tol = 5;
                }
        }
        else if(current == DRAW_R && mousepackage.rb == false){
            //printf("--> RMB released while in DRAW_R\n");
            event_enum = RUP;
            state_machine();
        }

}


/* *************************************************************************** */
/* *************************************************************************** */
/* *************************************************************************** */
/* *****************       MOUSE-MANAGEMENT FUNCTIONS        ***************** */
/* *************************************************************************** */
/* *************************************************************************** */
/* *************************************************************************** */
/* *************************************************************************** */


int mouse_send_command(uint32_t command){

    if(policy_set == 1){
        mouse_interruptions(0);
    }
    if (writeto_Controller(WRITE_MOUSE) != 0) {
		printf("ERROR: writeto_Controller failed.\n");
		return -1;
	}
	if (mouse_manageCommand(command) != 0) {
		printf("ERROR: mouse_manageCommand failed.\n");
	}
    if(policy_set == 1){
        mouse_interruptions(1);
    }

    return 0;
}

int read_Mouse() {
	uint8_t stat;
	uint8_t data;
	int numTries = TRIES;

	while (numTries > 0) {
		util_sys_inb(KBC_CMD_REG, &stat); /* assuming it returns OK */
		/* loop while 8042 output buffer is empty */
		if (stat & OBF) {
			util_sys_inb(OUT_BUF, &data); /* assuming it returns OK */
			if ((stat & (PAR_ERR | TO_ERR)) == 0) {
				return data;
			} else
				return -1;
		}
		numTries--;
		linger();
	}

    return 0;
}

int writeto_Mouse(uint8_t cmd, uint8_t port){
	uint8_t stat;
	int numTries = TRIES;

	while (numTries > 0) {
		util_sys_inb(KBC_CMD_REG, &stat); /* assuming it returns OK */

		/* loop while 8042 input buffer is not empty */
		if ((stat & IBF) == 0) {
			sys_outb(port, cmd); /* no args command */
			return 0;
		}
		numTries--;
		linger();
	}

    return 0;
}

int writeto_Controller(uint8_t cmd){

	uint8_t stat;
	int numTries = TRIES;

	while (numTries > 0) {
		util_sys_inb(KBC_CMD_REG, &stat); /* assuming it returns OK */

		/* loop while 8042 input buffer is not empty */
		if ((stat & IBF) == 0) {
			sys_outb(KBC_CMD_REG, cmd); /* no args command */
			return 0;
		}
		numTries--;
		linger();
	}

    return -1;

}

int mouse_manageCommand(uint8_t cmd) {
	uint8_t data;
    int attempts = TRIES;

    do{
        do{
            if (writeto_Mouse(cmd, OUT_BUF) != 0){
                //printf("Error was returned here ...");
                return -1;
            }

            data = read_Mouse();

            if(data != ACK && data != NACK && data != ERROR){
                // Command byte registered in the platform
                //printf("!! read_Mouse() WARNING - Package was being sent ... Implementing workarounds.\n");
                break;
            }

            attempts--;

        }while(data == NACK && attempts > 0);

        while(data != ACK && data != NACK && data != ERROR && attempts > 0){
            //printf("!! read_Mouse() WARNING - Packet was not RESPONSE - It was 0x%x ... Ignoring ...\n  --> (Expected: 0xFA | 0xFE | 0xFC)\n", data);
            data = read_Mouse();
            attempts--;
        }
    }while( data != ACK && data != ERROR && attempts > 0 );


	if (data == ACK){
		return 0;
    }
	else{
		return -1;
    }
}

