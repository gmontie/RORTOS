#include <xc.h>
#include "defs.h"
#include "PortIO.h"
#include "PortA_IO.h"

static unsigned ReadA(unsigned BitField);
static void SetA(uint16_t What);
static void ClearA(uint16_t What);
static unsigned PeekA(void);
static void PokeA(unsigned Value);

static PortsIO This = 
{
   .Read = ReadA,
   .Set = SetA,
   .Clear = ClearA,
   .Peek = PeekA,
   .Poke = PokeA
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
PortsIO * InitPortA(void)
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
static unsigned ReadA(unsigned BitField)
{
   unsigned Results = PORTA & BitField;
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
static void SetA(uint16_t What)
{
   unsigned Current = PORTA;
   PORTA = (Current & What);
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
static void ClearA(uint16_t What)
{
   unsigned Current = PORTA;
   PORTA = (Current & (~What));
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
static unsigned PeekA(void)
{
   return PORTA;
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
static void PokeA(unsigned Value)
{
   PORTA = Value;
}
