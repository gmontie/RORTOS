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
#include "Fops.h"
#include "Register.h"
#include "Registers.h"
#include "ReMeCi.h"
#include "SysCalls.h"

#define REMECI_DEVICE "Protocol Console"
#define BUFFER_LEN    0x10
#define LST_ELEMENT   0x0F

// This defines some machros which can be used to convert between Hex and
// numerical values.
#define NibbleToHex(b) ((b < 10)?(b + '0'):((b - 10) + 'A'))
#define HexToNibble(b) ((b >= 'A')?((b - 'A') + 10):(b - '0'))

#define ETB 0x140 // '@' == 0x40

// States for building up a command from the communications Object.
typedef enum
{
   Next,
   Accept
}LINE_STATES;

/* Private functions */
//static unsigned IOCTL(byte WhichOne);
static void ParseInput(void); // The function to parse one line of input.
static Boolean LineInPut(void); // The function to build up one line of input
static void Tokenizer(void);
static void ProtocolReset(void);
static int NextAddress( void );

/* Private members of the Protocal Module */
// A one line buffer used to build up a single command.
static byte Line[ 16 ];
static const char * Ptr;
static int  Address; // Register address
static int  Value; // Value for Register
static byte Len; // Current length of characters in Line
static fops * Fi = 0; // A communications object such as a UART.
static LINE_STATES EntryStates; // The current state for the protocol state 

static volatile IndicatorBits * SystemStat;
static volatile RequestBits * Request;
static UpdateQueue SaveQue;

// machine.
static Register * Element; // A register element pointer

// Service operations used..
static Service This =
{
   //.Used = True,
   .DeviceType = Console,
   .DeviceClass = COMMUNICATIONS,
   .Discription = REMECI_DEVICE,
   .AuxUse = True,
   .Id = (0x0100 + (int)Console),
   //.WaitingOnMask = UART1Rdy,
   .Arg = 0,
   .FnType = ProcessFn,
   .Thread = Tokenizer,
   .Write = 0,
   .Reset = ProtocolReset,
   .IsReady = LineInPut,
   .Next = NextAddress,
};

/***********************************************************************/
/*     Function: InitProtocol                                          */
/*                                                                     */
/*        Input: None                                                  */
/*       Output: None                                                  */
/*     Overview: To initialize the Protocol Object.                    */
/*                                                                     */
/***********************************************************************/
Service * NewReMeCI(fops * Interface)
{
   Fi = Interface;
   Element = getRegisterSet();
   Request = getRequestBits();
   EntryStates = Next;
   Address = 0;
   Value = 0;
   Len = 0;
   This.Status.Allocated = True;
   This.Driver = Interface;
   SystemStat = getSystemStat();
   switch(Fi->Resource)
   {
      case _Uart_1:
         This.WaitingOnMask = UART1Rdy;
         break;
      case _Uart_2:
         This.WaitingOnMask = UART2Rdy;
      default:
         This.WaitingOnMask = NONE_BLKD;
   }
   return &This;
}

/***********************************************************************/
/*     Function: ParseInput                                            */
/*        Input: A dummy void pointer so as to conform to DevObject    */
/*               signature requirements.                               */
/*       Output: None                                                  */
/*     Overview: To parse the input once one line of input has been    */
/*               built up.                                             */
/*                                                                     */
/***********************************************************************/
static void ParseInput(void)
{
   byte Adh;
   byte Adl;
   char Ch;
   
   Value=0;
   Address=0;
   switch (Line[0])
   {
      case '#': // Send the contents of the whole list of Registers
         for (Address = 0; Address < MEMORY_SIZE; Address++)
         {
            Adl = 0xFF & Element[Address].Value; // Get Register Elements Low value
            Adh = (Element[Address].Value >> 8); // Get Register Elements High value
            Fi->PutCh('V'); // Send this Registers value
            Fi->PutCh(Adh);
            Fi->PutCh(Adl);
            Fi->PutCh('.');
            Element[Address].Changed = 0; // Changed = False;
         }
         Fi->PutCh(ETB); // Indicate end of packet transmission
         break;
      case 'n':
      case 'N': // Send the name of name Register
          T3CONbits.TON = 0;
          for (Address = 0; Address < MEMORY_SIZE; Address++)
          {
              Ptr=getRegistersName(Address);
              while((Ch=*Ptr++)) 
                  Fi->PutCh(Ch);
              Fi->PutCh('\n');
              Fi->PutCh('\r');
          }
          Fi->PutCh(ETB);
          break;
      case '!': // Send all of the elements which have changed due to readings
         for (Address = 0; Address < MEMORY_SIZE; Address++)
         {
            if (Element[Address].Changed) // If the elements has changed
            { // Break the 16 bit value into 2 8 bit values
               Adl = 0xFF & Element[Address].Value;
               Adh = (Element[Address].Value >> 8);
               // Send Address
               Fi->PutCh('A');
               Fi->PutCh((byte) Address);
               Fi->PutCh('V');
               Fi->PutCh(Adh);
               Fi->PutCh(Adl);
               Fi->PutCh('.');
               Element[Address].Changed = 0; //Changed = False;
               if( Element[Address].isROM && Element[Address].CanChange)
               {
                  Element[Address].ROMChanged = 1;
                  SystemStat->CfgDirty = 1;
               }
            }
         }                                                     
         Fi->PutCh(ETB); // Indicate end of packet transmission
         break;
      case 'l':
      case 'L':  // Load value from register
         // VXX<ETB>
         Address = Line[1];
         Adl = 0xFF & Element[Address].Value;
         Adh = (Element[Address].Value >> 8);
         Fi->PutCh('V');
         Fi->PutCh(Adh);
         Fi->PutCh(Adl);
         Fi->PutCh(ETB); // Indicate end of packet transmission
         break;
      case 's':
      case 'S': // Store value to register @ address
         //  S - mnemonic; AA byte wise address; VVVV - 16 bit value.
         // SAVXX
         if ((Address = Line[1]) != 0)
         {  // Calculate the value
            Value = Line[3];
            Value <<= 8;
            Value |= Line[4];
            Element[Address].Changed = 1; // Changed = True
            if ((Element[Address].Value != Value) && Element[Address].CanChange)
            {  // If and only if the value is different
               Element[Address].Value = Value; // Update the value
               Element[Address].Changed = 1; // Changed = True
               // If the element is a persistent value then enqueue the address
               if( Element[Address].isROM ) // If this is a ROM value indicate
               {  // that it is not yet saved to persistent memory
                  Element[Address].Saved = False; 
                  Element[Address].ROMChanged = 1;
                  SystemStat->CfgDirty = 1;
               }
            }
         }
         Fi->PutCh(ETB); // Indicate end of packet transmission
         break;
      case 'y': // Send Sync signal
      case 'Y': //
         Fi->PutCh(ETB);
         break;
   }
   Len = 0;
   memset(Line, 0, 10);
   EntryStates = Next;
   asm("DISI  #0x05"); // Disable interrupts for 5 instruction cycles
   Fi->Release();
   Request->Uart1Block = 1;
}

/***********************************************************************/
/*     Function: LineInPut                                             */
/*                                                                     */
/*        Input: None                                                  */
/*       Output: None                                                  */
/* Side Effects: Changes the contents of the character array Len. Also */
/*               changes the value of the index variable called Len.   */
/*     Overview: In and organized way capture the input from the user  */
/*               and Store it into the variable Line for later use.    */
/*               Also implement functionality for basie editing of the */
/*               input from the user.                                  */
/*                                                                     */
/***********************************************************************/
static Boolean LineInPut(void)
{
   static char Character = 0;
   static int RtnValue = 0;

   while ((Fi->Available()) && (EntryStates != Accept))
   { // Array Maintenance ComponentBitss
      switch (EntryStates)
      {
         case Next:
            RtnValue = Fi->GetCh();
            if (RtnValue >= 0)
            {
               Character = (unsigned char) (0x00FF & RtnValue);
               switch (Character) // Parse ch
               {
                  case CR: // Carriage return.
                  case LINE_FEED: // End of input
                     switch(Line[0])
                     {
                        //case 0:
                        case '\r':
                           Len = 0;
                           Line[ Len++ ] = Character; 
                           EntryStates = Accept;
                           break;
                        case 's':
                        case 'S':
                           if(Len < 5)
                              Line[ Len++ ] = Character; 
                           else
                           {
                              if(Len == 0x05)
                              {
                                 EntryStates = Accept;
                                 Line[ Len ] = 0x0A;
                              }
                              else
                              {
                                 memset(Line, 0, (Len + 1));
                                 Len = 0;
                              }
                           }
                           break;
                        case 'l':
                        case 'L':
                           if(Len < 4)
                              Line[ Len++ ] = Character;                           
                           else
                              EntryStates = Accept;
                           break;
                        default:
                           EntryStates = Accept;
                     }
                     break;
                  default: // In the default case the Line is being built up
                     if (Len < LST_ELEMENT) // && (Character > 0x1F))                     
                        Line[ Len++ ] = Character; // put it at the current end of
               }
            }
            break;
         case Accept:
            Fi->Flush(); // Flush the buffers
         default:
            Character = 0;
            EntryStates = Next;
      }
   }
   return (EntryStates == Accept);
}

/***********************************************************************/
/*     Function: Tokenizer                                             */
/*                                                                     */
/*        Input: None                                                  */
/*       Output: None                                                  */
/* Side Effects: Changes the contents of the character array Len. Also */
/*               changes the value of the index variable called Len.   */
/*     Overview: In and organized way capture the input from the user  */
/*               and Store it into the variable Line for later use.    */
/*               Directly call the parser once the input is Tokenized  */
/*                                                                     */
/***********************************************************************/
static void Tokenizer(void)
{
   static char Character = 0;
   static int RtnValue = 0;

   while ((Fi->Available()) && (EntryStates != Accept))
   { // Array Maintenance ComponentBitss
      switch (EntryStates)
      {
         case Next:
            RtnValue = Fi->GetCh();
            if (RtnValue >= 0)
            {
               Character = (unsigned char) (0x00FF & RtnValue);
               switch (Character) // Parse ch
               {
                  case CR: // Carriage return.
                  case LINE_FEED: // End of input
                     switch(Line[0])
                     {
                        case '\r':
                           Len = 0;
                           Line[ Len++ ] = Character; 
                           EntryStates = Accept;
                           break;
                        case 's':
                        case 'S':
                           if(Len < 5)
                              Line[ Len++ ] = Character; 
                           else
                           {
                              if(Len == 0x05)
                              {
                                 EntryStates = Accept;
                                 Line[ Len ] = 0x0A;
                              }
                              else
                              {
                                 memset(Line, 0, (Len + 1));
                                 Len = 0;
                              }
                           }
                           break;
                        case 'l':
                        case 'L':
                           if(Len < 4)
                              Line[ Len++ ] = Character;                           
                           else
                           {
                              EntryStates = Accept;
                           }
                           break;
                        default:
                           EntryStates = Accept;
                     }
                     break;
                  default: // In the default case the Line is being built up
                     if (Len < LST_ELEMENT) // && (Character > 0x1F))                     
                        Line[ Len++ ] = Character; // put it at the current end of
               }
            }
            break;
         case Accept:
            Fi->Flush(); // Flush the buffers
         default:
            Character = 0;
            EntryStates = Next;
      }
   }
   Fi->Release();
   if (EntryStates == Accept) 
      ParseInput();
   else
      Request->Uart1Block = 1;
}

/***********************************************************************/
/*     Function:                                                       */
/*        Input:                                                       */
/*                                                                     */
/*       Output:                                                       */
/*     Overview:                                                       */
/*                                                                     */
/*                                                                     */
/***********************************************************************/
static int NextAddress( void )
{
  unsigned Results = DEVICE_EMPTY;

  if(SaveQue.Count > 0)
  {
    Results = SaveQue.Buffer[SaveQue.Head++];
    SaveQue.Head &= (UPDATE_RAPAROUND);
    SaveQue.Count--;
  }
  else
     SaveQue.Count = 0; // Make sure that count does not go below zero;

  return Results;
}

/***********************************************************************/
/*     Function:                                                       */
/*        Input:                                                       */
/*                                                                     */
/*       Output:                                                       */
/*     Overview:                                                       */
/*                                                                     */
/*                                                                     */
/***********************************************************************/
static void ProtocolReset(void)
{
   Address = 0; // RegisterElement address
   Value = 0; // Value of the RegisterElement
   Len = 0; // Current length of characters in Line
   EntryStates = 0; // The current state for the protocol state
   This.DeviceType = Console;
   This.Driver = Fi;
}

/****************************************************************************/
/*     Function: itoa                                                       */
/*                                                                          */
/*        Input: A 16 bit value, and a character buffer for the result      */
/*       Output: None                                                       */
/* Side Effects: NONE                                                       */
/*                                                                          */
/*     Overview: Implements the itoa funtion which used to be part of gcc   */
/*               and now is not.                                            */
/*                                                                          */
/****************************************************************************/
void itoa(unsigned Value, char * Buffer)
{
   int n = Value; // n here is the mathematical definition of n
   // n: n in N {n is in the set of Natural Numbers}
   int Index;
   int Count = 0;
   char Ch = 0;
   char cStack[16];

   for (Index = 0; ((Index < 16) & (n > 0)); n /= 10, Index++)
      cStack[Index] = (n % 10) + '0';

   if (Index != 0)
   {
      while (Index)
      {
         Ch = cStack[--Index];
         Buffer[Count++] = Ch;
      }
   }
   else
   {
      Buffer[Count] = '0';
   }
}

