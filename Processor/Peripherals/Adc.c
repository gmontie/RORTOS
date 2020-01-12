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
#include <string.h>
#include <p33FJ128GP802.h>
#include "System.h"
#include "defs.h"
#include "Adc.h"


// Functions for use by the ADC Object
static unsigned ReadADC(void); // Take an ADC Reading (Not implemented here)
 void BubbleSort(void); // Sort the results so that NextADC can do it's job
 void ShellSort(void); // Shell short
static void DigitalFilter(void); // Calculate the next ADC reading
static void StartAD1(void); // Start the ADC Peripheral
static void StopAD1(void);
static void StartADReading(void); // Start the DMA for the ADC
static void StopADReading(void);
static void initDma0(void); // Initialize the DMA for the ADC
static void ClrBuffers(void);
static void SetupADISR(void);

// Private data which the ADC uses!
 int BufferA[NUMBER_OF_SAMPLES] __attribute__((space(dma)));
 int BufferB[NUMBER_OF_SAMPLES] __attribute__((space(dma)));
 int Transfr[NUMBER_OF_SAMPLES];
static volatile long Sum;
static int Index;
static unsigned x;
static unsigned n;
static int y;
volatile int Temp;
 Boolean InUse;
 unsigned int DmaBuffer = 0;
static int * BuffPtr;
static int * Walk;
static IndicatorBits * SystemStat;
static Register * Reg;

// Statically set up the ADC Object
static AdOps This =
{
   .Reading = ReadADC,
   .Start = StartAD1,
   .Stop = StopAD1,
   .StartReading = StartADReading,
   .StopReading = StopADReading,
   .Next = DigitalFilter,
   //.Sort = BubbleSort,
   .Sort = ShellSort,
   .Clear = ClrBuffers,
   .ReadingReady = False
};

/****************************************************************************/
/*                                                                          */
/*     Function: ADC Init                                                   */
/*        Input: RegisterFile Pointer to where to put the results           */
/*       Return: None                                                       */
/* Side Effects:                                                            */
/*     Overview: This function sets up the ADC peripheral for the           */
/*               dsPIC33F series.                                           */
/*                                                                          */
/****************************************************************************/
AdOps * ADCInit(unsigned Index)
{  // AD1CON1 control register
   AD1CON1bits.ADON = 0;     // Configure only, Dont turn on ADC
   AD1CON1bits.ADSIDL = 0;   // Continue module operation in Idle mode
   AD1CON1bits.ADDMABM = 1;  // DMA buffers are built in conversion order mode
   AD1CON1bits.AD12B = 1;    // 12 bit mode
   AD1CON1bits.FORM = 0;     // 0b01; // Data Output Format: Signed integer
   AD1CON1bits.SSRC = 0b111; // Internal counter ends sampling starts conversion (auto-convert)
   AD1CON1bits.SIMSAM = 1;   // Sample multiple channels individually in sequence. (default))
   AD1CON1bits.ASAM = 1;     // Sampling done not begins immediately after last conversion. (default)
   AD1CON1bits.SAMP = 0;     // Default
   AD1CON1bits.DONE = 0;     // Make sure that DONE is cleared
   
   // AD1CON2 control register
   AD1CON2bits.VCFG = 0b001;  // Use Vcc and Gnd for Vr+ and Vr-
   AD1CON2bits.CSCNA = 1;     // Do not Scan inputs
   AD1CON2bits.CHPS = 1;      // Converts CH0 only
   //AD1CON2bits.BUFS = 0;    // Read only bit
   AD1CON2bits.SMPI = 1; //b111; // b0111; // Generate interrupt completion after 8th sample
   AD1CON2bits.BUFM = 1;      // Starts filling at 0x0 then 0x8 on next interrupt
   AD1CON2bits.ALTS = 1;      // Always uses channel input selects for Sample A (default)

   // AD1CON3 control register
   AD1CON3bits.ADRC = 0;    // Use System clock for ADC Timing.
   AD1CON3bits.SAMC = 0x17; //0x1F;//0x1F; 23 TAD Auto Sample Timing bits
   AD1CON3bits.ADCS = 0x2F; //0x3F;// 0x3F; // Auto-Sample time bits 22118400 / (4 * 64) is 47 TAD

   // AD1CON4 control register
   AD1CON4bits.DMABL = 0b0101; // 32 words of buffer to each analog input

   // AD1CHS0 control register
   AD1CHS0bits.CH0NB = 0; // channel 0 Negative input is Vref+ AN1 is input
   AD1CHS0bits.CH0SB = 1; // Channel 0 positive input is AN1
   AD1CHS0bits.CH0NA = 0; // channel 0 Negative input is Vref-
   AD1CHS0bits.CH0SA = 1; // Channel 0 positive input is AN1

   AD1CSSLbits.CSS1 = 1;
   
    // AD1CH123 Control Register
   AD1CHS123bits.CH123SA = 1;
   AD1CHS123bits.CH123NA = 0;
   AD1CHS123bits.CH123SB = 1;
   AD1CHS123bits.CH123NB = 0;
   
   Reg = getRegister(Index);
   SystemStat = getSystemStat();
   Walk = BuffPtr = BufferA;

   InUse = False;

   SetupADISR();
   initDma0();
   
   return &This;
}

/****************************************************************************/
/*                                                                          */
/*     Function: SetupADISR                                                 */
/*        Input: None                                                       */
/*       Return: None                                                       */
/*     Overview: Sets up the Interrupt Service routeen for the ADC Unit     */
/*                                                                          */
/****************************************************************************/
static void SetupADISR(void)
{
   IPC3bits.AD1IP = 2; // Next to lowest priority interrupt
   IFS0bits.AD1IF = 0; // Clear Interrupt Flags
}

/****************************************************************************/
/*                                                                          */
/*     Function: StartAD1                                                   */
/*        Input: None                                                       */
/*       Return: None                                                       */
/*     Overview: Start the ADC peripheral.                                  */
/*                                                                          */
/*                                                                          */
/****************************************************************************/
static void StartAD1(void)
{
   AD1CON1bits.ADON = 1; // Start ADC
}

/****************************************************************************/
/*                                                                          */
/*     Function: StopAD1                                                    */
/*        Input: None                                                       */
/*       Return: None                                                       */
/*     Overview: Stop the ADC peripheral.                                   */
/*                                                                          */
/*                                                                          */
/****************************************************************************/
static void StopAD1(void)
{
   AD1CON1bits.ADON = 0; // Stop ADC
}

/****************************************************************************/
/*                                                                          */
/*     Function: initDma0                                                   */
/*        Input: None                                                       */
/*       Return: None                                                       */
/*     Overview: This function setup the dsPIC33 DMA to be used with the    */
/*               ADC peripheral.                                            */
/*  DMA0 configuration:                                                     */
/*    Direction: Read from peripheral address 0-x300 (ADC1BUF0) and write   */
/*               to DMA RAM                                                 */
/*        AMODE: Register indirect with post increment                      */
/*         MODE: Continuous, Ping-Pong Mode                                 */
/*          IRQ: ADC Interrupt                                              */
/*               ADC stores results stored alternatively between BufferA[]  */
/*               and BufferB[]                                              */
/*                                                                          */
/****************************************************************************/
static void initDma0(void)
{
   DMA0CON=0;
   DMA0CONbits.AMODE = 0; // Configure DMA for Register indirect with post increment
   DMA0CONbits.MODE = 2; // Configure DMA for Continuous Ping-Pong mode

   DMA0PAD = (unsigned) &ADC1BUF0; // Store the address of the ADC1 Result buffer
   DMA0CNT = (NUMBER_OF_SAMPLES - 1);

   DMA0REQ = 13;

   DMA0STA = __builtin_dmaoffset(BufferA);
   DMA0STB = __builtin_dmaoffset(BufferB);

   IFS0bits.DMA0IF = 0; // Clear the DMA interrupt flag bit
   IEC0bits.DMA0IE = 1; // Set the DMA interrupt enable bit

   DMA0CONbits.CHEN = 0;
}

/****************************************************************************/
/*                                                                          */
/*     Function: SortResults                                                */
/*        Input: None                                                       */
/*       Return: None                                                       */
/*     Overview: This function sets up the calculation for the digital      */
/*               filter (next function down). The digital filter removes    */
/*               the upper 4th and lower 4th of the samples so that only    */
/*               the middle values are kept and used.                       */
/*                                                                          */
/****************************************************************************/
 void BubbleSort(void)
{
   InUse = True; // Set Semiphore
   Walk = BuffPtr;
   n = NUMBER_OF_SAMPLES;

   // Sort Samples
   for (x = 0; x < NUMBER_OF_SAMPLES; x++)
   {      
      for (y = 0; y < n - 1; y++)
      {
         if (Transfr[y] > Transfr[y + 1])
         {
            Temp = Transfr[y + 1];
            Transfr[y + 1] = Transfr[y];
            Transfr[y] = Temp;
         }
      }
   }
}

/****************************************************************************/
/*                                                                          */
/*     Function: ShellSort                                                  */
/*        Input: None                                                       */
/*       Return: None                                                       */
/*     Overview: This function sets up the calculation for the digital      */
/*               filter (next function down). The digital filter removes    */
/*               the upper 4th and lower 4th of the samples so that only    */
/*               the middle values are kept and used.                       */
/*                                                                          */
/****************************************************************************/
// Start with the largest gap and work down to a gap of 1
 void ShellSort(void)
{
   int gap;
   int i;
   int j;
   n = NUMBER_OF_SAMPLES;
   
   x = n;
   x = n >> 1;
   for (; x > 0; x = (x >> 1))
   {
      gap = Transfr[x];
      // Do a gapped insertion sort for this gap size.
      // The first gap elements a[0..gap-1] are already in gapped order
      // keep adding one more element until the entire array is gap sorted
      for (i = gap; i < n; i += 1)
      {  // add a[i] to the elements that have been gap sorted
         // save a[i] in temp and make a hole at position i
         Temp = Transfr[i];
         // shift earlier gap-sorted elements up until the correct location for a[i] is found
         for (j = i; ((j >= gap) && (Transfr[j - gap] > Temp)); j -= gap)
         {
            Transfr[j] = Transfr[j - gap];
         }
         // put temp (the original a[i]) in its correct location
         Transfr[j] = Temp;
      }
   }
}

/****************************************************************************/
/*                                                                          */
/*     Function: NextADC                                                    */
/*        Input: None                                                       */
/*       Return: None                                                       */
/*     Overview: This function is used to update the registers with the     */
/*               newest ADC Values. The use of a type of digital filter     */
/*               is used here. The values from the Transfer buffer are      */
/*               sorted, and then middle 16 values out of the 32 samples    */
/*               are used to calculate the sum.                             */
/*                                                                          */
/****************************************************************************/
static void DigitalFilter(void)
{
   // Use only the middle 16 samples to calculate the sum
   for (Sum = 0, Index = 8; Index < 24; Index++) Sum += Transfr[Index];
   InUse = False; // Clr Semiphore
   Sum >>= (SHIFTS - 1); // Divide by 16
   Reg->Value = (0x0FFF & Sum); // Make sure that value fits.
   Sum = 0;
   Reg->Changed = True; // Indicate the we have a new value
   This.ReadingReady = False; // Signal that the DMA must now get a new reading
}

/****************************************************************************/
/*                                                                          */
/*     Function: ClrBuffers                                                 */
/*        Input: None                                                       */
/*       Return: None                                                       */
/*     Overview: Clear the DMA buffers so that the next reading can start   */
/*               from scratch.                                              */
/*                                                                          */
/****************************************************************************/
static void ClrBuffers(void)
{
   memset(BufferA, 0, sizeof(BufferA));
   memset(BufferB, 0, sizeof(BufferA));
}

/****************************************************************************/
/*                                                                          */
/*     Function: StartADReading                                             */
/*        Input: None                                                       */
/*       Return: None                                                       */
/*     Overview: This function triggers the DMA and ADC to start taking     */
/*               readings. Start the DMA interrupts.                        */
/*                                                                          */
/****************************************************************************/
static void StartADReading(void)
{
   DMA0CONbits.CHEN = 1;
}

/****************************************************************************/
/*                                                                          */
/*     Function: StartADReading                                             */
/*        Input: None                                                       */
/*       Return: None                                                       */
/*     Overview: This function triggers the DMA and ADC to start taking     */
/*               readings. Start the DMA interrupts.                        */
/*                                                                          */
/****************************************************************************/
static void StopADReading(void)
{
   DMA0CONbits.CHEN = 0;
}

/****************************************************************************/
/*                                                                          */
/*     Function: Get one ADC Calculation                                    */
/*        Input: None                                                       */
/*       Return: 16 bit ADC reading                                         */
/*     Overview: This function is not implemented                           */
/*                                                                          */
/****************************************************************************/
static unsigned ReadADC(void)
{
   return Reg->Value;
}

/****************************************************************************/
/*                                                                          */
/*     Function: DMA 0 Interrupt                                            */
/*     Overview: This interrupt is triggered when a set of ADC readings     */
/*               have filled up either BufferA or BufferB. Basically the    */
/*               silicon will perform AD readings in the background. The    */
/*               DMA will perform the transfer and the results are          */
/*               calculated in the superloop.                               */
/*                                                                          */
/****************************************************************************/

/*=============================================================================
_DMA0Interrupt(): ISR name is chosen from the device linker script.
=============================================================================*/
void __attribute__((interrupt, no_auto_psv)) _DMA0Interrupt(void)
{
   if (InUse == False) DmaBuffer ^= 1;
   if (DmaBuffer == 0)
      memcpy(Transfr, BufferA, sizeof(BufferA));
   else
      memcpy(Transfr, BufferA, sizeof(BufferB));
   This.ReadingReady = True; // Indicate that a new sample set is ready
   //SystemStat->AdRdy  = True;
   IFS0bits.DMA0IF = 0; // Clear the DMA0 Interrupt Flag
}
