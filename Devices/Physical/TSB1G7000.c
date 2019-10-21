#include "p24fxxxx.h"
#include "TSB1G7000.h"

//typedef struct tsb1g7000
//{
  //struct DisplayOps;
//  struct DisplayOps * (InitFn)(void);
//  void (*Clear)(void);
//
//  void (*Write)(char *);
//  void (*Lower)(char *);
//  void (*Display)(DisplayLine *);
//}TSB1G_7000;

static dsops Tsb1G7000 =
{
    .InitFn = NewLcd,
    .Clear = TSB1G7000_Clear,
    .IsBusy = IsTSB1G7000_Busy,
    .IsFree = IsTSB1G7000_Free,
    .Display = TSB1G7000_Display,
    .WriteUpper = TSB1G7000_WriteUpper,
    .WriteLower = TSB1G7000_WriteLower
};

struct DisplayOps * NewLcd(void)
{
    TSB1G7000_Init();
    return &Tsb1G7000;
}

/***********************************************************************/
/*  Function: LCDInit                                                  */
/*                                                                     */
/*  Input: None.                                                       */
/*  Output: None.                                                      */
/*                                                                     */
/*  Overview: This is a LCD intialising routine which should be called */
/*  before show the characters on LCD.                                 */
/*                                                                     */
/***********************************************************************/
void TSB1G7000_Init(void)
{
  unsigned int i;
	
  // PMCON  
  PMCONbits.PMPEN	= 1; // PMP enabled
  PMCONbits.PSIDL	= 0; // Continue module operation in Idle mode
  PMCONbits.ADRMUX	= 0; // Address and data appear on separate pins
  PMCONbits.PTBEEN	= 0; // Byte Enable Port disabled
  PMCONbits.PTWREN	= 1; // Write Enable Strobe Port enabled
  PMCONbits.PTRDEN	= 1; // Read/Write Strobe Port enabled
  PMCONbits.CSF		= 2; // PMCS1 and PMCS2 function as chip select
  PMCONbits.ALP		= 1; // Address Latch Polarity Active-high (PMALL and PMALH)
  PMCONbits.CS2P	= 1; // Chip Select 2 Polarity Active-high
  PMCONbits.CS1P	= 1; // Chip Select 1 Polarity Active-high
  PMCONbits.BEP		= 1; // Byte Enable Active-high
  PMCONbits.WRSP	= 1; // Master Mode, Write Strobe active-high
  PMCONbits.RDSP	= 1; // Master Mode, Read/write strobe active-high
	
  // PMMODE  
  PMMODEbits.IRQM	= 0;   // No interrupt generated
  PMMODEbits.INCM	= 0;   // No increment or decrement of address
  PMMODEbits.MODE16	= 0;   // 8-bit data mode
  PMMODEbits.MODE	= 3;   // Master mode 1(PMCSx, PMRD/PMWR, PMENB, PMBE, PMA<x:0> and PMD<7:0>)
  PMMODEbits.WAITB	= 3;   // Data wait of 4Tcy; multiplexed address phase of 4 Tcy
  PMMODEbits.WAITM	= 0xF; // Read to Byte Enable Strobe: Wait of additional 15 Tcy
  PMMODEbits.WAITE	= 3;   // Data Hold After Strobe: Wait of 4 Tcy
	
  // PMADDR
  PMADDR 	= 0x0000; // For LCD, there is no address, so zero is assigned.
	
  // PMAEN  
  PMAEN 	= 0x0001;// PMA15:2 function as port I/O, PMALH/PMALL enabled

  for (i = 0; i < 40000; i++);  
  Write_TSB1G7000(TSB1G7000_Instruction, DataLength8); // Set the default function, DL:8-bit
	
  for (i = 0; i < 80; i++);  
  Write_TSB1G7000(TSB1G7000_Instruction, DonCoffBoff); // Set the display control, turn on LCD
	
  for (i = 0; i < 40; i++);  
  Write_TSB1G7000(TSB1G7000_Instruction, CursorIncNS); // Set the entry mode, set cursor in increase mode
	
  for (i = 0; i < 40; i++);
  Write_TSB1G7000(TSB1G7000_Instruction, CursorNonShift);	// Set cursor shift, shift right
	
  for (i = 0; i < 400; i++);  
  TSB1G7000_Clear(); // Clear the display, clear display
}

/*********************************************************************/
/*                                                                   */
/* Function: LCDclear                                                */
/*                                                                   */
/*    Input: None                                                    */
/*   Output: None.                                                   */
/*                                                                   */
/* Overview: Clear the LCD Display                                   */
/*                                                                   */
/*********************************************************************/
void TSB1G7000_Clear( void )
{
  Write_TSB1G7000(TSB1G7000_Instruction, ClearDisplay);
}

/*********************************************************************/
/* Function: LCDProcessEvents                                        */
/*                                                                   */
/* Input: None.                                                      */
/* Output: None.                                                     */
/*                                                                   */
/* Overview: This is a state mashine to issue commands and data to   */
/* LCD. Must be called periodically to make LCD message processing.  */
/*                                                                   */
/*********************************************************************/
void TSB1G7000_Display(void * vPtr)
{	
    int i = 0;
    int j;
    char Ch;
    DisplayLine * Message = (DisplayLine *)vPtr;

    Write_TSB1G7000(TSB1G7000_Instruction, LCD_DDRAM1(0));
    for (j = 0; j < 250; j++); // Delay
    while(((Ch = Message->U[i]) != 0) && (i < CHARACTERS_PER_LINE))
    {
        for (j = 0; j < 30; j++); // Delay
        Write_TSB1G7000(TSB1G7000_Data, Ch);
    }

    for (j = 0; j < 250; j++); // Delay

    Write_TSB1G7000(TSB1G7000_Instruction, LCD_DDRAM2(0));
    for (j = 0; j < 250; j++); // Delay
    while(((Ch = Message->L[i]) != 0) && (i < CHARACTERS_PER_LINE))
    {
        for (j = 0; j < 30; j++); // Delay
        Write_TSB1G7000(TSB1G7000_Data, Ch);
    }
}

/*********************************************************************/
/*                                                                   */
/* Function:                                                         */
/*                                                                   */
/* Input: None                                                       */
/* Output: None.                                                     */
/*                                                                   */
/* Overview:                                                         */
/*                                                                   */
/*********************************************************************/
Boolean IsTSB1G7000_Busy( void )
{
    //PMADDR = TSB1G7000_Instruction;
    //return (PMDIN1 & 0x80);
    Boolean Results = False;

    PMADDR = TSB1G7000_Instruction;
    if((PMDIN1 & 0x80) != 0)
      Results = True;
    return Results;
}

/*********************************************************************/
/*                                                                   */
/* Function:                                                         */
/*                                                                   */
/* Input: None                                                       */
/* Output: None.                                                     */
/*                                                                   */
/* Overview:                                                         */
/*                                                                   */
/*********************************************************************/
Boolean IsTSB1G7000_Free( void )
{
    Boolean Results = False;

    PMADDR = TSB1G7000_Instruction;
    if((PMDIN1 & 0x80) == 0)
      Results = True;
    return Results;
}

/*********************************************************************/
/*                                                                   */
/* Function: LCDwrite                                                */
/*                                                                   */
/* Input: The address ie inputType, and the Character to display on  */
/*        the LCD                                                    */
/* Output: None.                                                     */
/*                                                                   */
/* Overview: Write a single character to the LCD display             */
/*                                                                   */
/*********************************************************************/
void Write_TSB1G7000(const unsigned char DataType, const unsigned char Data)
{
  PMADDR = DataType;
  PMDIN1 = Data;
}

/***********************************************************************/
/* Function: LCDwriteLine                                              */
/*                                                                     */
/* Overview: Write a message to the LCD Display                        */
/*                                                                     */
/* Input: Which Line of the LCD to write to, and a pointer to          */
/*        the beggining of the message to be written.                  */
/* Output: None.                                                       */
/*                                                                     */
/***********************************************************************/
void TSB1G7000_WriteUpper(const char * Msg)
{
    int i = 0;
    int j;
    Write_TSB1G7000(TSB1G7000_Instruction, LCD_DDRAM1(0));
    for (j = 0; j < 250; j++); // Delay
    while((Msg[i] != 0) && (i < CHARACTERS_PER_LINE))
    {
        for (j = 0; j < 30; j++); // Delay
        Write_TSB1G7000(TSB1G7000_Data, Msg[i++]);
    }
}

void TSB1G7000_WriteLower(const char * Msg)
{
    int i = 0;
    int j;
    Write_TSB1G7000(TSB1G7000_Instruction, LCD_DDRAM2(0));
    for (j = 0; j < 250; j++); // Delay
    while((Msg[i] != 0) && (i < CHARACTERS_PER_LINE))
    {
        for (j = 0; j < 30; j++); // Delay
        Write_TSB1G7000(TSB1G7000_Data, Msg[i++]);
    }
}
