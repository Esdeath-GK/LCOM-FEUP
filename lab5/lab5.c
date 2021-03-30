// IMPORTANT: you must include the following line in all your C files
#include <lcom/lcf.h>

#include <lcom/lab5.h>

#include <stdint.h>
#include <stdio.h>

// Any header files included below this line should have been created by you
#include "videocard_essentials.h"
#include "kbd_essentials.h"
#include "timer.h"
#include "i8254.h"
#include "utils.c"


extern int success;
extern vbe_mode_info_t vbe_info_struct;

int main(int argc, char *argv[]) {
  // sets the language of LCF messages (can be either EN-US or PT-PT)
  lcf_set_language("EN-US");

  // enables to log function invocations that are being "wrapped" by LCF
  // [comment this out if you don't want/need it]
  lcf_trace_calls("/home/lcom/labs/lab5/trace.txt");

  // enables to save the output of printf function calls on a file
  // [comment this out if you don't want/need it]
  lcf_log_output("/home/lcom/labs/lab5/output.txt");

  // handles control over to LCF
  // [LCF handles command line arguments and invokes the right function]
  if (lcf_start(argc, argv))
    return 1;

  // LCF clean up tasks
  // [must be the last statement before return]
  lcf_cleanup();

  return 0;
}

int(video_test_init)(uint16_t mode, uint8_t delay){

    if(delay < 0){
      printf("Invalid delay value ... please retry ...\n");
      return -1;
    }

    if( vg_init(mode) == NULL ){
      printf("[DEBUG] video_test_init: vg_setmode returned an error\n");
      return -1;
    }

	  tickdelay(micros_to_ticks(delay * SECOND_MIC));

    if( vg_exit() != 0 ){
      printf("[DEBUG] video_test_init: vg_exit() returned an error\n");
      return -1;
    }

    return 0;
}

int(video_test_rectangle)(uint16_t mode, uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t color) {
  
  // Declare the VRAM-Mapping address.
    void* address;

  // Initiate VG_mode and returned mapping address.
    address = vg_init(mode);

  // Set up the rectangle based on the address obtained.
    int returnVal;
    returnVal = make_rectangle(x, y, width, height, color, address);

  // Wait for ESC_BREAK code.
    keyboard_detect_break();

  // Return to text mode.
    vg_exit();

  if(returnVal == -1){
    printf("\nERROR REGISTERED IN TRACE.TXT!\n");
    printf("--> Rectangle dimensions would go beyond resolution limits ... -1 returned\n");
  }

  if(returnVal == -2){
    printf("\nERROR REGISTERED IN TRACE.TXT!\n");
    printf("--> Color value is not acceptable in the mode given ... -2 returned\n");
  }

  return 0;
}

int(video_test_pattern)(uint16_t mode, uint8_t no_rectangles, uint32_t first, uint8_t step) {
  
  // Declare the VRAM-Mapping address.
    void* address;

  // Extract VRAM mapping address.
    address = vg_init(mode);

    uint32_t currentcolor = first;

  // Obtain the side measures of the rectangles.
    int X_lenght;
    int Y_lenght;
    int integerValue;

      // X_lenght:
        integerValue = vbe_info_struct.XResolution % no_rectangles;
        X_lenght = ( (vbe_info_struct.XResolution - integerValue) / no_rectangles );

      // Y_lenght:
        integerValue = vbe_info_struct.YResolution % no_rectangles;
        Y_lenght = ( (vbe_info_struct.YResolution - integerValue) / no_rectangles );

    for(int row = 0; row < no_rectangles; row++){
      for(int col = 0; col < no_rectangles; col++){
        currentcolor = colorPattern(row, col, first, step, no_rectangles);
        make_rectangle(X_lenght * col, Y_lenght * row, X_lenght, Y_lenght, currentcolor, address);
      }
    }

    
  // Wait for ESC_BREAK code.
    keyboard_detect_break();

  // Return to text mode.
    vg_exit();

    return 0;

}

int(video_test_xpm)(xpm_map_t xpm, uint16_t x, uint16_t y) {

  // Declare the VRAM-mapping address
    void* address;
  
  // Extract (& generate) VRAM mapping address.
    address = vg_init(XPM_0X105);

  // SET-UP XPM
    set_up_XPM(xpm, x, y, address, 1, NULL, 1);

  // Wait for ESC_BREAK code.
    keyboard_detect_break();

  // Return to text mode.
    vg_exit();

    return 0;
}

int(video_test_move)(xpm_map_t xpm, uint16_t xi, uint16_t yi, uint16_t xf, uint16_t yf,
                     int16_t speed, uint8_t fr_rate) {
  
  if(!(xf == xi || yf == yi)){
      printf("[ERROR] - Invalid input ... Either xi = xf or yi = yf. None of these conditions have been checked ...\n");
      return -1;
  }

  // Declare the VRAM-mapping address and allocated XPM info.
    void* address;
    uint8_t* XPM_link;
  
  // Extract (& generate) VRAM mapping address.
    address = vg_init(XPM_0X105);

  // SET-UP XPM (no drawing)
    XPM_link = set_up_XPM(xpm, 0, 0, address, 0, NULL, 0);

  // MOVE THE XPM
    move_XPM(XPM_link, xi, yi, xf, yf, speed, fr_rate, address);

  // Return to text mode.
    vg_exit();

  return 0;
}

int(video_test_controller)(){

  // First step: define vg_vbe_contr_info_t and set all its contents to zero.
    vg_vbe_contr_info_t capabilities;
    VBEINFO_ vbe_info_struct;
    memset(&capabilities, 0, sizeof(vg_vbe_contr_info_t));
    memset(&vbe_info_struct, 0, sizeof(VBEINFO_));
    char VBE2_string[4]={"VBE2"};

  // Second step: Call 0x00 function and extract the vbe_controller information
    reg86_t reg86;
	  mmap_t mapping;

    memset(&reg86, 0, sizeof(reg86));
    memset(&mapping, 0, sizeof(mapping));

    lm_alloc(VBEINFOBLOCK_SIZE, &mapping);
    uint8_t* aux = mapping.virt;

    for(int i = 0; i < 4; i++){
      *(aux+i) = VBE2_string[i];
    }

    reg86.intno = 0x10;
    reg86.ah = INVOKE_AH;               // AH register must be set to 0x4F whenever we invoke a function.
    reg86.al = 0x00;                    // AL register must be set to 0x00 whenever we wish to extract VBE's controller info.
    reg86.es = PB2BASE(mapping.phys);   /* set a segment base */
    reg86.di = PB2OFF(mapping.phys);    /* set the offset accordingly */
    if (sys_int86(&reg86) != OK) {      /* call BIOS */
      printf("vbe_extract_MODEinformation: sys_int86() has failed.\n");
      return -1;
    }

    memcpy(&vbe_info_struct, mapping.virt, VBEINFOBLOCK_SIZE);

    lm_free(&mapping);

  // Assign data:
    // VBE signature
      for(int i = 0; i < 4; i++){
        capabilities.VBESignature[i] =  vbe_info_struct.VbeSignature[i];
      }
    
    // VBE version
      for(int i = 0; i < 4; i++){
        capabilities.VBEVersion[i] =  vbe_info_struct.VbeVersion[i];
      }

    // OEM string
      int NewPhysAddress = ((vbe_info_struct.OemStringPtr >> 16) << 4) | PB2OFF(vbe_info_struct.OemStringPtr);
      int difference = NewPhysAddress - mapping.phys;
      capabilities.OEMString = (char*)((vir_bytes)mapping.virt + difference);

    // VideoMode List
      NewPhysAddress = ((vbe_info_struct.VideoModePtr >> 16) << 4) | PB2OFF(vbe_info_struct.VideoModePtr);
      difference = NewPhysAddress - mapping.phys;
      capabilities.VideoModeList =  (uint16_t*)((vir_bytes)mapping.virt + difference);
    
    // TotalMemory
      capabilities.TotalMemory =  vbe_info_struct.TotalMemory;

    // OEM Vendor - Name
      NewPhysAddress = ((vbe_info_struct.OemVendorNamePtr >> 16) << 4)  | PB2OFF(vbe_info_struct.OemVendorNamePtr);
      difference = NewPhysAddress - mapping.phys;
      capabilities.OEMVendorNamePtr =  (char*)((char*)mapping.virt + difference);
    
    // OEM Product - Name
      NewPhysAddress = ((vbe_info_struct.OemProductNamePtr >> 16) << 4) | PB2OFF(vbe_info_struct.OemProductNamePtr);
      difference = NewPhysAddress - mapping.phys;
      capabilities.OEMProductNamePtr =  (char*)((char*)mapping.virt + difference);

    // OEM Product - Rev
      NewPhysAddress = ((vbe_info_struct.OemProductRevPtr >> 16) << 4) | PB2OFF(vbe_info_struct.OemProductRevPtr);
      difference = NewPhysAddress - mapping.phys;
      capabilities.OEMProductRevPtr =  (char*)((char*)mapping.virt + difference);
  
    if(vg_display_vbe_contr_info(&capabilities) != 0){
      printf("Error ...\n");
      return 1;
    }

  return 0;
}
