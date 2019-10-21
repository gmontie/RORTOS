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
#include "System.h"
#include "Registers.h"
#include "Meter.h"
#include "NVRam.h"

//////////////////////////////////////////////////////////////////////////////
// Constants Area
//
#define DELTA 0x16

//////////////////////////////////////////////////////////////////////////////
// Type declarations
//
typedef enum
{
   SET_LOWER, CHK_LOWER, SET_UPPER, CHK_UPPER, SET_MID, CHK_MID
} METER_STATES;

typedef enum
{
   CHK_READING, SORT_SAMPLES, NEXT_READING, CHK_HI_LO, PROCESS_SAMPLES, TAKE_READING
} ADC_SAMPLING_STATES;

//////////////////////////////////////////////////////////////////////////////
// Private Methods
//
static void HandleSamples(void);
static void ReadMeter(void);
static void SelfCalibrate(void);
static float ReadVoltage(void);

static void ResetAverage(void);
static void NewSample(void);
static void CheckHiLo(unsigned);
static void HiLoReset(void);

//////////////////////////////////////////////////////////////////////////////
// Private members
//
static METER_STATES MeterState;
static ADC_SAMPLING_STATES ADC_SampleState;
static Queue Samples;
static AdOps * Adc;
static NVManager * NVMemory; // Flash memory of some kind... I2C or SPI.
static unsigned OriginalPWM;
static long Sum;
static unsigned AverageOfSamples = 0;
static unsigned aNewSample = 0;
static int Address; // Register address
static Register * Element; // A register element pointer
static float Slope;
static float Offset;
static volatile float Volts; // = Slope * AverageOfSamples + Offset;
static Boolean Calibrated = False;

static VMeter This =
{
   .ProcessSamples = HandleSamples,
   .ReadMeter = ReadMeter,
   .AutoCalibrate = SelfCalibrate,
   .ReadVoltage = ReadVoltage
};

/******************************************************************************/
/*   Istantiate Module                                                        */
/******************************************************************************/
VMeter * InitMeter(Register * UI, AdOps * ChipsADC, NVManager * MemManager)
{
   Element = UI;
   MeterState = SET_LOWER;
   Adc = ChipsADC;
   NVMemory = MemManager;
   Adc->StartADC(); // Start the ADC Peripheral
   Adc->StartReading(); //
   ADC_SampleState = CHK_READING;

   if ((UI[COUNTS_AT_MAX].Value != 0) &&
           (UI[COUNTS_AT_MIN].Value != 0) &&
           (UI[MaxVolts].Value != 0) &&
           (UI[MinVolts].Value != 0))
   {
      Calibrated = True;
      Element[STATUS_WORD].Calibrated = True;
      Element[STATUS_WORD].Value = Element[STATUS_WORD].Status;
   }

   if (Calibrated)
   {
      float DeltaVolts = UI[MaxVolts].Value - UI[MinVolts].Value;
      float DeltaCount = UI[COUNTS_AT_MAX].Value - UI[COUNTS_AT_MIN].Value;
      Slope = DeltaVolts / DeltaCount;
      Offset = UI[Offset_d].Value + (UI[Offset_N].Value * 65536.0);
   }
   return &This;
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
         Adc->NextADCReading();
         aNewSample = Element[ANALOG1].Value;
         ADC_SampleState = CHK_HI_LO;
         break;
      case CHK_HI_LO:
         TimesChecked++;
         CheckHiLo(aNewSample);
         if (TimesChecked > 170)
         {
            HiLoReset();
            TimesChecked = 0;
         }
         ADC_SampleState = PROCESS_SAMPLES;
         break;
      case PROCESS_SAMPLES:
         NewSample();
         ADC_SampleState = TAKE_READING;
         break;
      case TAKE_READING:
         ReadMeter();
         //Element[STATUS_WORD].FTDone = True;
         //Element[STATUS_WORD].Calibrated = True;
         ADC_SampleState = CHK_READING;
         break;
   }
}

/******************************************************************************/
/* Self Calibrate State Machine                                               */
/******************************************************************************/
void SelfCalibrate(void)
{
   switch (MeterState)
   {
      case SET_LOWER:
         // Clear Calibration Register Values
         for (Address = COUNTS_AT_MAX; Address < V1; Address++)
         {
            Element[Address].Value = 0;
            Element[Address].Changed = True;
         }
         // Set new PWM Value
         OriginalPWM = Element[PWM_VALUE].Value;
         Element[PWM_VALUE].Value = Element[V1].Value;
         Element[PWM_VALUE].Changed = True;
         // Indicate that we are still Self Calibrating
         Element[STATUS_WORD].Calibrating = True;
         Element[STATUS_WORD].Calibrated = False;
         Element[STATUS_WORD].Value = Element[STATUS_WORD].Status;
         Element[STATUS_WORD].Changed = True;
         // Reset values to generate new reading
         // Load 32-bit period value (msw), Load 32-bit period value (lsw)
         DelayTimer(0x07, 0xFFFF);
         MeterState = CHK_LOWER; //LWR_S_TM;
         break;
      case CHK_LOWER:
         if (Element[STATUS_WORD].FTDone)
         {
            Element[STATUS_WORD].FTDone = 0;
            Element[COUNTS_AT_MIN].Value = Element[Average_Counts].Value;
            //NVMemory->StoreRegister(COUNTS_AT_MIN, &Element[COUNTS_AT_MIN]);
            Element[COUNTS_AT_MIN].Changed = True;
            Element[MinVolts].Value = 1;
            //NVMemory->StoreRegister(MinVolts, &Element[MinVolts]);
            Element[MinVolts].Changed = True;
            MeterState = SET_UPPER;
         }
         break;
      case SET_UPPER:
         // Set new PWM Value
         HiLoReset();
         ResetAverage();
         Element[PWM_VALUE].Value = Element[V10].Value;
         Element[STATUS_WORD].Value = Element[STATUS_WORD].Status;
         Element[STATUS_WORD].Changed = True;
         // Reset values to generate new reading
         // Load 32-bit period value (msw), Load 32-bit period value (lsw)
         DelayTimer(0x01, 0xFFFF);
         MeterState = CHK_UPPER; //UPR_S_TM;
         break;
      case CHK_UPPER:
         if (Element[STATUS_WORD].FTDone)
         {
            Element[COUNTS_AT_MAX].Value = Element[COUNTS_AT_MAX].Value;
            //NVMemory->StoreRegister(COUNTS_AT_MAX, &Element[COUNTS_AT_MAX]);
            Element[COUNTS_AT_MAX].Changed = True;
            Element[MaxVolts].Value = 10; //Element[V10].Value;
            //NVMemory->StoreRegister(MaxVolts, &Element[MaxVolts]);
            Element[MaxVolts].Changed = True;
            double DeltaVolts = 9.0; //10 - 1; //HiVolts - LoVolts;
            double DeltaCount = Element[COUNTS_AT_MAX].Value - Element[COUNTS_AT_MIN].Value;
            Slope = DeltaVolts / DeltaCount;
            MeterState = SET_MID;
         }
         break;
      case SET_MID:
         // Set new PWM Value
         Element[PWM_VALUE].Value = Element[V5].Value;
         Element[PWM_VALUE].Changed = True;
         // Reset values to generate new reading
         // Load 32-bit period value (msw), Load 32-bit period value (lsw)
         DelayTimer(0x03, 0xFFFF);
         MeterState = CHK_MID;
         break;
      case CHK_MID:
         if (Element[STATUS_WORD].FTDone)
         {
            double Volts = Slope * AverageOfSamples;
            Offset = 5.0 - Volts;
            int Offset_n = (int) Offset;
            // Save Offset Numberator
            Element[Offset_N].Value = (unsigned)Offset;
            //NVMemory->StoreRegister(Offset_Nr, &Element[Offset_Nr]);
            Element[Offset_N].Changed = True;
            // Save Offset Denominator
            Element[Offset_d].Value = ((Offset - Offset_n) * 65536) + 1;
            //NVMemory->StoreRegister(Offset_dom, &Element[Offset_dom]);
            Offset = Element[Offset_d].Value + (Element[Offset_N].Value * 65536.0);
            Element[Offset_d].Changed = True;
            // Restor Old PWM Value
            Element[PWM_VALUE].Value = OriginalPWM;
            Element[PWM_VALUE].Changed = True;
            Element[STATUS_WORD].Calibrating = False;
            Element[STATUS_WORD].Calibrated = True;
            Element[STATUS_WORD].Value = Element[STATUS_WORD].Status;
            Element[STATUS_WORD].Changed = True;
            MeterState = SET_LOWER;
            Calibrated = True;
         }
         break;
   }
}

/******************************************************************************/
/*                                                                            */
/******************************************************************************/
void ReadMeter(void)
{
   if (Calibrated) //Element[STATUS_WORD].Calibrated)
   {
         //float X = AverageOfSamples;
         float Temp;
         unsigned long Volts_n;
         unsigned Denom;

         Volts = (Slope * AverageOfSamples * 65536) + Offset; // + b; // y = Mx + b
         Volts_n = (unsigned long)Volts;
         Volts_n >>= 16;
         Temp = Volts / 65536.0;
         Denom = (unsigned) (65536.0 * (Temp - Volts_n));
         Element[Measured_N].Value = Volts_n;
         Element[Measured_N].Changed = True;
         Element[Measured_N].Value = Denom;
         Element[Measured_N].Changed = True;
   }
}

/******************************************************************************/
/*                                                                            */
/******************************************************************************/
float ReadVoltage(void)
{
   float Volts = 0.0;
   if (Calibrated) //Element[STATUS_WORD].Calibrated)
   {
      Volts = ((Slope * AverageOfSamples) * 65536) + Offset; // y = Mx + b
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
      AverageOfSamples = (unsigned) (Sum >> IO_POWER_OF_2); // Divide by 32 the # of samples.
   }

   Element[Average_Counts].Value = AverageOfSamples;
   Element[Average_Counts].Changed = True;

   // Record the new sample
   Samples.Buffer[Samples.Tail++] = aNewSample;
   Samples.Tail &= IO_RAPAROUND;
}


/******************************************************************************/
/*                                                                            */
/******************************************************************************/
void CheckHiLo(unsigned Sample)
{
   //if (Sample > Element[Lowest].Value)
   if (Sample > Element[Highest].Value)
      Element[Highest].Value = Sample;

   //if (Sample < Element[Highest].Value)
   if (Sample < Element[Lowest].Value)
      Element[Lowest].Value = Sample;

   if (Element[Lowest].Value == 0)
   {
      if (Sample == 0)
         Element[Lowest].Value = Element[Highest].Value - 1;
      else
         Element[Lowest].Value = Sample;
   }
}

/******************************************************************************/
/*                                                                            */
/******************************************************************************/
void HiLoReset(void)
{
   Element[Lowest].Value = 0;
   Element[Lowest].Changed = True;
   Element[Highest].Value = AverageOfSamples;
   Element[Highest].Changed = True;
   AverageOfSamples = 0;
}

