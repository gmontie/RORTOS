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
#ifndef DEVICE_H
#define DEVICE_H

#include "defs.h"
#include "Registers.h"

typedef enum
{
   RAW = 0, 
   M25LC320, 
   MC25LC256, 
   MCP23S17, 
   CD4051,
   Console, 
   FeedBack, 
   PortB_IO, 
   Pwm, 
   FAN,
   FANS,
   FAN_BANK,
   SHARED_MUX,
   USER_THREAD,
   VOLT_METER,
   ANALOG_WLKR,
   NTXD1HX103,
   ROM_SETTINGS,
} DeviceTypes;

typedef enum
{
    FLASH_MEM, 
    MULTIPLEXER, 
    PWM_CTL_DEV, 
    THERM,
    COMMUNICATIONS,
    ANALOG_INPUT,
    ANALOG_OUTPUT,
    DIGITAL_IO,
    SYSTEM_DEV,
}DevClass;

typedef enum{ None = 0, Run, Block, Terminate}StateRequest;

typedef struct
{
    union
    {
        struct
        {
            unsigned Allocated    : 1;
            unsigned Sleeping     : 1;
            unsigned Ready        : 1;
            unsigned Alarm        : 1;

            unsigned              : 12;
        };
        unsigned Field;
    };
    StateRequest Request;
} ThreadStates;

#define NO_OF_DEVICES 8

typedef enum{ReadFn, ReadingFn, PeekFn, NextFn}InputFnTypes;
typedef enum{WriteFn, PokeFn}OutputFnTypes;
typedef enum{UpDateFn, DeviceFn, ProcessFn}DevFnTypes;


 // Defines a generic device Object.
typedef struct service
{   // Public Members
    DeviceTypes    DeviceType;
    DevClass       DeviceClass;
    ThreadStates   Status;
    const char   * Discription;
    unsigned       AuxUse;
    unsigned       Id;

    // Descriptor MainThread;
    BlockingOn     WaitingOnMask;

    // Arguments, or a pointer to other structs
    union
    {
        struct service * Dev;
        void           * Arg;
    };

    DevFnTypes     FnType;        // Filled out by Object/Requester
    
    // Maine Thread  
    union 
    {
       void (*UpDate)(void*);
       void (*Device)(struct service*);
       void (*Thread)(void);
    };
    //  End of embedded descriptor MaineThread

    InputFnTypes   InputType;
    
    // Reading Data
    union
    {
       unsigned (*Read)(byte); // byte is for the address of the register
       float    (*Reading)(void);
       unsigned (*Peek)(void);
       int      (*Next)(void);
    };
    
    // Writing Data
    void    (*Write)(byte Where, unsigned What);
    void    (*Poke)(unsigned);
    
    // Common Operations
    union
    {
       void    (*Reset)(void);
       void    (*Flush)(void);
    };
    
    // Predicate to tell if driver is ready
    Boolean (*IsReady)(void);
    
    // Sub Driver usually used internally
    void     *Driver;
}Service;

Service * NewDevice(void * );
Service * NewDeviceCpy(Service * Obj);
Boolean ReadyTrue(void);
Boolean ReadyFalse(void);

#endif
