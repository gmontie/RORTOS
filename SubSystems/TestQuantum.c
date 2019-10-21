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
#include "TestThread.h"

#define DISCRIPTION_TEST_THREAD "Test Thread"

static Boolean AllwaysReady(void){return True;}
static void TakeMyTime(void);
static unsigned Counter1;
static unsigned Counter2;
static unsigned Counter3;


Service * TestThrdInit(void)
{
   Service * Results = 0;

   if ((Results = NewDevice(0)) != 0)
   {
      Results->Discription = DISCRIPTION_TEST_THREAD;
      Results->IsReady = AllwaysReady;
      Results->Peek = 0;
      Results->Poke = 0;
      Results->Read = 0;
      Results->Id = (0x0100 + ((int) USER_THREAD) * 0x20),
      Results->DeviceType = USER_THREAD;
      Results->FnType = ProcessFn;
      Results->UpDate = TakeMyTime;
      Results->Dev = Results;
   }
   return Results;
}

static void TakeMyTime(void)
{
     do
     {
        for(Counter1 = 0; Counter1 < 0xFFFF; Counter1++)
           for(Counter2 = 0; Counter2 < 0xFFFF; Counter2++)
              for(Counter3 = 0; Counter3 < 0xFFF; Counter3++);
     }while(LOOPING_FOREVER);
}