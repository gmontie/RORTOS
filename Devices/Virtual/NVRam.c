#include "defs.h"
#include "NVRam.h"
#include "Registers.h"
#include "Device.h"
#include "Register.h"
#include "NVRegisters.h"

#define ROM_REGISTER_SUB_SYSTEM  "Read Only Register Sub-System"

/* Private functions */
void SaveValues(void);
void ResetValues(void);
unsigned ROM_Value(byte);
Boolean Running(void);
void WriteROM(byte Address, unsigned Value);

static IndicatorBits * SysStatus;
static Register * SubSystem;
static NVRegisters * ROR_Interface;
static Device * Com;

static int Address; // Register address
//static int Value; // Value for Register

// Device operations used..
static Device This =
{
   .Used = True,
   .DeviceType = READ_ONLY_REGISTER_SUBSYSTEM,
   .Discription = ROM_REGISTER_SUB_SYSTEM,
   .AuxUse = 0,
   .Id = (0x0100 + (int)READ_ONLY_REGISTER_SUBSYSTEM),
   .WaitingOnMask = ROM_READY_BIT,
   .Arg = 0,
   .FnType = ProcessFn,
   .UpDate = SaveValues,
   .Read = ROM_Value,
   .Write = WriteROM,
   .Reset = ResetValues,
   //.Flush = Flush,
   .IsReady = Running,
   .Read = ROM_Value,
};

Device * NewRom(Register * MemorySubSys, NVRegisters * NVInterface, Device * IO)
{
   SysStatus = GetSystemStat();
   ROR_Interface = NVInterface;
   Com = IO;
   SubSystem = MemorySubSys;
   
   return &This;
}

void SaveValues(void)
{
   while((Address = Com->Next()) > 0)
   {
      ROR_Interface->Store(Address, &SubSystem[Address]);
   }
   SysStatus->ROM_Rdy = False;
}

void FlushValue(void)
{
   ROR_Interface->Reset();
   ROR_Interface->Clear(SubSystem);
}

void ResetValues(void)
{
   ROR_Interface->Reset();
   ROR_Interface->Clear(SubSystem);
   //ConfigDefaults(Firmware);
   ROR_Interface->StoreRegisters(SubSystem);
}

unsigned ROM_Value(byte Adrs)
{
   return SubSystem[Adrs].Value;
}

Boolean Running(void)
{
   return False;
}

void WriteROM(byte Addrs, unsigned Value)
{
   SubSystem[Addrs].Value = Value;
   ROR_Interface->Store(Addrs, &SubSystem[Addrs]);
}