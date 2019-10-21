#/**************************************************************************/
/*                                                                        */
/*  Programmer: Gregory L Montgomery                                      */
/*                                                                        */
/*                                                                        */
/*                                                                        */
/*  COPYING: (See the file COPYING for the GNU General Public License).   */
/*  this program is free software, and you may redistribute it and/or     */
/*  modify it under the terms of the GNU General Public License as        */
/*  published by the Free Software Foundation                             */
/*                                                                        */
/*  This program is distributed in the hope that it will be useful,       */
/*  but WITHOUT ANY WARRANTY; without even the implied warranty of        */
/*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                  */
/*  See the GNU General Public License for more details.                  */
/*                                                                        */
/*  You should have received a copy of the GNU General Public License     */
/*  along with this program. See the file 'COPYING' for more details      */
/*  if for some reason you did not get a copy of the GNU General Public   */
/*  License a copy can be obtained by write to the Free Software          */
/*  Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */
/*                                                                        */
/*  You are welcome to use, share and improve this program.               */
/*  You are forbidden to prevent anyone else to use, share and improve    */
/*  what you give them.                                                   */
/*                                                                        */
/**************************************************************************/
#include <xc.h>
#include <math.h>
#include <float.h>
#include <string.h>
#include <stdlib.h>
#include "Register.h"
#include "Device.h"
#include "Registers.h"
#include "PowerSupply.h"
#include "System.h"

//////////////////////////////////////////////////////////////////////////////
// Constants Area
//
#define PWM_PERIOD 0x400 //0x300

//////////////////////////////////////////////////////////////////////////////
// Private Methods
//
static void UpdatePowerSupply(void);
static void CheckUserSettings(void);

static PS This =
{
   .UpdatePowerSupply = UpdatePowerSupply,
   .CheckUserSettings = CheckUserSettings
};

//////////////////////////////////////////////////////////////////////////////
// Type declarations
//
static Register * Element;
//static Device * VoltMeter;
unsigned long Numer;
unsigned Denom;

//////////////////////////////////////////////////////////////////////////////
// Private members
//
//static long double Voltage;
static long double UserVoltage;

/******************************************************************************/
/*   Istantiate Module                                                        */
/******************************************************************************/
PS * InitPowerSupply(Register * Regs, Device * Vm)
{
   Element = Regs;
   //VoltMeter = Vm;

   return &This;
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
static void UpdatePowerSupply(void)
{
   //float ErrorVoltage = 0.0;
   //unsigned Value;

   /*
   if ((Element[STATUS_WORD].Stats.Mode) && (Element[STATUS_WORD].Stats.Calibrated))
   {
      if (Element[STATUS_WORD].Stats.FTDone)
      {
         if (UserVoltage > 0.5)
         { // Voltage Tracking mode
            Voltage = VoltMeter->ReadVoltage();

            ErrorVoltage = (UserVoltage - Voltage) * 100;
            Value = fabs(ErrorVoltage);
            if (ErrorVoltage > 0.001)
            {
               if (PWM_Reg->Value > Value)
               {
                  PWM_Reg->Value -= Value;
                  PWM_Reg->Stats.Changed = True;
               }
            }

            if (ErrorVoltage < 0.001)
            {
               if ((PWM_Reg->Value + Value) < 0x3FF)
               {
                  PWM_Reg->Value += Value;
                  PWM_Reg->Stats.Changed = True;
               }
            }
         }
         if(PWM_Reg->Value > 1023)
            PWM_Reg->Value = 1023;
         DelayTimer(0, 0x1FFF);
      }
      OC1RS = PWM_Reg->Value;
   }
   else
   { // We are in PWM Mode
      if (OC1RS != PWM_Reg->Value)
         OC1RS = PWM_Reg->Value;
   }
   */
}

/****************************************************************************/
/*                                                                          */
/*     Function:                                                            */
/*        Input: None                                                       */
/*       Return: None                                                       */
/*     Overview:                                                            */
/*                                                                          */
/****************************************************************************/
static void CheckUserSettings(void)
{
   Boolean Changed = False;

   if (Numer != Element[Voltage_N].Value)
   {
      Numer = Element[Voltage_N].Value;
      Changed = True;
   }

   if (Denom != Element[Voltage_d].Value)
   {
      Denom = Element[Voltage_d].Value;
      Changed = True;
   }

   if (Changed)
   {
      UserVoltage = (Numer << 16) + Denom;
      UserVoltage /= 65536;
   }
}
