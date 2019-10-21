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
#ifndef PROTOCOL_H
#define PROTOCOL_H

#include "defs.h"
#include "Fops.h"
#include "Device.h"
#include "Register.h"
#include "Registers.h"

/**************************************************************************/
/*                                                                        */
/*  This is the                                                           */
/*           REGISTER MEMORY COMMUNICATION INTERFACE PROTOCOL             */
/*             ReMeCi Protocol                                            */
/*                                                                        */
/**************************************************************************/

#define MEMORY_START 0

#define BUFFER_SIZE  32
#define LAST_ELEMENT 31

// Circular buffer 
#define QUEUE_SIZE        16
#define UPDATE_RAPAROUND  15
typedef struct
{
  byte Head;
  byte Tail;
  byte Count;
  byte Buffer[QUEUE_SIZE];
}UpdateQueue;

// Initialize a protocol object and return a pointer to it.
Service * NewReMeCI(fops *);
void itoa(unsigned Value, char * Buffer);

#endif
