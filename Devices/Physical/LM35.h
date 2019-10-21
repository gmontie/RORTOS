#ifndef LM35_H
#define LM35_H
#include "defs.h"
// 1.28 volts full scale
// This means that for each ADC count we have 
// 
#define VOLTS_PER_COUNT 1.28 / 1024.0
#define VOLTAGE_PER_DEGREE_C 0.01

void UpdateLM35Reading( void );
byte GetTempReading(void);
#endif
