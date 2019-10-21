#ifndef MUX_H
#define MUX_H

#include "defs.h"
#include "Registers.h"
#include "Register.h"
#include "Device.h"

#define MAX_ADRS 0x07

Device * NewMux(Register * MuxRegister);

#endif