/* 
 * File:   QEI.h
 * Author: glm
 *
 * Created on February 11, 2014, 12:49 PM
 */

#ifndef QEI_H
#define	QEI_H

#include "defs.h"
#include "RegisterElement.h"
#include "Registers.h"

#ifndef	__cplusplus

typedef struct
{
    void (*Update)(void);
    void (*StartQEI)(void);
    Boolean ReadingReady;
}QeiOps;

QeiOps * QEI_Init(RegisterFile * UI);
#else
extern "C" {
#endif



#ifdef	__cplusplus
}
#endif

#endif	/* QEI_H */

