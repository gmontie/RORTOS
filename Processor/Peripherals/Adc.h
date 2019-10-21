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
#ifndef ADC_H
#define ADC_H

#include "defs.h"
#include "Register.h"
#include "Registers.h"

// Base 2 Math starts here
#define DOTPOS_NONE          5 // ADCShortToString will not show dot
#define DOTPOS_TRAIL_ZEROS  -1 // ADCShortToString will not show dot and fill front space with zeros
#define SHIFTS               5 // 2^5 => 32 which is our divisor
#define NUMBER_OF_SAMPLES    (1 << SHIFTS) // Total number of samples

/*****************************************************************************/
/*                                                                           */
/* The AdOps structure defines an Analog to Didital Converter like Object    */
/* which can be used by sensors and other Objects. It can also stand alone.  */
/*                                                                           */
/*****************************************************************************/
typedef struct
{
    unsigned (*Reading)(void);        // Return the ADC reading
    void (*Start)(void);       // Start ADC Peripheral
    void (*Stop)(void);
    void (*StartReading)(void);   // Start DMA readings for the ADC
    void (*StopReading)(void);    // Stop DMA readings for the ADC
    void (*Sort)(void);
    void (*Next)(void); // Calculate the next ADC reading
    void (*Clear)(void);
    Boolean ReadingReady;         // Indicate if the next reading is ready.
}AdOps;

AdOps *  ADCInit( unsigned ); // Initialize the ADC Object.

#endif
