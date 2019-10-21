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
#include "Register.h"
#include "Registers.h"
#include "SubTemperature.h"

#define TEMPERATURE_SUB_SYSTEM  "Temperature Sub-System"


static Register * RegisterPtr;

// Device operations used..
static SubSystem This =
{
   .Used = True,
   .SubSystemType = TEMP_READING,
   .Discription = TEMPERATURE_SUB_SYSTEM,
   .Id = (0x0100 + (int)TEMP_READING),
   .WaitingOnMask = SAVED_TO_ROM,
   .Arg = 0,
   .ProcessFn = 0,//_Process,
   .HandleSubSystem = 0, //_HandleSettings,
   .Flush = 0,  //_ResetValues,
   .Reset = 0, //_ResetValues,
};

SubSystem * InitTEMP(Register * Registers)
{
   RegisterPtr = Registers;
   return &This;
}

