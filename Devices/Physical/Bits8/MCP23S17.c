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

#define Delay5Cyclse() Nop(); Nop(); Nop(); Nop(); Nop(); 
#define Delay11Cyclse() Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop();

#define NBR_BYTE_REGISTERS     0x15
#define DISCRIPTION_M25LC256  "MCP23S17 SPI IO Expander Device"
#define MCP23S17Select         CS2

#define DEFAULT_CONFIG 0x00 // Defualt Configuration for MCP23S17
#define DEFAULT_IODIR  0x00 // All are outputs
#define DEFAULT_LATCH  0xAA // Default Latch output

#define WRITE_MCP23S17 0x40
#define READ_MCP23S17  0x41

int MCP23S17Registers[NBR_BYTE_REGISTERS];

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
static SpiOps * BusIO;

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
   
   LoadMcp23S17Registes();
   LoadMcp23S17Registes();
   WriteDefaultConfiguration();
   Mcp23S17Write(GPIOA, 0x55);
   Mcp23S17Write(GPIOB, 0xAA);
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
   // Turn the lights on!
   MCP23S17Select = 0; // Select Device   
   Delay11Cyclse();
   SpiIO->PutCh(WRITE_MCP23S17);
   SpiIO->PutCh(IODIRA);
   SpiIO->PutCh(0); // Set all to outputs in IODIRA
   SpiIO->PutCh(0); // Set all to outputs in IODIRB
   Delay11Cyclse();
   MCP23S17Select = 1; // Deselect Device
   Delay11Cyclse();
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
   if (GpIoReg->Value != MCP23S17Registers[OLATA])
   {
      MCP23S17Select = 0; // Select Device
      Nop();
      Nop();
      SpiIO->PutCh(WRITE_MCP23S17);
      SpiIO->PutCh(GPIOA);
      SpiIO->PutCh(GpIoReg->Value);
      Delay11Cyclse();
      MCP23S17Select = 1; // Deselect Device
   }
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
   MCP23S17Select = 0; // Select Device
   Nop();
   Nop();
   SpiIO->PutCh(WRITE_MCP23S17);
   SpiIO->PutCh(Where);
   SpiIO->PutCh(What);
   Delay5Cyclse();
   MCP23S17Select = 1; // Deselect Device
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
   uint16_t i = IODIRA;
   
   MCP23S17Select = 0; // Select Device
   Delay5Cyclse(); // Wait a small amount of time
   
   SpiIO->PutCh(READ_MCP23S17);
   SpiIO->PutCh(i);
   for(i = IODIRA; i <= OLATB; i++)
   {
      SpiIO->PutCh(0);
      Delay11Cyclse();
      MCP23S17Registers[i] = (byte)SpiIO->GetCh(); // Get the results
   }
   Delay11Cyclse(); // Wait a small amount of time
   MCP23S17Select = 1; // Deselect Device
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
static uint16_t ReadIn(byte Where)
{
   uint16_t Results = 0;
   MCP23S17Select = 0; // Select Device
   Delay5Cyclse(); // Wait a small amount of time
   
   SpiIO->PutCh(READ_MCP23S17);
   SpiIO->PutCh(Where);
   SpiIO->PutCh(0); // Write command to read the registes
   Delay5Cyclse();
   Results |= SpiIO->GetCh(); // Get the results
   Delay5Cyclse();
   MCP23S17Select = 1; // Deselect Device
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

