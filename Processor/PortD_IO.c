#include <xc.h>
#include "defs.h"
#include "PortIO.h"
#include "PortB_IO.h"

static unsigned ReadD(unsigned BitField);
static void SetD(uint16_t What);
static void ClearD(uint16_t What);
static unsigned PeekD(void);
static void PokeD(unsigned Value);

static PortsIO This = 
{
   .Read = ReadD,
   .Set = SetD,
   .Clear = ClearD,
   .Peek = PeekD,
   .Poke = PokeD
};

/***********************************************************************/
/*                                                                     */
/*     Function: IO Control                                            */
/*        Input: None                                                  */
/*       Output: None                                                  */
/*                                                                     */
/*     Overview: IO Control for Memory                                 */
/*                                                                     */
/***********************************************************************/
PortsIO * InitPortD(void)
{
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
static unsigned ReadD(unsigned BitField)
{
   unsigned Results = PORTD & BitField;
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
static void SetD(uint16_t What)
{
   unsigned Current = PORTD;
   PORTD = (Current & What);
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
static void ClearD(uint16_t What)
{
   unsigned Current = PORTD;
   PORTD = (Current & (~What));
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
static unsigned PeekD(void)
{
   return PORTD;
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
static void PokeD(unsigned Value)
{
   PORTD = Value;
}
