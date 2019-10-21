/**************************************************************************/
/*                                                                        */
/*  Programmer: Gregory L Montgomery                                      */
/*                                                                        */
/*                                                                        */
/*                                                                        */
/*                                                                        */
/**************************************************************************/
#include <xc.h>
#include <string.h>
#include "defs.h"
#include "System.h"
#include "Quantum.h"

// This file contains the implementation for a counter object called a
// Counter object. The purpose of this type of Object is for debugging.
// It provides a quite mechanism used to show if the Microcontroller is
// running and that the communications are working.

#define NUMBER_OF_FBs 2 // Compile time allocation

static CounterData CounterTable[NUMBER_OF_FBs];

static Boolean AllocCounter(int * Index);
static Boolean RunMe(void);
static void Increment(void * Args);
static void Decrement(void * Args);
static Boolean RmCounterData(int Id);
static int FindDataIndex(int Id);

static Quantum This = 
{
   .Inc = Increment,
   .Dec = Decrement,
   .Rdy = RunMe,
   .Clear = RmCounterData,
};

/****************************************************************************/
/*                                                                          */
/*     Function: FbInit                                                     */
/*        Input: None                                                       */
/*       Return: A Pointer to a feed back object.                           */
/*     Overview: This is the initialization for a Feedback Object.          */
/*                                                                          */
/****************************************************************************/
Quantum * NewQuantum(Register * Location, Register * Max)
{
  int i = 0;

  if(AllocCounter(&i))
  {
    CounterTable[i].ID = i;
    CounterTable[i].Used = True;
    CounterTable[i].MaxCountPtr = Max;
    CounterTable[i].CounterPtr = Location;
    CounterTable[i].Counter = 0;
    This.DataPtr = &CounterTable[i];
  }
  return &This;
}

/****************************************************************************/
/*                                                                          */
/*     Function: Increment                                                  */
/*        Input: A pointer to an FbData Object which is to be incremented   */
/*       Return: None                                                       */
/*     Overview: This function increments the FbData Object which is passed */
/*               to it.                                                     */
/*                                                                          */
/****************************************************************************/
static void Increment(void * Args)
{
  CounterData * DataPtr = (CounterData *)Args;
  Register * CounterValue = DataPtr->CounterPtr;
  Register * MaxCount = DataPtr->MaxCountPtr;

  if(DataPtr->Counter > MaxCount->Value)
  {
    DataPtr->CounterPtr->Value++;
    if(CounterValue->Value > 360) CounterValue->Value = 0;
    CounterValue->Changed = Yes; // Changed = True = Yes;
    DataPtr->Counter = 0;
  }
  else
    DataPtr->Counter++;
}

/****************************************************************************/
/*                                                                          */
/*     Function: Decrement                                                  */
/*        Input: A pointer to an FbData Object                              */
/*       Return: None                                                       */
/*     Overview: This function decrements the FbData Object which is passed */
/*               to it.                                                     */
/*                                                                          */
/****************************************************************************/
static void Decrement(void * Args)
{
  CounterData * DataPtr = (CounterData *)Args;
  Register * CounterValue = DataPtr->CounterPtr;
  Register * MaxCount = DataPtr->MaxCountPtr;
  
  if(DataPtr->Counter > MaxCount->Value)
  {
    CounterValue->Value--;
    if(CounterValue->Value > 360) CounterValue->Value = 360;
    CounterValue->Changed = 1; //Changed = True;
    DataPtr->Counter = 0;
  }
  else
    DataPtr->Counter++;
}

/****************************************************************************/
/*                                                                          */
/*     Function: RunMe                                                      */
/*        Input: None                                                       */
/*       Return: Alway True                                                 */
/*     Overview: This is used to trigger the running of the FeedBack        */
/*               Objects.                                                   */
/*                                                                          */
/****************************************************************************/
static Boolean RunMe(void){return True;}

/****************************************************************************/
/*     Function: FbClear                                                    */
/*                                                                          */
/*        Input: A pointer to an FbData Object device.                      */
/*       Output: None                                                       */
/* Side Effects: This function resets FbObject to zero                      */
/*                                                                          */
/****************************************************************************/
static Boolean RmCounterData(int Id)
{
   int Index = FindDataIndex(Id);
   Boolean Results = False;
   
   if(Index >= 0) 
   {
      memset(&CounterTable[Index], 0, sizeof(CounterData));
      Results = True;
   }    
   return Results;
}

/****************************************************************************/
/*                                                                          */
/*     Function:                                                            */
/*        Input: None                                                       */
/*       Return: None                                                       */
/*     Overview:                                                            */
/*                                                                          */
/****************************************************************************/
static int FindDataIndex(int Id)
{
   int i = 0;
   int Results = -1;
   
   do
   {
      if((CounterTable[i].Used) && (CounterTable[i].ID == Id))
         Results = i;
      else
         i++;
   }while((i < NUMBER_OF_FBs) && (Results < 0));
   
   return i;
}

/****************************************************************************/
/*                                                                          */
/*     Function: DataAlloc                                                  */
/*        Input: A pointer to an integer which will contain the index       */
/*               to a newly allocated FbData Object.                        */
/*       Output: Success or Failure                                         */
/*     Overview: This function searches the FbData table to find an unused  */
/*               FbData Object. When a free FbData Object is found the      */
/*               index pointer is updated with the index of the free        */
/*               Data Object from the table.                                */
/*                                                                          */
/****************************************************************************/
static Boolean AllocCounter(int * Index)
{
  Boolean Results = False;
  int i = 0;

  do
  {
    if(CounterTable[i].Used) // if used
      i++; // Go to the next element in the table
    else
    { // Found a free spot so lets exit.
      Results = True;
      *Index = i;
    }
  }while((Results == False) && (i < NUMBER_OF_FBs));
  return Results;
}
