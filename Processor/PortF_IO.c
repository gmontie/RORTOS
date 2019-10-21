#include <xc.h>
#include "defs.h"
#include "PortIO.h"
#include "PortB_IO.h"

static unsigned ReadF(unsigned BitField);
static void SetF(uint16_t What);
static void ClearF(uint16_t What);
static unsigned PeekF(void);
static void PokeF(unsigned Value);

static PortsIO This = 
{
   .Read = ReadF,
   .Set = SetF,
   .Clear = ClearF,
   .Peek = PeekF,
   .Poke = PokeF
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
PortsIO * InitPortF(void)
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
static unsigned ReadF(unsigned BitField)
{
   unsigned Results = PORTF & BitField;
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
static void SetF(uint16_t What)
{
   unsigned Current = PORTF;
   PORTF = (Current & What);
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
static void ClearF(uint16_t What)
{
   unsigned Current = PORTF;
   PORTF = (Current & (~What));
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
static unsigned PeekF(void)
{
   return PORTF;
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
static void PokeF(unsigned Value)
{
   PORTF = Value;
}
