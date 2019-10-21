#include <p33Fxxxx.h>
#include <string.h>
#include "defs.h"
#include "Spi2.h"

static inline void ConfigureSPI2PortPins(void);
void SelectSPI2Mode(SPI_MODES);
void SPI2InterruptsMode(Boolean );
void SetSPI216BitMode(Boolean );
void SPI2Reset( void );
void Spi2Flush(void);
void BlockWriteSPI2( byte *, byte  );
void WriteSPI2(unsigned );
void BlockReadSPI2( byte *, byte  );
unsigned ReadSPI2(void);

// Private variables/members
static Queue Spi2Tx;
static Queue Spi2Rx;
static byte  Temp;

static sops _SPI2_ =
  {
    .Resource = _SPI_2,
    .TxCount = &Spi2Tx.Count,
    .RxCount = &Spi2Rx.Count,
    .IntModeOn = SPI2InterruptsMode,
    .Set16BitMode = SetSPI216BitMode,
    .Reset = SPI2Reset,
    .Flush = Spi2Flush,
    .Read = ReadSPI2,
    .Write = WriteSPI2,
    .BlockRead = BlockReadSPI2,
    .BlockWrite = BlockWriteSPI2,
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
static inline void ConfigureSPI2PortPins(void)
{ // Setup SPI2 PINS on PIC24 Chip
  //           FEDCBA9876543210
  // TRISD = 0b1111111101001101;
  RPOR6bits.RP12R = 10;  // Master MOSI  PIN 71 RD11
  TRISDbits.TRISD11 = 0; // Set RD11 for output
  RPINR22bits.SDI2R = 3; // Master MISO  PIN 70 RD10
  TRISDbits.TRISD10 = 1; // Set RD10 for intput
  RPOR2bits.RP4R = 11;   // Master SCLK  PIN 69 RD9
  TRISDbits.TRISD9 = 0;  // Set RD9 for output
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
sops * InitSPI2(SPI_MODES Mode)
{ // Setup Port PINS for MOSI, MISO, SCK
  ConfigureSPI2PortPins();

  // Configure SPI2 Peripheral
  // Turn off SPI Interrupts
  IEC2bits.SPI2IE = 0;
  SPI2BUF = 0; // Page 170 of the data sheet Step 1 clear the SPIxBUF

  // Step 3 Page 170
  // Clear the MSTEN (SPIxCON1<5>) = 0
  SPI2STATbits.SPISIDL = 0;
  SPI2STATbits.SISEL = 3;

  SPI2CON1bits.DISSCK = 0; // Disable = 1 SCKx pin (Master mode only)
  SPI2CON1bits.DISSDO = 0; // Disable = 1 SD0x pin
  SPI2CON1bits.MSTEN = 1; // Master Enable = 1, Slave = 0

  //SPI2CON1bits.SPRE = 7; // Secondary Prescale
  //SPI2CON1bits.PPRE = 3; // Primary Prescale
  SPI2CON1bits.SPRE = 7; // Secondary Prescale
  SPI2CON1bits.PPRE = 3; // Primary Prescale
  SPI2CON1bits.SSEN = 1; // Slave Select pin SSx
  SPI2CON1bits.SMP    = 0; // SDI Sampled End of Data = 1, Middle of Data = 0
  SPI2CON1bits.MODE16 = 0;

  SelectSPI2Mode(Mode);

  SPI2CON2 = 0x0000;

  // The Microchip document is wrong
  // We need to read until the
  // SPI buffer/FIFO is empty
  SPI2STATbits.SPIROV &= 0;

  SPI2Reset(); // Clear the SPI2 Queues.
  SPI2InterruptsMode(False);

  // SPI is now set up so enable it.
  SPI2STATbits.SPIEN    = 1;
  return &_SPI2_;
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
void SPI2InterruptsMode(Boolean InteruptsOn)
{
  if(InteruptsOn)
  {
    IFS2bits.SPI2IF = 0; // Clear SPI2 Service Routeen Interrupt Flag
    IFS2bits.SPF2IF = 0; // Clear SPI2 Error Interrupt Flag
    IPC8bits.SPI2IP = 1; // Set SPI2 Interrupt Priority Level to 1;
    IEC2bits.SPI2IE = 1; // Enable the SPI2 interrupt
    //IEC2bits.SPF2IE = 0; // Disable the SPI2 Error Interrupt.
    IEC2bits.SPF2IE = 1; // Enable the SPI2 Error Interrupt.
  }
  else
  {
    IFS2bits.SPI2IF = 0; // Clear SPI2 Service Routeen Interrupt Flag
    IFS2bits.SPF2IF = 0; // Clear SPI2 Error Interrupt Flag
    IEC2bits.SPI2IE = 0; // Enable the SPI2 interrupt
    //IEC2bits.SPF2IE = 0; // Disable the SPI2 Error Interrupt.
    IEC2bits.SPF2IE = 0; // Enable the SPI2 Error Interrupt.
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
void SetSPI216BitMode(Boolean Mode)
{
  if(Mode)
    SPI2CON1bits.MODE16 = 1; // Word = 1 or Byte = 0 size communication
  else
    SPI2CON1bits.MODE16 = 0; // Word = 1 or Byte = 0 size communication
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
void SelectSPI2Mode(SPI_MODES Mode)
{
  switch(Mode)
    {
    case 0: // 0,1
      // Clock Plarity CPOL
      SPI2CON1bits.CKP = 0; // Idle high = 1 or Idle low = 0
      // Clock Phase   CPHA
      SPI2CON1bits.CKE = 0; // Data on Active->Idle = 1 Idle->Active->
      break;
    case 1: // 0,0
      // Clock Plarity CPOL
      SPI2CON1bits.CKP = 0; // Idle high = 1 or Idle low = 0
      // Clock Phase   CPHA
      SPI2CON1bits.CKE = 1; // Data on Active->Idle = 1 Idle->Active->
      break;
    case 2: // 1,1
      // Clock Plarity CPOL
      SPI2CON1bits.CKP = 1; // Idle high = 1 or Idle low = 0
      // Clock Phase   CPHA
      SPI2CON1bits.CKE = 0; // Data on Active->Idle = 1 Idle->Active->
      break;
    case 3: // 1,0
      // Clock Plarity CPOL
      SPI2CON1bits.CKP = 1; // Idle high = 1 or Idle low = 0
      // Clock Phase   CPHA
      SPI2CON1bits.CKE = 1; // Data on Active->Idle = 1 Idle->Active->
      break;
    default: // Default to Mode 0
      // Clock Plarity CPOL
      SPI2CON1bits.CKP = 0; // Idle high = 1 or Idle low = 0
      // Clock Phase   CPHA
      SPI2CON1bits.CKE = 0; // Data on Active->Idle = 1 Idle->Active->
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
void SPI2Reset( void )
{
  memset((char*)&Spi2Tx, 0, sizeof(Queue));
  memset((char*)&Spi2Rx, 0, sizeof(Queue));
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
void Spi2Flush(void)
{
  IFS2bits.SPI2IF = 1;
}

void SPIWIPPolling(void)
{
  unsigned char Status;
  do
  {
    //ChipSelect = 0;                   //Select Device
    WriteSPI2 ( 0x05 );               //Read Status Reg OpCode
    Status = ReadSPI2();              //Read Status Reg
    //ChipSelect = 1;                   //Deselect Device
  } while (Status & 0x01);            //Check for WIP bit Set
}

/********************************************************************
*     Function Name:    PutStringSPI                                *
*     Return Value:     void                                        *
*     Parameters:       address of write string storage location    *
*     Description:      This routine writes a string to the SPI bus.*  
********************************************************************/
//#define SpeedWrteSPI2(x,y) do{SPI2BUFF = *x++;while(!SPI2STATbits.SPITBF);}while(--y);
void BlockWriteSPI2( byte *BlockPtr, byte Length )
{
  do     // transmit data until PageSize
  {
    SPI2BUF = *BlockPtr++;               // initiate SPI bus cycle
    while( !SPI2STATbits.SPITBF );    // wait until 'BF' bit is set
  }while(--Length);
}

/********************************************************************
*     Function Name : WriteSPI2                                     *
*     Description   : This routine writes a single byte/word to     * 
*                     the SPI bus.                                  *
*     Parameters    : Single data byte/word for SPI bus             *
*     Return Value  : None                                          *
********************************************************************/
void WriteSPI2(unsigned Data)
{   
  if(SPI2CON1bits.MODE16)  /* word write */
    SPI2BUF = Data;
  else 
    SPI2BUF = XBty(Data);  /*  byte write  */
  while(SPI2STATbits.SPITBF);
  Data = SPI2BUF;               //Avoiding overflow when reading
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
//#define SpeedRdSPI2(x,y)do{SPI2STATbits.SPIROV &= 0;SPI2BUF=0;while (!SPI2STATbits.SPIRBF);SPI2STATbits.SPIRBF ? (PI2STATbits.SPIROV &= 0;*x++=(byte)SPI2BUF;):0}while(--y)
void BlockReadSPI2( byte *BlockPtr, byte Length )
{
  do        // stay in loop until length = 0
  { 
    SPI2STATbits.SPIROV &= 0;
    SPI2BUF = 0x00; // initiate bus cycle
    while (!SPI2STATbits.SPIRBF);
    /* Check for Receive buffer full status bit of status register*/
    if (SPI2STATbits.SPIRBF)
    {
      SPI2STATbits.SPIROV &= 0;
      if (SPI2CON1bits.MODE16)
	*BlockPtr++ = SPI2BUF; /* return word read */
      else
	*BlockPtr++ = (byte)XBty(SPI2BUF); /* return byte read */
    }
  }while(--Length);// reduce string length count by 1
}

/******************************************************************************
*     Function Name :   ReadSPI2                                              *
*     Description   :   This function will read single byte/ word  from SPI   *
*                       bus. If SPI is configured for byte  communication     *
*                       then upper byte of SPIBUF is masked.                  *         
*     Parameters    :   None                                                  *
*     Return Value  :   contents of SPIBUF register                           *
******************************************************************************/
unsigned ReadSPI2(void)
{         
  SPI2STATbits.SPIROV &= 0;
  SPI2BUF = 0x00;               // initiate bus cycle
  while(!SPI2STATbits.SPIRBF);
   /* Check for Receive buffer full status bit of status register*/
  if (SPI2STATbits.SPIRBF)
  { 
    SPI2STATbits.SPIROV &= 0;              
    if (SPI2CON1bits.MODE16)
      return SPI2BUF;        /* return word read */
    else
      return XBty(SPI2BUF);  /* return byte read */
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
void __attribute__((interrupt, no_auto_psv))_SPI2Interrupt(void)
{
  // Receive
  Temp = SPI2BUF;
  if(Spi2Rx.Count < IO_BUFFER_SIZE)
  {
    Spi2Rx.Buffer[Spi2Rx.Tail++] = Temp;
    Spi2Rx.Tail &= IO_RAPAROUND;
    Spi2Rx.Count++;
  }

  // Transmit
  if(Spi2Tx.Count)
  {
    SPI2BUF = Spi2Tx.Buffer[Spi2Tx.Head++];
    Spi2Tx.Head &= IO_RAPAROUND;
    Spi2Tx.Count--;
  }
  
  // Clear the overflow flag.
  SPI2STATbits.SPIROV &= 0;

  // Clear the SPI fault status bit 
  // and fault interrupt bits.
  IFS2bits.SPI2IF = 0;

  // Enable the event interrupt
  IEC2bits.SPI2IE = 1; 
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
void __attribute__((interrupt, no_auto_psv))_SPI2ErrInterrupt(void)
{
  /*
    Standard ISR handler code here:
    Clear the interrupt flag (set in hardware when interrupt occured), disable 
    further interrupts on the SPI.
  */
  IFS2bits.SPF2IF = 0;
  IEC2bits.SPF2IE = 0;
}


