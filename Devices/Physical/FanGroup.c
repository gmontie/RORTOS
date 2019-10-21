/**************************************************************************/
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
#include <stdlib.h>
#include "Register.h"
#include "Registers.h"
#include "PinConfig.h"
#include "FanGroup.h"
#include "System.h"

#ifndef TACH_TIMER_OVERFLOW
#define TACH_TIMER_OVERFLOW  0x0080
#endif

#ifndef FAN_CALCULATION_DONE
#define FAN_CALCULATION_DONE 0x0010
#endif
#define PWM_PERIOD 0x400 //0x300

#define FAN_COLLECTION "Fan Collection"

static Register * AverageFanCounts;
static Register * FanCounts;
static Register * MinSpeed;
static Register * MaxSpeed;
static Register * PWMPtr;

static IndicatorBits * SystemStat;

static void getCounts(void);
static void ResetStats(void);
static void CalibrateFans(PwmThread);

static long Total   = 0;
static unsigned Counts = 0;

static volatile unsigned CounterOne __attribute__ ((address(0x100))); // TMR1 Timer 1 ;

static Service This =
{
   .DeviceType=FAN_BANK,
   .DeviceClass = PWM_CTL_DEV,
   .Discription=FAN_COLLECTION,
   .AuxUse = 0,
   .Arg = 0, //
   .Id = (0x0100 + (int)ANALOG_WLKR),
   .Thread=getCounts,
   .Read=0,
   .Reset = ResetStats,
   .Write=0,
   .IsReady=0,
   .Driver=0,  
};

/****************************************************************************/
/*                                                                          */
/*     Function:                                                            */
/*        Input: None                                                       */
/*       Return: None                                                       */
/* Side Effects:                                                            */
/*     Overview: Assign OC1 to be output on RF13                            */
/*                                                                          */
/****************************************************************************/
Service * NewFanGroup(VBLOCK * BlkPtr, PwmThread Pwm)
{
   FanCounts = getRegister( BlkPtr->NextValue() );
   AverageFanCounts = getRegister( BlkPtr->NextValue() );
   MinSpeed = getRegister( BlkPtr->NextValue() );
   MaxSpeed = getRegister( BlkPtr->NextValue() );
   PWMPtr = getRegister( BlkPtr->NextValue() );
   SystemStat = getSystemStat();
   Timer1Init(0xFFFF, 0x01);
   CalibrateFans(Pwm);
   return &This;
}


/****************************************************************************/
/*                                                                          */
/*     Function: ChangeFanState                                             */
/*        Input: Relay Stat                                                 */
/*       Return: None                                                       */
/* Side Effects:                                                            */
/*     Overview: Turn the fans on or off via Relay                          */
/*                                                                          */
/****************************************************************************/
void ChangeFanState(Boolean _On_)
{
   RELAY = _On_;
}


/****************************************************************************/
/*                                                                          */
/*     Function: GetFanSpeed                                                */
/*        Input: PortNumber is the number of the port that the Tachometer   */
/*               is on. Port A is 0, Port B is 1 Port C is 2 and so on.     */
/*       Output: Success or Failure. If Success a positive number is        */
/*               returned. In the case that there is a Failure a negitive   */
/*               number is returned.                                        */
/* Side Effects: Changes a bit in bit field FLAGS. Also changes the         */
/*               value of Timer/Counter 1.                                  */
/*                                                                          */
/*     Overview: This function has been moved from being an interrupt       */
/*               driven procedure to a buisy waiting function so that       */
/*               the computation can be interruptable. The interruptability */
/*               in computing the fan speeds improves other critical        */
/*               functionality by reducing the amount of temperal resrouces */
/*               durring interrupt time.                                    */
/* Notes:                                                                   */
/*                                                                          */
/****************************************************************************/
static void getCounts(void)
{
   INTCON2bits.INT0EP = ON;
   IEC0bits.INT0IE = ON; // Enable External Interrupt 0
   // Clear Timer Interrupt Flags
   IFS0bits.T1IF = 0; //Clear the Timer1 interrupt status flag
   IEC0bits.T1IE = 1; //Enable Timer1 interrupts
   Total = 0;
   Counts = 0;
   CounterOne = 0; // Defined as an absolute var at the address of Counter One

   // Turn on Timer
   TIMER1_ON = ON; // Turn on Timer
}

/****************************************************************************/
/*                                                                          */
/*     Function: ResetStats                                                 */
/*        Input: None                                                       */
/*       Return: None                                                       */
/* Side Effects:                                                            */
/*     Overview: Save the current value of total in Average Fan Counts      */
/*                                                                          */
/****************************************************************************/
static void ResetStats(void)
{
   AverageFanCounts->Value = (Total >> 4);
   AverageFanCounts->Changed = True;
   Total = 0;
}

/****************************************************************************/
/*                                                                          */
/*     Function: Calibrate Fans                                             */
/*        Input: None                                                       */
/*       Return: None                                                       */
/* Side Effects: None                                                       */
/*     Overview: This function records the lower and upper fan speed limits */
/*                                                                          */
/****************************************************************************/
static void CalibrateFans(PwmThread Fn)
{
  RELAY = 1;
  PWMPtr->Value = 0;
  PWMPtr->Changed = True;
  Delay(65000);
  Fn();
  Delay(65000);
  Delay(65000);
  Delay(65000);
  Delay(65000);
  Delay(65000);
  Delay(65000);
  Delay(65000);
  Delay(65000);
  Delay(65000);
  Delay(65000);
  Delay(65000);
  Delay(65000);
  Delay(65000);
  getCounts();
  while(SystemStat->FTDone != True);
  MinSpeed->Value = FanCounts->Value;
  MinSpeed->Changed = True;

  PWMPtr->Value = 0;
  PWMPtr->Changed = True;
  Delay(65000);
  Fn();
  Delay(65000);
  Delay(65000);
  Delay(65000);
  Delay(65000);
  Delay(65000);
  Delay(65000);
  Delay(65000);
  Delay(65000);
  Delay(65000);
  Delay(65000);
  getCounts();
  while(SystemStat->FTDone != True);
  MinSpeed->Value = FanCounts->Value;
  MinSpeed->Changed = True;
}

/****************************************************************************/
/*                                                                          */
/*     Function: AdjustPWMValue                                             */
/*        Input: None                                                       */
/*       Return: None                                                       */
/* Side Effects:                                                            */
/*     Overview: Changes the control signal to the fans                     */
/*                                                                          */
/****************************************************************************/
void AdjustPWMValue(void)
{
//  int Results = abs(Fan.FanSpeed->Value - Fan.SetPoint->Value);
//  int FnSpeed = Fan.FanSpeed->Value;
//  int StPoint = Fan.SetPoint->Value;
//
//  if(FnSpeed > StPoint)
//  {
//    if(Results > 4)
//      Fan.FanPWM->Value += 8;
//    else
//      if(Results > 2)
//        Fan.FanPWM->Value += 3;
//      else
//          Fan.FanPWM->Value += 1;
//      if(Fan.FanPWM->Value > 1023)
//        Fan.FanPWM->Value = 1023;
//    Fan.FanPWM->Status.Changed = 1; //Changed = True;
//  }
//  else
//  {
//    if(FnSpeed < StPoint)
//    {
//      if(Results > 4)
//        Fan.FanPWM->Value -= 8;
//      else
//        if(Results > 2)
//          Fan.FanPWM->Value -= 3;
//        else
//            Fan.FanPWM->Value -= 1;
//        if(Fan.FanPWM->Value > 1023)
//          Fan.FanPWM->Value = 0;
//      Fan.FanPWM->Status.Changed = 1; //Changed = True;
//    }
//  }
}

/****************************************************************************/
/*                                                                          */
/*     Function: ChangeFanState                                             */
/*        Input: None                                                       */
/*       Return: None                                                       */
/* Side Effects:                                                            */
/*     Overview: Turn the fan on or off                                     */
/*                                                                          */
/****************************************************************************/
void SetPointFallowsTemp(void)
{
}


/****************************************************************************/
/*                                                                          */
/*     Function: _T3Interrupt                                               */
/*        Input: None                                                       */
/*       Return: None                                                       */
/*     Overview: The Timer3Init                                             */
/*                                                                          */
/****************************************************************************/
void __attribute__((interrupt, shadow, no_auto_psv)) _T1Interrupt(void)
{
   SystemStat->Overflow=True;  
   IEC0bits.T1IE = 0; // Disable Timer1 interrupts
   IEC0bits.INT0IE = 0; // Turn off External Interrupt 0
   FanCounts->Value = 0;
   FanCounts->Changed = True;
   // Turn off Timer
   TIMER1_ON = OFF; // Stop Timer
   SystemStat->FTDone = True;
   IFS0bits.T1IF = 0; // Clear the Timer1 interrupt status flag
}

/****************************************************************************/
/*                                                                          */
/*     Function: _INT1Interrupt  (External Interrupt)                       */
/*        Input: None                                                       */
/*       Return: None                                                       */
/*     Overview: The Timer3Init                                             */
/*                                                                          */
/****************************************************************************/
void __attribute__((__interrupt__, auto_psv)) _INT0Interrupt(void)
{
   Counts++;
   if(Counts >= 2) 
   {
      FanCounts->Value = (unsigned)CounterOne;
      Total += FanCounts->Value;
      FanCounts->Changed = True;
      TIMER1_ON = OFF; // Stop Timer
      SystemStat->FTDone = True;
      IEC0bits.INT0IE = 0; // Turn off External Interrupt 0
   }
   IFS0bits.INT0IF = 0;    // Clear the INT1 interrupt flag 
}
