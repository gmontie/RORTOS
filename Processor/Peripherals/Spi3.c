#include <p24Fxxxx.h>
#include <string.h>
#include "defs.h"
#include "Spi3.h"

static inline void ConfigureSPI3PortPins(void);
void SelectSPI3Mode(SPI_MODES);
void SPI3InterruptsMode(Boolean );
void SetSPI316BitMode(Boolean );
void SPI3Reset( void );
void Spi3Flush(void);
void BlockWriteSPI3( byte *, byte  );
void WriteSPI3(unsigned );
void BlockReadSPI3( byte *, byte  );
unsigned ReadSPI3(void);

// Private variables/members
static Queue Spi3Tx;
static Queue Spi3Rx;
static byte  Temp;

static sops _SPI3_ =
  {
    .Resource = _SPI_2,
    .TxCount = &Spi3Tx.Count,
    .RxCount = &Spi3Rx.Count,
    .IntModeOn = SPI3InterruptsMode,
    .Set16BitMode = SetSPI316BitMode,
    .Reset = SPI3Reset,
    .Flush = Spi3Flush,
    .Read = ReadSPI3,
    .Write = WriteSPI3,
    .BlockRead = BlockReadSPI3,
    .BlockWrite = BlockWriteSPI3,
  };


/****************************************************************************/
/*     Function:                                                            */
/*                                                                          */
/*        Input: None                                                       */
/*       Output: None                                                       */
/* Side Effects:                                                            */
/*     Overview:                                                            */
/* Notes:                                                                   */
/*                                                                          */
static inline void ConfigureSPI3PortPins(void)
{ // Setup SPI3 PINS on PIC24 Chip
  RPOR13bits.RP26R = 33;  // Master MOSI  PIN 14 RG9
  TRISGbits.TRISG7 = 0; // Set RD11 for output
  //RPINR22bits.SDI2R = 3; // Master MISO  PIN 70 RD10
  //TRISDbits.TRISD10 = 1; // Set RD10 for intput
  RPOR9bits.RP19R = 32;   // Master SCLK  PIN 11 RG7
  TRISGbits.TRISG8 = 0;  // Set RD9 for output
}

/****************************************************************************/
/*     Function: InitSPI                                                    */
/*                                                                          */
/*        Input: None                                                       */
/*       Output: None                                                       */
/* Side Effects: Changes the internal chip settings for SPI                 */
/*     Overview: Initialize the use of SPI by the processor                 */
/* Notes:                                                                   */
/*                                                                          */
/****************************************************************************/
sops * InitSPI3(SPI_MODES Mode)
{ // Setup Port PINS for MOSI, MISO, SCK
  ConfigureSPI3PortPins();

  // Configure SPI3 Peripheral
  // Turn off SPI Interrupts
  IEC5bits.SPI3IE = 0;
  SPI3BUF = 0; // Page 170 of the data sheet Step 1 clear the SPIxBUF

  // Step 3 Page 170
  // Clear the MSTEN (SPIxCON1<5>) = 0
  SPI3STATbits.SPISIDL = 0;
  SPI3STATbits.SISEL = 3;

  SPI3CON1bits.DISSCK = 0; // Disable = 1 SCKx pin (Master mode only)
  SPI3CON1bits.DISSDO = 0; // Disable = 1 SD0x pin
  SPI3CON1bits.MSTEN = 1; // Master Enable = 1, Slave = 0

  //SPI3CON1bits.SPRE = 7; // Secondary Prescale
  //SPI3CON1bits.PPRE = 3; // Primary Prescale
  SPI3CON1bits.SPRE = 7; // Secondary Prescale
  SPI3CON1bits.PPRE = 3; // Primary Prescale
  SPI3CON1bits.SSEN = 1; // Slave Select pin SSx
  SPI3CON1bits.SMP    = 0; // SDI Sampled End of Data = 1, Middle of Data = 0
  SPI3CON1bits.MODE16 = 1;

  SelectSPI3Mode(Mode);

  SPI3CON2 = 0x0000;

  // The Microchip document is wrong
  // We need to read until the
  // SPI buffer/FIFO is empty
  SPI3STATbits.SPIROV &= 0;

  SPI3Reset(); // Clear the SPI3 Queues.
  SPI3InterruptsMode(False);

  // SPI is now set up so enable it.
  SPI3STATbits.SPIEN    = 1;
  return &_SPI3_;
}


/****************************************************************************/
/*     Function:                                                            */
/*                                                                          */
/*        Input: None                                                       */
/*       Output: None                                                       */
/* Side Effects:                                                            */
/*     Overview:                                                            */
/* Notes:                                                                   */
/*                                                                          */
void SPI3InterruptsMode(Boolean InteruptsOn)
{
  if(InteruptsOn)
  {
    IFS5bits.SPI3IF = 0; // Clear SPI3 Service Routeen Interrupt Flag
    IFS5bits.SPF3IF = 0; // Clear SPI3 Error Interrupt Flag
    IPC22bits.SPI3IP = 2; // Set SPI3 Interrupt Priority Level to 1;
    IEC5bits.SPI3IE = 1; // Enable the SPI3 interrupt
    //IEC2bits.SPF2IE = 0; // Disable the SPI3 Error Interrupt.
    IEC5bits.SPF3IE = 1; // Enable the SPI3 Error Interrupt.
  }
  else
  {
    IFS5bits.SPI3IF = 0; // Clear SPI3 Service Routeen Interrupt Flag
    IFS5bits.SPF3IF = 0; // Clear SPI3 Error Interrupt Flag
    IEC5bits.SPI3IE = 0; // Enable the SPI3 interrupt
    //IEC5bits.SPF3IE = 0; // Disable the SPI3 Error Interrupt.
    IEC5bits.SPF3IE = 0; // Enable the SPI3 Error Interrupt.
  }
}

/****************************************************************************/
/*     Function:                                                            */
/*                                                                          */
/*        Input: None                                                       */
/*       Output: None                                                       */
/* Side Effects:                                                            */
/*     Overview:                                                            */
/* Notes:                                                                   */
/*                                                                          */
void SetSPI316BitMode(Boolean Mode)
{
  if(Mode)
    SPI3CON1bits.MODE16 = 1; // Word = 1 or Byte = 0 size communication
  else
    SPI3CON1bits.MODE16 = 0; // Word = 1 or Byte = 0 size communication
}

/****************************************************************************/
/*     Function: SelectMode                                                 */
/*                                                                          */
/*        Input: Which of the 4 SPI modes to be selected.                   */
/*       Output: None                                                       */
/* Side Effects: Changes the internal configuraton of the way SPI will      */
/*               sample it's data.                                          */
/*     Overview: This function selects which of the 4 SPI modes will be     */
/*               use. This is as specified in the Total Phase document as   */
/*               well as the SPI reference manual from Microchip.           */
/* Notes:                                                                   */
/*                                                                          */
/****************************************************************************/
void SelectSPI3Mode(SPI_MODES Mode)
{
  switch(Mode)
    {
    case 0: // 0,1
      // Clock Plarity CPOL
      SPI3CON1bits.CKP = 0; // Idle high = 1 or Idle low = 0
      // Clock Phase   CPHA
      SPI3CON1bits.CKE = 0; // Data on Active->Idle = 1 Idle->Active->
      break;
    case 1: // 0,0
      // Clock Plarity CPOL
      SPI3CON1bits.CKP = 0; // Idle high = 1 or Idle low = 0
      // Clock Phase   CPHA
      SPI3CON1bits.CKE = 1; // Data on Active->Idle = 1 Idle->Active->
      break;
    case 2: // 1,1
      // Clock Plarity CPOL
      SPI3CON1bits.CKP = 1; // Idle high = 1 or Idle low = 0
      // Clock Phase   CPHA
      SPI3CON1bits.CKE = 0; // Data on Active->Idle = 1 Idle->Active->
      break;
    case 3: // 1,0
      // Clock Plarity CPOL
      SPI3CON1bits.CKP = 1; // Idle high = 1 or Idle low = 0
      // Clock Phase   CPHA
      SPI3CON1bits.CKE = 1; // Data on Active->Idle = 1 Idle->Active->
      break;
    default: // Default to Mode 0
      // Clock Plarity CPOL
      SPI3CON1bits.CKP = 0; // Idle high = 1 or Idle low = 0
      // Clock Phase   CPHA
      SPI3CON1bits.CKE = 0; // Data on Active->Idle = 1 Idle->Active->
    }
}

/****************************************************************************/
/*     Function: Initialize varivariables                                   */
/*                                                                          */
/*        Input: None                                                       */
/*       Output: None                                                       */
/* Side Effects: None                                                       */
/*     Overview: SPI Transmit/Receive                                       */
/*                                                                          */
/* Notes:                                                                   */
/*                                                                          */
/****************************************************************************/
void SPI3Reset( void )
{
  memset((char*)&Spi3Tx, 0, sizeof(Queue));
  memset((char*)&Spi3Rx, 0, sizeof(Queue));
}

/****************************************************************************/
/*     Function:                                                            */
/*                                                                          */
/*        Input: None                                                       */
/*       Output: None                                                       */
/* Side Effects:                                                            */
/*     Overview:                                                            */
/* Notes:                                                                   */
/*                                                                          */
/****************************************************************************/
void Spi3Flush(void)
{
  IFS5bits.SPI3IF = 1;
}

//void SPIWIPPolling(void)
//{
//  unsigned char Status;
//  do
//  {
//    //ChipSelect = 0;                   //Select Device
//    WriteSPI3 ( 0x05 );               //Read Status Reg OpCode
//    Status = ReadSPI3();              //Read Status Reg
//    //ChipSelect = 1;                   //Deselect Device
//  } while (Status & 0x01);            //Check for WIP bit Set
//}

/********************************************************************
*     Function Name:    PutStringSPI                                *
*     Return Value:     void                                        *
*     Parameters:       address of write string storage location    *
*     Description:      This routine writes a string to the SPI bus.*  
********************************************************************/
//#define SpeedWrteSPI3(x,y) do{SPI3BUFF = *x++;while(!SPI3STATbits.SPITBF);}while(--y);
void BlockWriteSPI3( byte *BlockPtr, byte Length )
{
  do     // transmit data until PageSize
  {
    SPI3BUF = *BlockPtr++;               // initiate SPI bus cycle
    while( !SPI3STATbits.SPITBF );    // wait until 'BF' bit is set
  }while(--Length);
}

/********************************************************************
*     Function Name : WriteSPI3                                     *
*     Description   : This routine writes a single byte/word to     * 
*                     the SPI bus.                                  *
*     Parameters    : Single data byte/word for SPI bus             *
*     Return Value  : None                                          *
********************************************************************/
void WriteSPI3(unsigned Data)
{   
  if(SPI3CON1bits.MODE16)  /* word write */
    SPI3BUF = Data;
  else 
    SPI3BUF = XBty(Data);  /*  byte write  */
  while(SPI3STATbits.SPITBF);
  Data = SPI3BUF;               //Avoiding overflow when reading
}


/********************************************************************
*     Function Name:    getsSPI                                     *
*     Return Value:     void                                        *
*     Parameters:       address of read string storage location and *
*                       length of string bytes to read              *
*     Description:      This routine reads a string from the SPI    *
*                       bus.  The number of bytes to read is deter- *
*                       mined by parameter 'length'.                *
********************************************************************/
//#define SpeedRdSPI3(x,y)do{SPI3STATbits.SPIROV &= 0;SPI3BUF=0;while (!SPI3STATbits.SPIRBF);SPI3STATbits.SPIRBF ? (PI2STATbits.SPIROV &= 0;*x++=(byte)SPI3BUF;):0}while(--y)
void BlockReadSPI3( byte *BlockPtr, byte Length )
{
  do        // stay in loop until length = 0
  { 
    SPI3STATbits.SPIROV &= 0;
    SPI3BUF = 0x00; // initiate bus cycle
    while (!SPI3STATbits.SPIRBF);
    /* Check for Receive buffer full status bit of status register*/
    if (SPI3STATbits.SPIRBF)
    {
      SPI3STATbits.SPIROV &= 0;
      if (SPI3CON1bits.MODE16)
	*BlockPtr++ = SPI3BUF; /* return word read */
      else
	*BlockPtr++ = (byte)XBty(SPI3BUF); /* return byte read */
    }
  }while(--Length);// reduce string length count by 1
}

/******************************************************************************
*     Function Name :   ReadSPI3                                              *
*     Description   :   This function will read single byte/ word  from SPI   *
*                       bus. If SPI is configured for byte  communication     *
*                       then upper byte of SPIBUF is masked.                  *         
*     Parameters    :   None                                                  *
*     Return Value  :   contents of SPIBUF register                           *
******************************************************************************/
unsigned ReadSPI3(void)
{         
  SPI3STATbits.SPIROV &= 0;
  SPI3BUF = 0x00;               // initiate bus cycle
  while(!SPI3STATbits.SPIRBF);
   /* Check for Receive buffer full status bit of status register*/
  if (SPI3STATbits.SPIRBF)
  { 
    SPI3STATbits.SPIROV &= 0;              
    if (SPI3CON1bits.MODE16)
      return SPI3BUF;        /* return word read */
    else
      return XBty(SPI3BUF);  /* return byte read */
  }
  return -1;  /* RBF bit is not set return error*/
}

/****************************************************************************/
/*     Function:                                                            */
/*                                                                          */
/*        Input: None                                                       */
/*       Output: None                                                       */
/* Side Effects: None                                                       */
/*     Overview:                                                            */
/*                                                                          */
/* Notes:                                                                   */
/*                                                                          */
/****************************************************************************/
void __attribute__((interrupt, auto_psv))_SPI3Interrupt(void)
{
  // Receive
  Temp = SPI3BUF;
  if(Spi3Rx.Count < IO_BUFFER_SIZE)
  {
    Spi3Rx.Buffer[Spi3Rx.Tail++] = Temp;
    Spi3Rx.Tail &= IO_RAPAROUND;
    Spi3Rx.Count++;
  }

  // Transmit
  if(Spi3Tx.Count)
  {
    SPI3BUF = Spi3Tx.Buffer[Spi3Tx.Head++];
    Spi3Tx.Head &= IO_RAPAROUND;
    Spi3Tx.Count--;
  }
  
  // Clear the overflow flag.
  SPI3STATbits.SPIROV &= 0;

  // Clear the SPI fault status bit 
  // and fault interrupt bits.
  IFS5bits.SPI3IF = 0;

  // Enable the event interrupt
  IEC5bits.SPI3IE = 1;
}

/****************************************************************************/
/*     Function:                                                            */
/*                                                                          */
/*        Input: None                                                       */
/*       Output: None                                                       */
/* Side Effects: None                                                       */
/*     Overview:                                                            */
/*                                                                          */
/* Notes:                                                                   */
/*                                                                          */
/****************************************************************************/
void __attribute__((interrupt, no_auto_psv))_SPI3ErrInterrupt(void)
{
  /*
    Standard ISR handler code here:
    Clear the interrupt flag (set in hardware when interrupt occured), disable 
    further interrupts on the SPI.
  */
  IFS2bits.SPF2IF = 0;
  IEC2bits.SPF2IE = 0;
}


