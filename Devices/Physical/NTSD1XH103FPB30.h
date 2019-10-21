
#ifndef NTSD1XH103FPB30_H
#define NTSD1XH103FPB30_H

#include "Device.h"
#include "System.h"
#include "Register.h"

typedef struct _therm_table
{
   int       Temperature;
   uint32_t  Resistence;
   uint32_t     AjdTemp; // Adjusted Temperature is 64k * Temperature
}ThermTable;

typedef struct _Thermister
{
   unsigned     Id;
   Boolean      Used;
   Register  *  Counts; // AD Reading
   Register  *  Temperature; // Output Temperature Calculation
   Boolean      State;
}Thermister;

Device * NewTherm(Register * Counts, Register * Temp);
void Test(void);
#endif