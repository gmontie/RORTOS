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
#include <math.h>
#include <float.h>
#include <string.h>
#include "defs.h"
#include "Registers.h"
#include "Register.h"
#include "CD74HCx4051.h"
#include "Analog.h"
#include "Device.h"
#include "Process.h"
#include "Blk.h"

//////////////////////////////////////////////////////////////////////////////
// Constants Area
//
#define DELTA                0x16
#define SETTLE_COUNT            2
#define MAX_UPDATE_COUNTS      15
#define MUX_RAPAROUND      0b0011

//////////////////////////////////////////////////////////////////////////////
// Private Methods
//
static void HandleSamples( void );
void NewSample(void);
static Boolean ReadingReady(void);

#define DISCRIPTION_ANLG_METER "Analog Volt Meter"

//////////////////////////////////////////////////////////////////////////////
// Private members
//
static ADC_SAMPLING_STATES ADC_SampleState;
static iQueue Samples;
static AdOps * Adc;
static Service * cd4051;
static long Sum;
static unsigned AverageOfSamples = 0;
static unsigned aNewSample = 0;
static unsigned AverageCounts;
static unsigned RapAround;
static unsigned Skips;
static unsigned CurrentIndex;
static Register * ADReading; // A Pointer to the Raw ADC Reading Register
static IndicatorBits * SystemStat;

static Service This =
{
   //.Used = True,
   .DeviceType = ANALOG_WLKR,
   .DeviceClass = ANALOG_INPUT,
   .Discription = DISCRIPTION_ANLG_METER,
   .AuxUse = 0,
   .Id = (0x0100 + (int)ANALOG_WLKR),
   .Arg = 0, //
   .FnType = ProcessFn,
   .Thread = HandleSamples,
   .Reading = 0, //ReadVoltage,
   .Write = 0,   //WriteCalData,
   .Reset = 0,   //ResetAverage,
   .Flush = 0,   //ResetAverage,
   .IsReady = ReadingReady,
};

/******************************************************************************/
/*   Instantiate Module                                                       */
/******************************************************************************/
Service * NewAnalog(AdOps * ChipsADC, Service * MuxIn, VBLOCK * Blk)
{
   Adc = ChipsADC;

   ADReading = getRegister( Blk->NextValue() );
   SystemStat = getSystemStat();
   if(MuxIn != 0)
   {
      cd4051 = MuxIn;
      RapAround = cd4051->Read( 0 );
      CurrentIndex = 0;
   }
   else
   {
      cd4051 = 0;
      RapAround = 0;
   }
   ADC_SampleState = START;
   Skips = SETTLE_COUNT;
   This.Status.Allocated = True;
   This.Driver = ChipsADC;
   This.Arg = (void *)ADC_SampleState;
   Adc->Start(); // Start the ADC Peripheral
   //Adc->StartReading();
   return &This;
}

/******************************************************************************/
/*  Update ADC Reading                                                        */
/******************************************************************************/
static void HandleSamples(void)
{  
   switch (ADC_SampleState)
   {
      case START:
         Adc->StartReading();
         ADC_SampleState = READING_READY;
         break;
      case READING_READY:
         if (Adc->ReadingReady)
            ADC_SampleState = SORT_SAMPLES;
         break;
      case SORT_SAMPLES:
         Adc->Sort();
         ADC_SampleState = SKIP;
         break;
      case SKIP:
         Adc->Next(); // Display results after digitally filtering reading
         if(Skips == 0)
         {
            ADC_SampleState = NEXT_READING;
            Skips = SETTLE_COUNT;
         }
         else
         {
            Skips--;
            ADC_SampleState = START;
         }
         break;
      case NEXT_READING:
         if ( cd4051 )
         {
            cd4051->Thread();
            CurrentIndex++;
            CurrentIndex &= RapAround;
            cd4051->Poke(CurrentIndex);
         }
         ADC_SampleState = REPEAT;
         break;
      case REPEAT:
         ADC_SampleState = START;
         SystemStat->AdRdy = False;
         break;
   }
}

/******************************************************************************/
/*  Predicate to return if a reading is ready to be read                      */
/******************************************************************************/
static Boolean ReadingReady(void)
{
   return Adc->ReadingReady;
}

/******************************************************************************/
/*                                                                            */
/******************************************************************************/
 void NewSample(void)
{
   // Calculate new Sum and Average
   if (Samples.Count < IO_BUFFER_SIZE)
   {
      Samples.Count++;
      Sum += aNewSample;
      AverageOfSamples = (unsigned) (Sum / Samples.Count);
   }
   else
   {
      Sum -= Samples.Buffer[Samples.Head++];
      Samples.Head &= IO_RAPAROUND;
      Sum += aNewSample;
      AverageOfSamples = (unsigned) (Sum >> (IO_POWER_OF_2)); // Divide by 32 the # of samples.
   }

   AverageCounts = AverageOfSamples;

   // Record the new sample
   Samples.Buffer[Samples.Tail++] = aNewSample;
   Samples.Tail &= IO_RAPAROUND;
}
