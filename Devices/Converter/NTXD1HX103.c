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
#include <xc.h>
#include <math.h>
#include <float.h>
#include <string.h>
#include "defs.h"
#include "Registers.h"
#include "Register.h"
#include "CD74HCx4051.h"
#include "Analog.h"
#include "Device.h"
#include "Process.h"
#include "Blk.h"
#include "Servlet.h"


static byte TermConversion[] = {255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,
255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,254 ,252 ,250 ,248 ,246 ,244 ,242 ,241 ,
239 ,237 ,235 ,234 ,232 ,231 ,229 ,228 ,226 ,225 ,223 ,222 ,221 ,219 ,218 ,217 ,216 ,214 ,213 ,212 ,
211 ,210 ,209 ,208 ,206 ,205 ,204 ,203 ,202 ,201 ,200 ,199 ,198 ,198 ,197 ,196 ,195 ,194 ,193 ,192 ,
191 ,190 ,190 ,189 ,188 ,187 ,186 ,186 ,185 ,184 ,183 ,183 ,182 ,181 ,180 ,180 ,179 ,178 ,178 ,177 ,
176 ,175 ,175 ,174 ,173 ,173 ,172 ,171 ,171 ,170 ,170 ,169 ,168 ,168 ,167 ,167 ,166 ,165 ,165 ,164 ,
164 ,163 ,162 ,162 ,161 ,161 ,160 ,160 ,159 ,159 ,158 ,157 ,157 ,156 ,156 ,155 ,155 ,154 ,154 ,153 ,
153 ,152 ,152 ,151 ,151 ,150 ,150 ,149 ,149 ,148 ,148 ,147 ,147 ,146 ,146 ,145 ,145 ,145 ,144 ,144 ,
143 ,143 ,142 ,142 ,141 ,141 ,140 ,140 ,140 ,139 ,139 ,138 ,138 ,137 ,137 ,137 ,136 ,136 ,135 ,135 ,
134 ,134 ,134 ,133 ,133 ,132 ,132 ,131 ,131 ,131 ,130 ,130 ,129 ,129 ,129 ,128 ,128 ,127 ,127 ,127 ,
126 ,126 ,125 ,125 ,125 ,124 ,124 ,124 ,123 ,123 ,122 ,122 ,122 ,121 ,121 ,120 ,120 ,120 ,119 ,119 ,
119 ,118 ,118 ,118 ,117 ,117 ,116 ,116 ,116 ,115 ,115 ,115 ,114 ,114 ,113 ,113 ,113 ,112 ,112 ,112 ,
111 ,111 ,111 ,110 ,110 ,110 ,109 ,109 ,108 ,108 ,108 ,107 ,107 ,107 ,106 ,106 ,106 ,105 ,105 ,105 ,
104 ,104 ,104 ,103 ,103 ,102 ,102 ,102 ,101 ,101 ,101 ,100 ,100 ,100 ,99 ,99 ,99 ,98 ,98 ,98 ,
97 ,97 ,97 ,96 ,96 ,96 ,95 ,95 ,95 ,94 ,94 ,93 ,93 ,93 ,92 ,92 ,92 ,91 ,91 ,91 ,
90 ,90 ,90 ,89 ,89 ,89 ,88 ,88 ,88 ,87 ,87 ,87 ,86 ,86 ,86 ,85 ,85 ,84 ,84 ,84 ,
83 ,83 ,83 ,82 ,82 ,82 ,81 ,81 ,81 ,80 ,80 ,80 ,79 ,79 ,78 ,78 ,78 ,77 ,77 ,77 ,
76 ,76 ,76 ,75 ,75 ,74 ,74 ,74 ,73 ,73 ,73 ,72 ,72 ,72 ,71 ,71 ,70 ,70 ,70 ,69 ,
69 ,69 ,68 ,68 ,67 ,67 ,67 ,66 ,66 ,66 ,65 ,65 ,64 ,64 ,64 ,63 ,63 ,62 ,62 ,62 ,
61 ,61 ,60 ,60 ,60 ,59 ,59 ,58 ,58 ,58 ,57 ,57 ,56 ,56 ,55 ,55 ,55 ,54 ,54 ,53 ,
53 ,52 ,52 ,52 ,51 ,51 ,50 ,50 ,49 ,49 ,48 ,48 ,47 ,47 ,47 ,46 ,46 ,45 ,45 ,44 ,
44 ,43 ,43 ,42 ,42 ,41 ,41 ,40 ,40 ,39 ,39 ,38 ,37 ,37 ,36 ,36 ,35 ,35 ,34 ,34 ,
33 ,32 ,32 ,31 ,31 ,30 ,29 ,29 ,28 ,27 ,27 ,26 ,26 ,25 ,24 ,24 ,23 ,22 ,21 ,21 ,
20 ,19 ,18 ,18 ,17 ,16 ,15 ,14 ,14 ,13 ,12 ,11 ,10 ,9 ,8 ,7 ,6 ,5 ,4 ,3 ,
2 ,1 ,0 ,-1 ,-2 ,-3 ,-4 ,-6 ,-7 ,-9 ,-10 ,-12 ,-13 ,-15 ,-17 ,-18 ,-20 ,-23 ,-25 ,-27 ,
-30 ,-32 ,-35 ,-39 ,-40 ,-40 ,-40 ,-40 ,-40 ,-40 ,-40 };

#define DISCRIPTION_NTXD1HX103 "NTXD1HX103 Thermister"

/******************************************************************************/
/*                                                                            */
/******************************************************************************/
static void ConvertTemp(Servlet * Conv)
{
   unsigned Raw = Conv->Reference->Value;
   Raw >>= 3; // May need to be 3
   if(Raw < sizeof(TermConversion))
   {
      Conv->Output->Value = TermConversion[Raw];
      Conv->Output->Changed = True;
   }
}

/******************************************************************************/
/*                                                                            */
/******************************************************************************/
Servlet * NewNTXD1HX103(VBLOCK * Blk)
{
   Servlet * This = NewServlet();
   This->Reference = getRegister( Blk->NextValue() );
   This->Output = getRegister( Blk->NextValue() );
   This->SystemStat = getSystemStat();
   This->DeviceType = NTXD1HX103;
   This->DeviceClass = ANALOG_INPUT;
   This->Discription = DISCRIPTION_NTXD1HX103;
   This->Id = (0x0100 + (int)NTXD1HX103 + (unsigned)This);
   This->Run = ConvertTemp;

   return This;
}