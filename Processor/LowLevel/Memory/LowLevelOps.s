.include "xc.inc"

.global _DisInterrupts
.global _EnInterrupts

.section .text

/******************************************************************************
  Flash Row Read 
  Read a ROWs of memory, upper PM byte is discarded
*******************************************************************************/
;int flashRowRead(uint16_t nvmAdru, uint16_t nvmAdr, uint16_t *roWBufPtr);
_DisInterrupts:
; Disable interrupts While the KEY sequence is written
   MOV     #0x00E0, W0
   PUSH    SR
   ;MOV     #0x00E0, W0
   IOR     SR
   
   
_EnInterrupts:
   POP     SR
   
   