#include <xc.h>
#include "Register.h"
#include "Registers.h"
#include "NTXD1HX103.h"
#include "Spi1.h"
#include "System.h"
#include "CD74HCx4051.h"
#include "Adc.h"
#include "Analog.h"
#include "PWM.h"
#include "MCP23S17.h"
#include "SysCalls.h"
#include "Process.h"
#include "Diag.h"
#include "Blk.h"

#define DISCRIPTION_SUPERLOOP "The Super Loop"

static void RazalDazzell(void);
static void Pong(void);
static void Kapow(void);

static Service      * PWM;
static Service      * Analog;
static Service      * AuxPtr;
static Service      * Mcp23S17;

// Other interfaces
static AdOps      * Adc; 
static Servlet    * Therm1;
static Servlet    * Therm2;
static Servlet    * Therm3;
static Servlet    * Therm4;
static ADC_SAMPLING_STATES * ADC_State;

static void SuperLoop(void);

static Service This =
{
   .Discription = DISCRIPTION_SUPERLOOP,
   .IsReady = ReadyTrue,
   .Peek = 0,
   .Poke = 0,
   .Read = 0,
   .Id = (0x0101 + ((int) USER_THREAD) * 0x20),
   .DeviceType = USER_THREAD,
   .FnType = ProcessFn,
   .Thread = SuperLoop,
   .Dev = 0,
};

/****************************************************************************/
/*                                                                          */
/*     Function: upDate                                                     */
/*        Input: None                                                       */
/*       Output: None                                                       */
/*     Overview: This thread checks the Uart IO and updates the shared      */
/*               register space.                                            */
/*                                                                          */
/*                                                                          */
/*                                                                          */
/****************************************************************************/
static void SuperLoop(void)
{
   unsigned Counter = 0;

   while (LOOPING_FOREVER)
   {
      PWM->Thread();

      Analog->Thread();

      //Exec(_SPI_1, Mcp23S17->Thread);
      
      switch (*ADC_State)
      {
         case START:
         case READING_READY:
         case SORT_SAMPLES:
         case SKIP:
         case NEXT_READING:
         case REPEAT:
            switch (Counter++)
            {
               case 0: Therm1->Run(Therm1); break;
               case 1: Therm2->Run(Therm2); break;
               case 2: Therm3->Run(Therm3); break;
               case 3: Therm4->Run(Therm4); break;
               default: Counter = 0;
            }
            Counter &= (SERVLETS_AVAIL - 1);
      }
   }
}

/****************************************************************************/
/*                                                                          */
/*     Function: InitComm Thread                                            */
/*        Input: A pointer to the shared memory (registers)                 */
/*       Output: A pointer to a newly allocated Devcie Object               */
/*     Overview: Receives one character of information in from the UART     */
/*                                                                          */
/****************************************************************************/
Service * InitalizeLoop(fops * Spi)
{
   VBLOCK * BlkPtr;

      // Instantiate PWM Signal
   PWM = NewPWM( &OC1RS, PWM_VALUE );
   PWM->Write(0, 1233);
   PWM->Thread();
   
   BlkPtr = NewBlock( 2 );
   BlkPtr->Add( GPIO );
   BlkPtr->Add( IOLAT );
   Mcp23S17 = InitMCP23S17( BlkPtr, Spi );
      
   // Instantiate AD Converter and Voltmeter
   Adc = ADCInit( AD_READING );
   // Setup CD4051/74HC4051 Mux
   BlkPtr = NewBlock( 6 );
   BlkPtr->Add( MUXER );
   BlkPtr->Add( AD_READING );
   BlkPtr->Add( ANALOG1 );
   BlkPtr->Add( ANALOG2 );
   BlkPtr->Add( ANALOG3 );
   BlkPtr->Add( ANALOG4 );
   AuxPtr = Init74HC4051( BlkPtr );
   
   // Setup Analog state machine
   // Analog may need to be pulled into the super loop.... not sure yet
   BlkPtr = NewBlock( 2 );
   BlkPtr->Add( MUXER );
   BlkPtr->Add( AD_READING );
   Analog = NewAnalog( Adc, AuxPtr, BlkPtr );
   ADC_State = (ADC_SAMPLING_STATES *)Analog->Arg;
   
   BlkPtr = NewBlock( 2 );
   BlkPtr->Add( ANALOG1 );
   BlkPtr->Add( DEGREES_F1 );
   Therm1 = NewNTXD1HX103( BlkPtr );

   BlkPtr = NewBlock( 2 );
   BlkPtr->Add( ANALOG2 );
   BlkPtr->Add( DEGREES_F2 );
   Therm2 = NewNTXD1HX103( BlkPtr );

   BlkPtr = NewBlock( 2 );
   BlkPtr->Add( ANALOG3 );
   BlkPtr->Add( DEGREES_F3 );
   Therm3 = NewNTXD1HX103( BlkPtr );
   
   BlkPtr = NewBlock( 2 );
   BlkPtr->Add( ANALOG4 );
   BlkPtr->Add( DEGREES_F4 );
   Therm4 = NewNTXD1HX103( BlkPtr );

   // Show that the MCP23S17 SPI Expander works
   Pong();
   Kapow();
   RazalDazzell();
   
   return &This;
}

static void Pong(void)
{
   int i;

   for (i = 0; i < 7; i++)
   {
      Mcp23S17->Write(GPIOA, (~1));
      Mcp23S17->Write(GPIOB, (~1));
      Delay(125);
      Delay(125);

      Mcp23S17->Write(GPIOA, (~2));
      Mcp23S17->Write(GPIOB, (~2));
      Delay(125);
      Delay(125);

      Mcp23S17->Write(GPIOA, (~4));
      Mcp23S17->Write(GPIOB, (~4));
      Delay(125);
      Delay(125);

      Mcp23S17->Write(GPIOA, (~8));
      Mcp23S17->Write(GPIOB, (~8));
      Delay(125);
      Delay(125);

      Mcp23S17->Write(GPIOA, (~16));
      Mcp23S17->Write(GPIOB, (~16));
      Delay(125);
      Delay(125);

      Mcp23S17->Write(GPIOA, (~32));
      Mcp23S17->Write(GPIOB, (~32));
      Delay(125);
      Delay(125);

      Mcp23S17->Write(GPIOA, (~64));
      Mcp23S17->Write(GPIOB, (~64));
      Delay(125);
      Delay(125);

      Mcp23S17->Write(GPIOA, (~128));
      Mcp23S17->Write(GPIOB, (~128));
      Delay(125);
      Delay(125);

      Mcp23S17->Write(GPIOA, (~64));
      Mcp23S17->Write(GPIOB, (~64));
      Delay(125);
      Delay(125);

      Mcp23S17->Write(GPIOA, (~32));
      Mcp23S17->Write(GPIOB, (~32));
      Delay(125);
      Delay(125);

      Mcp23S17->Write(GPIOA, (~16));
      Mcp23S17->Write(GPIOB, (~16));
      Delay(125);
      Delay(125);

      Mcp23S17->Write(GPIOA, (~8));
      Mcp23S17->Write(GPIOB, (~8));
      Delay(125);
      Delay(125);

      Mcp23S17->Write(GPIOA, (~4));
      Mcp23S17->Write(GPIOB, (~4));
      Delay(125);
      Delay(125);

      Mcp23S17->Write(GPIOA, (~2));
      Mcp23S17->Write(GPIOB, (~2));
      Delay(125);
      Delay(125);
   }
}

static void Kapow(void)
{
   int i;
   
   for (i = 0; i < 2; i++)
   {
      Mcp23S17->Write(GPIOA, (~129));
      Mcp23S17->Write(GPIOB, (~129));
      Delay(125);
      Delay(125);
      Delay(125);
      Delay(125);

      Mcp23S17->Write(GPIOA, (~66));
      Mcp23S17->Write(GPIOB, (~66));
      Delay(125);
      Delay(125);
      Delay(125);
      Delay(125);

      Mcp23S17->Write(GPIOA, (~102));
      Mcp23S17->Write(GPIOB, (~102));
      Delay(125);
      Delay(125);
      Delay(125);
      Delay(125);

      Mcp23S17->Write(GPIOA, (~60));
      Mcp23S17->Write(GPIOB, (~60));
      Delay(125);
      Delay(125);
      Delay(125);
      Delay(125);

      Mcp23S17->Write(GPIOA, (~24));
      Mcp23S17->Write(GPIOB, (~24));
      Delay(125);
      Delay(125);
      Delay(125);
      Delay(125);
   }
}

static void RazalDazzell(void)
{
   int i;
   
   for (i = 0; i < 7; i++)
   {
      Mcp23S17->Write(GPIOA, (~0x49));
      Mcp23S17->Write(GPIOB, (~0x54));
      Delay(125);
      Delay(125);

      Mcp23S17->Write(GPIOA, (~0xAA));
      Mcp23S17->Write(GPIOB, (~0x55));
      Delay(125);
      Delay(125);

      Mcp23S17->Write(GPIOA, (~0));
      Mcp23S17->Write(GPIOB, (~0x25));
      Delay(125);
      Delay(125);

      Mcp23S17->Write(GPIOA, (~0x55));
      Mcp23S17->Write(GPIOB, (~0xAA));
      Delay(125);
      Delay(125);

      Mcp23S17->Write(GPIOA, (~1));
      Mcp23S17->Write(GPIOB, (~1));
      Delay(125);
      Delay(125);

      Mcp23S17->Write(GPIOA, (~0x54));
      Mcp23S17->Write(GPIOB, (~0x49));
      Delay(125);
      Delay(125);

      Mcp23S17->Write(GPIOA, (~64));
      Mcp23S17->Write(GPIOB, (~64));
      Delay(125);
      Delay(125);

      Mcp23S17->Write(GPIOA, (~0xAA));
      Mcp23S17->Write(GPIOB, (~0x80));
      Delay(125);
      Delay(125);

      Mcp23S17->Write(GPIOA, (~0x91));
      Mcp23S17->Write(GPIOB, (~0x54));
      Delay(125);
      Delay(125);

      Mcp23S17->Write(GPIOA, (~0x54));
      Mcp23S17->Write(GPIOB, (~0x91));
      Delay(125);
      Delay(125);

      Mcp23S17->Write(GPIOA, (~0xA9));
      Mcp23S17->Write(GPIOB, (~0x52));
      Delay(125);
      Delay(125);

      Mcp23S17->Write(GPIOA, (~0x1));
      Mcp23S17->Write(GPIOB, (~0x1));
      Delay(125);
      Delay(125);

      Mcp23S17->Write(GPIOA, (~0x128));
      Mcp23S17->Write(GPIOB, (~0x128));
      Delay(125);
      Delay(125);

      Mcp23S17->Write(GPIOA, (~0x00));
      Mcp23S17->Write(GPIOB, (~0x00));
      Delay(125);
      Delay(125);
   }
}