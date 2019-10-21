/**************************************************************************/
/*                                                                        */
/*  Programmer: Gregory L Montgomery                                      */
/*                                                                        */
/*                                                                        */
/*                                                                        */
/*                                                                        */
/**************************************************************************/
#ifndef IMAGE_HEADER_H
#define IMAGE_HEADER_H

#include "defs.h"

#define VersionSpace   6
#define SoftwareStr   16

typedef struct
{
    uint16_t CRC;
    uint32_t AppEntry;
    uint32_t EndingAddress;
    byte     Date[SoftwareStr]; // Data of Manufacture
    byte     ModelNumber[SoftwareStr]; // Product Model
    byte     HardwareReve[SoftwareStr]; // Hareware Revision
    byte     BootLoaderRev[SoftwareStr]; // 
    byte     FirmwareRev[SoftwareStr]; // 
    byte     CompilerRev[SoftwareStr]; // 
    byte     DevEnviornment[SoftwareStr]; // 
}ImageHeader;

#define ImageStart      0x0000200 /* This is true for all Applications! */
#define HeadeerSize     sizeof(ImageHeader) /* Calculated by hand 18 bytes*/
#define HeaderLocation  0x00000200 /*  */

#endif
