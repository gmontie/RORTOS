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
#include "FeedBack.h"
#include "Process.h"

// This file contains the implementation for a counter object called a
// feedback object. The purpose of this type of Object is for debugging.
// It provides a quite mechanism used to show if the Microcontroller is
// running and that the communications are working.

#define DISCRIPTION_FEEDBACK "Feedback Indicator"
#define NUMBER_OF_FBs 2

static struct fb_ops  FbDevTable[NUMBER_OF_FBs];

static Boolean AllwaysReady(void){return True;}
static Boolean Fb_Dev_Alloc(int * Index);
static void Increment(Service * Pt);
static void Decrement(Service * Pt);
static void FbClear(Service * Pt);

/****************************************************************************/
/*                                                                          */
/*     Function: FbInit                                                     */
/*        Input: None                                                       */
/*       Return: A Pointer to a feed back object.                           */
/*     Overview: This is the initialization for a Feedback Object.          */
/*                                                                          */
/****************************************************************************/
Service * FbInit(unsigned RegisterAddress, unsigned Max, FbTypes FnType)
{
  int i=0;
  struct fb_ops * _FbDev = 0;
  Service * Results = 0;

  if(Fb_Dev_Alloc(&i))
  {
    _FbDev = &FbDevTable[i];
    _FbDev->CounterPtr = getRegister(RegisterAddress);
    _FbDev->MaxCountPtr = getRegister(Max);
    _FbDev->Counter = 0;
    _FbDev->Clear = FbClear;
    _FbDev->Dec = Decrement;
    _FbDev->Inc = Increment;
    if((Results = NewDevice(_FbDev)) != 0)
    {
       Results->Discription = DISCRIPTION_FEEDBACK;
       Results->IsReady = AllwaysReady;
       Results->Peek = 0;
       Results->Poke = 0;
       Results->Read = 0;
       Results->Id = (0x0100 + (int)FeedBack + i * 0x10),
       Results->DeviceType = FeedBack;
       Results->FnType = DeviceFn;
       Results->Dev = Results;
       Results->Driver = _FbDev;
       switch(FnType)
       {
          case INC:
             Results->Device = Increment;
             break;
          case DEC:
             Results->Device = Decrement;
             break;
          case CLR:
             Results->Device = FbClear;
             break;
       }
    }
  }
  return Results;
}

/****************************************************************************/
/*     Function: FbClear                                                    */
/*                                                                          */
/*        Input: A pointer to an FbData Object device.                      */
/*       Output: None                                                       */
/* Side Effects: This function resets FbObject to zero                      */
/*                                                                          */
/****************************************************************************/
static void FbClear(Service * Pt)
{
   if((Pt != 0) && (Pt->DeviceType == FeedBack))
   {
      FbOps * Fb = (FbOps*)Pt->Driver;
      Fb->Counter = 0;
   }
}

/****************************************************************************/
/*                                                                          */
/*     Function: Increment                                                  */
/*        Input: A pointer to an FbData Object which is to be incremented   */
/*       Return: None                                                       */
/*     Overview: This function increments the FbData Object which is passed */
/*               to it.                                                     */
/*                                                                          */
/****************************************************************************/
static void Increment(Service * Pt)
{
   if((Pt != 0) && (Pt->DeviceType == FeedBack))
   {
      FbOps * Fb = (FbOps*)Pt->Driver;
      Register * CounterPtr = Fb->CounterPtr;
      
      if(Fb->Counter < Fb->MaxCountPtr->Value)
         Fb->Counter++;
      else
      {
         Fb->Counter = 0;
         CounterPtr->Value++;
         if(CounterPtr->Value > 360)
            CounterPtr->Value = 0;
         CounterPtr->Changed = 1; //Changed = True;
      }
  }
}

/****************************************************************************/
/*                                                                          */
/*     Function: Decrement                                                  */
/*        Input: A pointer to an FbData Object                              */
/*       Return: None                                                       */
/*     Overview: This function decrements the FbData Object which is passed */
/*               to it.                                                     */
/*                                                                          */
/****************************************************************************/
static void Decrement(Service * Pt)
{
   if ((Pt != 0) && (Pt->DeviceType == FeedBack))
   {
      FbOps * Fb = (FbOps*) Pt->Driver;
      Register * CounterPtr = Fb->CounterPtr;
      
      if (Fb->Counter > 0)
         Fb->Counter--;
      else
      {
         Fb->Counter = Fb->MaxCountPtr->Value;
         if (CounterPtr->Value > 0)
            CounterPtr->Value--;
         else
            CounterPtr->Value = 360;
         CounterPtr->Changed = 1; //Changed = True;
      }
   }
}

/****************************************************************************/
/*                                                                          */
/*     Function: Fb_Dev_Alloc                                               */
/*        Input: A pointer to an intager which will contain the index       */
/*               to a newly allocated FbDevice                              */
/*                                                                          */
/*       Return: Success or Failure                                         */
/*     Overview: This function searches the FbDevice table to find an       */
/*               unused FbDevice. When a free FbDevice is found the Index   */
/*               ponter is updated with the index of the free FbObject      */
/*               from the table.                                            */
/*                                                                          */
/****************************************************************************/
static Boolean Fb_Dev_Alloc(int * Index)
{
  Boolean Results = False;
  int i = 0;

  do
  {
    if(FbDevTable[i].Used) // if used
      i++; // Go to the next element in the table
    else
    { // Found a free spot so lets exit.
      Results = True;
      *Index = i;
      FbDevTable[i].Used = True;
    }
  }while((Results == False) && (i < NUMBER_OF_FBs));
  return Results;
}
