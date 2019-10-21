.include "xc.inc"

.equ    FLASH_ROW_ERASE,   0x4041
.equ    FLASH_PAGE_ERASE,  0x4042
.equ    FLASH_ROW_WRITE,   0x4001

.global _flashRowRead
.global _flashRowErase
.global _flashRowWrite
.global _flashRowModify
.global _EraseFlashPage

.section .text

/******************************************************************************
  Flash Row Read 
  Read a ROWs of memory, upper PM byte is discarded
*******************************************************************************/
;int flashRowRead(uint16_t nvmAdru, uint16_t nvmAdr, uint16_t *roWBufPtr);
_flashRowRead:
   PUSH     TBLPAG
   MOV      W0, TBLPAG
   MOV      #32, W3 

readNext:     
   TBLRDL   [W1], [W2++]
   TBLRDH   [W1++], W6    ; Discard PM upper byte
   DEC      W3, W3
   BRA      NZ, readNext

   CLR 	   W0
	;MOV     #5,W0
   POP     TBLPAG
   RETURN

/******************************************************************************
  Flash Row Modify 
  Read EIGHT rows (PAGE) of memory, upper PM byte is discarded
*******************************************************************************/
_flashRowModify:
   SL 	W0, #7, W0 	
   ADD	W0, W3, W3	
				
modifyNext:     
   MOV	[W2++],[W3++]	; Discard PM upper byte
   DEC     W1, W1
   BRA     NZ, modifyNext

   RETURN

/******************************************************************************
  Flash Row Erase 
  Erase a ROW of memory
*******************************************************************************/
 ;int flashRowErase(uint16_t nvmAdru, uint16_t nvmAdr);
_flashRowErase:
   PUSH    TBLPAG

   MOV     W0, TBLPAG	; Init Pointer to row to be erased
   TBLWTL  W1, [W1]	; Dummy write to select the row

		; Setup NVCON for page erase
   MOV   	#FLASH_ROW_ERASE, W3
   MOV     W3, NVMCON
   BSET 	W3, #WR
; Disable interrupts While the KEY sequence is written
   MOV     #0x00E0, W0
   PUSH    SR
   ;MOV     #0x00E0, W0
   IOR     SR
; Write the KEY Sequence
   MOV     #0x55, W0
   MOV     W0, NVMKEY
   MOV     #0xAA, W0
   MOV     W0, NVMKEY
; Start the erase operation
   BSET    NVMCON, #WR
; Insert two NOPs after the erase cycle (required)
   NOP
   NOP
;Re-enable interrupts, if needed
   POP      SR


erase_Wait:     
   BTSC   NVMCON, #WR
   BRA    erase_Wait

   CLR    W0
   POP    TBLPAG
   RETURN


/******************************************************************************
  Erase Flash Pagem
                                                                                                          
*******************************************************************************/
      ; int EraseFlashPage(unsigned NVRadmdru, unsigned NVRamAddress);
_EraseFlashPage:
   PUSH    TBLPAG

   MOV     W0, TBLPAG	; Init Pointer to row to be erased
   TBLWTL  W1, [W1]	; Dummy write to select the row
      
      ; Setup NVMCON to erase one page of Program Memory
   MOV     #0x4042, W0
      ;MOV #FLASH_PAGE_ERASE, W0
   MOV     W0, NVMCON
      ; Disable interrupts while the KEY sequence is written
   MOV     #0x00E0, W0
   PUSH    SR
   IOR     SR
      ; Write the KEY Sequence
   MOV     #0x55, W0
   MOV     W0, NVMKEY
   MOV     #0xAA, W0
   MOV     W0, NVMKEY
      ; Start the erase operation
   BSET    NVMCON, #WR
      ; Insert two NOPs after the erase cycle (required)
   NOP 
   NOP
      ;Re-enable interrupts, if needed
   POP     SR
   POP     TBLPAG
   RETURN ;

/******************************************************************************
  Flash RoW Program
  Program a roWs  of memory, 
  Each roW contains 96 bytes of data (32 instructions, with upper PM byte == NOP) 
*******************************************************************************/
;int flashRoWWrite(uint16_t nvmAdru, uint16_t nvmAdr, uint16_t *rowBufPtr);
_flashRowWrite:
   PUSH    TBLPAG
   MOV     W0, TBLPAG		; Init Pointer to row to be programed
		
   MOV	#0,  W6
   MOV 	#32, W3

pinst_loop: 
   TBLWTL  [W2++], [W1]
   TBLWTH  W6, [W1++]		; load 0x00 into 3rd byte (Will be decoded as NOP should PC attempt to execute)

   DEC     W3, W3
   BRA     NZ, pinst_loop

			  ; Setup NVCON for row program
   MOV     #FLASH_ROW_WRITE, W7
   MOV     W7, NVMCON
   BSET 	  W7, #WR
   DISI 	  #5				; Block all interrupt with priority <7 for next 5 instructions	
   MOV     #0x55, W0
   MOV     W0, NVMKEY
   MOV     #0xAA, W0
   MOV     W0, NVMKEY		
   MOV     W7, NVMCON		; Start Program Operation
   NOP
   NOP

prog_Wait:     
   BTSC    NVMCON, #WR
   BRA     prog_Wait

   CLR 	W0
   POP     TBLPAG
   RETURN

.end
