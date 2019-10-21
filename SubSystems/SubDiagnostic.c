#include "defs.h"
#include "Register.h"
#include "SubSystem.h"
#include "FeedBack.h"
#include "SubDiagnostic.h"

#define DIAGNOSTICS_SUB_SYSTEM  "Internal Diagnostics Sub-System"

// Private Methods
static void ResetDiagnostics(void);
static void _Process(void);
static void HandleDiagnostics(void);

// Standard Device Interfaces
Device    * FbCounterInc;
Device    * FbCounterDec;

// Device operations used..
static SubSystem This =
{
   .Used = True,
   .SubSystemType = DIAGNOSTIC,
   .Discription = DIAGNOSTICS_SUB_SYSTEM,
   .Id = (0x0100 + (int)DIAGNOSTIC),
   .WaitingOnMask = 0,
   .Arg = 0,
   .ProcessFn = _Process,
   .HandleSubSystem = HandleDiagnostics,
   .Flush = ResetDiagnostics,
   .Reset = ResetDiagnostics,
};

/****************************************************************************/
/*                                                                          */
/*     Function: NewRom                                                     */
/*        Input:                                                            */
/*       Return:                                                            */
/*     Overview:                                                            */
/*                                                                          */
/****************************************************************************/
SubSystem * InitDiagnostics(Register * Registers)
{
   // Instantiate Increment Counter
   FbCounterInc = FbInit(&Registers[FEEDBC1], &Registers[MAX_FB1], INC);   
   
   // Instantiate Decrement Counter
   FbCounterDec = FbInit(&Registers[FEEDBC2], &Registers[MAX_FB2], DEC); 
   
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
   while(LOOPING_FOREVER)
   {
      // Update Feeb back units.
      FbCounterInc->DeviceFn(FbCounterInc);
      FbCounterDec->DeviceFn(FbCounterDec);   
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
static void HandleDiagnostics(void)
{
      // Update Feeb back units.
      FbCounterInc->DeviceFn(FbCounterInc);
      FbCounterDec->DeviceFn(FbCounterDec);   
}

/****************************************************************************/
/*                                                                          */
/*     Function: NewRom                                                     */
/*        Input:                                                            */
/*       Return:                                                            */
/*     Overview:                                                            */
/*                                                                          */
/****************************************************************************/
static void ResetDiagnostics(void)
{
   FbOps * FbDev;
   FbDev = (FbOps *)FbCounterInc->Driver;
   FbDev->Clear(FbCounterInc);
   FbDev = (FbOps *)FbCounterDec->Driver;
   FbDev->Clear(FbCounterDec);
}