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
#include <p33Fxxxx.h>
#include <stdio.h>
#include <string.h>
#include "defs.h"
#include "Fops.h"
#include "System.h"
#include "Uart1.h"

// Maybe there is a way around including these in this file.
#include "Register.h"
#include "Registers.h"

static Queue Uart1Tx;
static Queue Uart1Rx;
static Boolean Uart1Txing;

// Global for debugging
static unsigned long _Fcy;
static unsigned long _Fosc;

static volatile IndicatorBits * ISS;

static void UART1Init( long BaudRate );
static void Uart1Reset( void );
static void Uart1SetBaudRate(unsigned long Baud);
static void Uart1PutChar(unsigned Ch);
static int Uart1GetChar( void );
static void Uart1WriteStr(char *);
static void Uart1WriteConstStr(const char *);
static Boolean Uart1CharAvail(void);
static void ClearBit(void);

static volatile unsigned short TimeOut;

static fops Uart1Device =
{
   .Flush    = Uart1Reset,
   .PutCh    = Uart1PutChar,
   .GetCh    = Uart1GetChar,
   .PutStr   = Uart1WriteStr,
   .PutCnst  = Uart1WriteConstStr,
   .Available = Uart1CharAvail,
   .Reset    = Uart1Reset,
   .Release  = ClearBit,
   .Usr      = &Uart1Rx.Count,  
   .Resource = _Uart_1
};

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
struct FileOperations * InitUart1(void)
{
    UART1Init(115200L);
    ISS = getSystemStat();
    return &Uart1Device;
}

/****************************************************************************/
/*     Function: Clear indicator bit set by interrupt                       */
/*                                                                          */
/*        Input: None                                                       */
/*       Output: None                                                       */
/* Side Effects: None                                                       */
/*     Overview:                                                            */
/*                                                                          */
/* Notes:                                                                   */
/*                                                                          */
/****************************************************************************/
static void ClearBit(void){ISS->Uart1Rdy = 0;}

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
void UART1Init(long BaudRate)
{
   // Set directions of UART IOs
   U1MODE = 0;
   U1STA = 0;

   U1MODEbits.RTSMD = 1;
   U1MODEbits.UARTEN = 1;
   U1MODEbits.USIDL = 0; // Form operation in Idle Mode

   U1STAbits.UTXEN = 1;

   // reset RX flag
   IPC2bits.U1RXIP = 6; // Set as highest Interrupt priority
   IFS0bits.U1RXIF = 0;
   IEC0bits.U1RXIE = 1; // Enable Receive Interrupts

   Uart1Reset();

   Uart1SetBaudRate(BaudRate); // Otherwise use the users Baud rate value.
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
void Uart1Reset(void)
{
   memset((char*) &Uart1Tx, 0, sizeof (Queue));
   memset((char*) &Uart1Rx, 0, sizeof (Queue));
   Uart1Txing = False;
}

/****************************************************************************/
/*     Function: Uart1_SetBaudRate                                          */
/*                                                                          */
/*        Input: The desired baud rate.                                     */
/*       Output: None                                                       */
/* Side Effects: Changes the hardware baud rate for uart 1                  */
/*                                                                          */
/* Notes:                                                                   */
/*                                                                          */
/****************************************************************************/
void Uart1SetBaudRate(unsigned long Baud)
{
    //unsigned long Fcy = ((CRYSTAL) >> 1);
    unsigned long Results = 0;
     // the max baud rate with BRGH=0 is FCY/16
    _Fcy = Fcy;
    _Fosc = Fosc;
    if (Baud > (Fcy >> 4)) // Baud > (Fcy / 16)
    {
         // calc BRG with high baud rate (BRGH = 1)
         U1MODEbits.BRGH = 1;
         Results = ((Fcy / Baud) / 4) - 1;
    }
    else
    {
         // calc BRG with low baud rate (BRGH = 0)
         U1MODEbits.BRGH = 0;
         Results = ((Fcy / Baud) / 16) - 1;
    }
    //Results = (_Fcy / (Baud << 4) )  - 1; // (Baud * 16) - 1
    U1BRG = Results;
}

/****************************************************************************/
/*     Function: PutChar                                                    */
/*                                                                          */
/*        Input: One character to be sent out of the UART one               */
/*       Output: None                                                       */
/* Side Effects: None                                                       */
/*     Overview: Transmit one character of information out of UART 1        */
/*                                                                          */
/* Notes:                                                                   */
/*                                                                          */
/****************************************************************************/
void Uart1PutChar(unsigned Ch)
{	
    while(U1STAbits.UTXBF == 1);
    asm("DISI  #0x03");
    U1TXREG = (char)Ch;
}

/****************************************************************************/
/*                                                                          */
/*     Function:                                                            */
/*        Input: None                                                       */
/*       Return: None                                                       */
/*     Overview:                                                            */
/*                                                                          */
/****************************************************************************/
Boolean Uart1CharAvail(void)
{
    return ((Uart1Rx.Count == 0) ? False : True);
}

/****************************************************************************/
/*     Function: GetChar                                                    */
/*                                                                          */
/*        Input: None                                                       */
/*       Output: The next character available, or an error code indicating  */
/*               that there are no characters available.                    */
/* Side Effects: None                                                       */
/*     Overview: This function gets the next character from the internal    */
/*               uart buffer.                                               */
/* Notes:                                                                   */
/*                                                                          */
/****************************************************************************/
int Uart1GetChar(void)
{
   unsigned Results = DEVICE_EMPTY;

   if (Uart1Rx.Count > 0)
   {
      Results = Uart1Rx.Buffer[Uart1Rx.Head++];
      Uart1Rx.Head &= (IO_RAPAROUND);
      Uart1Rx.Count--;
   }
   else
      Uart1Rx.Count &= 0; // Make sure that count does not go below zero;

   return Results;
}

/****************************************************************************/
/*     Function: GetChar                                                    */
/*                                                                          */
/*        Input: None                                                       */
/*       Output: The next character available, or an error code indicating  */
/*               that there are no characters available.                    */
/* Side Effects: None                                                       */
/*     Overview: This function gets the next character from the internal    */
/*               uart buffer.                                               */
/* Notes:                                                                   */
/*                                                                          */
/****************************************************************************/
void Uart1WriteStr(char * BufferPtr)
{
   char Ch = 0;
   while ((Ch = *BufferPtr++))
   {
      if (Uart1Tx.Count < IO_BUFFER_SIZE)
      {
         Uart1Tx.Buffer[Uart1Tx.Tail++] = Ch;
         Uart1Tx.Tail &= IO_RAPAROUND;
         Uart1Tx.Count++;
      }

      if (!Uart1Txing)
      {
         Uart1Txing = Yes;
         U1STAbits.UTXEN = 1;
         U1TXREG = Ch;
      }
   }
}

/****************************************************************************/
/*     Function: GetChar                                                    */
/*                                                                          */
/*        Input: None                                                       */
/*       Output: The next character available, or an error code indicating  */
/*               that there are no characters available.                    */
/* Side Effects: None                                                       */
/*     Overview: This function gets the next character from the internal    */
/*               uart buffer.                                               */
/* Notes:                                                                   */
/*                                                                          */
/****************************************************************************/
void Uart1WriteConstStr(const char * BufferPtr)
{
   char Ch = 0;
   while ((Ch = *BufferPtr++))
   {
      if (Uart1Tx.Count < IO_BUFFER_SIZE)
      {
         Uart1Tx.Buffer[Uart1Tx.Tail++] = Ch;
         Uart1Tx.Tail &= IO_RAPAROUND;
         Uart1Tx.Count++;
      }

      if (!Uart1Txing)
      {
         Uart1Txing = Yes;
         U1STAbits.UTXEN = 1;
         U1TXREG = Ch;
      }
   }
}

/****************************************************************************/
/*     Function: GetChar                                                    */
/*                                                                          */
/*        Input: None                                                       */
/*       Output: The next character available, or an error code indicating  */
/*               that there are no characters available.                    */
/* Side Effects: None                                                       */
/*     Overview: This function gets the next character from the internal    */
/*               uart buffer.                                               */
/* Notes:                                                                   */
/*                                                                          */
/****************************************************************************/
void Uart1Write(unsigned * BufferPtr, unsigned char Length)
{
   int i = 0;

   while (i < Length)
   {
      if (Uart1Tx.Count < IO_BUFFER_SIZE)
      {
         Uart1Tx.Buffer[Uart1Tx.Tail++] = *(char *) BufferPtr;
         Uart1Tx.Tail++;
         Uart1Tx.Tail &= IO_RAPAROUND;
         Uart1Tx.Count++;
      }
      i++;
   }
}

/****************************************************************************/
/*     Function: Read                                                       */
/*                                                                          */
/*        Input: A Pointer to the Buffer to place the data into, and        */
/*               The number of characters to be transferred                 */
/*       Output: The amount of characters actually read                     */
/* Side Effects: None                                                       */
/*     Overview: This function gets the next character from the internal    */
/*               uart buffer.                                               */
/* Notes:                                                                   */
/*                                                                          */
/****************************************************************************/
unsigned int Uart1Read(unsigned char * BufferPtr, unsigned char Length)
{
   unsigned int i = 0;

   while (i++ < Length)
   {
      if (Uart1Rx.Count > 0)
      {
         *BufferPtr = Uart1Rx.Buffer[Uart1Rx.Head++];
         Uart1Rx.Head++;
         Uart1Rx.Head &= IO_RAPAROUND;
         Uart1Rx.Count--;
      }
   }
   return (i + 1);
}

/****************************************************************************/
/*                                                                          */
/*     Function: Uart 1 receive interrupt service routeen.                  */
/*        Input: None                                                       */
/*       Return: None                                                       */
/*     Overview: Enqueues a new Character into Uart 0's Queue               */
/*                                                                          */
/****************************************************************************/
void __attribute__((interrupt, no_auto_psv))_U1RXInterrupt(void)
{
   
   if (Uart1Rx.Count < IO_BUFFER_SIZE)
   {
      Uart1Rx.Buffer[Uart1Rx.Tail++] = U1RXREG;
      Uart1Rx.Tail &= IO_RAPAROUND;
      Uart1Rx.Count++;
   }

   ISS->Uart1Rdy = 1;
   U1STAbits.FERR = 0; // Bit2 *Read Only Bit*
   U1STAbits.OERR = 0; // Bit1 *Read Only Bit*
   IFS0bits.U1RXIF = 0;
}

/****************************************************************************/
/*     Function: Interrupt service routeen                                  */
/*                                                                          */
/*        Input: None                                                       */
/*       Output: None                                                       */
/* Side Effects: None                                                       */
/*     Overview: Receives one character of information in from the UART     */
/*                                                                          */
/* Notes:                                                                   */
/*                                                                          */
/****************************************************************************/
void __attribute__((interrupt, no_auto_psv)) _U1TXInterrupt(void)
{
   if (Uart1Tx.Count > 0)
   {
      U1TXREG = Uart1Tx.Buffer[Uart1Tx.Head++];
      Uart1Tx.Head &= IO_RAPAROUND;
      Uart1Tx.Count--;
   }

   if (Uart1Tx.Count == 0)
   {
      Uart1Txing = No;
   }

   U1STAbits.FERR = 0; // Bit2 *Read Only Bit*
   U1STAbits.OERR = 0; // Bit1 *Read Only Bit*
   IFS0bits.U1TXIF = 0;
}
