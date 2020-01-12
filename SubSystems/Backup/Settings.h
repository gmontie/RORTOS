#ifndef SETTINGS_H
#define SETTINGS_H

#include "Registers.h"
#include "Device.h"
#include "Settings.h"

Service * InitSettings(NVRegisters  * RomDev, unsigned * Rev, unsigned RevLength);

#endif