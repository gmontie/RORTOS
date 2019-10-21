#include <xc.h>
#include "defs.h"
#include "Device.h"
#include "Register.h"
#include "Registers.h"
#include "PinConfig.h"
#include "Relays.h"
#include "SubSystem.h"

#define RELAY_DISCRIPTION "Relay Controls"

#define ON_OFF_RLY     RB_IO1    // PORTBbits.RB10
#define DIRECTION_RLY  RB_IO2   //  PORTBbits.RB11

static void _UpDate(void);
static unsigned _Read(byte);
static void _Write(byte WhichBit, uint16_t What);
static Boolean _IsReady(void){return True;}
static unsigned _Peek(void);
static void _Poke(unsigned Value);

// Device operations used..
static Device This =
{
   .DeviceType = RELAY,
   .Discription=RELAY_DISCRIPTION,
   .Id = (0x0100 + (int)RELAY),
   .Arg = 0,
   .FnType = UpDateType,
   .UpDate = _UpDate,
   .Read = _Read,
   .Peek = _Peek,
   .Write = _Write,
   .Poke = _Poke, 
   .IsReady = _IsReady,
   .Driver = 0
};

static Register * ConfigRegister; // A register element pointer
static RequestBits * ConfigBits;

/***********************************************************************/
/*                                                                     */
/*     Function: IO Control                                            */
/*        Input: None                                                  */
/*       Output: None                                                  */
/*                                                                     */
/*     Overview: IO Control for Memory                                 */
/*                                                                     */
/***********************************************************************/
Device * InitReleays(Register * RegisterPtr)
{
   ConfigRegister = RegisterPtr;
   ConfigBits = (RequestBits *)&ConfigRegister->Value;
   return &This;
}

/***********************************************************************/
/*                                                                     */
/*     Function: IO Control                                            */
/*        Input: None                                                  */
/*       Output: None                                                  */
/*                                                                     */
/*     Overview: IO Control for Memory                                 */
/*                                                                     */
/***********************************************************************/
static void _UpDate(void)
{   
   ON_OFF_RLY = (ConfigBits->Field & RELAY_ON_OFF) ? 1 : 0;
   DIRECTION_RLY = (ConfigBits->Field & RELAY_DIRCTN) ? 1 : 0;
   // ON_OFF_RLY = ConfigBits->OnOffRly;
   // DIRECTION_RLY = ConfigBits->DirectionRly;
}

/***********************************************************************/
/*                                                                     */
/*     Function: IO Control                                            */
/*        Input: None                                                  */
/*       Output: None                                                  */
/*                                                                     */
/*     Overview: IO Control for Memory                                 */
/*                                                                     */
/***********************************************************************/
static unsigned _Read(byte BitField)
{
   unsigned Results = PORTB & BitField;
   return Results;
}

/***********************************************************************/
/*                                                                     */
/*     Function: IO Control                                            */
/*        Input: None                                                  */
/*       Output: None                                                  */
/*                                                                     */
/*     Overview: IO Control for Memory                                 */
/*                                                                     */
/***********************************************************************/
static void _Write(byte WhichBits, uint16_t What)
{
   PORTB |= (WhichBits & What);
}

/***********************************************************************/
/*                                                                     */
/*     Function: IO Control                                            */
/*        Input: None                                                  */
/*       Output: None                                                  */
/*                                                                     */
/*     Overview: IO Control for Memory                                 */
/*                                                                     */
/***********************************************************************/
static unsigned _Peek(void)
{
   return PORTB;
}

/***********************************************************************/
/*                                                                     */
/*     Function: IO Control                                            */
/*        Input: None                                                  */
/*       Output: None                                                  */
/*                                                                     */
/*     Overview: IO Control for Memory                                 */
/*                                                                     */
/***********************************************************************/
static void _Poke(unsigned Value)
{
   PORTB = Value;
}
