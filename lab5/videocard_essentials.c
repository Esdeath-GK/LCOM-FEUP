#include <lcom/lcf.h>
#include <machine/int86.h>
#include <lcom/xpm.h>
#include "i8254.h"

#include <stdbool.h>
#include <stdint.h>
#include "videocard_essentials.h"
#include "timer.h"
#include "kbd_essentials.h"

/* ********************************************************************************************* */

int success = 1;
vbe_mode_info_t vbe_info_struct;
void *video_mem;
xpm_image_t img;
extern uint8_t universal_key_scanned;
extern uint32_t counter;                

/* ****************************** AUXILIARY FUNCTIONS ****************************************** */

uint8_t assignSpecificBit(uint8_t original, uint8_t clipboard, int bitOriginal, int bitClipboard){
    int iszero;
    // Determine if the specific bit is zero or not in the original number
        iszero = (original & BIT(bitOriginal)) ? 0 : 1;
    
    if(iszero == 0){
        return ( (uint8_t) (clipboard | BIT(bitClipboard)) );
    }   
    else{
        return clipboard;
    }
}

uint8_t cleanbits(uint8_t original, uint8_t preserve){

    uint8_t start = 0;

    for(int i = 0; i < preserve; i++){
        start = start | BIT(i);
    }

    //printf("[NOTE] cleanbits: %d & %d = %d", original, start, (original & start));

    return ( (uint8_t) (original & start) );
}

int ceil(float num) {
    int inum = (int)num;
    if (num == (float)inum) {
        return inum;
    }
    return inum + 1;
}

uint32_t colorPattern(uint16_t row, uint16_t col, uint32_t original, uint8_t step, uint8_t no_rectangles){

    static uint8_t trans, red, green, blue;
    static int extracted = 0;

    if(vbe_info_struct.MemoryModel == 4){
        // Just one byte
            return ((original + (row * no_rectangles + col) * step) % (1 << vbe_info_struct.BitsPerPixel));
    }
    else{
            // Step one ... extract (if not done)
            if(extracted == 0){
                trans =     cleanbits( (uint8_t) (original >> vbe_info_struct.RsvdFieldPosition)   , vbe_info_struct.RsvdMaskSize);
                red =       cleanbits( (uint8_t) (original >> vbe_info_struct.RedFieldPosition)    , vbe_info_struct.RedMaskSize);
                green =     cleanbits( (uint8_t) (original >> vbe_info_struct.GreenFieldPosition)  , vbe_info_struct.GreenMaskSize);
                blue =      cleanbits( (uint8_t) (original >> vbe_info_struct.BlueFieldPosition)   , vbe_info_struct.BlueMaskSize);
                //printf("---> For the color 0x%x the extracted intensities were: Transparency: %d (0x%x) // Red: %d (0x%x) // Green: %d (0x%x) // Blue: %d (0x%x).\n", original, trans, trans, red, red, green, green, blue, blue);
                extracted = 1;
            }

            // Step two ... Obtain new colors
                uint8_t newRed, newGreen, newBlue;
                newRed = (uint8_t)((red + col * step) % (1 << vbe_info_struct.RedMaskSize));
                newGreen = (uint8_t)((green + row * step) % (1 << vbe_info_struct.GreenMaskSize));
                newBlue = (uint8_t)((blue + (col + row) * step) % (1 << vbe_info_struct.BlueMaskSize));
            
            // Step three ... Form new original color:

                uint32_t mergedColors;

                mergedColors = (newBlue | (newGreen << 8) | (newRed << 16) | (trans << 24));

                //printf("---> [FINAL: 0x%x] The new intensities were: Transparency: %d (0x%x) // Red: %d (0x%x) // Green: %d (0x%x) // Blue: %d (0x%x).\n", mergedColors, trans, trans, newRed, newRed, newGreen, newGreen, newBlue, newBlue);

            return mergedColors;
    }
}

/* **************************************** END OF AUXILIARY FUNCTIONS ************************************************* */




void* (vg_init)(uint16_t mode){

    int r;
    struct minix_mem_range mr; /*physical memory range*/

    memset(&vbe_info_struct, 0, sizeof(vbe_info_struct));

    vbe_extract_MODEinformation(mode, &vbe_info_struct);

    int total_size = vbe_info_struct.XResolution * vbe_info_struct.YResolution * ceil(vbe_info_struct.BitsPerPixel / (float)8);

    mr.mr_base = (phys_bytes) vbe_info_struct.PhysBasePtr;	
    mr.mr_limit = mr.mr_base + total_size;

    if( OK != (r = sys_privctl(SELF, SYS_PRIV_ADD_MEM, &mr))){
        panic("sys_privctl (ADD_MEM) failed: %d\n", r);
    }

    video_mem = vm_map_phys(SELF, (void *)mr.mr_base, total_size);

    if(video_mem == MAP_FAILED){
        panic("couldn't map video memory");
    }

    reg86_t reg86;
    memset(&reg86, 0, sizeof(reg86));
    reg86.intno = 0x10;             // BIOS's video services' interrupt handler.
    reg86.ah = INVOKE_AH;           // AH register must be set to 0x4F whenever we invoke a function.
    reg86.al = 0x02;                // AL register must be set to 0x02 whenever we wish to invoke function: Set VBE mode.
    reg86.bx = 1 << 14 | mode;      // Bit 14 must be set.
    if (sys_int86(&reg86) != OK) {
                printf("vg_init: sys_int86() has failed.\n");
                return NULL;
    }
    
    return video_mem;
}

int vbe_extract_MODEinformation(uint16_t mode, vbe_mode_info_t* vbe_extracted_MODEinfo){

    reg86_t reg86;
	mmap_t mapping;

    memset(&reg86, 0, sizeof(reg86));
    memset(&mapping, 0, sizeof(mapping));

	lm_alloc(MODEINFO_SIZE, &mapping);

	reg86.intno = 0x10;
	reg86.ah = INVOKE_AH;   // AH register must be set to 0x4F whenever we invoke a function.
    reg86.al = 0x01;        // AL register must be set to 0x01 whenever we wish to extract VBE info.
	reg86.es = PB2BASE(mapping.phys); /* set a segment base */
	reg86.di = PB2OFF(mapping.phys); /* set the offset accordingly */
	reg86.cx = mode;
	if (sys_int86(&reg86) != OK) { /* call BIOS */
		printf("vbe_extract_MODEinformation: sys_int86() has failed.\n");
		return -1;
	}

	memcpy(vbe_extracted_MODEinfo, mapping.virt, MODEINFO_SIZE);

	lm_free(&mapping);

	return 0;
}

int setPixel(uint16_t x, uint16_t y, uint32_t color, uint8_t byteVector[], void* address, int isIndexed) {

	if ((x < 0 || x >= vbe_info_struct.XResolution) || (y < 0 || y >= vbe_info_struct.YResolution)){ // input check
		return -1;
    }
    
    uint8_t *aux = address;
    int bytesPP = ceil(vbe_info_struct.BitsPerPixel / (float)8);
	
    // First step --> Move the address into the location (x,y)
        aux += (x + vbe_info_struct.XResolution * y) * (bytesPP);
    
    // Second step --> Begin attribution of colors
        if(isIndexed == 1){
	        *aux =  (uint8_t) color;
        }
        else if(isIndexed == 0){
            for(int g = 0; g < bytesPP; g++){
	            *(aux+g) = byteVector[g];
            }
        }
	return 0;

}

int make_rectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t color, void* address){
 
    if( (width+x) > vbe_info_struct.XResolution || (height+y) > vbe_info_struct.YResolution){
        printf("ERROR IN make_rectangle(): Rectangle measures go beyond the limits of the mode's resolution ...\n");
        return -1;
    }
    if( x < 0 || y < 0){
        printf("ERROR IN make_rectangle(): Rectangle measures go beyond the limits of the mode's resolution ...\n");
        return -1;
    }

    int bytesPP = ceil(vbe_info_struct.BitsPerPixel / (float)8);
    uint8_t* origin = address;

    if(vbe_info_struct.MemoryModel == 4){
        // The mode is index-based (0x105)

        // Make attribution of colors
            for(int i = 0; i < height; i++){
                for(int j = 0; j < width; j++){
                    setPixel(x + j, y + i, color, 0, origin, 1);
                }
            }
        
            if(color > 0xFF){
                printf(":: [TRACE.txt] ERROR IN make_rectangle(): Color value is invalid. Expected 1 byte only ... ::\n");
                return -2;
            }
    }
    else{
        // The mode is direct-color-based
            uint8_t* aux;
            aux = (origin+(y*vbe_info_struct.XResolution+x)*bytesPP);
            uint8_t storedValues[4];
            uint8_t storedSizes[4];

            uint8_t trans =     cleanbits( (uint8_t) (color >> vbe_info_struct.RsvdFieldPosition)   , vbe_info_struct.RsvdMaskSize);
            storedValues[3] = trans;
            storedSizes[3] = vbe_info_struct.RsvdMaskSize;
            uint8_t red =       cleanbits( (uint8_t) (color >> vbe_info_struct.RedFieldPosition)    , vbe_info_struct.RedMaskSize);
            storedValues[2] = red;
            storedSizes[2] = vbe_info_struct.RedMaskSize;
            uint8_t green =     cleanbits( (uint8_t) (color >> vbe_info_struct.GreenFieldPosition)  , vbe_info_struct.GreenMaskSize);
            storedValues[1] = green;
            storedSizes[1] = vbe_info_struct.GreenMaskSize;
            uint8_t blue =      cleanbits( (uint8_t) (color >> vbe_info_struct.BlueFieldPosition)   , vbe_info_struct.BlueMaskSize);
            storedValues[0] = blue;    
            storedSizes[0] = vbe_info_struct.BlueMaskSize;


            //printf("----------------------------------------------------------------------\n");
            //printf("----------------------------------------------------------------------\n");

            //printf("For the color 0x%x the extracted intensities were: Transparency: %d // Red: %d // Green: %d // Blue: %d.\n", color, storedValues[3], storedValues[2], storedValues[1], storedValues[0]);

            uint8_t byteVector[4]={0};

            int currentpos = 0;
            int currentbit = 0;
            int totalbitstransfered = 0;
            int storedbitreading = 0;
            for(int i = 0; i < 4; i++){
                while(storedSizes[i] > 0){
                    if(currentpos > 4 || totalbitstransfered == vbe_info_struct.BitsPerPixel){
                        break;
                    }
                    //uint8_t Dbefore = byteVector[currentpos];
                    byteVector[currentpos] = assignSpecificBit(storedValues[i], byteVector[currentpos], storedbitreading, currentbit);
                    //printf("(byteVector bit: %d) In storedSizes[%d] (%d), the byteVector was updated from %d to %d based on storedValues[%d] (%d) and current bit: %d", currentbit, i, storedSizes[i], Dbefore, byteVector[currentpos], i, storedValues[i], storedbitreading);
                    totalbitstransfered++;
                    storedSizes[i]--;
                    currentbit++;
                    storedbitreading++;
                    if(currentbit >= 8){
                        currentpos++;
                        currentbit = 0;
                    }
                }
                if(totalbitstransfered == vbe_info_struct.BitsPerPixel){
                    break;
                }
                storedbitreading = 0;
            }

            //printf("----------------------------------------------------------------------\n");
            //printf("----------------------------------------------------------------------\n");

        //printf("[DEBUG] --> byteVector[0] = 0x%x || byteVector[1] = 0x%x || byteVector[2] = 0x%x || byteVector[3] = 0x%x\n", byteVector[0], byteVector[1], byteVector[2], byteVector[3]);

        // Second step --> Begin attribution of colors
            for(int i = 0; i < height; i++){
                for(int j = 0; j < width; j++){
                    setPixel(x + j, y + i, color, byteVector, origin, 0);
                }
            }

    }

    return 0;
}

uint8_t* set_up_XPM(xpm_map_t xpm, uint16_t x, uint16_t y, void* address, int draw, uint8_t* oldAddress, int shouldFree){

    if(oldAddress != NULL){
        free(oldAddress);
        return NULL;
    }
    else{
        enum xpm_image_type type = XPM_INDEXED;

        uint8_t* XPM_address;

        XPM_address = xpm_load(xpm, type, &img);

        if(draw == 1){
            draw_XPM(XPM_address, x, y, address);
        }
        
        // Free XPN_allocated memory.
            if(shouldFree == 1){
                free(XPM_address);
                memset(&img, 0, sizeof(xpm_image_t));
                return NULL;
            }
            else{
                return XPM_address;
            }
    }
}

void draw_XPM(uint8_t* xpm_link, uint16_t x, uint16_t y, void* address){

    // Begin attribution of pixel colors:
        int colorPos = 0;
        uint8_t color;

        for(int i = 0; i < img.height; i++){
            for(int j = 0; j < img.width; j++){
                color = *(xpm_link+colorPos);
                colorPos++;
                setPixel(x + j, y + i, color, 0, address, 1);
            }
        }

}

void clean_XPM(uint8_t* xpm_link, uint16_t x, uint16_t y, void* address){

    // Begin attribution of black pixel (0) colors:

        for(int i = 0; i < img.height; i++){
            for(int j = 0; j < img.width; j++){
                setPixel(x + j, y + i, 0, 0, address, 1);
            }
        }
        
}

int move_XPM(uint8_t* xpm_link, uint16_t Xi, uint16_t Yi, uint16_t Xf, uint16_t Yf, int16_t speed, uint16_t fr_rate, void* address){

    if(!(Xf == Xi || Yf == Yi)){
      printf("[ERROR] - Invalid input ... Either xi = xf or yi = yf. None of these conditions have been checked ...\n");
      return -1;
    }

    // First step: Calculate the number of micro-seconds between each frame.
        float seconds_frame = 1 / (float)fr_rate;
        int microSeconds_frame = ceil((float)seconds_frame * SECOND_MIC);
        uint32_t ticks_frame = micros_to_ticks(microSeconds_frame);


    // Second step: Determine the static axis.
        int staticX = 0;
        int staticY = 0;
        int invertedMovement = 0;
        if(Xf == Xi){
            staticX = 1;
            invertedMovement = (Yf > Yi) ? 0 : 1;
            printf("STATIC X SELECTED (%d > %d):: invertedMovement = %d\n", Yf, Yi, invertedMovement);
        }
        else{ 
            staticY = 1;
            invertedMovement = (Xf > Xi) ? 0 : 1;
            printf("STATIC Y SELECTED (%d > %d):: invertedMovement = %d\n", Xf, Xi, invertedMovement);
        }

    // Third step: Set up keyboard & timer interruptions
        int r;
        uint8_t irq_timer0 = TIMER0_IRQ;
        uint8_t irq_kbd = BIT(0);
        uint8_t key_scanned = 0;
        int ipc_status;
        message msg;

        if( timer_subscribe_int(&irq_timer0) != 0){
            printf("Error setting up timer interruptions ... \n");
            return -1;
        }
        if( keyboard_subscribe_intr() != BIT(0)){
            printf("Error setting up keyboard interruptions ... \n");
            return -1;
        }

        /* XPM variables */
            int current_Y = Yi;
            int old_Y;
            int current_X = Xi;
            int old_X;
            int FINISH = 0;
            int first_execution = 1;
            int framesToWait = 1;           // If speed < 0, then it will be equal to speed.
        /* ************* */

        while (key_scanned != ESC_BREAK){

            /* MANAGING THE XPM - SPEED > 0 */
                if(FINISH == 0){
                    // Positive speed cases
                        if(speed > 0){
                            printf("... Positive speed (%d) ...\n", speed);
                            if(staticX == 1){
                                // Y-axis movement
                                    if(invertedMovement == 0){
                                        // Movement is not inverted (Yi < Yf)
                                        if(current_Y <= Yf){
                                            printf("current_Y marked as: %d\n", current_Y);
                                            if(first_execution == 1){
                                                first_execution = 0; 
                                            }
                                            else{
                                                clean_XPM(xpm_link, Xi, old_Y, address);
                                            }
                                            draw_XPM(xpm_link, Xi, current_Y, address);
                                            if( (current_Y + speed) > Yf){
                                                printf("---> Current_Y + speed exceeded the limit (%d > %d).", current_Y + speed, Yf);
                                                FINISH = 1;
                                            }
                                            else{
                                                old_Y = current_Y;
                                                current_Y = current_Y + speed;
                                            }
                                        }
                                    }
                                    else{
                                        // Movement is inverted (Yf < Yi)
                                        if(current_Y >= Yf){
                                            printf("[INVERTED] current_Y marked as: %d\n", current_Y);
                                            if(first_execution == 1){
                                                first_execution = 0; 
                                            }
                                            else{
                                                clean_XPM(xpm_link, Xi, old_Y, address);
                                            }
                                            draw_XPM(xpm_link, Xi, current_Y, address);
                                            if( (current_Y - speed) < Yf){
                                                printf("[INVERTED] ---> Current_Y + speed goes lower than the limit (%d > %d).", current_Y + speed, Yf);
                                                FINISH = 1;
                                            }
                                            else{
                                                old_Y = current_Y;
                                                current_Y = current_Y - speed;
                                            }
                                        }
                                    }
                            }
                            else{
                                // X-axis movement
                                    if(invertedMovement == 0){
                                        // Movement is not inverted (Yi < Yf)
                                        if(current_X <= Xf){
                                            printf("current_X marked as: %d\n", current_X);
                                            if(first_execution == 1){
                                                first_execution = 0; 
                                            }
                                            else{
                                                clean_XPM(xpm_link, old_X, Yi, address);
                                            }
                                            draw_XPM(xpm_link, current_X, Yi, address);
                                            if( (current_X + speed) > Xf){
                                                printf("---> Current_X + speed exceeded the limit (%d > %d).", current_X + speed, Xf);
                                                FINISH = 1;
                                            }
                                            else{
                                                old_X = current_X;
                                                current_X = current_X + speed;
                                            }
                                        }
                                    }
                                    else{
                                        // Movement is inverted (Yf < Yi)
                                        if(current_X >= Xf){
                                            printf("[INVERTED] current_X marked as: %d\n", current_X);
                                            if(first_execution == 1){
                                                first_execution = 0; 
                                            }
                                            else{
                                                clean_XPM(xpm_link, old_X, Yi, address);
                                            }
                                            draw_XPM(xpm_link, current_X, Yi, address);
                                            if( (current_X - speed) < Xf){
                                                printf("[INVERTED] ---> Current_X + speed goes lower than the limit (%d > %d).", current_X + speed, Xf);
                                                FINISH = 1;
                                            }
                                            else{
                                                old_X = current_X;
                                                current_X = current_X - speed;
                                            }
                                        }
                                    }
                            }
                        }
                }
            /* END OF XPM MANAGEMENT */

            /* MANAGING THE XPM - SPEED < 0 */
                if(FINISH == 0){
                    // Positive speed cases
                        if(speed < 0){
                            printf("... Negative speed (%d) ...\n", speed);
                            if(staticX == 1){
                                // Y-axis movement
                                    if(invertedMovement == 0){
                                        // Movement is not inverted (Yi < Yf)
                                        if(current_Y <= Yf){
                                            if(first_execution == 1){
                                                first_execution = 0;
                                                framesToWait = (-1) * speed;
                                            }
                                            else{
                                                clean_XPM(xpm_link, Xi, old_Y, address);
                                            }
                                            draw_XPM(xpm_link, Xi, current_Y, address);
                                            old_Y = current_Y;
                                            current_Y = current_Y + 1;
                                        }
                                    }
                                    else{
                                        // Movement is inverted (Yf < Yi)
                                        if(current_Y >= Yf){
                                            printf("[INVERTED] current_Y marked as: %d\n", current_Y);
                                            if(first_execution == 1){
                                                first_execution = 0;
                                                framesToWait = (-1) * speed; 
                                            }
                                            else{
                                                clean_XPM(xpm_link, Xi, old_Y, address);
                                            }
                                            draw_XPM(xpm_link, Xi, current_Y, address);
                                            old_Y = current_Y;
                                            current_Y = current_Y - 1;
                                        }
                                    }
                            }
                            else{
                                // X-axis movement
                                    if(invertedMovement == 0){
                                        // Movement is not inverted (Yi < Yf)
                                        if(current_X <= Xf){
                                            printf("current_X marked as: %d\n", current_X);
                                            if(first_execution == 1){
                                                first_execution = 0;
                                                framesToWait = (-1) * speed;  
                                            }
                                            else{
                                                clean_XPM(xpm_link, old_X, Yi, address);
                                            }
                                            draw_XPM(xpm_link, current_X, Yi, address);
                                            old_X = current_X;
                                            current_X = current_X + 1;
                                        }
                                    }
                                    else{
                                        // Movement is inverted (Yf < Yi)
                                        if(current_X >= Xf){
                                            printf("[INVERTED] current_X marked as: %d\n", current_X);
                                            if(first_execution == 1){
                                                first_execution = 0;
                                                framesToWait = (-1) * speed;  
                                            }
                                            else{
                                                clean_XPM(xpm_link, old_X, Yi, address);
                                            }
                                            draw_XPM(xpm_link, current_X, Yi, address);
                                            old_X = current_X;
                                            current_X = current_X - 1;
                                        }
                                    }
                            }
                        }
                }
            /* END OF XPM MANAGEMENT*/

            counter = 0;

            while( (counter < (ticks_frame * framesToWait)) && key_scanned != ESC_BREAK){
                //printf("Counter lower than ticks_frame (%d < %d)\n", counter, ticks_frame);
                if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0) {
                    printf("driver_receive failed with: %d", r);
                    continue;
                }

                if (is_ipc_notify(ipc_status)) { /* received notification */
                    switch (_ENDPOINT_P(msg.m_source)) {
                    case HARDWARE: /* hardware interrupt notification */
                        if (msg.m_notify.interrupts & irq_kbd) { /* subscribed interrupt */
                            kbc_ih();
                            key_scanned = universal_key_scanned;
                            //printf("--> Keyboard interruption registered! (Scancode = 0x%x)\n", key_scanned);
                        }
                        if (msg.m_notify.interrupts & irq_timer0){ /* subscribed interrupt */
                            timer_int_handler();
                            //printf("--> Counter incremented (New counter val: %d)\n", counter);
                        }
                        break;
                    default:
                        break; /* no other notifications expected: do*/
                    }
                }
            }

            if(FINISH == 1){
                if(staticX == 1){
                    clean_XPM(xpm_link, Xi, current_Y, address);
                    current_Y = Yf;
                    printf(" Moved current_Y to %d\n", current_Y);
                    draw_XPM(xpm_link, Xi, current_Y, address);
                    /* XPM LOCK */ FINISH = 2;
                } 
                else{
                    clean_XPM(xpm_link, current_X, Yi, address);
                    current_X = Xf;
                    printf(" Moved current_X to %d\n", current_X);
                    draw_XPM(xpm_link, current_X, Yi, address);
                    /* XPM LOCK */ FINISH = 2;                    
                }
            }
        }
            
    
    if (keyboard_unsubscribe_intr() != 0) {
        printf("Error unsubscribing keyboard ...\n");
        return -1;
    } 

    if (timer_unsubscribe_int() != 0) {
        printf("Error unsubscribing timer ...\n");
        return -1;
    }  

    set_up_XPM(0, 0, 0, NULL, 0, xpm_link, 0);

    return 0;
}



