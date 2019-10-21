/**************************************************************************/
/*                                                                        */
/*  Programmer: Gregory L Montgomery                                      */
/*                                                                        */
/*                                                                        */
/*                                                                        */
/*                                                                        */
/**************************************************************************/
#ifndef QUANTUM_H
#define QUANTUM_H

#include "defs.h"
#include "Registers.h"
#include "Register.h"

#define MEASURED 20670 // Measured frequency of Scheduler interrupt timer
#define QUANTUM   4 // Counts per second
#define Q_COUNTS (MEASURED / QUANTUM)

typedef struct
{
    Register  *        CounterPtr;
    Register  *        MaxCountPtr;
    int            Counter;
    int            ID;
    Boolean        Used;
}CounterData;

typedef struct Counter_ops
{
    CounterData *  DataPtr;
    Boolean        (*Clear)(int); // int Id
    void           (*Inc)(void *);
    void           (*Dec)(void *);
    Boolean        (*Rdy)(void);
}Quantum;

Quantum * NewQuantum(Register * Location, Register * Max);

#endif
