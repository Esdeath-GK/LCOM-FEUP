#include <lcom/lcf.h>
#include <machine/int86.h>
#include <lcom/xpm.h>
#include "i8254.h"

#include <stdbool.h>
#include <stdint.h>
#include "videocard_essentials.h"
#include "kbd_essentials.h"
#include "timer.h"
#include "proj_essentials.h"
#include "xpm_variables.c"
#include "cursor.h"

/* ********************************************************************************************* */

int success = 1;
vbe_mode_info_t vbe_info_struct;
int UniversalSize;
extern uint32_t counter;
int isNumber = 0;  
int REDwarning = 0;   // Number of incrementations of the RED field.


/* ****************************** AUXILIARY FUNCTIONS ****************************************** */

void setisNumber(int valor){
    isNumber = valor;
}


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

/* **************************************** END OF AUXILIARY FUNCTIONS ************************************************* */

void* (vg_init)(uint16_t mode){
    void *video_mem;
    int r;
    struct minix_mem_range mr; /*physical memory range*/

    memset(&vbe_info_struct, 0, sizeof(vbe_info_struct));

    vbe_extract_MODEinformation(mode, &vbe_info_struct);

    int total_size = vbe_info_struct.XResolution * vbe_info_struct.YResolution * ceil(vbe_info_struct.BitsPerPixel / (float)8);
    UniversalSize = total_size;

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

uint8_t* set_up_XPM(xpm_map_t xpm, uint16_t x, uint16_t y, void* address, int draw, uint8_t* oldAddress, int shouldFree, xpm_image_t* img){

    if(oldAddress != NULL){
        memset(img, 0, sizeof(xpm_image_t));
        free(oldAddress);
        return NULL;
    }
    else{
        enum xpm_image_type type = XPM_8_8_8_8;

        uint8_t* XPM_address;

        XPM_address = xpm_load(xpm, type, img);

        if(draw == 1){
            draw_XPM(XPM_address, x, y, address, *img, 0);
        }
        
        // Free XPN_allocated memory.
            if(shouldFree == 1){
                free(XPM_address);
                memset(img, 0, sizeof(xpm_image_t));
                return NULL;
            }
            else{
                return XPM_address;
            }
    }
}

void draw_XPM(uint8_t* xpm_link, uint16_t x, uint16_t y, void* address, xpm_image_t img, int black_val){

    // Begin attribution of pixel colors:
        int colorPos = 0;
        uint8_t byteVector[4] = {0};
        int onCloseTop = onTopClose_EG();

        for(int i = 0; i < img.height; i++){
            for(int j = 0; j < img.width; j++){
                if(isNumber == 0){
                    byteVector[0] = black_val == 1 ? 0x01 : *(xpm_link+colorPos*4+0);
                    byteVector[1] = black_val == 1 ? 0x01 : *(xpm_link+colorPos*4+1);
                    byteVector[2] = black_val == 1 ? 0x01 : *(xpm_link+colorPos*4+2);
                    byteVector[3] = black_val == 1 ? 0x00 : *(xpm_link+colorPos*4+3);
                }
                else{
                    byteVector[0] = (*(xpm_link+colorPos*4+0) == 0xFF) ? 0x01 : *(xpm_link+colorPos*4+0);
                    byteVector[1] = (*(xpm_link+colorPos*4+1) == 0xFF) ? 0x01 : *(xpm_link+colorPos*4+1);         
                    uint8_t turnNearEnd = ceil(DEFAULT_SECONDS / (float)2);
                    turnNearEnd = turnNearEnd == 0 ? turnNearEnd+2 : turnNearEnd;
                    int increment = (int)(255 / turnNearEnd);
                    if( (0x00 + increment * getREDwarning()) >= 255 ){
                        byteVector[2] = (*(xpm_link+colorPos*4+2) == 0xFF) ? 0xFF : *(xpm_link+colorPos*4+2);
                    }
                    else{
                        byteVector[2] = (*(xpm_link+colorPos*4+2) == 0xFF) ? (1 + increment * getREDwarning()) : *(xpm_link+colorPos*4+2);
                    }
                    byteVector[3] = (*(xpm_link+colorPos*4+2) == 0xFF) ? 0x00 : *(xpm_link+colorPos*4+3);
                }
                colorPos++;
                if(byteVector[0] != 0x2C || byteVector[1] != 0x2C || byteVector[2] != 0x2C){
                    if(byteVector[3] != 255){
                        if(onCloseTop == 0 && (xpm_link == XPM_blueCLOSEbutton || xpm_link == XPM_redCLOSEbutton)){
                            if(xpm_link == XPM_blueCLOSEbutton){
                                /* Darken blue intensity */byteVector[0] *= 0.75;
                                /* Darken green intensity */byteVector[1] *= 0.85;
                            }
                            else{
                                /* Darken red intensity */byteVector[2] *= 0.80;
                            }
                        }
                        setPixel(x + j, y + i, 0, byteVector, address, 0);
                    }
                }
            }
        }

}

void clean_XPM(uint8_t* xpm_link, uint16_t x, uint16_t y, void* address, xpm_image_t img){

    // Begin attribution of black pixel (0) colors:

        for(int i = 0; i < img.height; i++){
            for(int j = 0; j < img.width; j++){
                setPixel(x + j, y + i, 0, 0, address, 1);
            }
        }
        
}


int getTotalSize(){
    return UniversalSize;
}

vbe_mode_info_t getVbeModeInfo(){
    return vbe_info_struct;
}

void addREDwarning(int add){
    if(add==0){
        REDwarning = 0;
    }
    else
    {
        REDwarning += add;
    }
}

int getREDwarning(){
    return REDwarning;
}
