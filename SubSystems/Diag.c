#include <xc.h>
#include "FeedBack.h"
#include "Register.h"

#define DISCRIPTION_DIAG_THREAD "Diagnostic Thread"

static Service     * FbCounterUp;
static Service     * FbCounterDn;

/****************************************************************************/
/*                                                                          */
/*     Function:                                                            */
/*        Input: None                                                       */
/*       Return: None                                                       */
/*     Overview:                                                            */
/*                                                                          */
/****************************************************************************/
static Boolean DiagReady(void){ return True; }

/****************************************************************************/
/*                                                                          */
/*     Function: upDate                                                     */
/*        Input: None                                                       */
/*       Output: None                                                       */
/*     Overview: This thread checks the Uart IO and updates the shared      */
/*               register space.                                            */
/*                                                                          */
/*                                                                          */
/*                                                                          */
/****************************************************************************/
static void upDate(void) 
{   // Update Feeb back units.
    FbCounterUp->Device(FbCounterUp);
    FbCounterDn->Device(FbCounterDn);
}

/****************************************************************************/
/*                                                                          */
/*     Function: InitComm Thread                                            */
/*        Input: A pointer to the shared memory (registers)                 */
/*       Output: A pointer to a newly allocated Devcie Object               */
/*     Overview: Receives one character of information in from the UART     */
/*                                                                          */
/****************************************************************************/
Service * InitDiag(void) 
{
    Service * Results = 0;
    //Register * Registers = getRegisterSet();

    if ((Results = NewDevice(0)) != 0) 
    {
        // Instantiate Register based Diagnostic Process
        // 
        FbCounterUp = FbInit( FEEDBC1, MAX_FB1, INC );
        FbCounterDn = FbInit( FEEDBC2, MAX_FB2, DEC );
        Results->Discription = DISCRIPTION_DIAG_THREAD;
        Results->IsReady = DiagReady;
        Results->Peek = 0;
        Results->Poke = 0;
        Results->Read = 0;
        Results->Id = ( 0x0101 + ((int) USER_THREAD) * 0x20 ), Results->DeviceType = USER_THREAD;
        Results->FnType = ProcessFn;
        Results->Thread = upDate;
        Results->Dev = Results;
    }
    return Results;
}

