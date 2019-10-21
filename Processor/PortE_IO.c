#include <xc.h>
#include "defs.h"
#include "PortIO.h"
#include "PortB_IO.h"

static unsigned ReadE(unsigned BitField);
static void SetE(uint16_t What);
static void ClearE(uint16_t What);
static unsigned PeekE(void);
static void PokeE(unsigned Value);

static PortsIO This = 
{
   .Read = ReadE,
   .Set = SetE,
   .Clear = ClearE,
   .Peek = PeekE,
   .Poke = PokeE
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
PortsIO * InitPortE(void)
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
static unsigned ReadE(unsigned BitField)
{
   unsigned Results = PORTE & BitField;
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
static void SetE(uint16_t What)
{
   unsigned Current = PORTE;
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
static void ClearE(uint16_t What)
{
   unsigned Current = PORTE;
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
static unsigned PeekE(void)
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
static void PokeE(unsigned Value)
{
   PORTB = Value;
}
