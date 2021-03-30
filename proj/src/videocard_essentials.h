#ifndef VIDEO_ESSENTIALS_H
#define VIDEO_ESSENTIALS_H

/* ********************************************* DEFINITIONS ********************************************* */

// IMPORTANT NOTE: 0x2C is embraced as "transparent"

#define INVOKE_AH 0x4F                              // Mandatory AH value whenever a function is called.  
#define MByte 0x100000                              // One megabyte as in decimal
#define START_REG 0x0                               // Start register.
#define BYTE_BITS 8                                 // Number of bits in a byte.
#define XPM_0X105 0x105                             // Standard mode for XPM
#define VBEMODE_14C 0x14C

#define VBE_SIGNATURE_SIZE  4
#define PB2BASE(x) 			(((x) >> 4) & 0x0F000)
#define MODEINFO_SIZE		256
#define VBEINFOBLOCK_SIZE   512
#define SECOND_MIC          1000000

#pragma pack(1) 

typedef struct{
    uint8_t     VbeSignature[4];
    BCD         VbeVersion[2];
    phys_bytes  OemStringPtr;
    uint8_t     Capabilities[4];
    phys_bytes  VideoModePtr;
    uint16_t    TotalMemory;

    uint16_t    OemSoftwareRev;
    phys_bytes  OemVendorNamePtr;
    phys_bytes  OemProductNamePtr;
    phys_bytes  OemProductRevPtr;
    uint8_t     Reserved[222];
    uint8_t     OemData[256];
}VBEINFO_;

#pragma options align=reset

/* ******************************************************************************************************* */


/* ********* Prototypes definitions - Brief description ********* */

/**
 * @brief - Ceiling function.
 *
 * @return - Returns -1 in case of error. 0 in case of success.
 */
int ceil(float num) ;

int vbe_extract_MODEinformation(uint16_t mode, vbe_mode_info_t* vbe_extracted_MODEinfo);


/**
 * @brief - Cleans the bits of a given number after the marked position.
 *
 * @param original - Original number whose bits not part of the preserve's interval, you wish to set to zero.
 * 
 * @param preserve - Number of bits (counting from the LSB) wished to preserve.
 * 
 * @return - Returns the given number with the bits cleaned.
 */
uint8_t cleanbits(uint8_t original, uint8_t preserve);

/**
 * @brief - Copies a specific BIT from one byte to the other.
 *
 * @param original - Original number.
 * 
 * @param clipboard - Number to which the specific bit will be copied to.
 * 
 * @param bit - Bit number (0 - 7) of the original number we whish to copy.
 * 
 * @param bitClipboard - Bit number (0 - 7) we wish to copy the bit from the original to.
 * 
 * @return - Returns the given number with the modified BIT.
 */
uint8_t assignSpecificBit(uint8_t original, uint8_t clipboard, int bitOriginal, int bitClipboard);

/**
 * @brief - Sets the pixel at location (x,y) to the color indicated. The mode is taken into consideration.
 *
 * @param x - X coordenate of the pixel.
 * 
 * @param y - Y coordenate of the pixel.
 * 
 * @param color - The color you wish the set the pixel to.
 * 
 * @param byteVector - A vector containing the bytes to be passed.
 * 
 * @param address - The base address of the mapped memory.
 * 
 * @param isIndexed - Whether or not the mode is indexed.
 * 
 * @return - Returns -1 in case of error. 0 in case of success.
 */
int setPixel(uint16_t x, uint16_t y, uint32_t color, uint8_t byteVector[], void* address, int isIndexed);


/**
 * @brief - Reads an XPM-like sprite, sets up the sprite, then frees the space when it is no longer needed (if the user desires).
 *
 * @param xpm - Array with XPM (assuming indexed color mode)
 * 
 * @param x - Beginning coordinate x
 * 
 * @param y - Beginning coordinate y
 * 
 * @param address - Mapped address
 * 
 * @param draw - 0 in case the user does not want the function to draw the XPM. 1 otherwise.
 * 
 * @param oldAddress - The address of an already alocated XPM. If this pointer is not NULL, the function will simply free the XPM.
 * 
 * @param shouldFree - 0 in case the user does not want the function to free the alocated XPM. 1 otherwise.
 * 
 * @return - Returns 0 on success or the XPM_link if the shouldFree variable is zero.
 */
uint8_t* set_up_XPM(xpm_map_t xpm, uint16_t x, uint16_t y, void* address, int draw, uint8_t* oldAddress, int shouldFree, xpm_image_t* img);


/**
 * @brief - Draws the XPM on the mapped VRAM 
 *
 * @param xpm_link - Pointer to XPM
 * 
 * @param x - Beginning coordinate x
 * 
 * @param y - Beginning coordinate y
 * 
 * @param emptyChip - Fills an open spot on the table. If this value is set to one, the XPM color will be ignored and 
 *                    only the R: 0x00 / G: 0x00 / B: 0x01 will be painted on the @param address.
 * 
 * @return - Returns 0 on success.
 */
void draw_XPM(uint8_t* xpm_link, uint16_t x, uint16_t y, void* address, xpm_image_t img, int emptyChip);

/**
 * @brief - Cleans the XPM on the mapped VRAM
 *
 * @param xpm_link - Pointer to XPM
 * 
 * @param x - Beginning coordinate x
 * 
 * @param y - Beginning coordinate y
 * 
 * @return - Returns 0 on success.
 */
void clean_XPM(uint8_t* xpm_link, uint16_t x, uint16_t y, void* address, xpm_image_t img);

/**
 * @brief - Return UniversalSize
 * 
 * @return - UniversalSize.
 */
int getTotalSize();

/**
 * @brief - Return VBE Mode Information
 * 
 * @return - VBE Mode Information.
 */
vbe_mode_info_t getVbeModeInfo();

/**
 * @brief - Manage REDwarning
 * 
 * @return - Void function.
 */
void addREDwarning();

/**
 * @brief - Return REDwarning
 * 
 * @return - Value of REDwarning.
 */
int getREDwarning();

/**
 * @brief - Set isNumber with a value
 * 
 * @param - Value to set
 * 
 * @return - Void function.
 */
void setisNumber(int valor);

/* ------------------------------------------ */

#endif /* VIDEO_H */
