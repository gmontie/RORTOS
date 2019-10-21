#include "Registers.h"
#include "defs.h"
#include "LM35.h"

//extern SENSOR * const SensorArray[NUMBER_OF_SENSORS];
extern bank0 byte Register[REGISTER_MEMORY_SIZE]; 

void UpdateLM35Reading( void )
{
  byte   i;
  float Voltage;

  for(i = 0; i < NUMBER_OF_SENSORS; i++)
  {
    if((Register[SENSOR_BIT_FIELD] & (1 << i)) && 
       (Register[SENSOR_BIT_FIELD] & (1 << (i + 4))))
    {
      // Voltage = m x + b
      // m is VOLTS_PER_COUNT
      // x is ADC Counts
      // b is 0 because we have a zero offset
      //Voltage = VOLTS_PER_COUNT * SensorArray[i].AD_Counts;
      //SensorArray[i].Temperature = Voltage / VOLTAGE_PER_DEGREE_C;
    }
  }
}

byte GetTempReading(void)
{
  byte   i;
  int TempSum = 0;
  byte SumCount = 0;
  byte Results = 0;

  for(i = 0; i < NUMBER_OF_SENSORS; i++)
  { // Sensor is installed
    if((Register[SENSOR_BIT_FIELD] & (1 << i)) && 
       // Sensor reading is valid
       (Register[SENSOR_BIT_FIELD] & (i << (i+4))))
    {
      //TempSum += (int)SensorArray[i].Temperature;
      //SumCount++;
    }
  }
  return (byte)(TempSum / SumCount);
}
