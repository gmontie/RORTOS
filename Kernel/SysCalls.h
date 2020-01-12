#ifndef SYSTEM_CALLS_H
#define SYSTEM_CALLS_H

#include "Fops.h"

typedef void (*Fn)(void);

void Exec(Bus , Fn);
void Capture(Bus );
void Release(void);
void SystemCall(int);

#endif
