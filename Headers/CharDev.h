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
#ifndef CAR_DEVICE_H
#define CAR_DEVICE_H

#include "defs.h"
#include "RegisterElement.h"
#include "Fops.h"

// Standard IOCTL calls
#define READ_STATUS    1
#define ENABLE_WRITE   2
#define GET_PAGE_SIZE  3
#define GET_MEM_SIZE   4

typedef union
{
    struct
    {
        unsigned WIP : 1;
        unsigned WEL : 1;
        unsigned BP0 : 1;
        unsigned BP1 : 1;
        unsigned RESERVED : 3;
        unsigned WPEN : 1;
    };
    byte Stat;
}EEPromStatus;

// This is a generic File operations type of Object so as to set up a
// sudo OS.
typedef struct FileOperations
{
    void (*Flush)(void);
    void (*PutCh)(unsigned);
    int  (*GetCh)(void);
    void (*PutStr)(char *);
    int (*IOCTL)(unsigned);
    Boolean (*BlockWrite)(unsigned, byte *, byte); // Address, BufferPtr, Length
    Boolean (*BlockRead)(unsigned, byte *, byte); // Address, BufferPtr, Length
    Boolean (*Available)(void); // Indicate if a reading or results is ready
    byte * IndexPtr;
    void * User; // User Structure to include.
}fops;

#endif
