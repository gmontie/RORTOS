#include <xc.h>
#include "defs.h"
#include "PortIO.h"
#include "PortC_IO.h"

static unsigned ReadC(unsigned BitField);
static void SetC(uint16_t What);
static void ClearC(uint16_t What);
static unsigned PeekC(void);
static void PokeC(unsigned Value);

static PortsIO This = 
{
   .Read = ReadC,
   .Set = SetC,
   .Clear = ClearC,
   .Peek = PeekC,
   .Poke = PokeC
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
PortsIO * InitPortC(void)
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
static unsigned ReadC(unsigned BitField)
{
   unsigned Results = PORTC & BitField;
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
static void SetC(uint16_t What)
{
   unsigned Current = PORTC;
   PORTB = (Current & What);
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
static void ClearC(uint16_t What)
{
   unsigned Current = PORTC;
   PORTB = (Current & (~What));
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
static unsigned PeekC(void)
{
   return PORTC;
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
static void PokeC(unsigned Value)
{
   PORTB = Value;
}
