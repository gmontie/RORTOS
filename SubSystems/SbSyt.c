#include "defs.h"
#include "Register.h"
#include "Registers.h"
#include "SubSystem.h"

static Register * Registers;

void InitSubSystems(Register * Regs)
{
   Registers = Regs;
}

SubSystemBits * GetSubSysBits(void)
{
   return (SubSystemBits *)&Registers[CONFIG].Value;
}
