/**************************************************************************/
/*                                                                        */
/*  Programmer: Gregory L Montgomery                                      */
/*                                                                        */
/*                                                                        */
/*                                                                        */
/**************************************************************************/
#include "defs.h"
#include "SubSystem.h"
#include "M25LC320.h"
#include "Device.h"
#include "Register.h"
#include "Registers.h"
#include "SubThermister.h"

#define THERMISTER_SUB_SYSTEM  "Thermister Sub-System"

static Register * RegisterPtr;

// Device operations used..
static SubSystem This =
{
   .Used = True,
   .SubSystemType = THERMISTER_READING,
    .Discription = THERMISTER_SUB_SYSTEM,
   .Id = (0x0100 + (int)THERMISTER_READING),
   .WaitingOnMask = SAVED_TO_ROM,
   .Arg = 0,
   .ProcessFn = 0,//_Process,
   .HandleSubSystem = 0, //_HandleSettings,
   .Flush = 0,  //_ResetValues,
   .Reset = 0, //_ResetValues,
};

SubSystem * InitThermister(Register * Registers)
{
   RegisterPtr = Registers;
   return &This;
}

