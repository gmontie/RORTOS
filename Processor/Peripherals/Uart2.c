#include <p24Fxxxx.h>
#include <stdio.h>
#include <string.h>
#include "defs.h"
#include "CharDev.h"
#include "System.h"
#include "Uart2.h"

/*****************************************************************************
 * U2BRG register value and baudrate mistake calculation
 *****************************************************************************/
// Start Here #define BAUDRATEREG2 SYSCLK/32/BAUDRATE2 - 1
// Note1 Transform#define BUADRATEREG2 (SYSCLK/32)*(1/BAUDRATE2) - 1
//#define BAUDRATEREG2 (SYSCLK/(32 * BAUDRATE2)) - 1 // Final Eq...
//#define BAUD_RATE   ((SYSCLK/BAUDRATE) >> 5) - 1

//#if BAUDRATEREG2 > 255
//#error Cannot set up UART2 for the SYSCLK and BAUDRATE. Correct values in main.h and uart2.h files.
//#endif

//#define BAUDRATE_MISTAKE 1000 * (BAUDRATE2 - SYSCLK/32/(BAUDRATEREG2 + 1))/BAUDRATE2
//#if (BAUDRATE_MISTAKE > 2)||(BAUDRATE_MISTAKE < -2)
//#error UART2 baudrate mistake is too big  for the SYSCLK and BAUDRATE2. Correct values in uart2.c file.
//#endif 
static Queue Uart2Tx;
static Queue Uart2Rx;
static Boolean Uart2Txing;

static volatile unsigned short TimeOut;

//      struct FileOperations * (*InitFn)(void);
//      void (*Flush)(void) ;
//      void (*PutCh)(char);
//      int  (*GetCh)(void);
//      void (*PutStr)(char *);
//      Boolean (*Available)(void);
//      byte * IndexPtr;

static fops Uart2Device =
{
   .InitFn = InitUart2,
   .Flush = Uart2Reset,
   .PutCh = UART2PutChar,
   .GetCh = Uart2GetChar,
   .PutStr = Uart2WriteStr,
   .Available = Uart2CharAvail,
   .IndexPtr = &Uart2Rx.Count
};

struct FileOperations * InitUart2(void)
{
    UART2Init(38400);
    return &Uart2Device;
}
/*****************************************************************************
 * Function: UART2Init
 *
 * Precondition: None.
 *
 * Overview: Setup UART2 module.
 *
 * Input: None.
 *
 * Output: None.
 *
 *****************************************************************************/
void UART2Init( long BaudRate )
{
  Uart2PinConfig();
  // Set directions of UART IOs
  //U2BRG = BAUDRATEREG2;
  U2MODE = 0;
  U2STA = 0;
  U2MODEbits.UARTEN = 1;
  U2STAbits.UTXEN = 1;
  // reset RX flag
  IFS1bits.U2RXIF = 0;

  Uart2Reset();

  if(BaudRate == 0)
    Uart2SetBaudRate(38400);
  else
    Uart2SetBaudRate(BaudRate);
}

/****************************************************************************/
/*     Function: Uart2 Pint Configuration                                   */
/*                                                                          */
/*        Input: None                                                       */
/*       Output: None                                                       */
/* Side Effects: None                                                       */
/*     Overview: Initialize Hardware, and registers of the microcontroller  */
/*               for Uart 2.                                                */
/* Notes:                                                                   */
/*                                                                          */
/****************************************************************************/
void Uart2PinConfig(void)
{
  //U2RX = RP19 Pin 49 is Uart2 Rx
  RPINR19bits.U2RXR = 10;

  //RP25 = U2TX Pin 50 is Uart2 Tx
  RPOR8bits.RP17R = 5; //U2TX_IO;
}

/****************************************************************************/
/*     Function: Initialize varivariables                                   */
/*                                                                          */
/*        Input: None                                                       */
/*       Output: None                                                       */
/* Side Effects: None                                                       */
/*     Overview: Transmit one character of information out of UART 1        */
/*                                                                          */
/* Notes:                                                                   */
/*                                                                          */
/****************************************************************************/
void Uart2Reset( void )
{
  memset((char*)&Uart2Tx, 0, sizeof(Queue));
  memset((char*)&Uart2Rx, 0, sizeof(Queue));
  Uart2Txing = False;
}


/****************************************************************************/
/*     Function: Uart2_SetBaudRate                                          */
/*                                                                          */
/*        Input: The desired baud rate.                                     */
/*       Output: None                                                       */
/* Side Effects: Changes the hardware baud rate for uart 1                  */
/*                                                                          */
/* Notes:                                                                   */
/*                                                                          */
/****************************************************************************/
#define SYSCLK 8000000
void Uart2SetBaudRate(unsigned int Baud)
{
  //int Temp0 = SYSCLK / Baud;
  int Temp0 = SystemClock / Baud;
  int Temp1 = Temp0 >> 5;
  Temp1--;
  U2BRG = 16;//Temp1;
}

/****************************************************************************/
/*                                                                          */
/*     Function: Transmit one character out of Uart 0 without using the     */
/*               Que... this is a raw transmit function                     */
/*        Input: The Character to be transmitted                            */
/*       Return: None                                                       */
/*                                                                          */
/****************************************************************************/
void  UART2PutChar(char Ch)
{
    // wait for empty buffer  
    while(U2STAbits.UTXBF == 1);
      U2TXREG = Ch;
}

/****************************************************************************/
/*     Function: PutChar                                                    */
/*                                                                          */
/*        Input: One character to be sent out of the UART one               */
/*       Output: None                                                       */
/* Side Effects: None                                                       */
/*     Overview: Transmit one character of information out of UART 1        */
/*                                                                          */
/* Notes:                                                                   */
/*                                                                          */
/****************************************************************************/
void Uart2PutChar(char Ch)
{	
  if(Uart2Tx.Count < IO_BUFFER_SIZE)
  {
    Uart2Tx.Buffer[Uart2Tx.Tail++] = Ch;
    Uart2Tx.Tail &= (IO_RAPAROUND);
    Uart2Tx.Count++;
  }

  if(!Uart2Txing)
  {
    Uart2Txing = YES;
    U2STAbits.UTXEN = 1;
    U2TXREG = Ch;
  }
}

Boolean Uart2CharAvail(void)
{
    return ((Uart2Rx.Count == 0) ? False : True);
}
/****************************************************************************/
/*     Function: GetChar                                                    */
/*                                                                          */
/*        Input: None                                                       */
/*       Output: The next character available, or an error code indicating  */
/*               that there are no characters available.                    */
/* Side Effects: None                                                       */
/*     Overview: This function gets the next character from the internal    */
/*               uart buffer.                                               */
/* Notes:                                                                   */
/*                                                                          */
/****************************************************************************/
int Uart2GetChar( void )
{
  unsigned int Return = DEVICE_EMPTY;

  if(Uart2Rx.Count > 0)
  {
    Return = Uart2Rx.Buffer[Uart2Rx.Head++];
    Uart2Rx.Head &= (IO_RAPAROUND);
    Uart2Rx.Count--;
    if(!Uart2Rx.Count)
      Uart2Rx.ErrorStatus = BUFFER_EMPTY; //NO_CHARACTER_AVAILABLE;
  }
  return Return;
}

/****************************************************************************/
/*     Function: GetChar                                                    */
/*                                                                          */
/*        Input: None                                                       */
/*       Output: The next character available, or an error code indicating  */
/*               that there are no characters available.                    */
/* Side Effects: None                                                       */
/*     Overview: This function gets the next character from the internal    */
/*               uart buffer.                                               */
/* Notes:                                                                   */
/*                                                                          */
/****************************************************************************/
void Uart2WriteStr(char * BufferPtr)
{
  //int i = 0;
  char Ch = 0;
  while((Ch = *BufferPtr++))
  {
    if(Uart2Tx.Count < IO_BUFFER_SIZE)
    {
      Uart2Tx.Buffer[Uart2Tx.Tail++] = Ch;
      Uart2Tx.Tail &= (IO_RAPAROUND);
      Uart2Tx.Count++;
    }

    if(!Uart2Txing)
    {
      Uart2Txing = YES;
      U2STAbits.UTXEN = 1;
      U2TXREG = Ch;
    }
  }
}

/****************************************************************************/
/*     Function: GetChar                                                    */
/*                                                                          */
/*        Input: None                                                       */
/*       Output: The next character available, or an error code indicating  */
/*               that there are no characters available.                    */
/* Side Effects: None                                                       */
/*     Overview: This function gets the next character from the internal    */
/*               uart buffer.                                               */
/* Notes:                                                                   */
/*                                                                          */
/****************************************************************************/
void Uart2Write (unsigned char * BufferPtr, unsigned char Length)
{
  int i = 0;

  while(i < Length)
  {
    if(Uart2Tx.Count < IO_BUFFER_SIZE)
    {
      Uart2Tx.Buffer[Uart2Tx.Tail++] = *BufferPtr;
      Uart2Tx.Tail++;
      Uart2Tx.Tail &= (IO_RAPAROUND);
      Uart2Tx.Count++;
    }
    i++;
  }
}

/****************************************************************************/
/*     Function: Read                                                       */
/*                                                                          */
/*        Input: A Pointer to the Buffer to place the data into, and        */
/*               The number of characters to be transferred                 */
/*       Output: The amount of characters actually read                     */
/* Side Effects: None                                                       */
/*     Overview: This function gets the next character from the internal    */
/*               uart buffer.                                               */
/* Notes:                                                                   */
/*                                                                          */
/****************************************************************************/
unsigned int Uart2Read(unsigned char * BufferPtr, unsigned char Length)
{
  unsigned int i = 0;
  
  while(i < Length)
  {
    if(Uart2Rx.Count > 0)
    {
      *BufferPtr = Uart2Rx.Buffer[Uart2Rx.Head++];
      Uart2Rx.Head++;
      Uart2Rx.Head &= (IO_RAPAROUND);
      Uart2Rx.Count--;
    }
    else
      break;
    i++;
  }
  return (i + 1);
}

/****************************************************************************/
/*                                                                          */
/*     Function: Uart 1 receive interrupt service routeen.                  */
/*        Input: None                                                       */
/*       Return: None                                                       */
/*     Overview: Enqueues a new Character into Uart 0's Que                 */
/*                                                                          */
/****************************************************************************/
 //void Uart2::UART1_irq (void) __irq 
void __attribute__((interrupt, auto_psv))_U2RXInterrupt(void)
{
  // If Uart2Count is = IO_RAPAROUND
  // then data will be droped.
  
  if(Uart2Rx.Count < IO_BUFFER_SIZE)
  {
    Uart2Rx.Buffer[Uart2Rx.Tail++] = U2RXREG;
    Uart2Rx.Tail &= (IO_RAPAROUND);
    Uart2Rx.Count++;
    Uart2Rx.ErrorStatus = True; //CHARACTER_AVALABLE;
  }
/*   else */
/*   { */
/*     Uart2RxErrorStatus = DEVICE_OVERFLOW; */
/*   } */
  U2STAbits.FERR = 0;		//Bit2 *Read Only Bit*
  U2STAbits.OERR = 0;		//Bit1 *Read Only Bit*
  IFS0bits.U1RXIF = 0;
}

/****************************************************************************/
/*     Function: Interrupt service routeen                                  */
/*                                                                          */
/*        Input: None                                                       */
/*       Output: None                                                       */
/* Side Effects: None                                                       */
/*     Overview: Receives one character of information in from the UART     */
/*                                                                          */
/* Notes:                                                                   */
/*                                                                          */
/****************************************************************************/
void __attribute__ ((interrupt, no_auto_psv)) _U2TXInterrupt(void) 
{
  if(Uart2Tx.Count > 0)
  {
    U2TXREG = Uart2Tx.Buffer[Uart2Tx.Head++];
    Uart2Tx.Head &= (IO_RAPAROUND);
    Uart2Tx.Count--;
  }

  if(Uart2Tx.Count == 0)
  {
    Uart2Txing = NO;
  }

  U2STAbits.FERR = 0;		//Bit2 *Read Only Bit*
  U2STAbits.OERR = 0;		//Bit1 *Read Only Bit*
  IFS0bits.U1TXIF = 0;
}
