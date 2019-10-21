#ifndef NV_SETTINGS_SUB_SYSTEM_H
#define NV_SETTINGS_SUB_SYSTEM_H

#include "Fops.h"
#include "Register.h"
#include "SubSystem.h"

SubSystem * InitNvSubSystem(const unsigned * , Register * , fops * , int Start, int Length);

#endif
