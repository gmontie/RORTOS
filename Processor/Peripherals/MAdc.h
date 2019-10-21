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
#ifndef MADC_H
#define MADC_H

#include "defs.h"
#include "Register.h"
#include "Registers.h"

// Base 2 Math starts here
#define DOTPOS_NONE          5 // ADCShortToString will not show dot
#define DOTPOS_TRAIL_ZEROS  -1 // ADCShortToString will not show dot and fill front space with zeros
#define SHIFTS               5 // 2^5 => 32 which is our divisor
#define NUMBER_OF_SAMPLES    (1 << SHIFTS) // Total number of samples

/*****************************************************************************/
/*                                                                           */
/* The AdOps structure defines an Analog to Didital Converter like Object    */
/* which can be used by sensors and other Objects. It can also stand alone.  */
/*                                                                           */
/*****************************************************************************/
typedef struct
{
    unsigned (*ADC)(void);        // Return the ADC reading
    void (*StartADC)(void);       // Start ADC Peripheral
    void (*StartReading)(void);   // Start DMA readings for the ADC
    void (*Sort)(void);
    void (*NextADCReading)(void); // Calculate the next ADC reading
    Boolean ReadingsReady;         // Indicate if the next reading is ready.
}AdOps;

AdOps * ADCInit(Register * UI); // Initialize the ADC Object.
#endif
