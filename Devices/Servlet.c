#include <string.h>
// This is an abriviated device! Really a subset of the Device structure
#include "Register.h"
#include "Device.h"
#include "Servlet.h"

static Servlet  Servlets[SERVLETS_AVAIL];
static Boolean _Conv_Alloc(int *);
static void ClearSlate(Servlet *);


/****************************************************************************/
/*     Function: Clear                                                      */
/*                                                                          */
/*        Input: A pointer to a SerDev Object                               */
/*       Output: None                                                       */
/*     Overview: This function sets the Dov Object passed to zero.          */
/*                                                                          */
/****************************************************************************/
static void ClearSlate(Servlet * Conv)
{
  memset(Conv, 0, sizeof(Servlet));
}

/****************************************************************************/
/*                                                                          */
/*     Function: NewProtocallDevice                                         */
/*        Input: A pointer to a Serial Operations Object.                   */
/*       Output: A pointer to a Serial Servlet Object.                       */
/*     Overview: Receives one character of information in from the UART     */
/*                                                                          */
/****************************************************************************/
Servlet * NewServlet(void)
{
  int i = 0;
  Servlet * NewConv = 0;

  if(_Conv_Alloc(&i))
  {
    NewConv = &Servlets[i];
    ClearSlate(NewConv);
    NewConv->Id = i + 1;
    NewConv->Status.Allocated = True;
    NewConv->Discription = 0;
    NewConv->Clear = ClearSlate;
    NewConv->Used = True;
  }
  return NewConv;
}

/****************************************************************************/
/*     Function: _Dev_Alloc                                                 */
/*                                                                          */
/*        Input: A pointer to an available buffer which can be updated      */
/*               with the number of the table element to be used.           */
/*       Output: Success or Failure                                         */
/* Side Effects: The memory pointed to by * Index is changed                */
/*     Overview: Allocates an element from the Servlet Table.               */
/*                                                                          */
/****************************************************************************/
static Boolean _Conv_Alloc(int * Index)
{
   Boolean Results = False;
   int i = 0;

   do
   {
      if(Servlets[i].Used) // if used
        i++; // Go to the next element in the table
      else
      { // Found a free spot so lets exit.
         Results = True;
         *Index = i;      
      }
   }while((Results == False) && (i < SERVLETS_AVAIL));

   return Results;
}    
