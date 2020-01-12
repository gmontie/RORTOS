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
#include "defs.h"
#include "System.h"
#include "Register.h"
#include "PinConfig.h"
#include "CD74HCx4051.h"
#include "Process.h"
#include "Blk.h"


#define CD74HC4051 "CD74HC4051 MUX"

static void _Process(void);
static unsigned _Read(byte BitField);
static void _Write(byte WhichBits, uint16_t What);
static Boolean _IsReady(void){return True;}
static void ChangeAddress(unsigned Value);

// Service operations used..
static Service This =
{
   .DeviceType = CD4051,
   .DeviceClass = MULTIPLEXER,
   .Discription = CD74HC4051,
   .Arg = 0,
   .FnType = ProcessFn,
   .Thread = _Process,
   .Read = _Read,
   .Write = _Write,
   .Poke = ChangeAddress, 
   .IsReady = _IsReady,
   .Driver = 0
};

static Register * SharedMuxVar; // A register element pointer
static unsigned End;

/***********************************************************************/
/*                                                                     */
/*     Function: IO Control                                            */
/*        Input: None                                                  */
/*       Output: None                                                  */
/*                                                                     */
/*     Overview: IO Control for Memory                                 */
/*                                                                     */
/***********************************************************************/
Service * Init74HC4051(unsigned RegIndex, unsigned MaxValue)
{
   SharedMuxVar = getRegister(RegIndex);
   End = MaxValue - 1;
   This.Driver = &This;
   return &This;
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
static void _Process(void)
{
   SharedMuxVar->Value++;
   SharedMuxVar->Value &= End;
   SharedMuxVar->Changed = True;
   switch( SharedMuxVar->Value )
   {
      case 0: 
         ADDRS1 = 0;
         ADDRS2 = 0;
         ADDRS3 = 0;
         break;
      case 1: 
         ADDRS1 = 1;
         ADDRS2 = 0;
         ADDRS3 = 0;
         break;
      case 2: 
         ADDRS1 = 0;
         ADDRS2 = 1;
         ADDRS3 = 0;
         break;
      case 3: 
         ADDRS1 = 1;
         ADDRS2 = 1;
         ADDRS3 = 0;
         break;
      case 4: 
         ADDRS1 = 0;
         ADDRS2 = 0;
         ADDRS3 = 1;
         break;
      case 5: 
         ADDRS1 = 1;
         ADDRS2 = 0;
         ADDRS3 = 1;
         break;
      case 6: 
         ADDRS1 = 0;
         ADDRS2 = 1;
         ADDRS3 = 1;
         break;
      case 7: 
         ADDRS1 = 1;
         ADDRS2 = 1;
         ADDRS3 = 1;
         break;
   }
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
static unsigned _Read(byte Index)
{
   unsigned Results = 0;
   
   switch(Index)
   {
      case 0:
         Results = SharedMuxVar->Value;
         break;
      case 1:
         Results = SharedMuxVar->Value;
         break;
   }
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
static void _Write(byte WhichBits, uint16_t What)
{
   SharedMuxVar->Value |= (WhichBits & What);
   SharedMuxVar->Changed = True;
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
static void ChangeAddress(unsigned Address)
{
   unsigned CurrentIndex = (Address & 0x0007);
   SharedMuxVar->Value = CurrentIndex;
   SharedMuxVar->Changed = True;
   switch( CurrentIndex )
   {
      case 0: 
         ADDRS1 = 0;
         ADDRS2 = 0;
         ADDRS3 = 0;
         break;
      case 1: 
         ADDRS1 = 1;
         ADDRS2 = 0;
         ADDRS3 = 0;
         break;
      case 2: 
         ADDRS1 = 0;
         ADDRS2 = 1;
         ADDRS3 = 0;
         break;
      case 3: 
         ADDRS1 = 1;
         ADDRS2 = 1;
         ADDRS3 = 0;
         break;
      case 4: 
         ADDRS1 = 0;
         ADDRS2 = 0;
         ADDRS3 = 1;
         break;
      case 5: 
         ADDRS1 = 1;
         ADDRS2 = 0;
         ADDRS3 = 1;
         break;
      case 6: 
         ADDRS1 = 0;
         ADDRS2 = 1;
         ADDRS3 = 1;
         break;
      case 7: 
         ADDRS1 = 1;
         ADDRS2 = 1;
         ADDRS3 = 1;
         break;
   }
}
