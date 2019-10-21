/**************************************************************************/
/*                                                                        */
/*  Programmer: Gregory L Montgomery                                      */
/*                                                                        */
/*                                                                        */
/*                                                                        */
/**************************************************************************/
#include "defs.h"
#include "SubSystem.h"
#include "Device.h"
#include "Fan.h"
#include "Register.h"
#include "Registers.h"
#include "PinConfig.h"
#include "SubFans.h"

#define FAN_GROUP_SUB_SYSTEM  "Thermister Sub-System"

void CaptureFanCounts(void);
void CalcFanCounts(void * );
void ComputePreviousCapture(void);
Boolean _Fan_Alloc(int * Index);

static Register      * RegisterPtr;
static Register      * StatusRegister;
static Register      * RequestRegister;
static StatusBits    * SystemStatusBits = 0;
static RequestBits   * RequestingBits = 0;

static unsigned Previous;
static unsigned Current;
static long Total   = 0;

// Device operations used..
static SubSystem This =
{
   .Used = True,
   .SubSystemType = FAN_GROUP,
   .Discription = FAN_GROUP_SUB_SYSTEM,
   .Id = (0x0100 + (int)FAN_GROUP),
   .WaitingOnMask = SAVED_TO_ROM,
   .Arg = 0,
   .ProcessFn = 0,//_Process,
   .HandleSubSystem = 0, //_HandleSettings,
   .Flush = 0,  //_ResetValues,
   .Reset = 0, //_ResetValues,
};

SubSystem * InitFanCNTL(Register * Registers)
{
   RegisterPtr = Registers;
   StatusRegister     = GetStatusRegister();
   SystemStatusBits = (StatusBits *)&StatusRegister->Value;
   RequestRegister = GetRequestRegister();
   RequestingBits = (RequestBits *)&RequestRegister->Value;

   return &This;
}

// Below are incorrect declaration which are
// temporaroly made so as to get the code to
// compile.
unsigned Counts;
unsigned CounterOne; 
/****************************************************************************/
/*                                                                          */
/*     Function:                                                            */
/*        Input: None                                                       */
/*       Return: None                                                       */
/* Side Effects:                                                            */
/*     Overview: Assign OC1 to be output on RF13                            */
/*                                                                          */
/****************************************************************************/
void CaptureFanCounts(void)
{
    static Boolean Initial = True;

    if (!Initial)
    {
        if (SystemStatusBits->FTDone == 1)
        {
            // Algorithm to mine data gathered from IC9 Periferal
            ComputePreviousCapture();

            // Set up next reading!
            // Clear all Fan Status bits
            //Counts->Status = 0;

            // Setup Timer One
            CounterOne = 0; // Clear Counter One Timer/Counter

            // Turn on Interrupt.
           // IEC5bits.IC9IE = 1; // Enable IC9 Interrupts.

            // Turn on Timer 1
            T1CONbits.TON = 1;
        }
    }
    else
    { // Start Readings...
        Initial = False;
        // Clear all Fan Status bits
        //Counts->Status = 0;

        // Setup Timer One
        CounterOne = 0; // Clear Counter One Timer/Counter

        // Turn on Interrupt.
       // IEC5bits.IC9IE = 1; // Enable IC9 Interrupts.

        // Turn on Timer 1
        T1CONbits.TON = 1;
    }
}

/****************************************************************************/
/*                                                                          */
/*     Function: GetFanSpeed                                                */
/*        Input: PortNumber is the number of the port that the Tachometer   */
/*               is on. Port A is 0, Port B is 1 Port C is 2 and so on.     */
/*       Output: Success or Failure. If Success a positive number is        */
/*               returned. In the case that there is a Failure a negitive   */
/*               number is returned.                                        */
/* Side Effects: Changes a bit in bit field FLAGS. Also changes the         */
/*               value of Timer/Counter 1.                                  */
/*                                                                          */
/*     Overview: This function has been moved from being an interrupt       */
/*               driven procedure to a buisy waiting function so that       */
/*               the computation can be interruptable. The interruptability */
/*               in computing the fan speeds improves other critical        */
/*               functionality by reducing the amount of temperal resrouces */
/*               durring interrupt time.                                    */
/* Notes:                                                                   */
/*                                                                          */
/****************************************************************************/
void CalcFanCounts(void * Obj)
{
  byte Count  = 0;
  CounterOne = 0; // Defined as an absolute var at the address of Counter One
  T1CON = 0; // Clear and reset Timer 1
  T1CONbits.TCKPS = 1;
  TMR1 = 0;
  Total = 0;
  Register * Counts=((_Fan*)Obj)->FanCounts;

  // Clear status flags in FCB state bit map
  Counts->Status = 0;

  // Clear Timer Interrupt Flags
  IFS0bits.T1IF = 0; //Clear the Timer1 interrupt status flag
  IEC0bits.T1IE = 1; //Enable Timer1 interrupts

  // Turn on Timer
  T1CONbits.TON = 1; // Start Timer

  // For 2 pulses calculate the RPS value
  for(Count = 0; ((Count < 2) && (!SystemStatusBits->FTDone)); Count++)
  {
    //while((PORTFbits.RF8) && (Flags.Overflow == 0));
    //while((SystemStatusBits->Overflow == 0) && (!(PORTFbits.RF8)));
    if(Count == 0) CounterOne = 0;
  }
  Total = (unsigned)CounterOne;

  T1CONbits.TON = 0; // Stop Timer
  if(SystemStatusBits->Overflow)
  {
    SystemStatusBits->Overflow = 0;
    Total = 0;
  }

   SystemStatusBits->FTDone = 1;
  //Fan.StatusWord->Value = Flags.StatusBits;
  //Counts->Changed = 1; //Changed = True;

  if(Counts->Value != Total)
  {
    Counts->Value = Total;
    Counts->Changed = 1;//Changed = True;
  }
}

/****************************************************************************/
/*                                                                          */
/*     Function:                                                            */
/*        Input: None                                                       */
/*       Return: None                                                       */
/* Side Effects:                                                            */
/*     Overview: Assign OC1 to be output on RF13                            */
/*                                                                          */
/****************************************************************************/
void ComputePreviousCapture(void)
{
//    Current = IC9BUF;
//    Previous = IC9BUF;

    // Clear unused IC9 Buffer Memory
 //   while (IC9CON1bits.ICBNE)
 //       ScratchPad = IC9BUF;

    // If we have Previoud > Current
    if (Current < Previous)
    { // Put difference into FanCounts
        //Fan.FanCounts->Value = (unsigned) (Previous - Current);
        //Fan.FanCounts->Stats.Changed = 1; // Indicate FanCounts Changed.
    }

    //Fan.StatusWord->Value = Flags.StatusBits;
    //Fan.StatusWord->Stats.Changed = 1; // Changed = True;
}

/****************************************************************************/
/*     Function: _Dev_Alloc                                                 */
/*                                                                          */
/*        Input: A pointer to an available buffer which can be updated      */
/*               with the number of the table element to be used.           */
/*       Output: Success or Failure                                         */
/* Side Effects: The memory pointed to by * Index is changed                */
/*     Overview: Allocates an element from the Device Table.                */
/*                                                                          */
/****************************************************************************/
Boolean _Fan_Alloc(int * Index)
{
  Boolean Results = False;
  int i = 0;

  do
  {
    //if(Fans[i].Used) // if used
     if(True)
      i++; // Go to the next element in the table
    else
    { // Found a free spot so lets exit.
      Results = True;
      *Index = i;
    }
  }while((Results == False) && (i < NO_OF_FANS));
  return Results;
}    

