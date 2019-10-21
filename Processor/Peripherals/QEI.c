/**************************************************************************/
/*                                                                        */
/*  Programmer: Gregory L Montgomery                                      */
/*                                                                        */
/*                                                                        */
/*                                                                        */
/*  COPYING: (See the file COPYING for the GNU General Public License).   */
/*  this program is free software, and you may redistribute it and/or     */
/*  modify it under the terms of the GNU General Public License as        */
/*  published by the Free Software Foundation                             */
/*                                                                        */
/*  This program is distributed in the hope that it will be useful,       */
/*  but WITHOUT ANY WARRANTY; without even the implied warranty of        */
/*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                  */
/*  See the GNU General Public License for more details.                  */
/*                                                                        */
/*  You should have received a copy of the GNU General Public License     */
/*  along with this program. See the file 'COPYING' for more details      */
/*  if for some reason you did not get a copy of the GNU General Public   */
/*  License a copy can be obtained by write to the Free Software          */
/*  Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */
/*                                                                        */
/*  You are welcome to use, share and improve this program.               */
/*  You are forbidden to prevent anyone else to use, share and improve    */
/*  what you give them.                                                   */
/*                                                                        */
/**************************************************************************/
#include "System.h" // Contains the definitions for which chip is in use.

#ifdef PIC24F
   #include <p24Fxxxx.h>
#endif

#ifdef PIC24E
   #include <p24Exxxx.h>
#endif

#ifdef dsPIC33F
   #include <p33Fxxxx.h>
#endif

#ifdef dsPIC33E
   #include <p33Fxxxx.h>
#endif
#include <math.h>
#include "defs.h"
#include "QEI.h"

void StartQei(void);
void QEI_Update(void);

static RegisterFile * Register;

//typedef struct
//{
//    unsigned (*QEI)(void);
//    void (*StartQEI)(void);
//    Boolean ReadingReady;
//}QeiOps;

static QeiOps This =
{
    .Update = QEI_Update,
    .StartQEI = StartQei,
    .ReadingReady = False
};

void StartQei(void)
{
    QEI1CONbits.QEIEN = 1; // Enabled/Start QEI Module
}

QeiOps * QEI_Init(RegisterFile * UI)
{
    QEI1CONbits.QEIEN = 0; // Disable QEI Module
    QEI1CONbits.QEISIDL = 0; // Continue module operaton in idle mode
    QEI1CONbits.PIMOD = 0b001; // Every index event resets the position counter
    QEI1CONbits.IMV = 0; // Phase A/B match occurs when QEA/B == 0
    QEI1CONbits.INTDIV = 0; // 1:1 Prescale value
    QEI1CONbits.CNTPOL = 0; // Counter direction is positive unless modified by external up/down signal
    QEI1CONbits.GATEN = 0; // External gate signal does not affect position counter/timer operation
    QEI1CONbits.CCM = 0; // Quadrature Encode Interface (x4 mode) Count mode is selected

    QEI1IOCbits.QCAPEN = 0; // Positive edge detect of Home input does not trigger a position capture event
    QEI1IOCbits.FLTREN = 1; // Input pin digital filter is enabled
    QEI1IOCbits.QFDIV = 0; // 1:1 clock divide ration
    QEI1IOCbits.OUTFNC = 0; // QEI Module Output is disabed
    QEI1IOCbits.SWPAB = 0; // QEAx and QEBx are not swapped
    QEI1IOCbits.HOMPOL = 0; // Home Input is not inverted
    QEI1IOCbits.IDXPOL = 0; // Index Input is not inverted
    QEI1IOCbits.QEBPOL = 0; // QEB input is not inverted
    QEI1IOCbits.QEAPOL = 0; // QEA input is not inverted

    QEI1STATbits.PCHEQIEN = 0; // Position Counter >= Interrupt is Enabled
    QEI1STATbits.PCLEQIEN = 0; // Position Counter <= Interrupt is disabled
    QEI1STATbits.POSOVIEN = 0; // Position Counter Overflow disabled
    QEI1STATbits.PCIIEN = 0; // Position Counter (Homing) initialiation Process Compolete Interrupt disabled
    QEI1STATbits.VELOVIEN = 0; // Velocity Counter overflow Interrupt disabled
    QEI1STATbits.HOMIEN = 0; // Home Input Event Interrupt is disabled
    QEI1STATbits.IDXIEN = 0; // Index INput Event Interrupt disabled

    QEI1ICL = 4096;
    QEI1ICH = 0;

    QEI1LECL = 4096;
    QEI1LECH = 0;

    IFS3bits.QEI1IF = 0;
    IEC3bits.QEI1IE = 0;  //1; // Enable Interrupts

    Register = UI;
    
    return &This;
}

void QEI_Update(void)
{   int i = 0;
    Register[1].Value = POS1CNTL;
    Register[0].Value = POS1HLD; // POS1CNTH;
    //Register[2].Value = INDX1CNTH;
    //Register[3].Value = INDX1CNTL;
    while(i < 2)Register[i++].Stats.Changed = True;
}

void __attribute__ ((interrupt, no_auto_psv)) _QEI1Interrupt(void)
{
    if (POS1CNTH != 0)
    {
        if (POS1CNTH < 0)
        {
            POS1HLD = 0;
            POS1CNTL = 0; //4096;
        }
        else
        {
            POS1CNTH = 0;
            POS1HLD = 0;
            POS1CNTL = 4096;
        }
    }
    IFS3bits.QEI1IF = 0; // Clear Flag
}