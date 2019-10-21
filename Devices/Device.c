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
#include "Device.h"
#include "Process.h"

static Service  Devices[NO_OF_DEVICES];
static Boolean _Dev_Alloc(int *);
static void Clear(Service *);

static unsigned _Read(byte Unused){return 0;} // byte is for the address of the register
static void _Poke(unsigned Unused){}
static Boolean _IsReady(void){return False;}

/****************************************************************************/
/*                                                                          */
/*     Function: NewProtocallDevice                                         */
/*        Input: A pointer to a Serial Operations Object.                   */
/*       Output: A pointer to a Serial Service Object.                       */
/*     Overview: Receives one character of information in from the UART     */
/*                                                                          */
/****************************************************************************/
Service * NewDevice(void * Driver)
{
  int i;
  Service * NewDev = 0;

  if(_Dev_Alloc(&i))
  {
    NewDev = &Devices[i];
    Clear(NewDev);
    NewDev->Id = i + 1;
    NewDev->Status.Allocated = True;
    NewDev->Discription = 0;
    NewDev->IsReady = _IsReady;
    NewDev->Poke = _Poke;
    NewDev->Read = _Read;
    NewDev->Arg = 0;
    NewDev->FnType = 0;
    NewDev->Driver = 0;
    NewDev->Reset = 0;
  }
  return NewDev;
}

/****************************************************************************/
/*     Copy Constructor                                                     */
/*     Function: NewProtocallDevice                                         */
/*        Input: A pointer to a Service structure/Object                     */
/*       Output: A pointer to a newly allocated Devcie Object               */
/*     Overview: Receives one character of information in from the UART     */
/*                                                                          */
/****************************************************************************/
Service * NewDeviceCpy(Service * Obj)
{
  int i=0;
  Service * NewDev = 0;
  
  if(_Dev_Alloc(&i))
  {
   NewDev = &Devices[i];
   Clear(NewDev);
   NewDev->Id = i;
   NewDev->Status.Allocated = True;
   NewDev->DeviceType=Obj->DeviceType;
   NewDev->DeviceClass=Obj->DeviceClass;
   NewDev->Discription=Obj->Discription;
   NewDev->AuxUse=Obj->AuxUse;
   NewDev->WaitingOnMask = 0;
   NewDev->FnType = Obj->FnType;
   switch(NewDev->FnType)
   {
      case UpDateFn:
         NewDev->Arg = Obj->Arg;
         NewDev->UpDate = Obj->UpDate;
         break;
      case DeviceFn:
         NewDev->Dev = Obj->Dev;
         NewDev->Device = Obj->Device;
         break;
      case ProcessFn:
         NewDev->Arg = 0;
         NewDev->Thread = Obj->Thread;
         break;
   }
    
   NewDev->InputType = Obj->InputType;
   switch(NewDev->InputType)
   {
      case ReadFn:
         NewDev->Read = Obj->Read;
         break;
      case ReadingFn:
         NewDev->Reading = Obj->Reading;
      case PeekFn:
         NewDev->Peek = Obj->Peek;
      case NextFn:
         NewDev->Next = Obj->Next;
   }
   NewDev->IsReady=Obj->IsReady;
    
   NewDev->Write=Obj->Write;
   NewDev->Driver=Obj->Driver;
   NewDev->Reset = Obj->Reset;
  }
  return NewDev;
}

/****************************************************************************/
/*     Function: Clear                                                      */
/*                                                                          */
/*        Input: A pointer to a SerDev Object                               */
/*       Output: None                                                       */
/*     Overview: This function sets the Dov Object passed to zero.          */
/*                                                                          */
/****************************************************************************/
void Clear(Service * Dev)
{
  memset(Dev, 0, sizeof(Service));
}

/****************************************************************************/
/*     Function: _Dev_Alloc                                                 */
/*                                                                          */
/*        Input: A pointer to an available buffer which can be updated      */
/*               with the number of the table element to be used.           */
/*       Output: Success or Failure                                         */
/* Side Effects: The memory pointed to by * Index is changed                */
/*     Overview: Allocates an element from the Service Table.               */
/*                                                                          */
/****************************************************************************/
static Boolean _Dev_Alloc(int * Index)
{
  Boolean Results = False;
  int i = 0;

  do
  {
    if(Devices[i].Status.Allocated) // if used
      i++; // Go to the next element in the table
    else
    { // Found a free spot so lets exit.
      Results = True;
      *Index = i;
    }
  }while((Results == False) && (i < NO_OF_DEVICES));
  return Results;
}    

Boolean ReadyTrue(void){return True;}
Boolean ReadyFalse(void){return False;}
