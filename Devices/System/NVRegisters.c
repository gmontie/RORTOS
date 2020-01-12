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
#include "NVRegisters.h"
#include "Registers.h"
#include "Register.h"
#include "Device.h"
#include "Fops.h"

// static unsigned * RomBlock; // Only if malloc is used.
static unsigned   RomBlock[30];
static unsigned * Map;
static unsigned * ReverseMap;

// Public Methods otherwise Private Member functions
static Boolean CheckVersion(const unsigned * , unsigned , int );
static void SetVersion(const unsigned * , unsigned , int );
static inline void SaveSequence(unsigned , byte *, byte );
static void SaveRegister(unsigned);
static void LoadRegister(unsigned);
static void Load(void);
static void Save(void);
static int NVReset(void);

// Fix Me Need to finish porting Mem.c/Mem.h form PowerSupply.
NVRegisters This =
{
   .CheckFirmwareVersion = CheckVersion,
   .SetFirmwareVersion = SetVersion,
   .Load = LoadRegister,
   .Store = SaveRegister,
   .LoadRegisters = Load,
   .StoreRegisters = Save,
   .Reset = NVReset
};

// Private Member Objects
static fops * EEProm = 0;
static unsigned MemSize = 0;
static int PageSize = 0;
static unsigned Pages = 0;
static Register * Registers = 0;
static volatile IndicatorBits * ISS;
static int NbrOfRomRegisters = 0;

/***********************************************************************/
/*                                                                     */
/*     Function: InitRegister                                          */
/*        Input: A pointer to the register file of registers           */
/*               A pointer to a Flash memory device                    */
/*       Output: None                                                  */
/*     Overview: This function sets up the Flash memory management for */
/*               the Register File of registers.                       */
/*                                                                     */
/***********************************************************************/
NVRegisters * InitNvManager(Service * NvRam)
{
   Registers = getRegisterSet();
   if(NvRam->DeviceClass == FLASH_MEM)
   {
      EEProm = NvRam->Driver;
      This.Resource = EEProm->Resource;
   }
   PageSize = EEProm->IOCTL(GET_PAGE_SIZE);
   MemSize = EEProm->IOCTL(GET_MEM_SIZE); 
   Pages = EEProm->IOCTL(GET_NBR_OF_PAGES);
   NbrOfRomRegisters = readOnlyRegisters();
   ISS = getSystemStat();
   Map = getRomRegisterMap();
   ReverseMap = getRevRomRegMap();
   return &This;
}

/***********************************************************************/
/*                                                                     */
/*     Function: __Reset                                               */
/*        Input: None                                                  */
/*       Output: Current status of the Serial Service                  */
/*     Overview: This function attempts to reset a serial memory       */
/*               device.                                               */
/*                                                                     */
/***********************************************************************/
static int NVReset(void)
{
   int Results = EEProm->IOCTL(FP_RESET);
   Results = EEProm->IOCTL(READ_STATUS);
   return Results;
}

/***********************************************************************/
/*                                                                     */
/*     Function: CheckVersion                                          */
/*        Input: A constant pointer to a buffer which contains the     */
/*               current firmware version.                             */
/*               The index of the starting register which contains     */
/*               the current firmware version number.                  */
/*               The length of the version number which is to be       */
/*               checked.                                              */
/*       Output: Success or Failure                                    */
/*     Overview: This function runs a check on the firmware version    */
/*               stored in Flash memory, and is used as a check to     */
/*               tell if default values need to be set.                */
/*                                                                     */
/***********************************************************************/
static Boolean CheckVersion(const unsigned * NbrPtr, unsigned MemoryIndex, int Length)
{
   Boolean Results = True;
   Register * RegisterPtr = getRegister(MemoryIndex);

   do
   {
      if (*NbrPtr++ != RegisterPtr->Value)
         Results = False;
      RegisterPtr++;
   }while (--Length && Results);
   return Results;
}

/***********************************************************************/
/*                                                                     */
/*     Function: SetVersion                                            */
/*        Input: A constent pointer to a buffer which contains the     */
/*               current firmware version.                             */
/*               The index of the starting register which contains     */
/*               the current firmware version number.                  */
/*               The length of the verision number which is to be      */
/*               written to flash memory.                              */
/*       Output: None                                                  */
/*     Overview: This function is used to write the firmware version   */
/*               into flash memory.                                    */
/*                                                                     */
/***********************************************************************/
static void SetVersion(const unsigned * NbrPtr, unsigned MemoryIndex, int Length)
{
   Register * RegisterPtr = Registers;

   while (Length--)
   {
      RegisterPtr->Value = *NbrPtr++;
      RegisterPtr++;
   }
}

/***********************************************************************/
/*                                                                     */
/*     Function: LoadRegisters                                         */
/*        Input: A pointer to the register file                        */
/*       Output: None                                                  */
/*     Overview: This function will read one register from memory      */
/*                                                                     */
/***********************************************************************/
static void LoadRegister(unsigned WhichElement)
{
   unsigned Address = ReverseMap[WhichElement] * sizeof (Register);
   EEProm->BlockRead(Address, (byte*) Registers, sizeof (Register));
}

/***********************************************************************/
/*                                                                     */
/*     Function: SaveRegister                                          */
/*        Input: The index of the register to save                     */
/*               A Pointer to the element to be saved                  */
/*       Output: Success or Failure                                    */
/*     Overview: This function is used save a single memory Register   */
/*               to Flash memory.                                      */
/*                                                                     */
/***********************************************************************/
static inline void SaveSequence(unsigned Address, byte *DataPtr, byte Length)
{
   EEProm->IOCTL(READ_STATUS);
   if(EEProm->IOCTL(WRITE_ENABLED) == 0)
      EEProm->IOCTL(ENABLE_WRITE);
   EEProm->BlockWrite(Address, DataPtr, Length);
}

/***********************************************************************/
/*                                                                     */
/*     Function: SaveRegister                                          */
/*        Input: The index of the register to save                     */
/*       Output: Success or Failure                                    */
/*     Overview: This function is used save a single memory Register   */
/*               to Flash memory.                                      */
/*                                                                     */
/***********************************************************************/
static void SaveRegister(unsigned WhichElement)
{
   unsigned Address;
   
   if((Registers[WhichElement].isROM) && (Registers[WhichElement].CanChange))
   {
      Address = (WhichElement + sizeof(REGISTER_WIDTH));
      SaveSequence(Address, (byte*)&Registers[WhichElement].Value, REGISTER_WIDTH);
   }
}

/***********************************************************************/
/*                                                                     */
/*     Function: LoadRegister                                          */
/*        Input: A pointer to the Register File.                       */
/*       Output: None                                                  */
/*     Overview: This function is used to read in all of the non-      */
/*               volatile registers to Flash Memory.                   */
/*                                                                     */
/***********************************************************************/
void Load(void)
{
   int i;
   unsigned Address;
   unsigned Index;
   unsigned BytesNeeded;
   unsigned PagesNeeded;
   int BytesLeft;
   byte * BytePtr;


   // Start of algorithm
   if (Registers > 0)
   {
      // Initialization
      BytePtr = (byte*) RomBlock;
      BytesNeeded = NbrOfRomRegisters << (REGISTER_WIDTH - 1);
      if(BytesNeeded > PageSize)
         PagesNeeded = BytesNeeded / PageSize;
      else
         PagesNeeded = 1;
      Address = 0;
      Index = 0;
      for (i = 0; i < PagesNeeded; i++)
      {
         EEProm->BlockRead(Address, &BytePtr[Index], PageSize);
         Index += PageSize;
         Address += PageSize;
      }

      if((BytesLeft = BytesNeeded - (PagesNeeded * PageSize)) > 0)
         EEProm->BlockRead(Address, &BytePtr[Index], BytesLeft);

      for (i = 0, Index = 0; i < NbrOfRomRegisters; i++)
      {
         Index = Map[i];
         Registers[Index].Value = RomBlock[i];
      }
   }
}

/***********************************************************************/
/*                                                                     */
/*     Function: SaveRegisters                                         */
/*        Input: A pointer to the Register File                        */
/*       Output: Success or Failure                                    */
/*     Overview: This function is used to save all of the non-         */
/*               volatile registers to Flash Memory.                   */
/*                                                                     */
/***********************************************************************/
void Save(void)
{
   int i;
   unsigned Address = 0;
   unsigned Index = 0;
   byte * BytePtr = (byte*)RomBlock;
   unsigned BytesNeeded = NbrOfRomRegisters << (REGISTER_WIDTH - 1);
   unsigned PagesNeeded;
   int BytesLeft;

   ISS->CfgDirty = 0; // Stop Blocking
   if( BytesNeeded > PageSize)
      PagesNeeded = BytesNeeded / PageSize;
   else
      PagesNeeded = 1;
   
   if (Registers > 0)
   {
      for (i = 0, Index = 0; i <= NbrOfRomRegisters; i++)
      {
         Index = Map[i];
         RomBlock[i] = Registers[Index].Value;
      }
   
      // Save each Physical block making up the logical block
      for (i = 0, Index = 0; i < PagesNeeded; i++)
      {
         SaveSequence(Address, &BytePtr[Index], PageSize);
         Index += PageSize;
         Address += PageSize;
      }
       BytesLeft= BytesNeeded - (PagesNeeded * PageSize);
       if(BytesLeft > 0)
          SaveSequence(Address, &BytePtr[Index], BytesLeft);
   }
}
