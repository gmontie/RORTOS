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
#ifndef SYSTEM_H
#define SYSTEM_H

#include <xc.h>
#include "Register.h"
#include "Registers.h"

#if NO_CRYSTAL
  #define M_PLL    37 
  #define InClc	 7370000L
  #define Fosc    (InClc / 4) * (M_PLL + 2)
#else
  #define M_PLL     12
  #define InClc	 22118400L
  #define Fosc    (InClc / 4) * (M_PLL + 2)
#endif

#define TIMER1_ON T1CONbits.TON
#define TIMER2_ON T2CONbits.TON
#define TIMER3_ON T3CONbits.TON

#define Fcy	(Fosc >> 1)	// With PLL

#define PushSR __asm__ volatile("  PUSH SR")
#define PopSR  __asm__ volatile("  POP  SR")

#define mcuAtomicOr(variable, mask)   __asm__ volatile (" ior.w %0, [%1], [%1] \n": : "r"(mask), "r"(&(variable)): "cc")
#define mcuAtomicAnd(variable, mask)  __asm__ volatile (" and.w %0, [%1], [%1] \n": : "r"(mask), "r"(&(variable)): "cc")
#define mcuAtomicXor(variable, mask)  __asm__ volatile (" xor.w %0, [%1], [%1] \n": : "r"(mask), "r"(&(variable)): "cc")

typedef struct 
{
   volatile unsigned * Sfr;
   Register * SharedMem;   
}SharedPair;

typedef union
{
  int uint16_t;
  unsigned int int16_t;
  char int8_t[sizeof(int)];
  unsigned char uint8_t[sizeof(unsigned int)];
}IntTypesUnion;

void HardwareInit( void );
void InitStatus( void );
void Delay(int DelayValue);
void I2C_Delay(unsigned int Count);
inline void WaitForTx(int DelayValue);
void Timer1Init(unsigned, byte);
void Timer2Init(unsigned, byte); // Used for PWM
void Timer3Init(unsigned, byte);
void DelayTimer(unsigned Msw, unsigned Lsw); // Uses Timer 4 and 5

#endif	/* SYSTEM_H */

