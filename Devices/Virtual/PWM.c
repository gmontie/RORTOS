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
#include "Register.h"
#include "Registers.h"
#include "Process.h"
#include "System.h"
#include "Device.h"
#include "PWM.h"

#define pwm Sfr
#define RegPWM SharedMem

#define PWM_PERIOD 0x800 //0x300
#define PWM_DEVICE "PWM"

static inline void SetUPIC(void);
static void UpDatePWM(void);
static void PWMWrite(byte , unsigned );
static unsigned ReadPWM(void);
static Boolean PWMIsReady(void){return True;}
static void ResetTimer(void);

static unsigned PwmPeriod = PWM_PERIOD;

static SharedPair Dev =
{
  .pwm = 0, //&OC1R,
  .RegPWM = 0, 
};

// Service operations used..
static Service This =
{
   .DeviceType=Pwm,
   .DeviceClass=SYSTEM_DEV,
   .Discription=PWM_DEVICE,
   .AuxUse = 0,
   .Id = (0x0100 + (int)Pwm),
   .Thread = UpDatePWM, 
   .Arg = 0,
   .FnType = ProcessFn,
   .Peek = ReadPWM,
   .Write = PWMWrite,
   .Reset = ResetTimer,
   .IsReady = PWMIsReady,
};

/****************************************************************************/
/*                                                                          */
/*     Function: PWMInit                                                    */
/*        Input: None                                                       */
/*       Return: None                                                       */
/*     Overview: Initailize the PWM output                                  */
/*                                                                          */
/****************************************************************************/
Service * NewPWM(volatile unsigned * PwmAt, unsigned Index)
{
  Dev.pwm = PwmAt;
  Dev.RegPWM = getRegister(Index);

  /* set PWM duty cycle to 50% */
  OC1R       = 0x1000; // Determins the on time....
  *Dev.pwm   = 0x0; // Determins the period

  // Timer One Init to be tested later.....
  Timer2Init(PwmPeriod, 1);
  SetUPIC();
  TIMER2_ON = 1;
  This.Status.Allocated = True;
  This.Status.Ready = True;
  This.Driver = &This;
  return &This;
}

/****************************************************************************/
/*                                                                          */
/*     Function: ResetTimer                                                 */
/*        Input: None                                                       */
/*       Return: None                                                       */
/*     Overview: Reset the timer used for the PWM output                    */
/*                                                                          */
/****************************************************************************/
static void ResetTimer(void)
{
  TIMER2_ON = 0; // Turn off Timer
}

/****************************************************************************/
/*                                                                          */
/*     Function: SetUPIC                                                    */
/*        Input: None                                                       */
/*       Return: None                                                       */
/*     Overview: Assign OC1 to be output on RF13                            */
/*                                                                          */
/****************************************************************************/
static inline void SetUPIC(void)
{   // Setup Output Compare Peripheral
    OC1CONbits.OCM = 0b110; // PWM mode without fault protection
}

/****************************************************************************/
/*                                                                          */
/*     Function: Update PWM Value                                           */
/*        Input: None                                                       */
/*       Return: None                                                       */
/*     Overview: This function takes the PWM value from the Pwm Register    */
/*               (from the Register File) and puts it into the OC1RS        */
/*               special function register. This is how the PWM value is    */
/*               controlled.                                                */
/*                                                                          */
/****************************************************************************/
static void UpDatePWM(void)
{
    if(*Dev.pwm != Dev.RegPWM->Value) // Pwm.PWM_Reg->Value)
    {
       *Dev.pwm = Dev.RegPWM->Value;
       Dev.RegPWM->Changed = True;
    }
}

/****************************************************************************/
/*                                                                          */
/*     Function: PWMWrite                                                   */
/*        Input: Dummy byte                                                 */
/*               Mew PWM value                                              */
/*       Return: None                                                       */
/*     Overview: Change the PWM ration by writing a new value to the PWM    */
/*                                                                          */

/****************************************************************************/
void PWMWrite(byte Operation, unsigned Value)
{
   switch (Operation)
   {
      case CHANGE_PERIOD:
         PwmPeriod = Value;
         Timer2Init(PwmPeriod, 1);
         SetUPIC();
         TIMER2_ON = 1;
         break;
      case CHANGE_DUTY_CYCLE:
         *Dev.pwm = Value;
         Dev.RegPWM->Value = Value;
         Dev.RegPWM->Changed = True;
         break;
   }
}

/****************************************************************************/
/*                                                                          */
/*     Function: ReadPWM                                                    */
/*        Input: Dummy byte                                                 */
/*       Return: 16 bit value for the PWM                                   */
/*     Overview: Read the current value in the PWM OC1 register             */
/*                                                                          */
/****************************************************************************/
unsigned ReadPWM(void)
{
   return OC1RS;
}
