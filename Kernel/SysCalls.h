#ifndef SYSTEM_CALLS_H
#define SYSTEM_CALLS_H

#include "Fops.h"

// The blocked Index's for Processes 
// Blocked on IO
#define UART1Rdy     0
#define FTDONE       1
#define ANALOG       2
#define CALIBRATING  3
#define ROM_RDY      4
#define UART2Rdy     5
#define CONFIGDrty   6

#define BlckdSize    7

typedef void (*Fn)(void);

void Exec(Bus , Fn);
void WaitOn(unsigned);
void Capture(Bus );
void Release(void);
void SystemCall(int);

#endif