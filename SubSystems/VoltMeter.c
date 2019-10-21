#include "defs.h"
#include "Register.h"
#include "Registers.h"
//#include "SubSystem.h"
#include "VoltMeter.h"
#include "Adc.h"

#define VOLT_METER_SUB_SYSTEM  "Volt Meter Sub-System"

void CheckHiLo(unsigned Sample);
float UpdateVoltage(unsigned Average);
void SelfCalibrate(void);
void WriteCalData(byte WhichValue, unsigned Value);
static void ClearCfg(void);
static void HandleReadings(void);
static void _Process(void);

static inline Boolean CheckSelfCalibrated(void);
static inline Boolean CheckNvCalData(void);

static inline void HiLoReset(void);
static inline void ClearSelfCalData(void);
static inline void CheckStatus(void);
static inline void UpdateVoltate(void);
static inline void UpdateADReading(void);
static inline void SetToMinValue(void);
static inline void SetToMaxValue(void);
static inline void SetToMidValue(void);
static inline void UpdateMinValues(void);
static inline void UpdateMaxValues(void);
static inline void UpdateMidValues(void);
static inline void RestoreOriginalSettings(void);

//////////////////////////////////////////////////////////////////////////////
// Type declarations
//
typedef enum
{
   CHECK_STATUS, UPDATE_RAW, UPDATE_READING, SET_TO_MIN, UPDATE_MIN, SET_TO_MAX, UPDATE_MAX, SET_TO_MID, UPDATE_MID, DONE,
} VOLTMETER_STATES;


static Register * Element; // A register element pointer
static Register * Config;
static Device * PWM;
static Device * ADControl;
static unsigned long DeltaVolts;
static unsigned int DeltaCount;
//static unsigned int Temp;
static unsigned long Slope;
static unsigned long Offset;
static unsigned OriginalPWM = 0;

static IndicatorBits * SystemStatBits = 0;
static SubSystemBits * SubSystem_Bits = 0;
static RequestBits   * Request_Bits = 0;

static VOLTMETER_STATES VoltMeterState = DONE;

// Device operations used..
static SubSystem This =
{
   .Used = True,
   .SubSystemType = VOLT_METER,
   .Discription = VOLT_METER_SUB_SYSTEM,
   .Id = (0x0100 + (int) VOLT_METER),
   .WaitingOnMask = 0,
   .Arg = 0,
   .ProcessFn = _Process,
   .HandleSubSystem = HandleReadings,
   //.Flush = ResetDiagnostics,
   .Reset = ClearCfg,
};

/****************************************************************************/
/*                                                                          */
/*     Function: NewRom                                                     */
/*        Input:                                                            */
/*       Return:                                                            */
/*     Overview:                                                            */
/*                                                                          */
/****************************************************************************/
SubSystem * InitVoltMeter(Register * UI, AdOps * ChipsADC, Device * PwerControl)
{
   Element = UI;
   SystemStatBits = GetSystemStat();
   SubSystem_Bits = GetSubSysBits();
   Config = GetRequestBits();
   Request_Bits = (RequestBits *)&Config->Value;

   ADControl = ADConversionControl(UI, ChipsADC);
   PWM = PwerControl;
   VoltMeterState = CHECK_STATUS;
   SystemStatBits->SlfCalibrating = False;
   
   if(CheckNvCalData())
   {
      SubSystem_Bits->NvCalValuesSet = True;
      CheckStatus();
   }
   
   // Update A/D Readings
   ADControl->UpDate();

   // Update PWM Value
   PWM->UpDate();
   
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
static void _Process(void)
{
   while (LOOPING_FOREVER)
   {
      switch (VoltMeterState)
      {
         case CHECK_STATUS:
            CheckStatus();
            break;
         case UPDATE_RAW:
            UpdateADReading();
            
            if(Request_Bits->CalRequest)
               VoltMeterState = SET_TO_MIN;
            else
               SystemStatBits->SlfCalibrating = False;               
            break;
         case UPDATE_READING:
            UpdateVoltate();
            if(Request_Bits->CalRequest)
               VoltMeterState = SET_TO_MIN;
            else
               SystemStatBits->SlfCalibrating = False;               
            break;
         case SET_TO_MIN:
            // Indicate that we are still Self Calibrating
            Request_Bits->CalRequest = 0;
            Config->Changed = 1;
            SystemStatBits->SlfCalibrating = True;
            SubSystem_Bits->SelfCalibrated = False;
            Element[STATUS_WORD].Changed = True;
            // Clear Self Calibration Data 
            ClearSelfCalData();
            // Save Current Power Settings Value
            OriginalPWM = Element[PWM_VALUE].Value;
            // Set new PWM Value to Voltage @ 1 Volt
            SetToMinValue();
            // Reset values to generate new reading
            // Load 32-bit period value (msw), Load 32-bit period value (lsw)
            DelayTimer(0x07, 0xFFFF);
            VoltMeterState = UPDATE_MIN; //LWR_S_TM;
            break;
         case UPDATE_MIN:
            if (SystemStatBits->FTDone)
            {
               UpdateMinValues();
               VoltMeterState = SET_TO_MAX;
               SystemStatBits->FTDone = 0;
            }
            break;
         case SET_TO_MAX:
            // Set to Max Values
            SetToMaxValue();
            // Load 32-bit period value (msw), Load 32-bit period value (lsw)
            DelayTimer(0x01, 0xFFFF);
            VoltMeterState = UPDATE_MAX; //UPR_S_TM;
            break;
         case UPDATE_MAX:
            if (SystemStatBits->FTDone)
            {
               UpdateMaxValues();
               SystemStatBits->FTDone = 0;
               VoltMeterState = SET_TO_MID;
            }
            break;
         case SET_TO_MID:
            // Set new PWM Value
            SetToMidValue();
            // Reset values to generate new reading
            // Load 32-bit period value (msw), Load 32-bit period value (lsw)
            DelayTimer(0x03, 0xFFFF);
            VoltMeterState = UPDATE_MID;
            break;
         case UPDATE_MID:
            UpdateMidValues();
            if (SystemStatBits->FTDone)
            {
               SetToMidValue();

               VoltMeterState = DONE;
               SystemStatBits->FTDone = 0;
            }
            break;
         case DONE:
            SystemStatBits->SlfCalibrating = False;
            VoltMeterState = CHECK_STATUS;
            break;
      }
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
static void HandleReadings(void)
{
   switch (VoltMeterState)
   {
      case CHECK_STATUS:
         CheckStatus();
         break;
      case UPDATE_RAW:
         // Update A/D Readings
         ADControl->UpDate();

         // Update PWM Value
         PWM->UpDate();

         break;
      case UPDATE_READING:
         // Update A/D Readings
         ADControl->UpDate();

         // Update PWM Value
         PWM->UpDate();
         
         // If requested to 'self calibrate' change to first state
         // of the self calibration DFA
         if(Request_Bits->CalRequest)
         {
            VoltMeterState = SET_TO_MIN;
         }
         break;
      case SET_TO_MIN:
         // Indicate that we are still Self Calibrating
         SystemStatBits->SlfCalibrating = True;
         SubSystem_Bits->SelfCalibrated = False;
         Element[STATUS_WORD].Changed = True;
         // Clear Self Calibration Data 
         ClearSelfCalData();
         // Save Current Power Settings Value
         OriginalPWM = Element[PWM_VALUE].Value;
         // Set new PWM Value to Voltage @ 1 Volt
         SetToMinValue();
         // Reset values to generate new reading
         // Load 32-bit period value (msw), Load 32-bit period value (lsw)
         DelayTimer(0x07, 0xFFFF);
         VoltMeterState = UPDATE_MIN; //LWR_S_TM;
         break;
      case UPDATE_MIN:
         if (SystemStatBits->FTDone)
         {
            UpdateMinValues();
            VoltMeterState = SET_TO_MAX;
            SystemStatBits->FTDone = 0;
         }
         break;
      case SET_TO_MAX:
         // Set to Max Values
         SetToMaxValue();
         // Load 32-bit period value (msw), Load 32-bit period value (lsw)
         DelayTimer(0x01, 0xFFFF);
         VoltMeterState = UPDATE_MAX; //UPR_S_TM;
         break;
      case UPDATE_MAX:
         if (SystemStatBits->FTDone)
         {
            UpdateMaxValues();
            SystemStatBits->FTDone = 0;
            VoltMeterState = SET_TO_MID;
         }
         break;
      case SET_TO_MID:
         // Set new PWM Value
         SetToMidValue();
         // Reset values to generate new reading
         // Load 32-bit period value (msw), Load 32-bit period value (lsw)
         DelayTimer(0x03, 0xFFFF);
         VoltMeterState = UPDATE_MID;
         break;
      case UPDATE_MID:
         UpdateMidValues();
         if (SystemStatBits->FTDone)
         {
            SetToMidValue();

            VoltMeterState = DONE;
            SystemStatBits->FTDone = 0;
         }
         break;
      case DONE:
         VoltMeterState = CHECK_STATUS;
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
static inline Boolean CheckSelfCalibrated(void)
{
   Boolean Results = False;

   if ((Element[COUNTS_AT_MAX].Value != 0) &&
           (Element[COUNTS_AT_MIN].Value != 0) &&
           (Element[MaxVolts].Value != 0) &&
           (Element[MinVolts].Value != 0))
   {
      Results = True;
   }
   return Results;
}

/****************************************************************************/
/*                                                                          */
/*     Function: NewRom                                                     */
/*        Input:                                                            */
/*       Return:                                                            */
/*     Overview:                                                            */
/*                                                                          */
/****************************************************************************/
static inline Boolean CheckNvCalData(void)
{
   int i;
   Boolean Results = True;

   for (i = V1; ((Results == True) && (i <= V12)); i++)
   {
      if (Element[i].Value == 0)
         Results = False;
   }
   return Results;
}

/****************************************************************************/
/*                                                                          */
/*     Function: NewRom                                                     */
/*        Input:                                                            */
/*       Return:                                                            */
/*     Overview:                                                            */
/*                                                                          */
/****************************************************************************/
static inline void HiLoReset(void)
{
   Element[Lowest].Value = 0;
   Element[Lowest].Changed = True;
   Element[Highest].Value = 0; //AverageOfSamples;
   Element[Highest].Changed = True;
}

/****************************************************************************/
/*                                                                          */
/*     Function: NewRom                                                     */
/*        Input:                                                            */
/*       Return:                                                            */
/*     Overview:                                                            */
/*                                                                          */
/****************************************************************************/
static inline void ClearSelfCalData(void)
{
   int Address;
   // Clear Self Calibration Data 
   for (Address = COUNTS_AT_MAX; Address < V1; Address++)
   {
      Element[Address].Value = 0;
      Element[Address].Changed = True;
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
static inline void SetToMinValue(void)
{
   // Clear Buffers and average
   ADControl->Reset();
   // Set to Minimum values
   PWM->Write(CHANGE_DUTY_CYCLE, Element[MIN_SETTING].Value);
   Element[PWM_VALUE].Value = Element[MIN_SETTING].Value;
   Element[PWM_VALUE].Changed = True;
}

/****************************************************************************/
/*                                                                          */
/*     Function: NewRom                                                     */
/*        Input:                                                            */
/*       Return:                                                            */
/*     Overview:                                                            */
/*                                                                          */
/****************************************************************************/
static inline void SetToMaxValue(void)
{
   // Clear Buffers and average
   ADControl->Reset();
   PWM->Write(CHANGE_DUTY_CYCLE, Element[MAX_SETTING].Value);
   Element[PWM_VALUE].Value = Element[MAX_SETTING].Value;
   Element[PWM_VALUE].Changed = True;
}

/****************************************************************************/
/*                                                                          */
/*     Function: NewRom                                                     */
/*        Input:                                                            */
/*       Return:                                                            */
/*     Overview:                                                            */
/*                                                                          */
/****************************************************************************/
static inline void SetToMidValue(void)
{
   // Clear Buffers and average
   ADControl->Reset();
   // Set new PWM Value
   PWM->Write(CHANGE_DUTY_CYCLE, Element[MID_SETTING].Value);
   Element[PWM_VALUE].Value = Element[MID_SETTING].Value;
   Element[PWM_VALUE].Changed = True;
}

/****************************************************************************/
/*                                                                          */
/*     Function: NewRom                                                     */
/*        Input:                                                            */
/*       Return:                                                            */
/*     Overview:                                                            */
/*                                                                          */
/****************************************************************************/
static inline void UpdateMinValues(void)
{
   Element[COUNTS_AT_MIN].Value = ADControl->Peek();
   Element[COUNTS_AT_MIN].Changed = True;
   Element[MinVolts].Value = 1;
   Element[MinVolts].Changed = True;
}

/****************************************************************************/
/*                                                                          */
/*     Function: NewRom                                                     */
/*        Input:                                                            */
/*       Return:                                                            */
/*     Overview:                                                            */
/*                                                                          */
/****************************************************************************/
static inline void UpdateMaxValues(void)
{
   // Update AD readings when PWM set to pre-calculated
   // max value
   Element[COUNTS_AT_MAX].Value = ADControl->Peek();
   Element[COUNTS_AT_MAX].Changed = True;
   Element[MaxVolts].Value = 12;
   Element[MaxVolts].Changed = True;
   DeltaVolts = Element[MaxVolts].Value - Element[MinVolts].Value;
   DeltaVolts <<= 16;
   DeltaCount = Element[COUNTS_AT_MAX].Value - Element[COUNTS_AT_MIN].Value;
   Slope = DeltaVolts / DeltaCount;
}

/****************************************************************************/
/*                                                                          */
/*     Function: NewRom                                                     */
/*        Input:                                                            */
/*       Return:                                                            */
/*     Overview:                                                            */
/*                                                                          */
/****************************************************************************/
static inline void UpdateMidValues(void)
{
}

/****************************************************************************/
/*                                                                          */
/*     Function: NewRom                                                     */
/*        Input:                                                            */
/*       Return:                                                            */
/*     Overview:                                                            */
/*                                                                          */
/****************************************************************************/
static inline void RestoreOriginalSettings(void)
{
   // Restor Old PWM Value
   PWM->Write(CHANGE_DUTY_CYCLE, OriginalPWM);
   Element[PWM_VALUE].Value = OriginalPWM;
   Element[PWM_VALUE].Changed = True;
   SystemStatBits->SlfCalibrating = False;
   SubSystem_Bits->SelfCalibrated = True;
   Element[STATUS_WORD].Changed = True;
}

/****************************************************************************/
/*                                                                          */
/*     Function: NewRom                                                     */
/*        Input:                                                            */
/*       Return:                                                            */
/*     Overview:                                                            */
/*                                                                          */
/****************************************************************************/
static inline void CheckStatus(void)
{
   Boolean Checked;
   
   VoltMeterState = UPDATE_RAW; // Default state
   if (SubSystem_Bits->NvCalValuesSet)
   {
      Checked = CheckSelfCalibrated();
      if (SubSystem_Bits->SelfCalibrated != Checked)
      {
         SubSystem_Bits->SelfCalibrated = Checked;
         SystemStatBits->CfgDirty = True;
      }

      if (SubSystem_Bits->SelfCalibrated)
      {
         DeltaVolts = Element[MaxVolts].Value - Element[MinVolts].Value;
         DeltaVolts <<= 16;
         DeltaCount = Element[COUNTS_AT_MAX].Value - Element[COUNTS_AT_MIN].Value;
         Slope = DeltaVolts / DeltaCount;
         //Offset = Element[Offset_d].Value + (Element[Offset_N].Value * 65536.0);
         VoltMeterState = UPDATE_READING;
      }
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
static inline void UpdateVoltate(void)
{
   float Temp;
   float Volts;
   unsigned long Volts_n;
   unsigned Denom;
   //int Address;
   unsigned Counts;
   
   // If Calibrated then Take New Reading
   if (((SubSystem_Bits->SelfCalibrated) && (ADControl->IsReady())))
   {
      Counts = ADControl->Peek();
      Element[Average_Counts].Value = Counts;
      Element[Average_Counts].Changed = True;

      Volts = (Slope * Counts * 65536) + Offset; // + b; // y = Mx + b
      Volts_n = (unsigned long) Volts;
      Volts_n >>= 16;
      Temp = Volts / 65536.0;
      Denom = (unsigned) (65536.0 * (Temp - Volts_n));
      Element[Measured_N].Value = Volts_n;
      Element[Measured_N].Changed = True;
      Element[Measured_N].Value = Denom;
      Element[Measured_N].Changed = True;
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
static inline void UpdateADReading(void)
{
   // Update A/D Readings
   ADControl->UpDate();

   // Update PWM Value
   PWM->UpDate();

}

/****************************************************************************/
/*                                                                          */
/*     Function: NewRom                                                     */
/*        Input:                                                            */
/*       Return:                                                            */
/*     Overview:                                                            */
/*                                                                          */
/****************************************************************************/
static void ClearCfg(void)
{
   Element[COUNTS_AT_MAX].Value = 0;
   Element[COUNTS_AT_MAX].Saved = False;
   Element[COUNTS_AT_MIN].Value = 0;
   Element[COUNTS_AT_MIN].Saved = False;
   Element[MaxVolts].Value = 0;
   Element[MaxVolts].Saved = False;
   Element[MinVolts].Value = 0;
   Element[MinVolts].Saved = False;

   Element[Highest].Value = 0;
   Element[Highest].Saved = False;
   Element[Lowest].Value = 0;
   Element[Lowest].Saved = False;
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

