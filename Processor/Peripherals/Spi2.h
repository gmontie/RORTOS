#ifndef SPI2_H
#define SPI2_H

#include "defs.h"
#include "ProtoOps.h"
#define SpeedRdSPI2(x,y)do{  SPI2STATbits.SPIROV &= 0;  SPI2BUF=0;  while (!SPI2STATbits.SPIRBF);  if(SPI2STATbits.SPIRBF){SPI2STATbits.SPIROV &= 0;*x++=(byte)SPI2BUF;}}while(--y)
#define SpeedWrteSPI2(x,y) do{SPI2BUFF = *x++;while(!SPI2STATbits.SPITBF);}while(--y);

sops * InitSPI2(SPI_MODES);
void SPIWIPPolling(void);
void __attribute__((interrupt, no_auto_psv))_SPI2Interrupt(void);
void __attribute__((interrupt, no_auto_psv))_SPI2ErrInterrupt(void);
#endif
