/**************************************************************************/
/*                                                                        */
/*  Programmer: Gregory L Montgomery                                      */
/*                                                                        */
/*                                                                        */
/*                                                                        */
/*                                                                        */
/**************************************************************************/
#ifndef RTSP_API_H
#define RTSP_API_H

#include <stdint.h>

// Flash Memory is organised into ROWs of 32 instructions or 96 bytes
// RTSP allows the user to erage/program a row at a time

/*
 * FLASH ROW ERASE
 *
 * Parameters Definition:
 * nvmAdru:	Selects the upper 8bits of the location to program or erase in program flash memory
 * nvmAdr:  Selects the location to program or erase in program flash memory
   It must be aligned to 32 instruction boundary, LSB 6bits of address must be zero
 * Return Value:
 * Function returns ERROREE (or -1), if it is not successful
 * Function return ZERO, if successful
*/
int flashRowErase(uint16_t nvmAdru, uint16_t nvmAdr);

int EraseFlashPage(unsigned NVRadmdru, unsigned NVRamAddress);
/*
 * FLASH ROW READ
 *
 * Parameters Definition:
 * nvmAdru:	Selects the upper 8bits of the location to read in program flash memory
 * nvmAdr:  Selects the location to read in program flash memory
   It must be aligned to 32 instruction boundary, LSB 6bits of address must be zero
 * rowBufPtr Pointer to the data array in which read data will be stored

 
 * Return Value:
 * Function returns ERROREE (or -1), if it is not successful
 * Function return ZERO, if successful
*/
int flashRowRead(uint16_t nvmAdru, uint16_t nvmAdr, uint16_t *rowBufPtr);


/*
 * FLASH ROW WRITE
 *
 * Parameters Definition:
 * nvmAdru:	Selects the upper 8bits of the location to program or erase in program flash memory
 * nvmAdr:  Selects the location to program or erase in program flash memory
    It must be aligned to 32 instruction boundary, LSB 6bits of address must be zero
 * rowBufPtr Pointer to the data array that needs to be programmed 


 * Return Value:
 * Function returns ERROREE (or -1), if it is not successful
 * Function return ZERO, if successful
*/
int flashRowWrite(uint16_t nvmAdru, uint16_t nvmAdr, uint16_t *rowBufPtr);



#endif

