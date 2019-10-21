#ifndef UART_2_H
#define UART_2_H

#include "CharDev.h"

struct FileOperations * InitUart2(void);
void UART2Init( long BaudRate );
void Uart2PinConfig(void);
void Uart2Reset( void );
void Uart2SetBaudRate(unsigned int Baud);
void  UART2PutChar(char Ch);
void Uart2PutChar(char Ch);
int Uart2GetChar( void );
void Uart2WriteStr(char *);
Boolean Uart2CharAvail(void);
void __attribute__((interrupt, auto_psv))_U2RXInterrupt(void);
void __attribute__ ((interrupt, no_auto_psv)) _U2TXInterrupt(void); 
#endif
