/**************************************************************************/
/*                                                                        */
/*  Programmer: Gregory L Montgomery                                      */
/*                                                                        */
/*                                                                        */
/*                                                                        */
/*                                                                        */
/**************************************************************************/
#include <xc.h>
#include <stdint.h>
#include "defs.h"
#include "ImageHeader.h"

#define ImageEnd              0x000F0200

#define DateStr              "MM-DD-YYYY\0"
#define ModelStr             "ABCDEFGHIJKLMNO\0"
#define HardwareRevStr       "ABCDEFGHIJKLMNO\0"
#define BootLoaderRev        "00.00.01\0"
#define FirmwareRev          "00.03.01\0"
#define Compiler             "XC16 1.23\0"
#define Enviornment          "MPLAB X 3.20\0"

const ImageHeader ProgramHeader = //__attribute__((address(HeaderLocation))) =
{
    0, //0x9605, /* Crc */
    ImageStart, /* Etnry point for program */
    ImageEnd,
    {DateStr},
    {ModelStr},
    {HardwareRevStr}, 
    {BootLoaderRev}, 
    {FirmwareRev},
    {Compiler}, 
    {Enviornment} 
};

