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
#ifndef NV_REGISTERS_H
#define NV_REGISTERS_H

#include "defs.h"
#include "Register.h"
#include "Device.h"
#include "Fops.h"

typedef struct
{
    Boolean (*CheckFirmwareVersion)(const unsigned *, unsigned, int);
    void (*SetFirmwareVersion)(const unsigned *, unsigned, int);
    void (*Load)(unsigned);
    void (*Store)(unsigned);
    void (*LoadRegisters)(void);
    void (*StoreRegisters)(void);
    int (*Reset)(void);
    //void (*Release)(void);
    Bus Resource;
}NVRegisters;

NVRegisters * InitNvManager( Service * );

#endif
