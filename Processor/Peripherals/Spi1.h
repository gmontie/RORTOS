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
#ifndef SPI1_H
#define SPI1_H

#include "defs.h"
#include "Spi.h"
#include "Fops.h"

#define SpeedRdSPI1(x,y)do{  SPI1STATbits.SPIROV &= 0;  SPI1BUF=0;  while (!SPI1STATbits.SPIRBF);  if(SPI1STATbits.SPIRBF){SPI1STATbits.SPIROV &= 0;*x++=(byte)SPI1BUF;}}while(--y)
#define SpeedWrteSPI1(x,y) do{SPI1BUFF = *x++;while(!SPI1STATbits.SPITBF);}while(--y);

void * GetOps(void);

fops * InitSPI1(SPI_MODES);
void __attribute__((interrupt, auto_psv))_SPI1Interrupt(void);
void __attribute__((interrupt, no_auto_psv))_SPI1ErrInterrupt(void);
#endif
