#ifndef _SERIAL_H_
#define _SERIAL_H_

#include "stm32f10x.h"

#define Uart1_Length 2400
#define Uart2_Length 200
#define Uart3_Length 2400

#define EINT_USART1_TX_EMPTY    USART1_CR1 |= BIT7              //  开发送缓存空中断
#define EINT_USART2_TX_EMPTY    USART2_CR1 |= BIT7              //  开发送缓存空中断
#define EINT_USART3_TX_EMPTY    USART3_CR1 |= BIT7              //  开发送缓存空中断

typedef struct 
{
  unsigned short Uart1_R_T_Header;
  unsigned short Uart1_R_T_Tail;
  
  unsigned short Uart1_T_UnByte;

  unsigned short Uart1_R_UnByte;
  
  unsigned char Uart1_R_T_Array[Uart1_Length];	
	
}Uart1DataByte;

typedef struct 
{
  unsigned char Uart2_R_T_Header;
  unsigned char Uart2_R_T_Tail;
  
  unsigned char Uart2_T_UnByte;

  unsigned char Uart2_R_UnByte;
  
  unsigned char Uart2_R_T_Array[Uart2_Length];
	
}Uart2DataByte;

typedef struct 
{
  unsigned short Uart3_R_T_Header;
  unsigned short Uart3_R_T_Tail;
  
  unsigned short Uart3_T_UnByte;

  unsigned short Uart3_R_UnByte;
  
  unsigned char Uart3_R_T_Array[Uart3_Length];
	
}Uart3DataByte;

unsigned char Read_Uart1_Rx_Array(unsigned char *Buffer,unsigned short Read_Number);
unsigned char Write_Uart1_Tx_Array(unsigned char *Buffer,unsigned short Write_Number);
unsigned char Read_Uart2_Rx_Array(unsigned char *Buffer,unsigned short Read_Number);
unsigned char Write_Uart2_Tx_Array(unsigned char *Buffer,unsigned short Write_Number);
unsigned char Read_Uart3_Rx_Array(unsigned char *Buffer,unsigned short Read_Number);
unsigned char Write_Uart3_Tx_Array(unsigned char *Buffer,unsigned short Write_Number);

#endif
