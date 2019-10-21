;;
;; C Run-time startup module for dsPIC30 C compiler.
;; (c) Copyright 2009 Microchip Technology, All rights reserved
;;
;; Data initialization support.
;; The linker loads this version when the --data-init
;; option is selected.
;;

   .section .init,code
   .global __data_init_standard
;; 
;; Process data init template
;;
;; The template is null-terminated, with records
;; in the following format:
;;
;; struct data_record {
;;  char *dst;        /* destination ADDress  */
;;  int  len;         /* length in bytes      */
;;  int  format;      /* format code          */
;;  char dat[0];      /* variable length data */
;; };
;; 
;; Registers used:  w0 w1 w2 w3 w4 w5
;; 
;; Inputs:
;;  w0 <- tbloffset of initialization template
;;  w1 <- tblpage of initialization template
;; 
;; Outputs:
;;  (none)
;;
;; Calls:
;;  __memcpypd3
;; 
;;
;; Standard Version
;;
;;

   .equ __data_init, __data_init_standard
   .global __data_init

__data_init_standard:

        ; for all format codes, hi-bit set means there is a DST page at
        ; data[0]
   .equiv   FMT_CLEAR,0    ;  format codes
   .equiv   FMT_COPY2,1    ; 
   .equiv   FMT_COPY3,2    ; 

   ;#define ZERO      W0
   .equiv ZERO, W0
   ;#define TBLOFFSET W1
   .equiv TBLOFFSET, W1
   ;#define DSTOFFSET W2
   .equiv DSTOFFSET, W2
   ;#define LEN       W3
   .equiv LEN, W3
   ;#define UPPERBYTE W4
   .equiv UPPERBYTE, W4
   ;#define FORMAT    W5
   .equiv FORMAT, W5
   ;#define PAGE      W6
   .equiv PAGE, W6

   MOV       W1, _TBLPAG
   MOV       W0, TBLOFFSET
   CLR       ZERO
   BRA       4f

1:      
   add      TBLOFFSET,#2,TBLOFFSET
   addc     _TBLPAG                         ; ZERO must be tied to 0

   TBLRDL.w [TBLOFFSET], LEN
   ADD      TBLOFFSET, #2, TBLOFFSET
   ADDC     _TBLPAG                         ; ZERO must be tied to 0

   TBLRDL.w [TBLOFFSET], FORMAT
   ADD      TBLOFFSET, #2, TBLOFFSET
   ADDC     _TBLPAG                         ; ZERO must be tied to 0

   CLR      UPPERBYTE
   LSR      FORMAT, #7, PAGE                  ; PAGE <- stored DSWPAG
   AND      #0x7F, FORMAT                    ; zero out upper bits
   CP.b     FORMAT, #FMT_CLEAR
   BRA      NZ,  2f

        ;; FMT_CLEAR - clear destination memory
9:      
   CLR.b    [DSTOFFSET++]
   DEC      LEN, LEN
   BRA      GTU, 9b                              ; loop if not done 
   BRA      4f

        ;; FMT_COPY2, FMT_COPY3
2:      
   CP       FORMAT,#FMT_COPY2
   BRA      Z,   3f

   SETM     UPPERBYTE

        ;; standard memcpyd3
3:      
   RCALL __memcpypd3_std

4:      
   TBLRDL.w [TBLOFFSET],DSTOFFSET
   CP0      DSTOFFSET
   BRA      NZ,  1b

   RETURN

__memcpypd3_std:
;;
;; Copy data from program memory to data memory
;;
;; Registers used:  w0 w1 w2 w3 w4 w5
;;
;; Inputs:
;;  w0,w1 = source ADDress   (24 bits)
;;  w2 = destination ADDress (16 bits)
;;  w3 = number of bytes (even or odd)
;;  w4 = upper byte flag   (0 = false)
;;
;; Outputs:
;;  w0,w1 = next source ADDress (24 bits)
;;

1:
   TBLRDL.b [TBLOFFSET++], [DSTOFFSET++]  ; dst++ = lo byte
   DEC      LEN, LEN          ; num--
   BRA      Z, 2f           ; br if done         ( ADD one )

   TBLRDL.b [TBLOFFSET--], [DSTOFFSET++]  ; dst++ = hi byte
   DEC      LEN, LEN          ; num--
   BRA      Z, 4f           ; br if done         ( ADD two )

   CP0      UPPERBYTE             ; test upper flag
   BRA      NZ, 1f           ; br if false
 
3:      
   ADD      TBLOFFSET, #2, TBLOFFSET
   ADDC     _TBLPAG
   BRA      1b

1:      
   TBLRDH.b [TBLOFFSET], [w2++]    ; dst++ = upper byte
   DEC      W3, W3          ; num--
   BRA      NZ, 3b          ; br if not done

4:      
   INC      TBLOFFSET, TBLOFFSET
2:      
   ADD      TBLOFFSET, #1, TBLOFFSET
   ADDC     _TBLPAG
   RETURN                  ; exit


;.INClude "null_signature.s" 
   .section __xc16_signature, info, data
   .Word 0x0000
   .Word 0x0000
   .Word 0x0000

   .text
   