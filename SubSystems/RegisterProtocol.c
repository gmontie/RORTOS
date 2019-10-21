#include "defs.h"
#include "FeedBack.h"
#include "SubDiagnostic.h"
#include "Fops.h"
#include "ReMeCi.h"
#include "Uart1.h"

#define DIAGNOSTICS_SUB_SYSTEM  "Internal Diagnostics Sub-System"

// Private Methods
static void _Reset(void);
static void _Process(void);
static void _HandleSubSystem(void);

// Standard Device Interfaces
static Device    * CommInterface;

// Device operations used..
static SubSystem This =
{
   .Used = True,
   .SubSystemType = REGISTER_COM,
   .Discription = DIAGNOSTICS_SUB_SYSTEM,
   .Id = (0x0100 + (int)REGISTER_COM),
   .WaitingOnMask = 0,
   .Arg = 0,
   .ProcessFn = _Process,
   .HandleSubSystem = _HandleSubSystem,
   .Flush = _Reset,
   .Reset = _Reset,
};

/****************************************************************************/
/*                                                                          */
/*     Function: NewRom                                                     */
/*        Input:                                                            */
/*       Return:                                                            */
/*     Overview:                                                            */
/*                                                                          */
/****************************************************************************/
struct a_a * InitRegisterProtocol(Register * Registers, fops * IO)
{
   // Let the Protocol Object have access to the Uart Object
   CommInterface = NewReMeCI(Registers, IO);   
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
      // Update Communications
      if (CommInterface->IsReady())
      {
         CommInterface->UpDate();
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
static void _HandleSubSystem(void)
{
      // Update Communications
      if (CommInterface->IsReady())
      {
         CommInterface->UpDate();
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
static void _Reset(void)
{
   CommInterface->Reset();
}
