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
#include "NVRegisters.h"
#include "SubNvSettings.h"

#define ROM_REGISTER_SUB_SYSTEM  "Read Only Register Sub-System"

/* Private functions */
static  void _Process(void);
static  void _HandleSettings(void);
static  void _ResetValues(void);

static IndicatorBits  * SystemStatBits;
static SubSystemBits  * SubSystem_Bits;
static RequestBits    * Request_Bits;
static Register       * Element;
static Register       * ConfigReg;
static NVRegisters    * MemManager;
static const unsigned * Version;
static Device         * M25C320;

// Device operations used..
static SubSystem This =
{
   .Used = True,
   .SubSystemType = READ_ONLY_REGISTER_SUBSYSTEM,
   .Discription = ROM_REGISTER_SUB_SYSTEM,
   .Id = (0x0100 + (int)READ_ONLY_REGISTER_SUBSYSTEM),
   .WaitingOnMask = ROM_READY_BIT,
   .Arg = 0,
   .ProcessFn = _Process,
   .HandleSubSystem = _HandleSettings,
   .Flush = _ResetValues,
   .Reset = _ResetValues,
};

/****************************************************************************/
/*                                                                          */
/*     Function: NewRom                                                     */
/*        Input:                                                            */
/*       Return:                                                            */
/*     Overview:                                                            */
/*                                                                          */
/****************************************************************************/
SubSystem * InitNvSubSystem(const unsigned * Firmware, Register * Registers, fops * SPI1_IO, int Start, int Length)
{
   Element = Registers;
   Version = Firmware;
   
   SystemStatBits = GetSystemStat();
   SubSystem_Bits = GetSubSysBits();
   ConfigReg = GetRequestBits();
   Request_Bits = (RequestBits *)ConfigReg->Value;
   
   M25C320 = InitM25LC320(SPI1_IO);
   MemManager = InitNVRam(Registers, M25C320);
   MemManager->Reset();   
   MemManager->LoadRegisters(Registers);
   
   if (MemManager->CheckFirmwareVersion(Version, Registers, Start, Length) == 0)
   {
      ConfigDefaults(Version);
      MemManager->StoreRegisters(Registers);
   }
   
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
      if(Request_Bits->CommitBit)
      {
         MemManager->StoreRegisters(Element);
      }
      
      if(Request_Bits->ClearConfig) 
      {
         ConfigDefaults(Version);
         MemManager->StoreRegisters(Element);
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
static void _HandleSettings(void)
{
      if(Request_Bits->CommitBit)
      {
         MemManager->StoreRegisters(Element);
      }
      
      if(Request_Bits->ClearConfig) 
      {
         ConfigDefaults(Version);
         MemManager->StoreRegisters(Element);
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
static void _ResetValues(void)
{
   ConfigDefaults(Version);
}

