#ifndef VOLT_METER_SUB_SYSTEM_H
#define VOLT_METER_SUB_SYSTEM_H

#include "SubSystem.h"
#include "PWM.h"
#include "ADConverter.h"
#include "Register.h"
#include "Registers.h"
#include "Adc.h"

#define MAX_SETTING V12
#define MIN_SETTING V1
#define MID_SETTING V6
typedef enum{COUNTS_MAX, COUNTS_MIN, VOLTAGE_MAX, VOLTAGE_MIN}CalTypes;

SubSystem * InitVoltMeter(Register * UI, AdOps * ChipsADC, Device * PwerControl);

#endif
