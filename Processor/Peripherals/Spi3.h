#ifndef SPI3_H
#define SPI3_H

#include "defs.h"
#include "ProtoOps.h"
#define SpeedRdSPI3(x,y)do{  SPI3STATbits.SPIROV &= 0;  SPI3BUF=0;  while (!SPI3STATbits.SPIRBF);  if(SPI3STATbits.SPIRBF){SPI3STATbits.SPIROV &= 0;*x++=(byte)SPI3BUF;}}while(--y)
#define SpeedWrteSPI3(x,y) do{SPI3BUFF = *x++;while(!SPI3STATbits.SPITBF);}while(--y);

sops * InitSPI3(SPI_MODES);
//void SPIWIPPolling(void);
void __attribute__((interrupt, auto_psv))_SPI3Interrupt(void);
void __attribute__((interrupt, no_auto_psv))_SPI3ErrInterrupt(void);
#endif
