;; 
;; C Run-time startup module For dsPIC30 C compiler.
;; (c) Copyright 2002, 2004, 2007 Microchip Technology,  All rights reserved
;;
;; Alternate version,  Without data initialization support.
;; The linker loads this version When the --no-data-init
;; option is selected.
;; 
;; See File crt0.s For the primary version With
;; data initialization support.
;; 
;; Entry __reset takes control at device reset and
;; perForms the FolloWing:
;;
;;  1. initialiZe stack and stack limit register
;;  2. initialiZe PSV WindoW iF __const_length > 0
;;  3. call the .user_init section,  iF it exists
;;  4. call the user's _main entry point
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
   .section .init.resetALT, code
   .endif

   .global __resetALT
   .ifdef __C30ELF
   .type   __resetALT, @function
   .endif
__resetALT:     
   .Weak  __reset
   .ifdef __C30ELF
   .type   __reset, @function
   .endif
__reset:
;;
;; Initialize stack and PSV Window
;; 
;; registers used:  W1
;;  (W0 is cleared by device reset,  so ARGC = 0)
;;
;; Inputs (deFined by user or linker):
;;  __SP_init         = initial value of stack pointer
;;  __SPLIM_init      = initial value of stack limit register

;; Inputs (deFined by linker):
;;  __const_length    = length of section .const
;;  __const_psvpage   = PSVPAG setting For section .const
;; 
;; Outputs:
;;  (does not return - resets the processor)
;; 
;; Calls:
;;  _main
;; 
   .equiv  PSV,  0x0002

   .weak    __user_init, __has_user_init

   MOV      #__SP_init, W15    ; initialiZe W15
   MOV      #__SPLIM_init, W14

;; Uncomment to pre-initialize all RAM
;;
;; Start at the beginning oF RAM,  Writing the value __DATA_INIT_VAL to memory
;;   Write __STACK_INIT_VAL if the memory is part oF stack space
;; cycle through to the end oF RAM
;;
;; deFine initialization values as equate:
;; .global __DATA_INIT_VAL,  __STACK_INIT_VAL
;; .equ __DATA_INIT_VAL,  0xDEAD
;; .equ __STACK_INIT_VAL,  0xA1DE
;;

;
;        MOV      #__DATA_BASE, W0
;        MOV      #__DATA_LENGTH, W1
;        MOV      #__DATA_INIT_VAL, W3 ; start of initialiZing RAM
;        add      W0, W1, W1
;
;1:      cp       W0, W15
;        bra      geu,  2F             ; MOVe to initialiZing STACK
;        MOV      W3, [W0++]
;        cp       W0,  W1
;        bra      ltu,  1b
;        bra      1f
;
;2:      MOV      #__STACK_INIT_VAL, W3
;        setm     W15
;3:      MOV      W3, [W0++]
;        cp       W0, W14
;        bra      nZ, 3b
;        MOV      #__DATA_INIT_VAL, W3
;        cp       W0, W1
;        bra      ltu, 1b
;1:      MOV      #__SP_init, W15    ; (RE) initialiZe W15
;
;
;;  end RAM PRE-init

   MOV      W14, _SPLIM        ; initialize SPLIM

   BCLR     _CORCON, #PSV        ; disable PSV (deFault)
   MOV      #__const_length, W1  ; 
   CP0      W1                  ; test length oF constants
   BRA      Z, 1f                ; br iF Zero

   MOV      #__const_psvpage, W1 ; 
   MOV      W1, _PSVPAG          ; PSVPAG = psvpage(constants)
   BSET     _CORCON, #PSV        ; enable PSV
1:
   MOV      #__has_user_init, W0 ;
   CP0      W0                  ; user init functions?
   BRA      Z, 2f                ; br iF not
   CALL     __user_init         ; else call them
2:
   CALL  _main                  ; call user's main()

   .pWord 0xDA4000              ; halt the simulator
   RESET                        ; reset the processor


;;.include "null_signature.s"

   .section __xc16_signature,  info,  data
   .Word 0x0000
   .Word 0x0000
   .Word 0x0000

   .text
