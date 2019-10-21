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
#ifndef M25LC320_H
#define M25LC320_H

#include "defs.h"
#include "Fops.h"
#include "Register.h"
#include "Device.h"

#define WIP_WRITE_IN_PROGRESS 0x01
#define WEL_WRITE_ENABLED     0X02

typedef union
{
    struct
    {
        unsigned WIP : 1;
        unsigned WEL : 1;
        unsigned BP0 : 1;
        unsigned BP1 : 1;
        unsigned RESERVED : 3;
        unsigned WPEN : 1;
    };
    byte Stat;
}EEPromStatus;

//Function Prototypes
Service * InitM25LC320(fops *);

#endif
