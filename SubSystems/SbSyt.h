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
#ifndef SUB_SYSTEMS_H
#define SUB_SYSTEMS_H

#include "defs.h"
#include "Register.h"

#define NO_OF_SubSystems       8

typedef enum 
{
    NONE,
    READ_ONLY_REGISTER_SUBSYSTEM,
    VOLT_METER,
    DIAGNOSTIC, 
    REGISTER_COM,
    USER_THREAD
}SubSysTypes;

typedef enum{ThreadFn, DevFn}SubSysFnTypes;

typedef struct
{
    union
    {
        struct
        {
            unsigned SelfCalibrated : 1;
            unsigned NvCalValuesSet : 1;
            unsigned RomValuesSaved : 1;
            unsigned                : 1;
            
            unsigned                : 8;
        };
        unsigned Field;
    };    
}SubSystemBits; // Non Volitile configuration bits

#define SELF_CALIBRATED 0x0001
#define NV_VALUES_SET   0x0002
#define DEEP_SAVE       0x0004

 // Defines a generic device Object.
typedef struct a_a
{   // Public Members
    Boolean           Used;
    SubSysTypes       SubSystemType;
    const char      * Discription;
    unsigned          AuxUse;
    unsigned          Id;

    // Descriptor MainThread;
    unsigned          WaitingOnMask;

    void            * Arg;
 
    void              (*ProcessFn)(void);             // Filled out by Object/Requester
    void              (*HandleSubSystem)(void);
    
    // Common Operations
    void              (*Reset)(void);
    void              (*Flush)(void);
}SubSystem;

void InitSubSystems(Register * );
//SubSystem * CopySubSystem(SubSystem * );
SubSystemBits * GetSubSysBits(void);

#endif
