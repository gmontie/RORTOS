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
#include "System.h"
#include "MCP23S17.h"
#include "Fops.h"
#include "Device.h"
#include "PinConfig.h"

#include "Spi.h"

/*  Definition of exported register set */
#define iIODIR     0x00 // IO Direction Register 
#define iIPOL      0x01 // Input Polarity Port Register
#define iGPINTEN   0x02 // Interrupt-on-change Pin Assignments
#define iDEFVAL    0x03 // Default Compare Register
#define iINTCONF   0x04 // Interrupt Control Register
#define iIOCON     0x05 // Configuration Register
#define iGPPU      0x06 // GPIO Pull-up Register
#define iINTF      0x07 // Interrupt Flags Register
#define iINTCAP    0x08 // Interrupt Captured Value Port Register
#define iGPIO      0x09 // Register for Port A then Port B
#define iOLAT      0x0A // Output Latch Register

#define Delay5Cyclse() Nop(); Nop(); Nop(); Nop(); Nop(); 
#define Delay11Cyclse() Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop();

#define NBR_BYTE_REGISTERS     0x15
#define NBR_UINT_REGISTESRS    0x0B
#define DISCRIPTION_M25LC256  "MCP23S17 SPI IO Expander Device"
#define MCP23S17Select         CS2

#define DEFAULT_CONFIG 0xA000 // Defualt Configuration for MCP23S17
#define DEFAULT_IODIR  0x0000 // All are outputs
#define DEFAULT_LATCH  0xAAAA // Default Latch output

#define WRITE_MCP23S17 0b0100000000000000
#define READ_MCP23S17  0b0100000100000000

unsigned MCP23S17Registers[NBR_UINT_REGISTESRS] = 
{
   DEFAULT_IODIR, // iIODIR
   0x0000,    // iPOL
   0x0000,    // iGP INT ENABLE
   0x0000,    // iDEFVAL
   0x0000,    // iINTCONF
   DEFAULT_CONFIG, // iIOCON
   0x0000,    // iGPPU
   0x0000,    // iINTF
   0x0000,    // iINTCAP
   DEFAULT_LATCH,    // iGPIO
   DEFAULT_LATCH // iLAT
};

static Boolean DataAvailable(void);
 uint16_t IOCTL(byte Operation);
static void Mcp23S17Update(void);
static void WriteDefaultConfiguration(void);
static uint16_t ReadIn(byte);
static void LoadMcp23S17Registes(void);
static void Mcp23S17Write(byte Where, uint16_t What);

// Private Member Objects
static Register * GpIoReg;
static Register * LATReg;

//static sops * This;
static fops * SpiIO;
SpiOps * BusIO;

// Private File Operations Object
static struct service This =
{
   .DeviceType = MCP23S17,
   .DeviceClass = DIGITAL_IO,
   .Discription = DISCRIPTION_M25LC256,
   .Arg = 0,
   .FnType = ProcessFn,
   .Thread = Mcp23S17Update,
   .InputType = ReadFn,
   .Read = ReadIn,
   .Write = Mcp23S17Write,
   .Poke = 0,
   .IsReady = DataAvailable,
   .Reset = LoadMcp23S17Registes
   //.Driver = 0
};

/***********************************************************************/
/*                                                                     */
/*     Function: InitMCP23S17                                          */
/*                                                                     */
/*        Input: A Pointer to the a Memory Set                         */
/*               A Pointer to the Spi Object which interfaces to the   */
/*               MCP23S17                                              */
/*       Output: A Pointer to the file operations which this Object    */
/*               can perform.                                          */
/*                                                                     */
/*     Overview: This function Istantiates and initializes a MCP23S17  */
/*               file object.                                          */
/*                                                                     */
/***********************************************************************/
struct service * InitMCP23S17(VBLOCK * BlkPtr, fops * IODevice)
{
   MCP23S17Select = 1; // Deselect Device
   GpIoReg = getRegister(BlkPtr->IndexList[0]);
   LATReg = getRegister(BlkPtr->IndexList[1]);
   SpiIO = IODevice; // Save the reference to the IODevice
   This.Driver = IODevice;
   BusIO = (SpiOps *)SpiIO->Usr;
   WriteDefaultConfiguration();
   
   LoadMcp23S17Registes();
   return &This;
}

/***********************************************************************/
/*                                                                     */
/*     Function:                                                       */
/*        Input: None                                                  */
/*       Output: None                                                  */
/*                                                                     */
/*     Overview:                                                       */
/*                                                                     */
/*                                                                     */
/***********************************************************************/
static void WriteDefaultConfiguration(void)
{
   uint16_t Address = 0;
   //SPI_MODES CurrentMode = BusIO->CurrentMode;
   //BusIO->SelectSPI1Mode(MODE_3);
   
   MCP23S17Select = 0; // Select Device
   Nop();
   Nop();
   //Address = (WRITE_MCP23S17 | _IOCON); // Write IO configuration 
   Address = (WRITE_MCP23S17 | _IOCON); // Write IO configuration 
   SpiIO->PutCh(Address);
   SpiIO->PutCh(DEFAULT_CONFIG); // Disable auto increment
   Delay5Cyclse();
   MCP23S17Select = 1; // Deselect Device
   Delay11Cyclse(); // Delay before next command
   Delay11Cyclse(); // Wait a small amount of time
   Delay11Cyclse(); // Wait a small amount of time
   Delay11Cyclse(); // Wait a small amount of time
   Delay11Cyclse(); // Wait a small amount of time
   
   MCP23S17Select = 0; // Select Device
   Nop();
   Nop();
   SpiIO->PutCh(WRITE_MCP23S17 | _IODIR); // Set the Direction for the IO Pins
   SpiIO->PutCh(DEFAULT_IODIR); // Set IO Ports to output
   Delay5Cyclse();
   MCP23S17Select = 1; // Deselect Device   
   Delay11Cyclse(); // Delay before next command
   Delay11Cyclse(); // Wait a small amount of time
   Delay11Cyclse(); // Wait a small amount of time
   Delay11Cyclse(); // Wait a small amount of time
   Delay11Cyclse(); // Wait a small amount of time
   
   Address = (WRITE_MCP23S17 | _OLAT); // Write the default value for Port A & B
   MCP23S17Select = 0; // Select Device
   Nop();
   Nop();
   SpiIO->PutCh(Address);
   SpiIO->PutCh(DEFAULT_LATCH);
   Delay5Cyclse();
   MCP23S17Select = 1; // Deselect Device
   Delay11Cyclse(); // Delay before next command
   //BusIO->SelectSPI1Mode(CurrentMode);
}

/***********************************************************************/
/*                                                                     */
/*     Function: Is data available to be read (DataAvailable)          */
/*        Input: None                                                  */
/*       Output: None                                                  */
/*                                                                     */
/*     Overview: This is a predicate to insure that the device is      */
/*               ready to be read.                                     */
/*                                                                     */
/***********************************************************************/
static Boolean DataAvailable(void){return True;}

/***********************************************************************/
/*                                                                     */
/*     Function:                                                       */
/*        Input: None                                                  */
/*       Output: None                                                  */
/*                                                                     */
/*     Overview:                                                       */
/*                                                                     */
/*                                                                     */
/***********************************************************************/
static void Mcp23S17Update(void)
{
   uint16_t Address;
   //SPI_MODES CurrentMode = BusIO->CurrentMode;
   //BusIO->SelectSPI1Mode(MODE_3);

   /* Check if value got there */
   Address = (READ_MCP23S17 | _GPIO);
   MCP23S17Select = 0; // Select Device
   Nop();
   Nop();
   SpiIO->PutCh(Address); // Write command to read the registers
   Delay5Cyclse();
   MCP23S17Registers[iGPIO] = SpiIO->GetWd(); // Get the results
   Delay5Cyclse();
   MCP23S17Select = 1; // Deselect Device

   Delay11Cyclse(); // Wait a small amount of time

   Delay11Cyclse(); // Wait a small amount of time
   Delay11Cyclse(); // Wait a small amount of time
   Delay11Cyclse(); // Wait a small amount of time
   Delay11Cyclse(); // Wait a small amount of time

   /* Check if value got there */
   Address = (READ_MCP23S17 | _OLAT);
   MCP23S17Select = 0; // Select Device
   Nop();
   Nop();
   SpiIO->PutCh(Address); // Write command to read the registes
   Delay5Cyclse();
   MCP23S17Registers[iOLAT] = SpiIO->GetWd(); // Get the results
   Delay5Cyclse();
   MCP23S17Select = 1; // Deselect Device
   //BusIO->SelectSPI1Mode(CurrentMode);
   GpIoReg->Value = MCP23S17Registers[iGPIO];
   GpIoReg->Changed = True;
   LATReg->Value = MCP23S17Registers[iOLAT];
   LATReg->Changed = True;
}

/***********************************************************************/
/*                                                                     */
/*     Function:                                                       */
/*        Input: None                                                  */
/*       Output: None                                                  */
/*                                                                     */
/*     Overview:                                                       */
/*                                                                     */
/*                                                                     */
/***********************************************************************/
static void Mcp23S17Write(byte Where, uint16_t What)
{
   uint16_t Address = (WRITE_MCP23S17 | Where);
   //SPI_MODES CurrentMode = BusIO->CurrentMode;
   //BusIO->SelectSPI1Mode(MODE_3);
   
   MCP23S17Select = 0; // Select Device
   Nop();
   Nop();
   SpiIO->PutCh(Address);
   SpiIO->PutCh(What);
   Delay5Cyclse();
   MCP23S17Select = 1; // Deselect Device
   //BusIO->SelectSPI1Mode(CurrentMode);
}

/***********************************************************************/
/*                                                                     */
/*     Function:                                                       */
/*        Input: None                                                  */
/*       Output: None                                                  */
/*                                                                     */
/*     Overview:                                                       */
/*                                                                     */
/*                                                                     */
/***********************************************************************/
static void LoadMcp23S17Registes(void)
{
   uint16_t Address;
   uint16_t i;
   uint16_t j;
   //SPI_MODES CurrentMode = BusIO->CurrentMode;
   //BusIO->SelectSPI1Mode(MODE_0);
   
   for (i = iIODIR, j = _IODIR; i <= iOLAT; i++, j += 2)
   {
      Address = (READ_MCP23S17 | i);
      MCP23S17Select = 0; // Select Device
      Nop();
      Nop();
      SpiIO->PutCh(Address); // Write command to read the registes
      Delay5Cyclse();
      MCP23S17Registers[i] = SpiIO->GetWd(); // Get the results
      Delay5Cyclse();
      MCP23S17Select = 1; // Deselect Device
   }
   //BusIO->SelectSPI1Mode(CurrentMode);
}

/***********************************************************************/
/*                                                                     */
/*     Function:                                                       */
/*        Input: None                                                  */
/*       Output: None                                                  */
/*                                                                     */
/*     Overview:                                                       */
/*                                                                     */
/*                                                                     */
/***********************************************************************/
static uint16_t ReadIn(byte Addrs)
{
   uint16_t Results = 0;
   uint16_t Address = (READ_MCP23S17 | Addrs);
   //SPI_MODES CurrentMode = BusIO->CurrentMode;
   //BusIO->SelectSPI1Mode(MODE_3);
   
   MCP23S17Select = 0; // Select Device
   Nop();
   Nop();
   SpiIO->PutCh(Address); // Write command to read the registes
   Delay5Cyclse();
   Results = SpiIO->GetWd(); // Get the results
   Delay5Cyclse();
   MCP23S17Select = 1; // Deselect Device
   //BusIO->SelectSPI1Mode(CurrentMode);
   return Results;
}

/***********************************************************************/
/*                                                                     */
/*     Function: IO Control                                            */
/*        Input: None                                                  */
/*       Output: None                                                  */
/*                                                                     */
/*     Overview: IO Control for Memory                                 */
/*                                                                     */
/***********************************************************************/
 uint16_t IOCTL(byte Operation)
{
   int Results = Operation; // Load default value

   switch(Operation)
   {
      case DefaultConfiguration:
         WriteDefaultConfiguration();
         Results = True;
         break;
      case LoadRegisters:
         LoadMcp23S17Registes();
         Results = True;
         break;
      default:
         Results = False;
   }

   return Results;
}

