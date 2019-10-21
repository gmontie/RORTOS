/**************************************************************************/
/*                                                                        */
/*  Programmer: Gregory L Montgomery                                      */
/*                                                                        */
/*                                                                        */
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
#include <xc.h>
#include <string.h>
#include "defs.h"
#include "Fops.h"
#include "Device.h"
#include "Register.h"
#include "Registers.h"
#include "System.h"
#include "Protocol.h"

#define THIS_DEVICE "Protocol Console"

// This defines some machros which can be used to convert between Hex and
// numerical values.
#define NibbleToHex(b) ((b < 10)?(b + '0'):((b - 10) + 'A'))
#define HexToNibble(b) ((b >= 'A')?((b - 'A') + 10):(b - '0'))

#define ETB 0x40 // '@' == 0x40

// States for building up a command from the communications Object.
typedef enum
{
   Next,
   Accept
}LINE_STATES;

/* Private functions */
 unsigned IOCTL(byte WhichOne);
static void ParseInput(void); // The function to parse one line of input.
static Boolean LineInPut(void); // The function to build up one line of input
static void ProtocolReset(void);
static void Flush(void);

/* Private members of the Protocal Module */
// A one line buffer used to build up a single command.
static byte Line[ BUFFER_SIZE ];

static int Address; // Register address
static int Value; // Value for Register
static int Len; // Current length of characters in Line
static fops * Fi = 0; // A communications object such as a UART.
static LINE_STATES EntryStates; // The current state for the protocol state

// machine.
static Register * Element; // A register element pointer

// Device operations used..
static Device This =
{
   .Used = True,
   .DeviceType = Console,
   .Discription = THIS_DEVICE,
   .AuxUse = 0,
   .Id = (0x0100 + (int)Console),
   .WaitingOnMask = CON_READY_BIT,
   .Arg = 0,
   .FnType = ProcessFn,
   .UpDate = ParseInput,
   .Write = 0,
   .Reset = ProtocolReset,
   .Flush = Flush,
   .IsReady = LineInPut,
};

/***********************************************************************/
/*     Function: InitProtocol                                          */
/*                                                                     */
/*        Input: None                                                  */
/*       Output: None                                                  */
/*     Overview: To initialie the Protocal Object.                     */
/*                                                                     */
/***********************************************************************/
Device * InitCommInterface(Register * UI, fops * CmDv)
{
   Fi = CmDv;
   Element = UI;
   EntryStates = Next;
   Address = 0;
   Value = 0;
   Len = 0;
   This.Driver = CmDv;
   return &This;
}

/***********************************************************************/
/*     Function: ParseInput                                     */
/*        Input: A dummy void pointer so as to conform to DevObject    */
/*               signature requirments.                                */
/*       Output: None                                                  */
/*     Overview: To parse the input once one line of input has been    */
/*               built up.                                             */
/*                                                                     */
/***********************************************************************/
void ParseInput(void)
{
   byte Adh;
   byte Adl;

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
            }
         }                                                     
         Fi->PutCh(ETB); // Indicate end of packet transmission
         break;
      case 'l': // Load
      case 'L':
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
      case 'S': // Store value @ address
         //  S - mnemonic; AA byte wise address; VVVV - 16 bit value.
         // SAVXX
         if ((Address = Line[1]) != 0)
         {  // Calculate the value
            Value = Line[3];
            Value <<= 8;
            Value |= Line[4];
            if (Element[Address].Value != Value)
            {  // If and only if the value is different
               Element[Address].Value = Value; // Update the value
               Element[Address].Changed = 1; // Changed = True
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
                        case 0:
                        case '\r':
                           Len = 0;
                           EntryStates = Accept;
                           break;
                        case 's':
                        case 'S':
                           if(Len < 5)
                              Line[ Len++ ] = Character;                           
                           else
                              EntryStates = Accept;
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
                     if (Len < LAST_ELEMENT) // && (Character > 0x1F))                     
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
/*     Function:                                                       */
/*        Input:                                                       */
/*                                                                     */
/*       Output:                                                       */
/*     Overview:                                                       */
/*                                                                     */
/*                                                                     */
/***********************************************************************/
void ProtocolReset(void)
{
   Address = 0; // RegisterElement address
   Value = 0; // Value of the RegisterElement
   Len = 0; // Current length of characters in Line
   EntryStates = 0; // The current state for the protocol state
   This.DeviceType = Console;
   This.Driver = Fi;
   Len = 0;
   EntryStates = Next;
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
void Flush(void)
{
   Fi->Flush(); // Flush the buffers
   Len = 0;
   EntryStates = Next;
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

