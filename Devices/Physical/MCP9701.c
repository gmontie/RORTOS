#include "MCP9701.h"
#include "Sensor.h"
#include "Device.h"

#define MCP9701_DISCRIPTION "Analog Temperature Sensor"

SenDev * InitMCP9701(RegisterFile * Mem, AdOps * Adc, byte Index)
{
    SensorClass aw;
    SenDev * Device;// = NewSensor(Mem, Analog, aw);
    aw.AnCl = _MCP9701;
    Device = NewSensor(Mem, Analog, aw);
    Device->Discription = MCP9701_DISCRIPTION;
    Adc->SetADCSlot(&Device->Reading, Index);
    Device->Element = Mem;
    //Device->Readings = &Readings;
    Device->Ops.UpDate = DevUpDate;
    Device->Ops.IsReady = DevIsReady;
    return Device; 
}

