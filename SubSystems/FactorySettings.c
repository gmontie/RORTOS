#include <xc.h>
#include "Register.h"
#include "NVRegisters.h"
#include "FactorySettings.h"
#include "SysCalls.h"

#define DISCRIPTION_SAVE_SETTINGS_THREAD "Settings Thread"

static Boolean SettingsReady(void);
static void upDate(void);
static void InitSettings(unsigned MemoryIndex, const unsigned * Rev, unsigned RevLength) ;

// Service operations used..
static Service This =
{
   //.Used = True,
   .DeviceType = ROM_SETTINGS,
   .DeviceClass = SYSTEM_DEV,
   .Discription = DISCRIPTION_SAVE_SETTINGS_THREAD,
   .AuxUse = True,
   .Id = (0x0100 + (int)ROM_SETTINGS),
   .WaitingOnMask = CONFIGDrty,
   .Arg = 0,
   .FnType = ProcessFn,
   .Thread = upDate,
   .Write = 0,
   .Reset = 0,
   .IsReady = SettingsReady,
   .Next = 0,
};

static NVRegisters  * MemManager;
static Bus WhichBus;

static fops * sBus = 0; // A Bus/File Object.
static volatile RequestBits * Request;

/****************************************************************************/
/*                                                                          */
/*     Function:                                                            */
/*        Input: None                                                       */
/*       Return: None                                                       */
/*     Overview:                                                            */
/*                                                                          */
/****************************************************************************/
Service * NewFacortySettings(fops * IO, NVRegisters * ROM_Manager, const unsigned * NbrPtr, unsigned MemoryIndex, int Length)
{
   sBus = IO;
   WhichBus = IO->Resource;
   MemManager = ROM_Manager;
   Request = getRequestBits();
   InitSettings(MemoryIndex, NbrPtr, Length);
   return &This;
}

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
   Exec(WhichBus, MemManager->StoreRegisters);
   Request->BlkOnCfg = 1;
}

/****************************************************************************/
/*                                                                          */
/*     Function: InitComm Thread                                            */
/*        Input: A pointer to the hard coded firmware version               */
/*        Input: The length for the hard coded firmware version             */
/*       Output: NONE                                                       */
/*     Overview: Receives one character of information in from the UART     */
/*                                                                          */
/****************************************************************************/
static void InitSettings(unsigned MemoryIndex, const unsigned * Rev, unsigned RevLength) 
{
   // Load Non-Volatile register values from ROM   
   MemManager->LoadRegisters();
   
   // Test to make sure that this version is the same as saved ROM version
   if (MemManager->CheckFirmwareVersion(Rev, MemoryIndex, RevLength) == 0)
   {  // If the two differ then this is newer firmware so
      configDefaults(Rev);
      MemManager->StoreRegisters();
   }
}

