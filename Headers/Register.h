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
#ifndef REGISTER_H
#define REGISTER_H

// CCI must be used so that the bits will be ordered!
#include "defs.h"

#define SHIFT_BY 2 

typedef struct
{
    union
    {
        struct
        {
            unsigned Changed      : 1;
            unsigned ROMChanged   : 1;
            unsigned isROM        : 1;
            unsigned Saved        : 1;
            unsigned CanChange    : 1;
            unsigned              : 3;

            unsigned Ready        : 1;
            unsigned UpDated      : 1;
            unsigned              : 6;
        };
        unsigned Status;
    };
    unsigned Value;
} Register;

#define NO_FLAGS        0
#define CHANGED_BIT     0x0001
#define SAVE2ROM_BIT    0x0002
#define ROM_BIT         0x0004
#define SAVED_BIT       0x0008
#define CAN_CHANGE_BIT  0x0010
#define READY_BIT       0x0100
#define UP_DATED_BIT    0x0200

#endif
