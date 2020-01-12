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
#ifndef DEFS_H
#define DEFS_H

#include <stdint.h>

#ifndef FALSE
  #define FALSE 0
  #ifndef FAIL
    #define FAIL  0
    #define FAILURE 0
    #define OFF  0
    #ifndef TRUE
      #define TRUE !FALSE
      #ifndef SUCCESS
        #define SUCCESS !FAILURE
      #endif
      #ifndef ON
        #define ON !OFF
      #endif
    #endif
  #endif
#endif

#define HI 1
#define LO 0
#define Yes 1
#define No  0
#ifndef true
  #define true 1
  #ifndef false
    #define false 0
  #endif
#endif

#define NULL_CHAR           0x00   // Null Character
#define BACK_SPACE          0x08
#define LINE_FEED           0x0A
#define FORM_FEED           0x0C
#define RETURN              0x0D
#define CR                  RETURN // This is a Carriage Return
#define ESC                 0x1B   // Escape Character
#define SPACE               0x20   // Beginning of printable Characters
#define DELETE              0x7F   // This is the Delete Key

// BYTE type definition
#ifndef _BYTE_DEF_
#define _BYTE_DEF_
typedef unsigned char byte;
#endif   /* _BYTE_DEF_ */

#ifndef Boolean
typedef enum{False = false, True}Boolean;
#endif

#define MAX_VAL(x,y) (x > y) ? (x) : (y)
#define MIN_VAL(x,y) (x < y) ? (x) : (y)

#define XBty(x) (x & 0x00FF)
#define Lo(x)   (byte)(x & 0x00FF)
#define Hi(x)   (byte)((x >> 8) & 0x00FF)

#define LOOPING_FOREVER True

typedef union ichar
{
  unsigned short  i;
  unsigned char  Ch[ sizeof(unsigned short) ];
}Short_2_Char;

typedef union 
{
  unsigned        i;
  unsigned char  Ch[ sizeof(unsigned) ];
}Unsigned_2_Char;

typedef union
{
   struct
   {
      uint16_t Denominator;
      uint16_t Numerator;
   };
   uint32_t Result;
}Frational;

#define IO_POWER_OF_2     6
#define IO_BUFFER_SIZE   64
#define IO_RAPAROUND     63
#define BUFFER_EMPTY     -1

#define DEVICE_EMPTY -1

// Circular buffer definition
typedef struct
{
  byte Head;
  byte Tail;
  byte Count;
  byte Buffer[IO_BUFFER_SIZE];
}Queue;

// Circular buffer definition
typedef struct
{
  byte Head;
  byte Tail;
  byte Count;
  unsigned Buffer[IO_BUFFER_SIZE];
}iQueue;

#endif
