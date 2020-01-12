#ifndef FACTORY_SETTINGS_H
#define FACTORY_SETTINGS_H

#include "Registers.h"
#include "Fops.h"
#include "NVRegisters.h"

Service * NewFacortySettings(fops * IO, NVRegisters * ROM_Manager, const unsigned * NbrPtr, unsigned MemoryIndex, int Length);

#endif