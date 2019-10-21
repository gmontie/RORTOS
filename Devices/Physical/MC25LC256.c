#include <xc.h>
#include "defs.h"
#include "System.h"
#include "ProtoOps.h"
#include "SerDev.h"
#include "MC25LC256.h"
#include "Fops.h"

//#define DELAY_COUNTS 3
//#define SPI_DELAY(x)for(x = 0; x < DELAY_COUNTS; x++) asm("nop");
#define TEST_LENGTH 20
#define DISCRIPTION_M25LC256 "MC25LC256 SPI Mem Device"
//#define MC25LC256Select PORTBbits.RB9
#define MC25LC256Select PORTBbits.RB10

// Definitions for 25xx256 Instruction Constents
typedef enum
{
   ReadInst =  0b0000011, // Read data from memory array beginning at selected address
   WriteInst = 0b00000010, // Write data to memory array beginning at selected address
   WrDiInst =  0b00000100, // Reset the write enable latch (disable write operations)
   WrEnInst =  0b00000110, // Set the write enable latch (enable write operations)
   RdSrInst =  0b00000101, // Read STATUS register
   WrSrInst =  0b00000001, // Write STATUS register
} LC256_InstructionSet;

static const LC256_InstructionSet READ = ReadInst;
static const LC256_InstructionSet WRITE = WriteInst;
static const LC256_InstructionSet WRDI = WrDiInst;
static const LC256_InstructionSet WREN = WrEnInst;
static const LC256_InstructionSet RDSR = RdSrInst;
static const LC256_InstructionSet WRSR = WrSrInst;

// Public Block Read/Block Write command through Object interface
// Otherwise private functions
static void ByteWriteMem(unsigned Address, byte Data );
static int ByteReadMem(unsigned Address);
static Boolean BlockWriteMem( unsigned Address, byte *DataPtr, byte Length );
static Boolean BlockReadMem( unsigned Address, byte *DataPtr, byte Length );
static void WIPPolling(void);
static void WriteEnable(void);
static int  ReadStatus(void);
static void PutChMem(unsigned);
static void PutStrMem(char *);
static void FlushAdrs(void);
static Boolean DataAvailable(void);
static int GetChMem(void);
static int IOCTL(unsigned Operation);

// Private Member Objects
static RegisterFile * Element;
static SerDev * This;
static sops * SpiIO;

// Private Members
static int PreviousAddress;
static byte DelayCounter;

// Private File Operations Object
static fops MC25LRomDevice =
{
   .BlockRead = BlockReadMem,
   .BlockWrite = BlockWriteMem,
   .Available = DataAvailable,
   .Flush = FlushAdrs,
   .GetCh = GetChMem,
   .IOCTL = IOCTL,
   .IndexPtr = 0, // No circular buffer used to Index is always 0
   .PutCh = PutChMem,
   .PutStr = PutStrMem
};

/***********************************************************************/
/*                                                                     */
/*     Function: InitMC25LC256                                         */
/*                                                                     */
/*        Input: A Pointer to the Register File                        */
/*               A Pointer to the Spi Object which interfaces to the   */
/*               MC25LC256                                             */
/*       Output: A Pointer to the file operations which this Object    */
/*               can perform.                                          */
/*                                                                     */
/*     Overview: This function Istantiates and initializes a MC25LC256 */
/*               file object.                                          */
/*                                                                     */
/***********************************************************************/
struct FileOperations * InitMC25LC256(RegisterFile * Regs, sops * anSpiDev)
{
   MC25LC256Select = 1; //Deselect Device
   SpiIO = anSpiDev;
   This = NewProtocallDevice(SpiIO);
   This->Active = True;
   This->Discription = DISCRIPTION_M25LC256;
   Element = Regs;
   return &MC25LRomDevice;
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
/*     Function: Flush/Reset device.                                   */
/*        Input: None                                                  */
/*       Output: None                                                  */
/*                                                                     */
/*     Overview: This is a predicate to insure that the device is      */
/*               ready to be read.                                     */
/*                                                                     */
/***********************************************************************/
static void FlushAdrs(void){PreviousAddress = 0;}

/***********************************************************************/
/*                                                                     */
/*     Function: WriteEnable                                           */
/*        Input: None                                                  */
/*       Output: None                                                  */
/*                                                                     */
/*     Overview: This routine sets the Write Enable Latch              */
/*                                                                     */
/***********************************************************************/
static void WriteEnable(void)
{
   MC25LC256Select = 0; //Select Device
   SpiIO->Write(WREN); 
   MC25LC256Select = 1; //Deselect Device
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
static int IOCTL(unsigned Operation)
{
   int Results = 0; // Load default value

   switch(Operation)
   {
      case 1: // Read Status
         Results = ReadStatus();
         break;
      case 2: // Write Enable
         WriteEnable();
         break;
      case 3: // Return Page Size
         Results = 64;
         break;
      case GET_MEM_SIZE:
         Results = 256;
         break;
   }
   return Results;
}

/***********************************************************************/
/*                                                                     */
/*     Function: Read Status                                           */
/*        Input: None                                                  */
/*       Output: None                                                  */
/*                                                                     */
/*     Overview: This routine reads the Status Register                */
/*                                                                     */
/***********************************************************************/
static int ReadStatus(void)
{
   int Results;

   MC25LC256Select = 0; // Select Device
   SpiIO->Write(RDSR);
   Results = SpiIO->Read();
   MC25LC256Select = 1; // Deselect Device
   return Results;
}

/***********************************************************************/
/*                                                                     */
/*     Function: PutChMem                                              */
/*        Input: The data to be written into memory                    */
/*       Output: None                                                  */
/*                                                                     */
/*     Overview: This function writes one byte of data to the next     */
/*               address after the previous address used.              */
/*                                                                     */
/***********************************************************************/
static void PutChMem(unsigned Value)
{
   ByteWriteMem(++PreviousAddress,(byte)XBty(Value));
}

/***********************************************************************/
/*                                                                     */
/*     Function: Put String into Memory (PutStrMem)                    */
/*        Input: A pointer to a character string                       */
/*       Output: None                                                  */
/*                                                                     */
/*     Overview: This function writes a string to the 25x256 memory    */
/*               starting at the next address after the previous       */
/*               address used.                                         */
/*                                                                     */
/***********************************************************************/
static void PutStrMem(char * Msg)
{
   if (Msg != 0)
      while (*Msg++)
         ByteWriteMem(++PreviousAddress, (byte) * Msg);
}

/***********************************************************************/
/*                                                                     */
/*     Function: GetChMem                                              */
/*        Input: None                                                  */
/*       Output: The data which was read from memory                   */
/*                                                                     */
/*     Overview: This function reads one byte of data to the next      */
/*               address after the previous address used.              */
/*                                                                     */
/***********************************************************************/
static int GetChMem(void)
{
   return ByteReadMem(++PreviousAddress);
}

/***********************************************************************/
/*                                                                     */
/*     Function: Byte Write Mem                                        */
/*        Input: The address of where in the 25x256 to write the byte  */
/*               to.                                                   */
/*               The byte of Data which is to be written.              */
/*       Output: None                                                  */
/*                                                                     */
/*     Overview: Write one byte of data to the 25x256 memory device.   */
/*                                                                     */
/***********************************************************************/
void ByteWriteMem(unsigned Address, byte Data)
{
   MC25LC256Select = 0; // Select Device
   SpiIO->Write(WRITE);
   SpiIO->Write(XBty(Address >> 8) );
   SpiIO->Write(XBty(Address) );
   SpiIO->Write(Data);
   MC25LC256Select = 1; // Deselect Device
   PreviousAddress = Address; // Keep last address used
   WIPPolling(); // Wait for Write to Complete
}

/***********************************************************************/
/*                                                                     */
/*     Function: Block Write Memory (BlockWriteMem)                    */
/*        Input: The Address of where to start writing.                */
/*               A pointer to the block of data which is to be written */
/*               in to memory                                          */
/*               The size of the block of data which is to be written  */
/*       Output: Success or failure. Since this is not really a        */
/*               communications device True is currently returned.     */
/*                                                                     */
/*     Overview: This is a binary block writing routeen.               */
/*                                                                     */
/***********************************************************************/
static Boolean BlockWriteMem(unsigned Address, byte *DataPtr, byte Length)
{
   MC25LC256Select = 0; // Select Device
   SpiIO->Write(WRITE);
   SpiIO->Write(XBty(Address >> 8));
   SpiIO->Write(XBty(Address));
   SpiIO->BlockWrite(DataPtr, Length);
   MC25LC256Select = 1; // Deselect Device
   PreviousAddress = Address; // Keep last address used
   WIPPolling(); // Wait for Write to Complete

   return True;
}

/***********************************************************************/
/*                                                                     */
/*     Function: Byte Read Memory (ByteReadMem)                        */
/*        Input: The address in memory of where to read one byte of    */
/*               data from.                                            */
/*       Output: None                                                  */
/*                                                                     */
/*     Overview: Read one byte of data from memory located at Address  */
/*                                                                     */
/***********************************************************************/
static int ByteReadMem(unsigned Address)
{
   int Results = 0;

   MC25LC256Select = 0; // Select Device
   SpiIO->Write(READ);
   SpiIO->Write(XBty(Address >> 8));
   SpiIO->Write(XBty(Address));
   Results = (int)SpiIO->Read();
   MC25LC256Select = 1; // Deselect Device
   return Results;
}

/***********************************************************************/
/*                                                                     */
/*     Function: Block Read Memory (BlockReadMem)                      */
/*        Input: The Address of where to start reading data.           */
/*               A pointer to the location for the data destination:   */
/*               Data read in from memory will go to where the pointer */
/*               location points.                                      */
/*               The size of the block of data which is to be read     */
/*       Output: Success or failure. Since this is not really a        */
/*               communications device True is currently returned.     */
/*                                                                     */
/*     Overview: This function reads in a block of data from memory    */
/*                                                                     */
/***********************************************************************/
static Boolean BlockReadMem(unsigned Address, byte *DataPtr, byte Length)
{
   MC25LC256Select = 0; // Select Device
   SpiIO->Write(READ);
   SpiIO->Write(XBty(Address >> 8));
   SpiIO->Write(XBty(Address));
   SpiIO->BlockRead(DataPtr, Length);
   MC25LC256Select = 1; // Deselect Device
   PreviousAddress = Address; // Keep last address used
   return True;
}

/***********************************************************************/
/*                                                                     */
/*     Function: WIPPolling                                            */
/*        Input: None                                                  */
/*       Output: None                                                  */
/*                                                                     */
/*     Overview: This routine loops until WIP = 0                      */
/*                                                                     */
/***********************************************************************/
static void WIPPolling(void)
{
   EEPromStatus Status;
   do
   {

      for(DelayCounter = 0; DelayCounter < 200; DelayCounter++)
      {
         asm("nop");
      }

      MC25LC256Select = 0; //Select Device
      SpiIO->Write(RDSR); 
      Status.Stat = SpiIO->Read();
      MC25LC256Select = 1; //Deselect Device
   }while (Status.WIP); //Check for WIP bit Set
}
