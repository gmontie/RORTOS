/**************************************************************************/
/*                                                                        */
/*  Programmer: Gregory L Montgomery                                      */
/*                                                                        */
/*                                                                        */
/*                                                                        */
/*  COPYING: (See the file COPYING for the GNU General Public License).   */
/*  this program is free software, and you may redistribute it and/or     */
/*  modify it under the terms of the GNU General Public License as        */
/*  published by the Free Software Foundation                             */
/*                                                                        */
/*  This program is distributed in the hope that it will be useful,       */
/*  but WITHOUT ANY WARRANTY; without even the implied warranty of        */
/*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                  */
/*  See the GNU General Public License for more details.                  */
/*                                                                        */
/*  You should have received a copy of the GNU General Public License     */
/*  along with this program. See the file 'COPYING' for more details      */
/*  if for some reason you did not get a copy of the GNU General Public   */
/*  License a copy can be obtained by write to the Free Software          */
/*  Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */
/*                                                                        */
/*  You are welcome to use, share and improve this program.               */
/*  You are forbidden to prevent anyone else to use, share and improve    */
/*  what you give them.                                                   */
/*                                                                        */
/**************************************************************************/
#include <xc.h>
#include "defs.h"
#include "System.h"
#include "IOPort.h"

// private:
Register * EnValue;
unsigned char DisplayValue;
int CurrentPattern;
int Position;
const unsigned int ADC_MASK = 0b000000000000111;

/****************************************************************************/
/*                                                                          */
/*     Function: IOPort                                                     */
/*        Input: None                                                       */
/*       Return: None                                                       */
/*     Overview: This function initializes IOPort functionality. The IOPort */
/*               is a generic IO Port output control for use with a         */
/*               veriety of projects.                                       */
/*                                                                          */
/****************************************************************************/
void IOPort(Register * RegisterAddress)
{
  EnValue = RegisterAddress;
  CurrentPattern = 0;
  Position = 0;
  DisplayValue = 0;
}

/****************************************************************************/
/*                                                                          */
/*     Function: Update                                                     */
/*        Input: None                                                       */
/*       Return: None                                                       */
/*     Overview: This function updates the display value from the serial    */
/*               port so that when WriteIOPort is called the new value      */
/*               which was recieved from the serial port is displayed or    */
/*               pushed out of the IO Port.                                 */
/*                                                                          */
/****************************************************************************/
void UpdatePort(void)
{
  if(EnValue->Changed) //Changed)
  {
    DisplayValue = EnValue->Value;
  }
}

/****************************************************************************/
/*                                                                          */
/*     Function: WriteIOPort                                                */
/*        Input: None                                                       */
/*       Return: None                                                       */
/*     Overview: This function writes out to the Led Board the actual value */
/*               which is contained at 'Positon' of the 'CurrentPattern'.   */
/*               In the case that the couting 'DispalyValue' is used the    */
/*               actual value of DispalyValue is written out to the Led     */
/*               Board.                                                     */
/*                                                                          */
/****************************************************************************/
void WriteIOPort(void)
{
  DisplayValue = PORTB & ADC_MASK;
}

/****************************************************************************/
/*                                                                          */
/*     Function: IncDisplay                                                 */
/*        Input: None                                                       */
/*       Return: None                                                       */
/*     Overview: This function increments the numerical value, or Pattern   */
/*               position would be displayed when WriteIOPort is called.    */
/*                                                                          */
/****************************************************************************/
void IncDisplay(void)
{
      DisplayValue++;
}

/****************************************************************************/
/*                                                                          */
/*     Function: DecDispaly                                                 */
/*        Input: None                                                       */
/*       Return: None                                                       */
/* Side Effects:                                                            */
/*     Overview: This function decrements the numerical value, or Pattern   */
/*               position would be displayed when WriteIOPort is called.    */
/*                                                                          */
/****************************************************************************/
void DecDisplay(void)
{
      DisplayValue--;
}

/****************************************************************************/
/*                                                                          */
/*     Function: SetIOPortValue                                             */
/*        Input: None                                                       */
/*       Return: None                                                       */
/* Side Effects:                                                            */
/*     Overview: This function allows the internal value which is for       */
/*               direct display to be directly set. This is the value       */
/*               used for directly displaying a number to the Led board.    */
/*                                                                          */
/****************************************************************************/
void SetIOPortValue(char Value)
{
   DisplayValue = Value;
}

/****************************************************************************/
/*                                                                          */
/*     Function: SetIOPortPosition                                          */
/*        Input: None                                                       */
/*       Return: None                                                       */
/* Side Effects:                                                            */
/*     Overview: This function sets the IOPorts Position.                   */
/*                                                                          */
/****************************************************************************/
void SetIOPortPPosition(int Value)
{
  Position = Value; 
}

/****************************************************************************/
/*                                                                          */
/*     Function: GetIOPortValue                                             */
/*        Input: None                                                       */
/*       Return: The Display Value for the Led Board                        */
/* Side Effects:                                                            */
/*     Overview: This function returns the internal value which is for      */
/*               direct display. This value is for directly despalying      */
/*               a number to the Led Board. This value can be incremented   */
/*               decremented, or directly set and then despalyed on the     */
/*               Led Board.                                                 */
/*                                                                          */
/****************************************************************************/
char GetIOPortValue(void)
{
  return DisplayValue;
}

/****************************************************************************/
/*                                                                          */
/*     Function: GetIOPortValue                                             */
/*        Input: None                                                       */
/*       Return: The Display Value for the Led Board                        */
/* Side Effects:                                                            */
/*     Overview: This function returns the internal value which is for      */
/*               direct display. This value is for directly despalying      */
/*               a number to the Led Board. This value can be incremented   */
/*               decremented, or directly set and then despalyed on the     */
/*               Led Board.                                                 */
/*                                                                          */
/****************************************************************************/
int GetIOPortPosition(void)
{
  return Position;
}
