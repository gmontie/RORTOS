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

/*   Register Address Definitions   */
#define _IODIR     0x00 // IO Direction Register 
#define _IPOL      0x02 // Input Polarity Port Register
#define _GPINTEN   0x04 // Interrupt-on-change Pin Assignments
#define _DEFVAL    0x06 // Default Compare Register
#define _INTCONF   0x08 // Interrupt Control Register
#define _IOCON     0x0A // Configuration Register
#define _GPPU      0x0C // GPIO Pull-up Register
#define _INTF      0x0E // Interrupt Flags Register
#define _INTCAP    0x10 // Interrupt Captured Value Port Register
#define _GPIO      0x12 // Register for Port A then Port B
#define _OLAT      0x14 // Output Latch Register

typedef enum{DefaultConfiguration, LoadRegisters}MCP_IOCTL_STATES;

//Function Prototypes
struct service * InitMCP23S17(VBLOCK * BlkPtr, fops * IODevice);
#endif
