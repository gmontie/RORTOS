/****************************************************************************/
/*                                                                          */
/*  Programmer: Gregory L Montgomery                                        */
/*                                                                          */
/*  Copyright © 2010-2019                                                   */
/*                                                                          */
/*  COPYING: (See the file COPYING.md for the GNU General Public License).  */
/*  this program is free software, and you may redistribute it and/or       */
/*  modify it under the terms of the GNU General Public License as          */
/*  published by the Free Software Foundation                               */
/*                                                                          */
/* This file is part of Gregory L Montgomery's code base collection Project.*/
/*                                                                          */
/*     Gregory L Montgomery's code base collection Project is free software:*/
/*     you can redistribute it and/or modify  it under the terms of the GNU */
/*     General Public License as published by the Free Software Foundation, */
/*     either version 3 of the License, or (at your option)                 */
/*     any later version.                                                   */
/*                                                                          */
/*     Gregory L Montgomery's code base collection Project is distributed   */
/*     in the hope that it will be useful, but WITHOUT ANY WARRANTY;        */
/*     without even the implied warranty of MERCHANTABILITY or FITNESS FOR  */
/*     A PARTICULAR PURPOSE.  See the GNU General Public License for more   */
/*     details.                                                             */
/*                                                                          */
/*     You should have received a copy of the GNU General Public License    */
/*     along with Gregory L Montgomery's code base collection Project.      */
/*     If not, see <https://www.gnu.org/licenses/>.                         */
/*                                                                          */
/****************************************************************************/
#ifndef SPI_H
#define SPI_H

#include "defs.h"

#define AsMaster True
#define AsSlave  False

typedef enum
{
	MODE_0 = 0, MODE_1, MODE_2, MODE_3
} SPI_MODES;

typedef struct SpiOperations 
{	
	SPI_MODES CurrentMode;
	void      (*IntModeOn)(Boolean); // User interrupts (true) none (false)
	void      (*Set16BitMode)(Boolean ); // 16 bit mode = true, 8 bit mode = false
    Boolean   (*Get16BitMode)(void);
	void      (*SelectSPI1Mode)(SPI_MODES); // Which mode to use
	void      (*SelectPrimaryPrescale)(byte);
    void      (*SelectSecondPrescale)(byte);
	void      (*Enable)(Boolean);
    void      (*Reset)(void);
    void      (*Flush)(void);
    int       (*Read)(void);
    void      (*Write)(unsigned);
	Boolean   (*BlockRead)(byte *, byte);
	Boolean   (*BlockWrite)(byte *, byte);
} SpiOps;

#endif
