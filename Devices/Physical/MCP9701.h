#ifndef MCP9701_H
#define MCP9701_H

#include "defs.h"
#include "Sensor.h"
#include "Adc.h"

SenDev * InitMCP9701(RegisterFile *,AdOps *, byte);

#endif
