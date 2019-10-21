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
#include "System.h"
#include "defs.h"
#include "MAdc.h"

// Functions for use by the ADC Object
unsigned ReadADC(void); // Take an ADC Reading (Not implemented here)
void SortResults(void); // Sort the results so that NextADC can do it's job
void NextADC(void); // Calculate the next ADC reading
void StartAD1(void); // Start the ADC Peripheral
void StartADReading(void); // Start the DMA for the ADC
void initDma0(void); // Initialize the DMA for the ADC

// Private data which the ADC uses!
static int BufferA[NUMBER_OF_SAMPLES] __attribute__((space(dma)));
static int BufferB[NUMBER_OF_SAMPLES] __attribute__((space(dma)));
static int Transfr[NUMBER_OF_SAMPLES];
static long Sum;
static int Index;
static int x;
static int n;
static int y;
static int Temp;
static Boolean InUse;
static unsigned int DmaBuffer = 0;
static int * BuffPtr;
static int * Walk;
static Register * Reg;

// Statically set up the ADC Object
static AdOps This =
{
   .ADC = ReadADC,
   .StartADC = StartAD1,
   .StartReading = StartADReading,
   .NextADCReading = NextADC,
   .Sort = SortResults,
   .ReadingReady = False
};


/****************************************************************************/
/*                                                                          */
/*     Function: SetupADISR                                                 */
/*        Input: None                                                       */
/*       Return: None                                                       */
/*     Overview: Sets up the Interrupt Service routeen for the ADC Unit     */
/*                                                                          */
/****************************************************************************/
void SetupADISR(void)
{
   IPC3bits.AD1IP = 2; // Next to lowest priority interrupt
   IFS0bits.AD1IF = 0; // Clear Interrupt Flags
}

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
AdOps * ADCInit(Register * UI)
{
   AD1CON1bits.ADON = 0; // Configure only, Dont turn on ADC
   AD1CON1bits.ADDMABM = 1; // DMA buffers are built in conversion order mode
   AD1CON1bits.AD12B = 1; // 12 bit mode
   AD1CON1bits.FORM = 0; // 0b01; // Data Output Format: Signed integer
   AD1CON1bits.SSRC = 7; // Internal counter ends sampling starts conversion (auto-convert)
   AD1CON1bits.SIMSAM = 1; // Sample multiple channels individually in sequence.
   AD1CON1bits.ASAM = 1; // Sampling begins immediately after last conversion.

   AD1CON2bits.ALTS = 0;
   AD1CON2bits.BUFM = 1; // Starts filling at 0x0 then 0x8 on next interrupt

   AD1CON2bits.CHPS = 0; // Converts CH0 only
   AD1CON2bits.CSCNA = 0; // Do not Scan inputs
   AD1CON2bits.SMPI = 0;//b0111; // Generate interrupt completion after 8th sample
   AD1CON2bits.VCFG = 0; // Use Vcc and Gnd for Vr+ and Vr-

   AD1CON3bits.ADRC = 0; // Use System clock for ADC Timing.
   AD1CON3bits.SAMC = 0x17; //0x1F;//0x1F;
   AD1CON3bits.ADCS = 0x2F; //0x3F;// 0x3F; // Auto-Sample time bits 22118400 / (4 * 64)

   AD1CON4bits.DMABL = 0b0101; // 64 words of buffer to each analog input

   AD1CHS0bits.CH0NB = 0; // channel 0 Negative input is Vref-
   AD1CHS0bits.CH0SB = 0; // Channel 0 positive input is AN1
   AD1CHS0bits.CH0NA = 0; // channel 0 Negative input is Vref-
   AD1CHS0bits.CH0SA = 0; // Channel 0 positive input is AN1

   Reg = UI;
   Walk = BuffPtr = BufferA;

   InUse = False;

   SetupADISR();
   initDma0();
   
   return &This;
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
void StartAD1(void)
{
   AD1CON1bits.ADON = 1; // Start ADC
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
void initDma0(void)
{
   DMA0CON=0;
   DMA0CONbits.AMODE = 0; // Configure DMA for Register indirect with post increment
   DMA0CONbits.MODE=2; // Configure DMA for Continuous Ping-Pong mode
   //DMA0CON |= 0x0002; // Configure DMA for Continuous Ping-Pong mode

   DMA0PAD = (int) &ADC1BUF0; // Store the address of the ADC1 Result buffer
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
void SortResults(void)
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
void NextADC(void)
{
   // Use only the middle 16 samples to calculate the sum
   for (Sum = 0, Index = 8; Index < 24; Index++) Sum += Transfr[Index];
   Sum >>= (SHIFTS - 1); // Divide by 16
   Reg->Value = 0x0FFF & Sum; // Make sure that value fits.
   Reg->Changed = True; // Indicate the we have a new value
   This.ReadingReady = False; // Signal that the DMA must now get a new reading

   InUse = False; // Clr Semiphore
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
void StartADReading(void)
{
   DMA0CONbits.CHEN = 1;
}

/****************************************************************************/
/*                                                                          */
/*     Function: Get one ADC Calculation                                    */
/*        Input: None                                                       */
/*       Return: 16 bit ADC reading                                         */
/*     Overview: This function is not implemented                           */
/*                                                                          */
/****************************************************************************/
unsigned ReadADC(void)
{
   return 0;
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
   {
      for (Index = 0; Index < (NUMBER_OF_SAMPLES - 1); Index++)
      {
         Transfr[Index] = BufferA[Index];
      }
   }
   else
   {
      for (Index = 0; Index < (NUMBER_OF_SAMPLES - 1); Index++)
      {
         Transfr[Index] = BufferB[Index];
      }
   }
   This.ReadingReady = True; // Indicate that a new sample set is ready
   IFS0bits.DMA0IF = 0; // Clear the DMA0 Interrupt Flag
}

