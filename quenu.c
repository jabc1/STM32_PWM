#include "serial.h"

Uart1DataByte Uart1;
Uart2DataByte Uart2;
Uart3DataByte Uart3;

/*********************************************************UART1*****************************************************************/
unsigned char Read_Uart1_Rx_Array(unsigned char *Buffer,unsigned short Read_Number)
{
  unsigned short i,num = 0;
  if((Read_Number == 0) || (Uart1.Uart1_R_UnByte == 0)) return 0;
  if(Read_Number > Uart1.Uart1_R_UnByte) num = Uart1.Uart1_R_UnByte;
  else num = Read_Number;
  for(i = 0;i < num;i++)
  {
    *(Buffer++) = Uart1.Uart1_R_T_Array[Uart1.Uart1_R_T_Header];
    Uart1.Uart1_R_T_Header = (Uart1.Uart1_R_T_Header == (Uart1_Length-1)) ? 0 : (Uart1.Uart1_R_T_Header+1);
  }
  Uart1.Uart1_R_UnByte -= num;
  return num;
}

unsigned char Write_Uart1_Tx_Array(unsigned char *Buffer,unsigned short Write_Number)
{
  unsigned short i;
  unsigned char n = 0;
  switch((Write_Number > Uart1_Length) ? 0 : 1)
  {
    case 0:
      n = 0;	
      break;
    case 1:
      for(i = 0;i < Write_Number;i++)
      {
        Uart1.Uart1_R_T_Array[Uart1.Uart1_R_T_Tail] = *(Buffer++);
        Uart1.Uart1_R_T_Tail = (Uart1.Uart1_R_T_Tail == (Uart1_Length-1)) ? 0 : (Uart1.Uart1_R_T_Tail+1);
      }
      Uart1.Uart1_T_UnByte += Write_Number;
      n = Write_Number;
      EINT_USART1_TX_EMPTY;
      break;
      
      default: break;
  }
  return n;
}

/*********************************************************UART2*****************************************************************/
unsigned char Read_Uart2_Rx_Array(unsigned char *Buffer,unsigned short Read_Number)
{
  unsigned short i,num = 0;
  if((Read_Number == 0) || (Uart2.Uart2_R_UnByte == 0)) return 0;
  if(Read_Number > Uart2.Uart2_R_UnByte) num = Uart2.Uart2_R_UnByte;
  else num = Read_Number;
  for(i = 0;i < num;i++)
  {
    *(Buffer++) = Uart2.Uart2_R_T_Array[Uart2.Uart2_R_T_Header];
    Uart2.Uart2_R_T_Header = (Uart2.Uart2_R_T_Header == (Uart2_Length-1)) ? 0 : (Uart2.Uart2_R_T_Header+1);
  }
  Uart2.Uart2_R_UnByte -= num;
  return num;
}

unsigned char Write_Uart2_Tx_Array(unsigned char *Buffer,unsigned short Write_Number)
{
  unsigned short i;
  unsigned char n = 0;
  switch((Write_Number > Uart2_Length) ? 0 : 1)
  {
    case 0:
      n = 0;	
      break;
    case 1:
      for(i = 0;i < Write_Number;i++)
      {
        Uart2.Uart2_R_T_Array[Uart2.Uart2_R_T_Tail] = *(Buffer++);
        Uart2.Uart2_R_T_Tail = (Uart2.Uart2_R_T_Tail == (Uart2_Length-1)) ? 0 : (Uart2.Uart2_R_T_Tail+1);
      }
      Uart2.Uart2_T_UnByte += Write_Number;
      n = Write_Number;
      EINT_USART2_TX_EMPTY;
      break;
      
      default: break;
  }
  return n;
}

/*********************************************************UART3*****************************************************************/
unsigned char Read_Uart3_Rx_Array(unsigned char *Buffer,unsigned short Read_Number)
{
  unsigned short i,num = 0;
  if((Read_Number == 0) || (Uart3.Uart3_R_UnByte == 0)) return 0;
  if(Read_Number > Uart3.Uart3_R_UnByte) num = Uart3.Uart3_R_UnByte;
  else num = Read_Number;
  for(i = 0;i < num;i++)
  {
    *(Buffer++) = Uart3.Uart3_R_T_Array[Uart3.Uart3_R_T_Header];
    Uart3.Uart3_R_T_Header = (Uart3.Uart3_R_T_Header == (Uart3_Length-1)) ? 0 : (Uart3.Uart3_R_T_Header+1);
  }
  Uart3.Uart3_R_UnByte -= num;
  return num;
}

unsigned char Write_Uart3_Tx_Array(unsigned char *Buffer,unsigned short Write_Number)
{
  unsigned short i;
  unsigned char n = 0;
  switch((Write_Number > Uart3_Length) ? 0 : 1)
  {
    case 0:
      n = 0;	
      break;
    case 1:
      for(i = 0;i < Write_Number;i++)
      {
        Uart3.Uart3_R_T_Array[Uart3.Uart3_R_T_Tail] = *(Buffer++);
        Uart3.Uart3_R_T_Tail = (Uart3.Uart3_R_T_Tail == (Uart3_Length-1)) ? 0 : (Uart3.Uart3_R_T_Tail+1);
      }
      Uart3.Uart3_T_UnByte += Write_Number;
      n = Write_Number;
      EINT_USART3_TX_EMPTY;        
      break;
      
      default: break;
  }
  return n;
}

/*********************************************************UART1中断***************************************************************/
void USART1_IRQHandler(void)
{
  if(USART1_RECEIVED)         //接收
  {
    if(Uart1.Uart1_R_UnByte < Uart1_Length)
    {	
      Uart1.Uart1_R_T_Array[Uart1.Uart1_R_T_Tail] = USART1_DR & 0x01ff;
      Uart1.Uart1_R_T_Tail = (Uart1.Uart1_R_T_Tail == (Uart1_Length-1)) ? 0 : (Uart1.Uart1_R_T_Tail+1);
      Uart1.Uart1_R_UnByte++;
    }	
    if(Uart1.Uart1_R_UnByte == Uart1_Length) Uart1.Uart1_R_UnByte = 0;
  }
  if(USART1_TX_EMPTY)         //发送
  {
    if(Uart1.Uart1_T_UnByte != 0)
    {
      USART1_DR = Uart1.Uart1_R_T_Array[Uart1.Uart1_R_T_Header] & 0x01ff;
      Uart1.Uart1_R_T_Header = (Uart1.Uart1_R_T_Header == (Uart1_Length-1)) ? 0 : (Uart1.Uart1_R_T_Header+1);
      Uart1.Uart1_T_UnByte--;
    }
    else
    {
      DINT_USART1_TX_EMPTY;
    }
  }
}

/*********************************************************UART2中断***************************************************************/
void USART2_IRQHandler(void)
{
  if(USART2_RECEIVED)         //接收
  {
    if(Uart2.Uart2_R_UnByte < Uart2_Length)
    {
      Uart2.Uart2_R_T_Array[Uart2.Uart2_R_T_Tail] = USART2_DR & 0x01ff;
      Uart2.Uart2_R_T_Tail = (Uart2.Uart2_R_T_Tail == (Uart2_Length-1)) ? 0 : (Uart2.Uart2_R_T_Tail+1);
      Uart2.Uart2_R_UnByte++;
    }
    if(Uart2.Uart2_R_UnByte == Uart2_Length) Uart2.Uart2_R_UnByte = 0;
  }
  if(USART2_TX_EMPTY)         //发送
  {
    if(Uart2.Uart2_T_UnByte != 0)
    {
      USART2_DR = Uart2.Uart2_R_T_Array[Uart2.Uart2_R_T_Header] & 0x01ff;
      Uart2.Uart2_R_T_Header = (Uart2.Uart2_R_T_Header == (Uart2_Length-1)) ? 0 : (Uart2.Uart2_R_T_Header+1);
      Uart2.Uart2_T_UnByte--;
    }
    else
    {
      DINT_USART2_TX_EMPTY;
    }
  }
}

/*********************************************************UART3中断***************************************************************/
void USART3_IRQHandler(void)
{
  if(USART3_RECEIVED)         //接收
  {
    if(Uart3.Uart3_R_UnByte < Uart3_Length)
    {
      Uart3.Uart3_R_T_Array[Uart3.Uart3_R_T_Tail] = USART3_DR & 0x01ff;
      Uart3.Uart3_R_T_Tail = (Uart3.Uart3_R_T_Tail == (Uart3_Length-1)) ? 0 : (Uart3.Uart3_R_T_Tail+1);
      Uart3.Uart3_R_UnByte++;
    }
    if(Uart3.Uart3_R_UnByte == Uart3_Length) Uart3.Uart3_R_UnByte = 0;
  }
  if(USART3_TX_EMPTY)         //发送
  {
    if(Uart3.Uart3_T_UnByte != 0)
    {
      USART3_DR = Uart3.Uart3_R_T_Array[Uart3.Uart3_R_T_Header] & 0x01ff;
      Uart3.Uart3_R_T_Header = (Uart3.Uart3_R_T_Header == (Uart3_Length-1)) ? 0 : (Uart3.Uart3_R_T_Header+1);
      Uart3.Uart3_T_UnByte--;
    }
    else
    {
      DINT_USART3_TX_EMPTY;
    }
  }
}
