#include <xc.h>
#include "Register.h"
#include "NVRegisters.h"
#include "Settings.h"
#include "SysCalls.h"

#define DISCRIPTION_SAVE_SETTINGS_THREAD "Settings Thread"

static Boolean SettingsReady(void);
static void upDate(void);

// Service operations used..
static Service This =
{
   //.Used = True,
   .DeviceType = ROM_SETTINGS,
   .DeviceClass = SYSTEM_DEV,
   .Discription = DISCRIPTION_SAVE_SETTINGS_THREAD,
   .AuxUse = True,
   .Id = (0x0100 + (int)ROM_SETTINGS),
   .WaitingOnMask = ROM_READY_BIT,
   .Arg = 0,
   .FnType = ProcessFn,
   .Thread = upDate,
   .Write = 0,
   .Reset = 0,
   .IsReady = SettingsReady,
   .Next = 0,
};

static NVRegisters  * MemManager;

/****************************************************************************/
/*                                                                          */
/*     Function:                                                            */
/*        Input: None                                                       */
/*       Return: None                                                       */
/*     Overview:                                                            */
/*                                                                          */
/****************************************************************************/
static Boolean SettingsReady(void){ return True; }

/****************************************************************************/
/*                                                                          */
/*     Function: upDate                                                     */
/*        Input: None                                                       */
/*       Output: None                                                       */
/*     Overview: This thread checks the Uart IO and updates the shared      */
/*               register space.                                            */
/*                                                                          */
/*                                                                          */
/*                                                                          */
/****************************************************************************/
static void upDate(void) 
{ 
   Exec(_SPI_1, MemManager->StoreRegisters);
}

/****************************************************************************/
/*                                                                          */
/*     Function: InitComm Thread                                            */
/*        Input: A pointer to the shared memory (registers)                 */
/*       Output: A pointer to a newly allocated Devcie Object               */
/*     Overview: Receives one character of information in from the UART     */
/*                                                                          */
/****************************************************************************/
Service * InitSettings(NVRegisters  * RomDev, unsigned * Rev, unsigned RevLength) 
{
   MemManager = RomDev;
   
   // Load Non-Volatile register values from ROM   
   MemManager->LoadRegisters();
   
   // Test to make sure that this version is the same as saved ROM version
   if (MemManager->CheckFirmwareVersion(Rev, ROM_1, RevLength) == 0)
   {  // If the two differ then this is newer firmware so
      configDefaults(Rev);
      MemManager->StoreRegisters();
   }
   
    return &This;
}

