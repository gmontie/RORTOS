/******************************************************************************
 * 
 * Degree Controls, Inc.
 *
 * Milford, New Hampshire, USA.
 *
 * This source code is the property of Degree Controls, Inc. Any duplication
 * is strictly prohibited by law.
 *
 * Filename: Timers.c
 *
 * Project: Ametek Fan Tray Controller
 *
 * Written by: Greg Montgomery, adapted from 
 *                              Eric Zweighaft
 *                                   and
 *                              Savitha Patil
 *
 * Date: 12/10/2007
 *
 * Revision History:  
 * Copied Templet from pre-existing files...
 *
 */
#ifndef VT100_H
#define VT100_H

//#include <LPC213x.h>
#include "defs.h"

#define SET__POS    0
#define FORCEPOS    1
#define SETATTESC   2
#define BLINK       3
#define UNDERLINE   4
#define INVERSE     5
#define EEOL        6
#define EEOL0       7
#define EBLN        8
#define ELINE       9
#define EES        10
#define EES0       11
#define EBS        12
#define ES         13
#define CRSUP      14
#define CRSDOWN    15
#define CRRIGHT    16
#define CRLEFT     17
#define NXTLN      18
#define ATTOFF     19
#define ATTOFF0    20
#define HIGHTLIGHT 21

#define RESET_ATTRIBUTES 0
#define BRIGHT           1
#define DIM              2
#define UNDERSCORE       4
//#define BLINK            5
#define REVERSE          7
#define HIDDEN           8

#define NULL_CHAR           0x00   // Null Character
#define BELL_SOUND          0x07
#define BACK_SPACE          0x08
#define H_TAB               0x09   // Horizontal Tab
#define LINE_FEED           0x0A
#define V_TAB               0x0B   // Vertical Tab
#define FORM_FEED           0x0C
#define RETURN              0x0D
//#define CR                  RETURN // This is a Carriage Return
#define DLE                 0x10   // This is Data Link Escape
#define ESC                 0x1B   // Escape Character
#define SPACE               0x20   // Beginning of printable Characters
#define DELETE              0x7F   // This is the Delete Key
#define ONLY_PRINTABLE      0x7F   // This is 127 Dec. This will force characters 
                                   // to be only printable.

#define fBLACK             30
#define fRED               31
#define fGREEN             32
#define fYELLOW            33
#define fBLUE              34
#define fMAGENTA           35
#define fCYAN              36
#define fWHITE             37

#define bBLACK             40
#define bRED               41
#define bGREEN             42
#define bYELLOW            43
#define bBLUE              44
#define bMAGENTA           45
#define bCYAN              46
#define bWHITE             47

#define MOVE_TO 'a'

/*
 * 	 Text modes
 */
#define MODE_NONE         '0'
#define MODE_BOLD         '1'
#define MODE_DIM          '2'
#define MODE_UNDERLINE    '4'
#define MODE_BLINK        '5'
#define MODE_REVERSED     '7'
#define MODE_CONCEALED    '8'


void ClearAttributes( void );
void Bell( void );
void SetPosition(int , int );
void SetColor(int , int );
void ClrLine( void );
void ResetVT( void );
void SetAttribute(char );
void InverseVidio( void );
void TestSet( void );

#endif
