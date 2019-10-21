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
#ifndef PROCESS_TABLE_H
#define PROCESS_TABLE_H

#include "defs.h"
#include "Device.h"
#include "Registers.h"
#include "SysCalls.h"

#define MAX_PROCESSES  0x05
#define STACK_SIZE     0x0100
#define dsPIC33_Size   16

#define _Defined(a)   _CDefine(a) // Stringify a
#define _CDefine(a)   #a

/*
    1.1 Created
    1.2 Ready
    1.3 Running
        1.3.1 Kernel mode
        1.3.2 User mode
    1.4 Blocked
    1.5 Terminated
*/
typedef enum{Ready=0, Running=1, Blocked=2, Terminated=3}ThreadsState;

typedef void (*UpDatePtr)(void*);
typedef void (*DevicePtr)(Service*);
typedef void (*ProcessPtr)(void);

/*****************************************************************************/
/*                                                                           */
/* The Process Object                                                        */
/*                                                                           */
/*                                                                           */
/*                                                                           */
/*****************************************************************************/
typedef struct process_entry 
{
    byte          Stack[STACK_SIZE] __attribute__((aligned(16)));

    unsigned      Splim; // Stack Limit
    unsigned      Address_1; // Return to if Running
    unsigned      Address_2; // Return to if Running
    ThreadsState  State;

    // Thread Data
    byte          Priority;
    
    // The used or not byte
    byte          Used;    
    unsigned      Quantum; // Filled out by System/Issuer
    Bus           Resource;
    DevFnTypes    FnType;        // Filled out by Object/Requester

    void       * Arg;
    
    union 
    {
        UpDatePtr  UpDateThread;  // Filled out by Object/Requester
        DevicePtr  DeviceThread;  // Filled out by Object/Requester
        ProcessPtr ProcessThread; // Filled out by Object/Requester
    };
    
    // Thread Context
    unsigned     PID;
    Service    * Dev;
    
    struct process_entry * Next;
} t_Process; //__attribute__((aligned(16)));


/*****************************************************************************/
/*                                                                           */
/* Operating Systm Object                                                    */
/*                                                                           */
/*                                                                           */
/*                                                                           */
/*****************************************************************************/
typedef struct
{
    void (*Run)(void);
    Boolean (*Add)(Service * );
    void (*WaitOn)(Service *, unsigned );
    void (*System)(int);
}Kernel;

Kernel * InitOS(void);

void __attribute__((interrupt, shadow, no_auto_psv)) _T3Interrupt(void);

#endif
