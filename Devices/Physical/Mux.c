#include <xc.h>
#include <stdint.h>
#include "defs.h"
#include "Registers.h"
#include "Register.h"
#include "PinConfig.h"
#include "Device.h"
#include "Mux.h"

static void WriteMux(byte, unsigned );
static void Output(void);
static unsigned ReadMux(byte);
static Boolean MuxReady(void);
static unsigned MuxIOCTL(byte);
static void Update(void * _);

static byte Address;
static Register * Element;

static Device Mux=
{
   .DeviceType = SHARED_MUX,
   .Discription = "Shared Mux Object",
   .IsReady = MuxReady,
   .IOCTL = MuxIOCTL,
   .Id = 1,
   .Used = True,
   .UpDate=Update,
   .Read=ReadMux,
   .Write=WriteMux,
   .Status=0
};


/***********************************************************************/
/*     Function:                                                       */
/*                                                                     */
/*        Input:                                                       */
/*       Output:                                                       */
/*     Overview:                                                       */
/*                                                                     */
/***********************************************************************/
Device * NewMux(Register * MuxRegister)
{
   Element = MuxRegister;
   Address = 0;
   return &Mux;
}

/***********************************************************************/
/*     Function:                                                       */
/*                                                                     */
/*        Input:                                                       */
/*       Output:                                                       */
/*     Overview:                                                       */
/*                                                                     */
/***********************************************************************/
static void Update(void * _)
{
   Output();
   Address++;
   Address &= MAX_ADRS;
   Element->Value=Address;
   Element->Changed=True;
}

/***********************************************************************/
/*     Function:                                                       */
/*                                                                     */
/*        Input:                                                       */
/*       Output:                                                       */
/*     Overview:                                                       */
/*                                                                     */
/***********************************************************************/
static void WriteMux(byte Where, uint16_t What)
{
   Address = (0x00FF & What);
}

/***********************************************************************/
/*     Function:                                                       */
/*                                                                     */
/*        Input:                                                       */
/*       Output:                                                       */
/*     Overview:                                                       */
/*                                                                     */
/***********************************************************************/
static void Output(void)
{
   //AdrsLn1 = (Address & 0x01);
   //AdrsLn2 = (Address & 0x02);
   //AdrsLn3 = (Address & 0x04);
}

/***********************************************************************/
/*     Function:                                                       */
/*                                                                     */
/*        Input:                                                       */
/*       Output:                                                       */
/*     Overview:                                                       */
/*                                                                     */
/***********************************************************************/
static unsigned ReadMux(byte Where)
{
   unsigned Results = (unsigned)Address;
   return Results;
}

/***********************************************************************/
/*     Function:                                                       */
/*                                                                     */
/*        Input:                                                       */
/*       Output:                                                       */
/*     Overview:                                                       */
/*                                                                     */
/***********************************************************************/
static Boolean MuxReady(void)
{
   return True;
}

/***********************************************************************/
/*     Function:                                                       */
/*                                                                     */
/*        Input:                                                       */
/*       Output:                                                       */
/*     Overview:                                                       */
/*                                                                     */
/***********************************************************************/
static unsigned MuxIOCTL(byte Bt)
{
   unsigned Results=0;
   switch(Bt)
   {
      case 0:
         Results=Bt;
         break;
      default:
         Results = (unsigned)Address;
   }
   return Results;
}


