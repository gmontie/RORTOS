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
#ifndef PROTO_OPS_H
#define PROTO_OPS_H

#include "defs.h"

//#define I2C_BRG	(((Fcy / 2 / Fsck) - 1) << 2) // 50Khz
//#define I2C_BRG	(((Fcy / 2 / Fsck) - 1) << 1) // 100Khz
#define I2C_BRG	(((Fcy / 2 / Fsck) - 1) >> 1) // 333Khz

// Define some fundamental enumerated types for use with Serial Operations
// Objects.
typedef enum{MODE_0=0, MODE_1, MODE_2, MODE_3}SPI_MODES;
typedef enum{_I2C_1, _I2C_2, _I2C_3, _SPI_1, _SPI_2, _SPI_3}Devices;

// Define some serial protocal operations for serial type of objects.
typedef struct ProtocallOperations
{
  Devices        Resource;
  byte         * TxCount;
  byte         * RxCount;
  void     (*IntModeOn)(Boolean);
  void     (*Set16BitMode)(Boolean);
  void     (*Reset)(void);
  void     (*Flush)(void);
  int      (*Read)(void);
  void     (*Write)(unsigned);
  void     (*BlockRead)(byte *, byte);
  void     (*BlockWrite)(byte *,byte);
}sops; 

#endif
