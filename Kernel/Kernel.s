;**************************************************************************** 
;*                                                                          * 
;*  Programmer: Gregory L Montgomery                                        * 
;*                                                                          * 
;*  Copyright © 2010-2019                                                   * 
;*                                                                          * 
;*  COPYING: (See the file COPYING.md for the GNU General Public License).  * 
;*  this program is free software, and you may redistribute it and/or       * 
;*  modify it under the terms of the GNU General Public License as          * 
;*  published by the Free Software Foundation                               * 
;*                                                                          * 
;* This file is part of Gregory L Montgomery's code base collection Project.* 
;*                                                                          * 
;*     Gregory L Montgomery's code base collection Project is free software:* 
;*     you can redistribute it and/or modify  it under the terms of the GNU * 
;*     General Public License as published by the Free Software Foundation, * 
;*     either version 3 of the License, or (at your option)                 * 
;*     any later version.                                                   * 
;*                                                                          * 
;*     Gregory L Montgomery's code base collection Project is distributed   * 
;*     in the hope that it will be useful, but WITHOUT ANY WARRANTY;        * 
;*     without even the implied warranty of MERCHANTABILITY or FITNESS FOR  * 
;*     A PARTICULAR PURPOSE.  See the GNU General Public License for more   * 
;*     details.                                                             * 
;*                                                                          * 
;*     You should have received a copy of the GNU General Public License    * 
;*     along with Gregory L Montgomery's code base collection Project.      * 
;*     If not, see <https://www.gnu.org/licenses/>.                         * 
;*                                                                          * 
;**************************************************************************** 
.include "xc.inc"
#include "Kernel.h"

.equ    IPL,     0x00E0 ; Set IPL0 through IPL2 to 1 to turn off interrupts

.section .text

; Declaration of stack size
.equ    _StackLim, 0x0FE
.global _StackLim

.equ    _StackAlc, 0x0100
.global _StackAlc

.end
   