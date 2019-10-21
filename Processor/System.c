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
#include "defs.h"
#include "PinConfig.h"
#include "System.h"

// Oscillator Initialization
static void OscillatorInit(void); // Make sure that function can not be seen
// outsize of this file.
static IndicatorBits * StatusRegister;

/****************************************************************************/
/*                                                                          */
/*     Function: HardwareInit                                               */
/*        Input: None                                                       */
/*       Return: None                                                       */
/*     Overview: This function contains the calls to all of the functions   */
/*               needed to bring up the processor.                          */
/*                                                                          */
/****************************************************************************/
void HardwareInit()
{
   RCONbits.SWDTEN = 0; // Turn watchdog timer off
   SystemConfig();
   Delay(30);
   OscillatorInit();
}

/****************************************************************************/
/*                                                                          */
/*     Function: InitStatus                                                 */
/*        Input: A reference to the Status Register                         */
/*       Return: None                                                       */
/*     Overview: This function Inits values of the Status register          */
/*                                                                          */
/****************************************************************************/
void InitStatus(void)
{
   StatusRegister = getSystemStat();
   StatusRegister->FTDone = True;
}

/****************************************************************************/
/*                                                                          */
/*     Function: OscillatorInit                                             */
/*        Input: None                                                       */
/*       Return: None                                                       */
/*     Overview: Sets up the Oscillator which is use for the processor      */
/*               clock.                                                     */
/*                                                                          */
/*     Internal Clock will PLL = 66,330,000 MHz                             */
/*                                                                          */
/*                                                                          */
/*   if CLKDIVbits.RCDIV0 = 0, then                                         */
/*   processor clock=32MHz ==> instruction cycle clock, FCY or FOSC/2=16MHz */
/*                                                                          */
/****************************************************************************/
static void OscillatorInit(void)
{
#ifdef CRYSTAL
   PLLFBD = M_PLL;
   CLKDIVbits.ROI = 1;
   CLKDIVbits.DOZE = 0;
   CLKDIVbits.DOZEN = 0;
   CLKDIVbits.FRCDIV = 0;
   CLKDIVbits.PLLPOST = 0;
   CLKDIVbits.PLLPRE = 0;

   OSCCONbits.NOSC = 0b011;
   OSCCONbits.CLKLOCK = 0;

   RCONbits.SWDTEN = 0; // Disable Watchdog timer!

   OSCTUN = 0;

   // Initiate Clock Switch to Primary OSC with PLL (NOSC=0b011)
   __builtin_write_OSCCONH(0b11); // Initiate Clock Switch to Internal FRC with PLL (NOSC = 0b011)
   __builtin_write_OSCCONL(0b01);

   asm("nop"); // Take a few cycles before testing for COSC != 0b11
   asm("nop");
   asm("nop");
   asm("nop");

   // Wait for Clock switch to occur
   while (OSCCONbits.COSC != 0b011);

   // Wait for PLL to lock
   while (OSCCONbits.LOCK != 1);

   asm("nop");
   asm("nop");
   asm("nop");
   asm("nop");
#else
   // Configure PLL prescaler, PLL postscaler, PLL divisor
   PLLFBD = M_PLL; //34; // M = 36 
   CLKDIVbits.PLLPOST = 0; // N2 = 2
   CLKDIVbits.PLLPRE = 0; // N1 = 2
   CLKDIVbits.DOZEN = 0;
   CLKDIVbits.FRCDIV = 0;

   OSCCONbits.NOSC = 0x001;
   OSCCONbits.CLKLOCK = 0;

   OSCTUN = 0; //0x40;

   // Initiate Clock Switch to Internal FRC with PLL (NOSC = 0b001)
   __builtin_write_OSCCONH(0x01);
   __builtin_write_OSCCONL(0x01);

   // Wait for Clock switch to occur
   while (OSCCONbits.COSC != 0b001);
   // Wait for PLL to lock
   while (OSCCONbits.LOCK != 1);
#endif
 }

/*****************************************************************************/
/*                                                                           */
/*****************************************************************************/
void I2C_Delay(unsigned int Count)
{
   while (--Count)
   {
      asm("nop");
      asm("nop");
   }
}

/****************************************************************************/
/*                                                                          */
/*     Function: Delay                                                      */
/*        Input: A Delay Value to be used to generate a delay               */
/*       Return: None                                                       */
/*     Overview: This is a simple (quick and dirty) delay function.         */
/*                                                                          */
/****************************************************************************/
void Delay(int DelayValue)
{
   volatile unsigned int i = 65535;
   volatile unsigned int j = DelayValue;

   while (j)
   {
      while (i)
      {
         i--;
      }
      j--;
   }
}

/****************************************************************************/
/*                                                                          */
/*     Function: Delay                                                      */
/*        Input: A Delay Value to be used to generate a delay               */
/*       Return: None                                                       */
/*     Overview: This is a simple (quick and dirty) delay function.         */
/*                                                                          */
/****************************************************************************/
inline void WaitForTx(int DelayValue)
{
   unsigned int i = 4095;
   unsigned int j = DelayValue;

   while (j)
   {
      while (i)
      {
         i--;
      }
      j--;
   }
}

/****************************************************************************/
/*                                                                          */
/*     Function: Timer1Init                                                 */
/*        Input: None                                                       */
/*       Return: None                                                       */
/*     Overview: The Timer1Init function initialized Timer 1                */
/*                                                                          */
/****************************************************************************/
void Timer1Init(unsigned ReloadValue, byte PreScaler)
{
  T1CON = 0x00; // Stops the Timer1 and reset control reg.
  T1CONbits.TCKPS = (PreScaler & 0x03);
  TMR1 = 0x00; // Clear contents of the timer register
  PR1 = ReloadValue; // Load the Period register with the value 0x8CFF
  IPC0bits.T1IP = 0x01; // Setup Timer1 interrupt for desired priority level
  
  // (this example assigns level 1 priority)
  IFS0bits.T1IF = 0; // Clear the Timer1 interrupt status flag
  IEC0bits.T1IE = 0; // Enable Timer1 interrupts
  T1CONbits.TON = 0; // Start Timer
}

/****************************************************************************/
/*                                                                          */
/*     Function: Timer2Init                                                 */
/*        Input: None                                                       */
/*       Return: None                                                       */
/*     Overview: The Timer3Init function initialized Timer 2                */
/*                                                                          */
/****************************************************************************/
void Timer2Init(unsigned ReloadValue, byte PreScaler)
{
  T2CON = 0x00;
  T2CONbits.TCKPS = (PreScaler & 0x03);
  TMR2 = 0x00; // Clear timer register
  PR2 = ReloadValue;// PWM_PERIOD; // Load the period value
  IPC1bits.T2IP = 0x05; // Set Timer 2 Interrupt Priority Level
  IFS0bits.T2IF = 0; // Clear Timer 2 Interrupt Flag
  IEC0bits.T2IE = 0; // Enable Timer 2 interrupt
  T2CONbits.TON = 1; // Start Timer
}

/****************************************************************************/
/*                                                                          */
/*     Function: Timer3Init                                                 */
/*        Input: None                                                       */
/*       Return: None                                                       */
/*     Overview: The Timer3Init function initialized Timer 3                */
/*                                                                          */
/****************************************************************************/
void Timer3Init(unsigned ReloadValue, byte PreScaler)
{
  T3CON = 0x00; // Stops the Timer1 and reset control reg.
  T3CONbits.TCKPS = (PreScaler & 0x03);
  TMR3 = 0x00; // Clear contents of the timer register
  PR3 = ReloadValue; // Load the Period register with the value 0x8CFF
  IPC0bits.T1IP = 0b110; // Setup Timer3 interrupt for desired priority level
  // (this example assigns level 1 priority)
  IFS0bits.T3IF = 0; // Clear the Timer1 interrupt status flag
  IEC0bits.T3IE = 0; // Enable Timer1 interrupts
  T3CONbits.TON = 0; // Start Timer
}

/****************************************************************************/
/*                                                                          */
/*     Function: Timer4Init                                                 */
/*        Input: None                                                       */
/*       Return: None                                                       */
/*     Overview: The Timer3Init function initialized Timer 2                */
/*                                                                          */
/****************************************************************************/
void DelayTimer(unsigned Msw, unsigned Lsw)
{
   PR5 = Msw; // Load 32-bit period value (msw)
   PR4 = Lsw; // Load 32-bit period value (lsw)

   T5CONbits.TON = 0; // Stop Timer 5
   T4CONbits.TON = 0; // Stop Timer 4
   T4CONbits.T32 = 1; // User 32 bit timer
   T4CONbits.TCS = 0; // Select internal instruction cycle clock
   T4CONbits.TGATE = 0; // Disable Gated Timer mode
   T4CONbits.TCKPS = 3; // 1:256
   TMR5 = 0x00; // Clear 32-bit Timer (msw)
   TMR4 = 0x00; // Clear 32-bit Timer (lsw)
   IPC7bits.T5IP = 0x01; // Set Timer 5 Interrupt Priority Level
   IFS1bits.T5IF = 0; // Clear Timer 5 Interrupt Flag
   IEC1bits.T5IE = 1; // Enable Timer 5 interrupt
   StatusRegister->FTDone = False;
   T4CONbits.TON = 1; // Turn on / Start 32 bit Timer
}

/****************************************************************************/
/*                                                                          */
/*     Function: _T5Interrupt                                               */
/*        Input: None                                                       */
/*       Return: None                                                       */
/*     Overview: The Timer3Init function initialized Timer 2                */
/*                                                                          */
/****************************************************************************/
void __attribute__((__interrupt__, no_auto_psv)) _T5Interrupt(void)
{
   /* Interrupt Service Routine code goes here */
   // StatusRegister->FTDone = True;
   T4CONbits.TON = 0;
   T5CONbits.TON = 0; // Stop Timer 5
   IFS1bits.T5IF = 0; // Clear Timer5 Interrupt Flag
}
