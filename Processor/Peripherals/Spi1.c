/****************************************************************************/
/*                                                                          */
/*  Programmer: Gregory L Montgomery                                        */
/*                                                                          */
/*  Copyright © 2010-2019                                                   */
/*                                                                          */
/*  COPYING: (See the file COPYING.md for the GNU General Public License).  */
/*  this program is free software, and you may redistribute it and/or       */
/*  modify it under the terms of the GNU General Public License as          */
/*  published by the Free Software Foundation                               */
/*                                                                          */
/* This file is part of Gregory L Montgomery's code base collection Project.*/
/*                                                                          */
/*     Gregory L Montgomery's code base collection Project is free software:*/
/*     you can redistribute it and/or modify  it under the terms of the GNU */
/*     General Public License as published by the Free Software Foundation, */
/*     either version 3 of the License, or (at your option)                 */
/*     any later version.                                                   */
/*                                                                          */
/*     Gregory L Montgomery's code base collection Project is distributed   */
/*     in the hope that it will be useful, but WITHOUT ANY WARRANTY;        */
/*     without even the implied warranty of MERCHANTABILITY or FITNESS FOR  */
/*     A PARTICULAR PURPOSE.  See the GNU General Public License for more   */
/*     details.                                                             */
/*                                                                          */
/*     You should have received a copy of the GNU General Public License    */
/*     along with Gregory L Montgomery's code base collection Project.      */
/*     If not, see <https://www.gnu.org/licenses/>.                         */
/*                                                                          */
/****************************************************************************/
#include <xc.h>
#include <string.h>
#include "defs.h"
#include "System.h"
#include "Spi1.h"
#include "Spi.h"
#include "Fops.h"

static void SPI1InterruptsMode(Boolean);
static void SetSPI116BitMode(Boolean);
static void SelectSPI1Mode(SPI_MODES);
static void SPI1Reset(void);
static void Spi1Flush(void);
static void WriteSPI1(unsigned);
static int  ReadSPI1(void);
static void SetPrimaryPrescale(byte);
static void SetSecondPrescale(byte);
static Boolean BlockWriteSPI1(byte *BlockPtr, byte Length);
static Boolean BlockReadSPI1(byte *BlockPtr, byte Length);
static void SPI_Enable(Boolean);
static unsigned GetSPIWd1(void);
static Boolean GetSPI116BitMode(void);
static void ClrRequest(void);
static volatile IndicatorBits * ISS;

// Private variables/members
static Queue Spi1Tx;
static Queue Spi1Rx;
static byte Temp;

static struct SpiOperations _SPI1_ = 
{
   .IntModeOn = SPI1InterruptsMode,
   .SelectSPI1Mode = SelectSPI1Mode,
   .Set16BitMode = SetSPI116BitMode,
   .Get16BitMode = GetSPI116BitMode,
   .SelectPrimaryPrescale = SetPrimaryPrescale,
   .SelectSecondPrescale = SetSecondPrescale,
   .Enable = SPI_Enable,
   .Read = ReadSPI1,
   .Write = WriteSPI1,
   .BlockRead = BlockReadSPI1,
   .BlockWrite = BlockWriteSPI1
};

static struct FileOperations This = 
{
   .Available = 0,
   .Flush = Spi1Flush,
   .Reset = SPI1Reset,
   .GetCh = ReadSPI1,
   .GetWd = GetSPIWd1,
   .PutCh = WriteSPI1,
   .Release = ClrRequest,
   .Resource = _I2C_1,
   .PutStr = 0,
};

/****************************************************************************/
/*                                                                          */
/*     Function: InitSPI                                                    */
/*                                                                          */
/*        Input: None                                                       */
/*       Output: None                                                       */
/* Side Effects: Changes the internal chip settings for SPI                 */
/*     Overview: Initialize the use of SPI by the processor                 */
/* Notes:                                                                   */
/*                                                                          */
/****************************************************************************/
fops * InitSPI1(SPI_MODES Mode)
{ // Setup Port PINS for MOSI, MISO, SCK
   // Configure SPI1 Peripheral
   // Turn off SPI Interrupts
   IEC0bits.SPI1IE = 0;
   SPI1BUF = 0; // Page 170 of the data sheet Step 1 clear the SPIxBUF

   SPI1STATbits.SPISIDL = 0;
   SPI1STATbits.SPIROV &= 0; // Read status zero it out then write it back.
 
   SPI1CON1bits.DISSCK = 0; // Disable = 1 SCKx pin (Master mode only)
   SPI1CON1bits.DISSDO = 0; // Disable = 1 SD0x pin
   SPI1CON1bits.MODE16 = 0; // 8 bit mode
   SPI1CON1bits.SMP = 0; // Sample data in the middle of output time.
   SPI1CON1bits.SSEN = 0; // Slave Select pin SSx VIA I/O not SPI Module
   SPI1CON1bits.MSTEN = 1; // Master Enable = 1, Slave = 0
   SPI1CON1bits.SPRE = 0; // Secondary Prescale Selects 1:1
   SPI1CON1bits.PPRE = 0; // Primary Prescale -> Selects 1:1
   SelectSPI1Mode(Mode);    

   SPI1CON2 = 0x0000;
   
   SPI1InterruptsMode(False);

   // SPI is now set up so enable it.
   SPI1STATbits.SPIEN = 1;
   ISS = getSystemStat();
   This.Usr = (void*)&_SPI1_;
   return &This;
}

/****************************************************************************/
/*                                                                          */
/*     Function:                                                            */
/*                                                                          */
/*        Input: None                                                       */
/*       Output: None                                                       */
/* Side Effects:                                                            */
/*     Overview:                                                            */
/* Notes:                                                                   */
/*                                                                          */
/****************************************************************************/
static void SPI1InterruptsMode(Boolean InteruptsOn)
{
   IEC0bits.SPI1IE = 0;
   if (InteruptsOn)
   {
      IFS0bits.SPI1IF = 0; // Clear SPI1 Service Routeen Interrupt Flag
      IEC0bits.SPI1IE = 1; // Enable the SPI1 interrupt
      IEC0bits.SPI1IE = 1; // Enable the SPI1 Error Interrupt.
   }
   else
   {
      IFS0bits.SPI1IF = 0; // Clear SPI1 Service Routeen Interrupt Flag
      IEC0bits.SPI1IE = 0; // Enable the SPI1 interrupt
      IEC0bits.SPI1IE = 0; // Enable the SPI1 Error Interrupt.
   }
   IPC2bits.SPI1IP = 7; // Medium priority for SPI Interrupt
}

/****************************************************************************/
/*                                                                          */
/*     Function:                                                            */
/*                                                                          */
/*        Input: None                                                       */
/*       Output: None                                                       */
/* Side Effects:                                                            */
/*     Overview:                                                            */
/* Notes:                                                                   */
/*                                                                          */
/****************************************************************************/
static void ClrRequest(void){ISS->SPI1Rdy = 0; }

/****************************************************************************/
/*                                                                          */
/*     Function:                                                            */
/*                                                                          */
/*        Input: None                                                       */
/*       Output: None                                                       */
/* Side Effects:                                                            */
/*     Overview:                                                            */
/* Notes:                                                                   */
/*                                                                          */
/****************************************************************************/
static void SetSPI116BitMode(Boolean Mode)
{
   if (Mode)
      SPI1CON1bits.MODE16 = 1; // Word = 1 or Byte = 0 size communication
   else
      SPI1CON1bits.MODE16 = 0; // Word = 1 or Byte = 0 size communication
}

/****************************************************************************/
/*                                                                          */
/*     Function:                                                            */
/*                                                                          */
/*        Input: None                                                       */
/*       Output: None                                                       */
/* Side Effects:                                                            */
/*     Overview:                                                            */
/* Notes:                                                                   */
/*                                                                          */
/****************************************************************************/
static Boolean GetSPI116BitMode(void) { return (SPI1CON1bits.MODE16 == 1); }

/****************************************************************************/
/*                                                                          */
/*     Function: SelectMode                                                 */
/*                                                                          */
/*        Input: Which of the 4 SPI modes to be selected.                   */
/*       Output: None                                                       */
/* Side Effects: Changes the internal configuraton of the way SPI will      */
/*               sample it's data.                                          */
/*     Overview: This function selects which of the 4 SPI modes will be     */
/*               use. This is as specified in the Total Phase document as   */
/*               well as the SPI reference manual from Microchip.           */
/* Notes:                                                                   */
/*                                                                          */
/****************************************************************************/
static void SelectSPI1Mode(SPI_MODES Mode)
{
   _SPI1_.CurrentMode = Mode;
   switch (Mode)
   {
      case 0: // 0,1
         // Clock Plarity CPOL
         SPI1CON1bits.CKP = 0; // Idle high = 1 or Idle low = 0
         // Clock Phase   CPHA
         SPI1CON1bits.CKE = 0; // Data on Active->Idle = 1 Idle->Active->
         break;
      case 1: // 0,0
         // Clock Plarity CPOL
         SPI1CON1bits.CKP = 0; // Idle high = 1 or Idle low = 0
         // Clock Phase   CPHA
         SPI1CON1bits.CKE = 1; // Data on Active->Idle = 1 Idle->Active->
         break;
      case 2: // 1,1
         // Clock Plarity CPOL
         SPI1CON1bits.CKP = 1; // Idle high = 1 or Idle low = 0
         // Clock Phase   CPHA
         SPI1CON1bits.CKE = 0; // Data on Active->Idle = 1 Idle->Active->
         break;
      case 3: // 1,0
         // Clock Plarity CPOL
         SPI1CON1bits.CKP = 1; // Idle high = 1 or Idle low = 0
         // Clock Phase   CPHA
         SPI1CON1bits.CKE = 1; // Data on Active->Idle = 1 Idle->Active->
         break;
      default: // Default to Mode 0
         // Clock Plarity CPOL
         SPI1CON1bits.CKP = 0; // Idle high = 1 or Idle low = 0
         // Clock Phase   CPHA
         SPI1CON1bits.CKE = 0; // Data on Active->Idle = 1 Idle->Active->
   }
}

/****************************************************************************/
/*                                                                          */
/*     Function: Initialize varivariables                                   */
/*                                                                          */
/*        Input: None                                                       */
/*       Output: None                                                       */
/* Side Effects: None                                                       */
/*     Overview:                                                            */
/*                                                                          */
/* Notes:                                                                   */
/*                                                                          */
/****************************************************************************/
static void SetPrimaryPrescale(byte Scale)
{
   SPI1CON1bits.PPRE = Scale; // Primary Prescale 
}

/****************************************************************************/
/*                                                                          */
/*     Function:                                                            */
/*                                                                          */
/*        Input: None                                                       */
/*       Output: None                                                       */
/* Side Effects: None                                                       */
/*     Overview:                                                            */
/*                                                                          */
/* Notes:                                                                   */
/*                                                                          */
/****************************************************************************/
static void SetSecondPrescale(byte Scale)
{
   SPI1CON1bits.SPRE = Scale; // Secondary Prescale
}

/****************************************************************************/
/*                                                                          */
/*     Function:                                                            */
/*                                                                          */
/*        Input: None                                                       */
/*       Output: None                                                       */
/* Side Effects: None                                                       */
/*     Overview:                                                            */
/*                                                                          */
/* Notes:                                                                   */
/*                                                                          */
/****************************************************************************/
static void SPI_Enable(Boolean Enabled)
{
   if(Enabled)
      SPI1STATbits.SPIEN = 1;
   else
      SPI1STATbits.SPIEN = 0;
}

/****************************************************************************/
/*                                                                          */
/*     Function:                                                            */
/*                                                                          */
/*        Input: None                                                       */
/*       Output: None                                                       */
/* Side Effects: None                                                       */
/*     Overview: SPI Transmit/Receive                                       */
/*                                                                          */
/* Notes:                                                                   */
/*                                                                          */
/****************************************************************************/
static void SPI1Reset(void)
{
   memset((char*) &Spi1Tx, 0, sizeof (Queue));
   memset((char*) &Spi1Rx, 0, sizeof (Queue));
}

/****************************************************************************/
/*                                                                          */
/*     Function: Spi1Fluch                                                  */
/*                                                                          */
/*        Input: None                                                       */
/*       Output: None                                                       */
/*     Overview: This function is to flush the SPI via turning off the      */
/*                                                                          */
/*                                                                          */
/****************************************************************************/
static void Spi1Flush(void)
{
   IFS0bits.SPI1IF = 1;
}

/****************************************************************************/
/*                                                                          */
/*     Function: BlockWriteSPI1                                             */
/*                                                                          */
/*        Input: The pointer to the block of memory which is to be written  */
/*               to the SPI device.                                         */
/*               How many bytes are to be written.                          */
/*       Output: None                                                       */
/* Side Effects:                                                            */
/*     Overview: This routine writes a string to the SPI bus.               */
/* Notes:                                                                   */
/*                                                                          */
/****************************************************************************/
static Boolean BlockWriteSPI1(byte *BlockPtr, byte Length)
{
   volatile byte Info;
   volatile byte Count = Length;
   do // transmit data until PageSize
   {
      Info = *BlockPtr++;
      SPI1BUF = Info; // initiate SPI bus cycle
      while((!SPI1STATbits.SPITBF) && (SPI1STATbits.SPIRBF)); // While still transmitting
      while(!SPI1STATbits.SPIRBF); // While receive is not finished
      Info = SPI1BUF;
   }while(--Count);
   return True;
}

/****************************************************************************/
/*                                                                          */
/*     Function:                                                            */
/*                                                                          */
/*        Input: None                                                       */
/*       Output: None                                                       */
/* Side Effects:                                                            */
/*     Overview:                                                            */
/* Notes:                                                                   */
/*                                                                          */
/****************************************************************************/
/********************************************************************
 *     Function Name:    getsSPI                                     *
 *     Return Value:     void                                        *
 *     Parameters:       address of read string storage location and *
 *                       length of string bytes to read              *
 *     Description:      This routine reads a string from the SPI    *
 *                       bus.  The number of bytes to read is deter- *
 *                       mined by parameter 'length'.                *
 ********************************************************************/
static Boolean BlockReadSPI1(byte *BlockPtr, byte Length)
{
   do // stay in loop until length = 0
   {
      SPI1STATbits.SPIROV &= 0;
      SPI1BUF = 0x00; // initiate bus cycle
      while (!SPI1STATbits.SPIRBF);
      /* Check for Receive buffer full status bit of status register*/
      if (SPI1STATbits.SPIRBF)
      {
         SPI1STATbits.SPIROV &= 0;
         if (SPI1CON1bits.MODE16)
            *BlockPtr++ = SPI1BUF; /* return word read */
         else
            *BlockPtr++ = (byte) XBty(SPI1BUF); /* return byte read */
      }
   }while (--Length); // reduce string length count by 1
   return True;
}

/****************************************************************************/
/*                                                                          */
/*     Function:                                                            */
/*                                                                          */
/*        Input: None                                                       */
/*       Output: None                                                       */
/* Side Effects:                                                            */
/*     Overview:                                                            */
/* Notes:                                                                   */
/*                                                                          */
/****************************************************************************/
/********************************************************************
 *     Function Name : WriteSPI1                                     *
 *     Description   : This routine writes a single byte/word to     *
 *                     the SPI bus.                                  *
 *     Parameters    : Single data byte/word for SPI bus             *
 *     Return Value  : None                                          *
 ********************************************************************/
static void WriteSPI1(unsigned Data)
{
   unsigned Tmp = XBty(Data);
   if (SPI1CON1bits.MODE16) /* word write */
      SPI1BUF = Data;
   else
      SPI1BUF = Tmp; //XBty(Data); /*  byte write  */

   while((!SPI1STATbits.SPITBF) && (SPI1STATbits.SPIRBF)); // While still transmitting
   while(!SPI1STATbits.SPIRBF); // While receive is not finished

   Data = SPI1BUF; //Avoiding overflow when reading
}


/****************************************************************************/
/*                                                                          */
/*     Function:                                                            */
/*                                                                          */
/*        Input: None                                                       */
/*       Output: None                                                       */
/* Side Effects:                                                            */
/*     Overview:                                                            */
/* Notes:                                                                   */
/*                                                                          */
/****************************************************************************/
/******************************************************************************
 *     Function Name :   ReadSPI1                                              *
 *     Description   :   This function will read single byte/ word  from SPI   *
 *                       bus. If SPI is configured for byte  communication     *
 *                       then upper byte of SPIBUF is masked.                  *
 *     Parameters    :   None                                                  *
 *     Return Value  :   contents of SPIBUF register                           *
 ******************************************************************************/
static int ReadSPI1(void)
{
   SPI1STATbits.SPIROV &= 0;
   SPI1BUF = 0x00; // initiate bus cycle

   while (!SPI1STATbits.SPIRBF);
   /* Check for Receive buffer full status bit of status register*/
   if (SPI1STATbits.SPIRBF)
   {
      SPI1STATbits.SPIROV &= 0;
      return SPI1BUF; /* return word read */
   }
   return -1; /* RBF bit is not set return error*/
}

/******************************************************************************
 *     Function Name :   ReadSPI1                                              *
 *     Description   :   This function will read single byte/ word  from SPI   *
 *                       bus. If SPI is configured for byte  communication     *
 *                       then upper byte of SPIBUF is masked.                  *
 *     Parameters    :   None                                                  *
 *     Return Value  :   contents of SPIBUF register                           *
 ******************************************************************************/
//static unsigned GetSPIWd1(void)
//{
//   SPI1STATbits.SPIROV &= 0;
//   SPI1BUF = 0x00; // initiate bus cycle
//   while (!SPI1STATbits.SPIRBF);
//   /* Check for Receive buffer full status bit of status register*/
//   if (SPI1STATbits.SPIRBF)
//   {
//      SPI1STATbits.SPIROV &= 0;
//      
//   }
//   return SPI1BUF; /* return word read */
//}

 static unsigned GetSPIWd1(void)
{
   SPI1STATbits.SPIROV &= 0;
   SPI1BUF = 0x00; // initiate bus cycle

   while((!SPI1STATbits.SPITBF) && (SPI1STATbits.SPIRBF)); // While still transmitting
   while(!SPI1STATbits.SPIRBF); // While receive is not finished

   return SPI1BUF;
 }

/****************************************************************************/
/*     Function:                                                            */
/*                                                                          */
/*        Input: None                                                       */
/*       Output: None                                                       */
/* Side Effects: None                                                       */
/*     Overview:                                                            */
/*                                                                          */
/* Notes:                                                                   */
/*                                                                          */
/****************************************************************************/
void __attribute__((interrupt, auto_psv))_SPI1Interrupt(void)
{
   // Receive
   Temp = SPI1BUF;
   if (Spi1Rx.Count < IO_BUFFER_SIZE)
   {
      Spi1Rx.Buffer[Spi1Rx.Tail++] = Temp;
      Spi1Rx.Tail &= IO_RAPAROUND;
      Spi1Rx.Count++;
   }

   // Transmit
   if (Spi1Tx.Count)
   {
      SPI1BUF = Spi1Tx.Buffer[Spi1Tx.Head++];
      Spi1Tx.Head &= IO_RAPAROUND;
      Spi1Tx.Count--;
   }

   // Clear the overflow flag.
   SPI1STATbits.SPIROV &= 0;

   // Clear the SPI fault status bit and fault interrupt bits.
   IFS0bits.SPI1IF = 0;

   // Enable the event interrupt
   IEC0bits.SPI1IE = 1;
}

/****************************************************************************/
/*     Function:                                                            */
/*                                                                          */
/*        Input: None                                                       */
/*       Output: None                                                       */
/* Side Effects: None                                                       */
/*     Overview:                                                            */
/*                                                                          */
/* Notes:                                                                   */
/*                                                                          */
/****************************************************************************/
void __attribute__((interrupt, no_auto_psv))_SPI1ErrInterrupt(void)
{
   /*
     Standard ISR handler code here:
     Clear the interrupt flag (set in hardware when interrupt occured), disable
     further interrupts on the SPI.
    */
   IFS2bits.SPI2IF = 0;
   IEC2bits.SPI2IE = 0;
}
