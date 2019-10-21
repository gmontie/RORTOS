/**************************************************************************/
/*                                                                        */
/*  Programmer: Gregory L Montgomery                                      */
/*                                                                        */
/*  COPYING: (See the file COPYING for the GNU General Public License).   */
/*  this program is free software, and you may redistribute it and/or     */
/*  modify it under the terms of the GNU General Public License as        */
/*  published by the Free Software Foundation                             */
/*                                                                        */
/*  This program is distributed in the hope that it will be useful,       */
/*  but WITHOUT ANY WARRANTY; without even the implied warranty of        */
/*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                  */
/*  See the GNU General Public License for more details.                  */
/*                                                                        */
/*  You should have received a copy of the GNU General Public License     */
/*  along with this program. See the file 'COPYING' for more details      */
/*  if for some reason you did not get a copy of the GNU General Public   */
/*  License a copy can be obtained by write to the Free Software          */
/*  Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */
/*                                                                        */
/*  You are welcome to use, share and improve this program.               */
/*  You are forbidden to prevent anyone else to use, share and improve    */
/*  what you give them.                                                   */
/*                                                                        */
/**************************************************************************/
/*
 * File:   main.c
 * Author: glm
 */
#ifdef PIC24
#include <p24Fxxxx.h>
#endif

#ifdef dsPIC33
#include <p33Fxxxx.h>
#endif

#include <i2c.h>
#include "A24Cxxx.h"

// Public interface through struct... otherwise private!
static Boolean WriteDeviceAt(unsigned Address, byte * Data, byte Length);
static Boolean ReadDeviceAt(unsigned Address, byte * Data, byte Length);

// Private Members.
static const byte ControlStatusLength = 3;
static byte ControlByte;
static int Index = 0;
static byte Ary[10];

static fops This =
{
    .BlockRead = ReadDeviceAt,
    .BlockWrite = WriteDeviceAt,
    .Available = False,
    .Flush = 0,
    .GetCh = 0,
    .IndexPtr = 0,
    .PutCh = 0,
    .PutStr = 0
};


/*****************************************************************************/
/*                                                                           */
/*****************************************************************************/
fops * InitA24CXXX(void)
{
   // Enable channel
   //OpenI2C1(I2C_ON, I2C_BRG);
   I2C1CONbits.I2CEN = 1; // Enable I2C1 Module.
   I2C1CONbits.STREN = 1; // Enable receive clock stretching

   I2C1BRG = I2C_BRG;
   
   // [Start Bit]    [Control Code] [Chip Slect Bits] [Read/Write]
   //     |                |                |              |
   //     |                |                |              |
   //     ----------       |                |              |
   //               |  ----|                |              |
   //               |  | -------------------|              |
   //               |  | | --------------------------------|
   //               |  | | |
   //               |  | | |
   ControlByte = 0b10100000; // Serial EEPROM Control Byte

   return &This;
}

/*****************************************************************************/
/*                                                                           */
/*****************************************************************************/
static Boolean WriteDeviceAt(unsigned Address, byte * Data, byte Length)
{
   Boolean Results = True;

   IdleI2C1(); // Wait to complete
   StartI2C1(); // Send the Start Bit

   if (Length > PAGE_LENGTH_24CXXX)
   {
      Results = False;
      Length = PAGE_LENGTH_24CXXX;
   }

   // Handle writing the Control Byte and 2 byte Address
   Ary[0] = ControlByte;
   Ary[1] = (byte) ((Address >> 8) & 0x00FF); // Address High
   Ary[2] = (byte) (Address & 0x00FF);
   Index = 0; // Index Starts from 0

   // Wait until start sequence is complete
   while (I2CCONbits.SEN);

   do
   { // Write Control Byte and Mem Address
      MasterWriteI2C1(Ary[Index++]);
      // Wait transmition to comlete
      while (I2CSTATbits.TBF);
   }while ((Index < ControlStatusLength) && (!I2CSTATbits.ACKSTAT));

   if (!I2CSTATbits.ACKSTAT)
   {
      do
      { // Write data to I2C Device
         MasterWriteI2C1(*Data++);
         // Wait transmition to comlete
         while (I2CSTATbits.TBF);
      }while ((--Length) && (!I2CSTATbits.ACKSTAT));

      if (I2CSTATbits.ACKSTAT)
         Results = False;
   }
   else
      Results = False;

   StopI2C1();
   while (I2C1CONbits.PEN);

   return Results;
}

/********************************************************************/
/*                                                                  */
/********************************************************************/
static Boolean ReadDeviceAt(unsigned Address, byte * Data, byte Length)
{
   Boolean Results = True;

   if (Length > PAGE_LENGTH_24CXXX)
   {
      Results = False;
      Length = PAGE_LENGTH_24CXXX;
   }

   // Handle writing the Control Byte and 2 byte Address
   Ary[0] = ControlByte;
   Ary[1] = (byte) ((Address >> 8) & 0x00FF); // Address High
   Ary[2] = (byte) (Address & 0x00FF); // Address High
   Index = 0; // Index Starts from 0

   IdleI2C1(); // Wait to complete
   StartI2C1(); // Send the Start Bit

   // send the address to read from the serial eeprom
   Index = 0;

   // Wait until start sequence is complete
   while (I2CCONbits.SEN);

   do
   { // Write Control Byte and Mem Address
      MasterWriteI2C1(Ary[Index++]);
      // Wait transmition to comlete
      while (I2CSTATbits.TBF);
   }while ((Index < ControlStatusLength) && (!I2C1STATbits.ACKSTAT));

   if (!I2C1STATbits.ACKSTAT)
   {
      StopI2C1();
      while (I2C1CONbits.PEN);

      // Now send restart Sequence
      RestartI2C1(); // Send the Restart condition
      I2C_Delay(100); // Waite for a little while

      // Transmit Control byte with
      MasterWriteI2C1((ControlByte | 1));
      // Wait transmition to comlete
      while (I2CSTATbits.TBF);

      if (!I2C1STATbits.ACKSTAT)
      {
         /*
         IdleI2C1(); // Wait to complete
         if((Status = (MastergetsI2C1(Length, Data, 100)) != 0))
            Results = False;
          */

         do
         {
            *Data++ = MasterReadI2C1(); // read one byte
            AckI2C1();
            IdleI2C1(); // Wait to complete
         }while (--Length);
      }
      else
         Results = False;
   }
   else
      Results = False;

   StopI2C1(); // Send the Stop condition
   IdleI2C1(); // Wait to complete

   return Results;
}
