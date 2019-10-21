;; 
;; C Run-time startup module For dsPIC30 C compiler.
;; (c) Copyright 2009 Microchip Technology,  All rights reserved
;;
;; Primary version,  With data initialization support.
;; The linker loads this version When the --data-init
;; option is selected.
;;
;; Standard 16-bit support,  For use With devices that do not support
;; Extended Data Space
;; 
;; See File crt1.s For the alternate version Without
;; data initialization support.
;; 
;; Entry __reset takes control at device reset and
;; perForms the FolloWing:
;;
;;  1. initialize stack and stack limit register
;;  2. initialize PSV WindoW iF __const_length > 0
;;  3. process the data initialization template
;;  4. call the .user_init section,  iF it exists
;;  5. call the user's _main entry point
;;
;; Assigned to section .init,  Which may be allocated
;; at a speciFic address in linker scripts. 
;;
;; IF a local copy oF this File is customized,  be sure
;; to choose a File name other than crt0.s or crt1.s.
;;
   .ifndef ffunction
   .section .init, code
   .else
   .section .init.resetPRI, code
   .endif

   .global __resetPRI
   .ifdef __C30ELF
   .type   __resetPRI, @function
   .endif
__resetPRI:     
   .weak  __reset
   .ifdef __C30ELF
   .type   __reset, @function
   .endif
__reset:
;;
;; InitialiZe stack,  PSV,  and data
;; 
;; registers used:  W0
;;
;; Inputs (deFined by user or linker):
;;  __SP_init
;;  __SPLIM_init
;; 
;; Outputs:
;;  (does not return - resets the processor)
;; 
;; Calls:
;;  __psv_init
;;  __data_init
;;  _main
;; 

   .weak    __user_init,  __has_user_init
        
   MOV      #__SP_init, W15    ; initialize W15
   MOV      #__SPLIM_init, W14

;; Uncomment to pre-initialize all RAM
;;
;; Start at the beginning oF RAM,  Writing the value __DATA_INIT_VAL to memory
;;   Write __STACK_INIT_VAL iF the memory is part oF stack space
;; cycle through to the end oF RAM
;;
;; define initialization values as equate:
.global __DATA_INIT_VAL,  __STACK_INIT_VAL
.equ __DATA_INIT_VAL,  0xDEAD
.equ __STACK_INIT_VAL,  0xA1DE
;;
   MOV      #__DATA_BASE, W0
   MOV      #__DATA_LENGTH, W1
   MOV      #__DATA_INIT_VAL, W3 ; start of initializing RAM
   add      W0, W1, W1

1:      
   CP       W0, W15
   BRA      GEU,  2f             ; move to initializing Stack
   MOV      W3, [W0++]
   CP       W0,  W1
   BRA      LTU,  1b
   BRA      1f

2:      
   MOV      #__STACK_INIT_VAL, W3
   SETM     W15
3:      
   MOV      W3, [W0++]
   CP       W0, W14
   BRA      NZ, 3b
   MOV      #__DATA_INIT_VAL, W3
   CP       W0, W1
   BRA      LTU, 1b
1:      
   MOV      #__SP_init, W15    ; (RE) initialize W15
;;  end RAM PRE-init

   MOV      W14, _SPLIM        ; initialize SPLIM
   NOP                        ; Wait 1 cycle

   RCALL    __psv_init        ; initialize PSV
   MOV      #__dinit_tbloffset, W0 ; W0, W1 = template
   MOV      #__dinit_tblpage, W1   ;
   RCALL    __data_init_standard  ; initialize data

   MOV      #__has_user_init, W0
   CP0      W0                ; user init functions?
   BRA      EQ, 1f             ; br iF not
   CALL     __user_init       ; else call them
1:
   CALL  _main                ; call user's main()

   .pword 0xDA4000            ; halt the simulator
   RESET                      ; reset the processor

   .ifdef ffunction
   .section .init.psv_init,  code
   .endif
   .global __psv_init
__psv_init:
;; 
;; InitialiZe PSV WindoW iF _constlen > 0
;; 
;; Registers used:  W0
;; 
;; Inputs (deFined by linker):
;;  __const_length
;;  __const_psvpage
;; 
;; Outputs:
;;  (none)
;; 
   .equiv   PSV,  0x0002

   BCLR     _CORCON,  #PSV        ; disable PSV (deFault)
   MOV      #__const_length,  W0  ; 
   CP0      W0                  ; test length oF constants
   BRA      Z, 1f                ; br iF zero

   MOV      #__const_psvpage,  W0 ; 
   MOV      W0,  _PSVPAG          ; PSVPAG = psvpage(constants)
   BSET     _CORCON,  #PSV        ; enable PSV

1:      
   RETURN                       ;  and exit


;;.include "null_signature.s"
   .section __xc16_signature,  info,  data
   .Word 0x0000
   .Word 0x0000
   .Word 0x0000

   .text

