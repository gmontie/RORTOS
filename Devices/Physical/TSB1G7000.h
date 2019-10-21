#ifndef TSB1G_7000_H
#define TSB1G_7000_H

#include "DisplyDev.h"

#define DISPLAY_LINES           2
#define CHARACTERS_PER_LINE    16
#define LCD_LINE1		0    // first line
#define LCD_LINE2		1    // second line

// Layout
//  0  1  2  3  4 ...  15 <- First Display Line
// 16 17 18 19 20 ...  31 <- Next and Last Display Line
//typedef DisplayLine[DISPLAY_LINES][CHARACTERS_PER_LINE];

//typedef DisplayLine char a[DISPLAY_LINES][CHARACTERS_PER_LINE];
typedef struct
{
    char U[CHARACTERS_PER_LINE];
    char L[CHARACTERS_PER_LINE];
}DisplayLine;

//typedef struct
//{
//   struct DisplayOps Ops;
//   void (*Display)(const DisplayLine *);
//}TSB1G7000;

#define TSB1G7000_Instruction	0    // instruction
#define TSB1G7000_Data		1    // data
#define TSB1G7000_IsBusy	0x80 // LCD is busy
#define TSB1G7000_IsFree	0    // LCD is free

#define ClearDisplay	0x01 // clear display
#define ReturnHome		0x02 // move cursor back to 00h position
#define CursorIncNS		0x06 // assign cursor as increase mode but screen not shift
#define CursorDecNS		0x04 // assign cursor as decrease mode but screen not shift
#define CursorIncS		0x07 // assign cursor as increase mode and screen shift
#define CursorDecS		0x05 // assign cursor as decrease mode adn screen shift
#define DonConBoff		0x0e // Display on, Cursor on, Blinking of Cursor off
#define DonConBon		0x0f // Display on, Cursor on, Blinking of Cursor on
#define DonCoffBoff		0x0c // Display on, Cursor off, Blinking of Cursor off
#define DoffCoffBoff	0x08 // Display off, Cursor off, Blinking of Cursor off
#define CursorShiftL	0x1c // Cursor shift enabled, shift right
#define CursorShiftR	0x18 // Cursor shift enabled, shift left
#define CursorNonShift	0x10 // Cursor shift disabled
#define DataLength4		0x20 // 4-bit data length
#define DataLength8		0x3C // 8-bit data length

// set 4-bit CGRAM address
#define LCD_CGRAM(Address)  (((Address) & 0x0F) | 0x40)
// set 7-bit DDRAM address, line 1
#define LCD_DDRAM1(Address) (((Address) & 0x0F) | 0x80)
// set 7-bit DDRAM address, line 2
#define LCD_DDRAM2(Address) (((Address) & 0x0F) | 0xC0)

struct DisplayOps * NewLcd(void);
void TSB1G7000_Init(void);
void TSB1G7000_Clear(void);
Boolean IsTSB1G7000_Busy( void );
Boolean IsTSB1G7000_Free(void);
void TSB1G7000_Display(void *);
void Write_TSB1G7000(const unsigned char DataType, const unsigned char Data);
void TSB1G7000_WriteUpper(const char *);
void TSB1G7000_WriteLower(const char *);
#endif
