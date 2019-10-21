/****************************************************************************/
/*                                                                          */
/*  Programmer: Gregory L Montgomery                                        */
/*                                                                          */
/*  Copyright © 2010-2019                                                   */
/*                                                                          */
/*  COPYING: (See the file COPYING.md for the GNU General Public License).  */
/*  this program is free software, and you may redistribute it and/or       */
/*  modify it under the terms of the GNU General Public License as          */
/*  published by the Free Software Foundation                               */
/*                                                                          */
/* This file is part of Gregory L Montgomery's code base collection Project.*/
/*                                                                          */
/*     Gregory L Montgomery's code base collection Project is free software:*/
/*     you can redistribute it and/or modify  it under the terms of the GNU */
/*     General Public License as published by the Free Software Foundation, */
/*     either version 3 of the License, or (at your option)                 */
/*     any later version.                                                   */
/*                                                                          */
/*     Gregory L Montgomery's code base collection Project is distributed   */
/*     in the hope that it will be useful, but WITHOUT ANY WARRANTY;        */
/*     without even the implied warranty of MERCHANTABILITY or FITNESS FOR  */
/*     A PARTICULAR PURPOSE.  See the GNU General Public License for more   */
/*     details.                                                             */
/*                                                                          */
/*     You should have received a copy of the GNU General Public License    */
/*     along with Gregory L Montgomery's code base collection Project.      */
/*     If not, see <https://www.gnu.org/licenses/>.                         */
/*                                                                          */
/****************************************************************************/
#include <string.h>
#include "defs.h"
#include "Blk.h"

static unsigned WriteIndex;
static unsigned ReadIndex;

// Methods
static unsigned HowMany(void);
static void ResetCounter(void);
static void FlushList(void);
static void ListAdd(unsigned);
static unsigned NextVal(void);
//static Boolean FirstTime;


static VBLOCK This = 
{
   .BlockSize = HowMany,
   .Reset = ResetCounter,
   .Flush = FlushList,
   .Add = ListAdd,
   .NextValue = NextVal,
   .Allocated = 0,
};

/***********************************************************************/
/*                                                                     */
/*     Function:                                                       */
/*        Input:                                                       */
/*       Output:                                                       */
/*                                                                     */
/*     Overview:                                                       */
/*                                                                     */
/***********************************************************************/
struct vBlock * NewBlock(unsigned Size)
{
   memset(This.IndexList, 0 , BLOCK_LST_SIZE);
   //if(This.IndexList > 0)
   //   free(This.IndexList);
   //This.IndexList = (unsigned *)malloc(sizeof(unsigned) * Size);
   ReadIndex = WriteIndex = 0;
   if(Size < BLOCK_LST_SIZE)
      This.Allocated = Size;
   else
      This.Allocated = (BLOCK_LST_SIZE - 1);
   return &This;   
}

/***********************************************************************/
/*                                                                     */
/*     Function:                                                       */
/*        Input:                                                       */
/*       Output:                                                       */
/*                                                                     */
/*     Overview:                                                       */
/*                                                                     */
/***********************************************************************/
static unsigned HowMany(void){return This.Allocated;}

/***********************************************************************/
/*                                                                     */
/*     Function:                                                       */
/*        Input:                                                       */
/*       Output:                                                       */
/*                                                                     */
/*     Overview:                                                       */
/*                                                                     */
/***********************************************************************/
static void ListAdd(unsigned Index)
{
   This.IndexList[WriteIndex] = Index;
   if(WriteIndex < This.Allocated) WriteIndex++;
}


/***********************************************************************/
/*                                                                     */
/*     Function:                                                       */
/*        Input:                                                       */
/*       Output:                                                       */
/*                                                                     */
/*     Overview:                                                       */
/*                                                                     */
/***********************************************************************/
static unsigned NextVal(void)
{ 
   unsigned Results = This.IndexList[ReadIndex];
   if(ReadIndex < This.Allocated) ReadIndex++;
   return Results;
}

/***********************************************************************/
/*                                                                     */
/*     Function:                                                       */
/*        Input:                                                       */
/*       Output:                                                       */
/*                                                                     */
/*     Overview:                                                       */
/*                                                                     */
/***********************************************************************/
static void FlushList(void)
{
   memset(This.IndexList, 0 , BLOCK_LST_SIZE);
}

/***********************************************************************/
/*                                                                     */
/*     Function:                                                       */
/*        Input:                                                       */
/*       Output:                                                       */
/*                                                                     */
/*     Overview:                                                       */
/*                                                                     */
/***********************************************************************/
static void ResetCounter(void)
{
   WriteIndex = 0;
   ReadIndex = 0;
}
