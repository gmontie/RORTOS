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
#include "VltMeter.h"
#include "Registers.h"
#include "Register.h"
#include "Device.h"
#include "Process.h"
#include "Adc.h"

//////////////////////////////////////////////////////////////////////////////
// Constants Area
//
#define DELTA 0x16
#define MAX_WAIT_STATES    7
#define MAX_UPDATE_COUNTS 15

//////////////////////////////////////////////////////////////////////////////
// Type declarations
//
//typedef struct
//{
//   byte Head;
//   byte Tail;
//   byte Count;
//   int Buffer[IO_BUFFER_SIZE];
//} CirculareBuffer;

typedef enum
{
   CHK_READING, SORT_SAMPLES, NEXT_READING, CHK_HI_LO, PROCESS_SAMPLES, TAKE_READING
} ADC_SAMPLING_STATES;

//typedef enum{COUNTS_MAX, COUNTS_MIN, VOLTAGE_MAX, VOLTAGE_MIN}VoltageSteps;

//////////////////////////////////////////////////////////////////////////////
// Private Methods
//
static void HandleSamples(void);
static float ReadVoltage(void);
static void ResetAverage(void);
static void NewSample(void);
static Boolean ReadingReady(void);
static void WriteCalData(byte WhichValue, unsigned Value);

#define DISCRIPTION_VOLT_METER  "Volt Meter"
static Service This =
{
   .Used = True,
   .DeviceType = VOLT_METER,
   .Discription = DISCRIPTION_VOLT_METER,
   .AuxUse = 0,
   .Id = (0x0100 + (int)VOLT_METER),
   .Arg = 0,
   .FnType = ProcessFn,
   .Thread = HandleSamples,
   .Reading = ReadVoltage,
   .Write = WriteCalData,
   .Reset = ResetAverage,
   .Flush = ResetAverage,
   .IsReady = ReadingReady,
};

//////////////////////////////////////////////////////////////////////////////
// Private members
//
static ADC_SAMPLING_STATES ADC_SampleState;
static iQueue Samples;
static AdOps * Adc;
static long Sum;
static unsigned AverageOfSamples = 0;
static unsigned aNewSample = 0;
static float Slope;
static float Offset;
static volatile float Volts; // = Slope * AverageOfSamples + Offset;
static Boolean Calibrated = False;
static unsigned AverageCounts;
static Register * Element; // A register element pointer
static IndicatorBits * SystemStat;

#ifdef THREADED_SYSTEM
#include "Descriptor.h"
static Descriptor Description = 
{
   .ProcessThread = HandleSamples,
   .WaitingOnMask = 0
};

/****************************************************************************/
/*                                                                          */
/*     Function:                                                            */
/*        Input: None                                                       */
/*       Return: None                                                       */
/*     Overview:                                                            */
/*                                                                          */
/****************************************************************************/
Descriptor * GetDDescriptor(void)
{
   return &Description;
}
#endif 

/******************************************************************************/
/*   Instantiate Module                                                        */
/******************************************************************************/
Service * NewVoltMeter(AdOps * ChipsADC)
{
   Element = getRegisterSet();
   SystemStat = GetSystemStat();
   Adc = ChipsADC;
   
   Adc->Start(); // Start the ADC Peripheral
   Adc->StartReading(); //
   ADC_SampleState = CHK_READING;
   Calibrated = False; //True;
   if ((Element[COUNTS_AT_MAX].Value != 0) &&
           (Element[COUNTS_AT_MIN].Value != 0) &&
           (Element[MaxVolts].Value != 0) &&
           (Element[MinVolts].Value != 0))
   {
      Calibrated = True;
      //Element[STATUS_WORD].Calibrated = True;
      //SystemStat->Calibrated = True;
      Element[STATUS_WORD].Value = Element[STATUS_WORD].Status;
   }

   if (Calibrated)
   {
      float DeltaVolts = Element[MaxVolts].Value - Element[MinVolts].Value;
      float DeltaCount = Element[COUNTS_AT_MAX].Value - Element[COUNTS_AT_MIN].Value;
      if(DeltaCount > 0)
          Slope = DeltaVolts / DeltaCount;
      Offset = Element[Offset_d].Value + (Element[Offset_N].Value * 65536.0);
   }
   This.Driver = ChipsADC;
   return &This;
}

/******************************************************************************/
/*  Set Calibration Data                                                      */
/******************************************************************************/
void WriteCalData(byte WhichValue, unsigned Value)
{
   switch (WhichValue)
   {
      case COUNTS_MAX:
         Element[COUNTS_AT_MAX].Value = Value;
         Element[COUNTS_AT_MAX].Saved = False;
         break;
      case COUNTS_MIN:
         Element[COUNTS_AT_MIN].Value = Value;
         Element[COUNTS_AT_MIN].Saved = False;
         break;
      case VOLTAGE_MAX:
         Element[MaxVolts].Value = Value;
         Element[MaxVolts].Saved = False;
         break;
      case VOLTAGE_MIN:
         Element[MinVolts].Value = Value;
         Element[MinVolts].Saved = False;
         break;
   }
}
/******************************************************************************/
/*  Update ADC Reading                                                        */
/******************************************************************************/
void HandleSamples(void)
{
   static int TimesChecked = 0;

   switch (ADC_SampleState)
   {
      case CHK_READING:
         if (Adc->ReadingReady)
            ADC_SampleState = SORT_SAMPLES;
         break;
      case SORT_SAMPLES:
         Adc->Sort();
         ADC_SampleState = NEXT_READING;
         break;
      case NEXT_READING:
         Adc->Next();
         aNewSample = Element[ANALOG1].Value;
         ADC_SampleState = CHK_HI_LO;
         break;
      case CHK_HI_LO:
         TimesChecked++;
         ADC_SampleState = PROCESS_SAMPLES;
         break;
      case PROCESS_SAMPLES:
         NewSample();
         ADC_SampleState = TAKE_READING;
         break;
      case TAKE_READING:
            ADC_SampleState = CHK_READING;
            //Element[0].AdReady = False;
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
float ReadVoltage(void)
{
   float Volts = 0.0;
   if (Calibrated) //Element[STATUS_WORD].Calibrated)
   {
      Volts = (Slope * AverageOfSamples * 65536) + Offset; // y = Mx + b
      Volts /= 65536;
   }
   return Volts;
}

/******************************************************************************/
/*                                                                            */
/******************************************************************************/
void ResetAverage(void)
{
   memset((char*) &Samples, 0, sizeof (Samples));
   aNewSample = AverageOfSamples = 0;
   Sum = 0;
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

   Element[Average_Counts].Value = AverageOfSamples;
   Element[Average_Counts].Changed = True;
   AverageCounts = AverageOfSamples;

   // Record the new sample
   Samples.Buffer[Samples.Tail++] = aNewSample;
   Samples.Tail &= IO_RAPAROUND;
}
