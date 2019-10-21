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
#include <math.h>
#include <float.h>
#include <string.h>
#include "defs.h"
#include "Registers.h"
#include "Register.h"
#include "ADConverter.h"
#include "Device.h"

//////////////////////////////////////////////////////////////////////////////
// Constants Area
//
#define DELTA 0x16
#define MAX_WAIT_STATES    7
#define MAX_UPDATE_COUNTS 15

typedef enum
{
   CHK_READING,
   NEXT_READING,
   PROCESS_SAMPLES,
   TAKE_READING,
}ADC_SAMPLING_STATES;

//////////////////////////////////////////////////////////////////////////////
// Private Methods
//
static void HandleSamples(void);
static unsigned ReadAverage(void);
static void ResetAverage(void);
static void ProcessReading(void);
static Boolean ReadingReady(void);

#define DISCRIPTION_ANLG_METER "Analog Volt Meter"
static Service This =
{
   .Used = True,
   .DeviceType = AD_CONVERTER,
   .Discription = DISCRIPTION_ANLG_METER,
   .AuxUse = 0,
   .Id = (0x0100 + (int)AD_CONVERTER),
   .Arg = 0,
   .FnType = UpDateType,
   .UpDate = HandleSamples,
   .Peek = ReadAverage,
   .Write = 0, //WriteCalData,
   .Reset = ResetAverage,
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
static unsigned New_Sample = 0;
static volatile float Volts; // = Slope * AverageOfSamples + Offset;
static Register * Element; // A register element pointer
static IndicatorBits * SystemStat;
static Boolean ReadingIsReady = False;

/******************************************************************************/
/*   Istantiate Module                                                        */
/******************************************************************************/
Service * ADConversionControl(Register * UI, AdOps * ChipsADC)
{
   Element = UI;
   Adc = ChipsADC;

   Adc->Start(); // Start the ADC Peripheral
   Adc->StartReading(); //
   This.Driver = ChipsADC;
   return &This;
}

/****************************************************************************/
/*                                                                          */
/*     Function: NewRom                                                     */
/*        Input:                                                            */
/*       Return:                                                            */
/*     Overview:                                                            */
/*                                                                          */
/****************************************************************************/
static unsigned ReadAverage(void)
{
   ReadingIsReady = False;
   return AverageOfSamples;
}

/****************************************************************************/
/*                                                                          */
/*     Function: NewRom                                                     */
/*        Input:                                                            */
/*       Return:                                                            */
/*     Overview:                                                            */
/*                                                                          */
/****************************************************************************/
static Boolean ReadingReady(void)
{
   return ReadingIsReady;
}

/****************************************************************************/
/*                                                                          */
/*     Function: NewRom                                                     */
/*        Input:                                                            */
/*       Return:                                                            */
/*     Overview:                                                            */
/*                                                                          */
/****************************************************************************/
static void HandleSamples(void)
{
   switch (ADC_SampleState)
   {
      case CHK_READING:
         if (Adc->ReadingReady)
         {
            Adc->Sort();
            ADC_SampleState = NEXT_READING;
         }
      case NEXT_READING:
         Adc->Next();
         ADC_SampleState = PROCESS_SAMPLES;
         break;
      case PROCESS_SAMPLES:
         ProcessReading();
         ADC_SampleState = TAKE_READING;
         break;
      case TAKE_READING:
         ADC_SampleState = CHK_READING;
         SystemStat->AdRdy = False;
         break;
   }
}

/****************************************************************************/
/*                                                                          */
/*     Function: NewRom                                                     */
/*        Input:                                                            */
/*       Return:                                                            */
/*     Overview:                                                            */
/*                                                                          */
/****************************************************************************/
static void ResetAverage(void)
{
   memset((char*) &Samples, 0, sizeof (Samples));
   New_Sample = AverageOfSamples = 0;
   Sum = 0;
}

/******************************************************************************/
/*                                                                            */
/******************************************************************************/
static void ProcessReading(void)
{
   New_Sample = Adc->Reading();
   // Calculate new Sum and Average
   if (Samples.Count < IO_BUFFER_SIZE)
   {
      Samples.Count++;
      Sum += New_Sample;
      AverageOfSamples = (unsigned) (Sum / Samples.Count);
   }
   else
   {
      Sum -= Samples.Buffer[Samples.Head++];
      Samples.Head &= IO_RAPAROUND;
      Sum += New_Sample;
      AverageOfSamples = (unsigned) (Sum >> (IO_POWER_OF_2)); // Divide by 32 the # of samples.
   }

   ReadingIsReady = True;
   
   // Record the new sample
   Samples.Buffer[Samples.Tail++] = New_Sample;
   Samples.Tail &= IO_RAPAROUND;
}
