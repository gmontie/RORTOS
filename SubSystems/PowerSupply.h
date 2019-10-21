#ifndef POWER_SUPPLY_H
#define POWER_SUPPLY_H

#include "Register.h"
#include "Registers.h"

typedef struct
{
    void (*UpdatePowerSupply)(void);
    void (*CheckUserSettings)(void);
}PS;

PS * InitPowerSupply(Register *, Device * Vm);

#endif