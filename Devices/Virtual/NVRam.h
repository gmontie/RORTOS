/**************************************************************************/
/*                                                                        */
/*  Programmer: Gregory L Montgomery                                      */
/*                                                                        */
/*                                                                        */
/*                                                                        */
/**************************************************************************/
#ifndef NV_RAM_H
#define NV_RAM_H

#include "defs.h"
#include "Register.h"
#include "Registers.h"
#include "NVRegisters.h"
#include "Device.h"

Device * NewRom(Register * MemorySubSys, NVRegisters * NVInterface, Device * IO);

#endif
