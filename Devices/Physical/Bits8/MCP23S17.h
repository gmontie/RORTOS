/**************************************************************************/
/*                                                                        */
/*  Programmer: Gregory L Montgomery                                      */
/*                                                                        */
/*                                                                        */
/*                                                                        */
/*                                                                        */
/**************************************************************************/
#ifndef MCP23S17_H
#define MCP23S17_H

#include "defs.h" // Generial definitions for the project
#include "Register.h" // Definition of a cell
#include "Device.h"
#include "Fops.h" // File Operations
#include "Blk.h"

typedef enum
{ 
   IODIRA = 0, 
   IODIRB, 
   IPOLA, 
   IPOLB, 
   GPINTENA, 
   GPINTENB, 
   DEFVALA, 
   DEFVALB, 
   INTCONA, 
   INTCONB, 
   IOCON_A, 
   IOCON_B, 
   GPPUA, 
   GPPUB, 
   INTFA, 
   INTFB, 
   INTCAPA, 
   INTCAPB, 
   GPIOA, 
   GPIOB, 
   OLATA, 
   OLATB
}UseBank0;

typedef enum{DefaultConfiguration, LoadRegisters}MCP_IOCTL_STATES;

//Function Prototypes
struct service * InitMCP23S17(VBLOCK * BlkPtr, fops * IODevice);
#endif
