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
#include <stdlib.h>
#include <string.h>
#include "defs.h"
#include "Device.h"
#include "System.h"
#include "Register.h"
#include "PinConfig.h"
#include "Blk.h"
#include "FanAUB0812VH.h"
#include "Servlet.h"


/****************************************************************************/
/*                                                                          */
/*  Duty Cycle      Speed RPM                                               */
/*    100%             3600                                                 */
/*    0-20%          500~1000                                               */
/*                                                                          */
/*  Wiring                                                                  */
/*  ------------------------------------                                    */
/*  YELLOW: Voltage Input                                                   */
/*   GREEN: FG Signal (Fan Speed Output)                                    */
/*   BLACK: GND                                                             */
/*    BLUE: PWM Control                                                     */
/*                                                                          */
/*  Operating Voltages and Rating                                           */
/*  10.8 to 13.2 Max                                                        */
/*  Rated at 12 Volts                                                       */
/*                                                                          */
/****************************************************************************/


#define NO_OF_FANS 4
#define DELTA_AUB8012V "Delta AUB0812VH"
#define DESCRIPTION "Delta Fan"

// Private Members
static void StoreFanCounts(Servlet * Conv);

IndicatorBits * SystemStat;
Register * FanCounts;
Register * FanSpeed;

/****************************************************************************/
/*                                                                          */
/*     Function: NewFanlDevice                                              */
/*        Input:                                                            */
/*       Output:                                                            */
/*     Overview:                                                            */
/*                                                                          */
/****************************************************************************/
Servlet * NewAUB0812VH(VBLOCK * BlkPtr) //Register * FanCounts, Register * FanRps)
{ 
   Servlet * This = NewServlet();
   FanCounts = getRegister(BlkPtr->IndexList[0]);
   FanSpeed = getRegister(BlkPtr->IndexList[1]);
   SystemStat = getSystemStat();
   This->DeviceType = FAN;
   This->DeviceClass = PWM_CTL_DEV;
   This->Discription = DELTA_AUB8012V;
   This->Id = (0x0100 + (int)NTXD1HX103 + (unsigned)This);
   This->Run = StoreFanCounts;
   return This;
}

/******************************************************************************/
/*                                                                            */
/******************************************************************************/
static void StoreFanCounts(Servlet * Conv)
{
   FanCounts->Value = Conv->Reference->Value;
   FanCounts->Changed = True;
}


