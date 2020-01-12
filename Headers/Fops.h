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
#ifndef CAR_DEVICE_H
#define CAR_DEVICE_H

#include "defs.h"

// Standard IOCTL calls
#define FP_RESET          1
#define READ_STATUS       2
#define WRITE_STATUS      3
#define WRITE_ENABLED     4
#define BUSY              5
#define ENABLE_WRITE      6
#define GET_PAGE_SIZE     7
#define GET_MEM_SIZE      8
#define GET_NBR_OF_PAGES  9

#define I2C_BRG	(((Fcy / 2 / Fsck) - 1) >> 1) // 333Khz

// Define some fundamental enumerated types for use with Serial Operations
// Objects.
typedef enum{_NONE_=0, _I2C_1, _I2C_2, _I2C_3, _SPI_1, _SPI_2, _SPI_3, _Uart_1, _Uart_2}Bus;

#define LAST_BUS _Uart_2

typedef struct FileOperations
{
  Bus         Resource;
  unsigned    Status;
  Boolean   (*BlockWrite)(unsigned, byte *, byte); // Address, BufferPtr, Length
  Boolean   (*BlockRead)(unsigned, byte *, byte); // Address, BufferPtr, Length  
  byte      * IndexPtr;
  int       (*IOCTL)(unsigned);
  void      (*Flush)(void);      // Flush the buffers
  void      (*PutCh)(unsigned);  // Put a character to a file
  int       (*GetCh)(void);     // Read a character from a file
  unsigned  (*GetWd)(void);     // 
  void      (*PutStr)(char *);  // Put a string of character out to the file
  void      (*PutCnst)(const char *);
  void      (*Reset)(void);     // Reset the peripheral
  Boolean   (*Available)(void); // Indicate if a reading or results is ready
  void      (*Release)(void);
  void       *Usr; // User Structure to include.
}fops;
#endif
