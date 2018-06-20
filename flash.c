#include <NB_conf.h>


/*
*********************************************************************************************************
*	函 数 名: bsp_GetSector
*	功能说明: 根据地址计算扇区首地址
*	形    参：无
*	返 回 值: 扇区首地址
*********************************************************************************************************
*/
u32 Flash_GetSector(u32 _ulWrAddr)
{
	u32 sector = 0;

	sector = _ulWrAddr & SECTOR_MASK;

	return sector;
}

/*
*********************************************************************************************************
*	函 数 名: bsp_ReadCpuFlash
*	功能说明: 读取CPU Flash的内容
*	形    参：_ucpDst : 目标缓冲区
*			 _ulFlashAddr : 起始地址
*			 _ulSize : 数据大小（单位是字节）
*	返 回 值: 0=成功，1=失败
*********************************************************************************************************
*/
u32 NB_ReadCpuFlash(u32 _ulFlashAddr, u8 *_ucpDst, u32 _ulSize)
{
	u32 i;

	/* 如果偏移地址超过芯片容量，则不改写输出缓冲区 */
	if (_ulFlashAddr + _ulSize > FLASH_BASE_ADDR + FLASH_SIZE)
	{
		return 1;
	}

	/* 长度为0时不继续操作,否则起始地址为奇地址会出错 */
	if (_ulSize == 0)
	{
		return 1;
	}

	for (i = 0; i < _ulSize; i++)
	{
		*_ucpDst++ = *(u8 *)_ulFlashAddr++;
	}

	return 0;
}

/*
*********************************************************************************************************
*	函 数 名: bsp_CmpCpuFlash
*	功能说明: 比较Flash指定地址的数据.
*	形    参: _ulFlashAddr : Flash地址
*			 _ucpBuf : 数据缓冲区
*			 _ulSize : 数据大小（单位是字节）
*	返 回 值:
*			FLASH_IS_EQU		0   Flash内容和待写入的数据相等，不需要擦除和写操作
*			FLASH_REQ_WRITE		1	Flash不需要擦除，直接写
*			FLASH_REQ_ERASE		2	Flash需要先擦除,再写
*			FLASH_PARAM_ERR		3	函数参数错误
*********************************************************************************************************
*/
u8 NB_CmpCpuFlash(u32 _ulFlashAddr, u8 *_ucpBuf, u32 _ulSize)
{
	u32 i;
	u8 ucIsEqu;	/* 相等标志 */
	u8 ucByte;

	/* 如果偏移地址超过芯片容量，则不改写输出缓冲区 */
	if (_ulFlashAddr + _ulSize > FLASH_BASE_ADDR + FLASH_SIZE)
	{
		return FLASH_PARAM_ERR;		/*　函数参数错误　*/
	}

	/* 长度为0时返回正确 */
	if (_ulSize == 0)
	{
		return FLASH_IS_EQU;		/* Flash内容和待写入的数据相等 */
	}

	ucIsEqu = 1;			/* 先假设所有字节和待写入的数据相等，如果遇到任何一个不相等，则设置为 0 */
	for (i = 0; i < _ulSize; i++)
	{
		ucByte = *(uint8_t *)_ulFlashAddr;

		if (ucByte != *_ucpBuf)
		{
			if (ucByte != 0xFF)
			{
				return FLASH_REQ_ERASE;		/* 需要擦除后再写 */
			}
			else
			{
				ucIsEqu = 0;	/* 不相等，需要写 */
			}
		}

		_ulFlashAddr++;
		_ucpBuf++;
	}

	if (ucIsEqu == 1)
	{
		return FLASH_IS_EQU;	/* Flash内容和待写入的数据相等，不需要擦除和写操作 */
	}
	else
	{
		return FLASH_REQ_WRITE;	/* Flash不需要擦除，直接写 */
	}
}

/*
*********************************************************************************************************
*	函 数 名: bsp_WriteCpuFlash
*	功能说明: 写数据到CPU 内部Flash。
*	形    参: _ulFlashAddr : Flash地址
*			 _ucpSrc : 数据缓冲区
*			 _ulSize : 数据大小（单位是字节）
*	返 回 值: 0-成功，1-数据长度或地址溢出，2-写Flash出错(估计Flash寿命到)
*********************************************************************************************************
*/
u8  NB_WriteCpuFlash(u32 _ulFlashAddr, u8 *_ucpSrc, u32 _ulSize)
{
	u32 i;
	u8 ucRet;
	u16 usTemp;
	FLASH_Status status = FLASH_COMPLETE;

	/* 如果偏移地址超过芯片容量，则不改写输出缓冲区 */
	if (_ulFlashAddr + _ulSize > FLASH_BASE_ADDR + FLASH_SIZE)
	{
		return 1;
	}

	/* 长度为0 时不继续操作  */
	if (_ulSize == 0)
	{
		return 0;
	}

	/* 长度为奇数时不继续操作  */
	if ((_ulSize % 2) != 0)
	{
		return 1;
	}	

	ucRet = NB_CmpCpuFlash(_ulFlashAddr, _ucpSrc, _ulSize);

	if (ucRet == FLASH_IS_EQU)
	{
		return 0;
	}

	DISABLE_INT();  		/* 关中断 */

	/* FLASH 解锁 */
	FLASH_Unlock();

  	/* Clear pending flags (if any) */
	FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);	

	/* 需要擦除 */
	if (ucRet == FLASH_REQ_ERASE)
	{
		status = FLASH_ErasePage(Flash_GetSector(_ulFlashAddr));
		if (status != FLASH_COMPLETE)
		{
			return 2;
		}		
	}

	/* 按字节模式编程（为提高效率，可以按字编程，一次写入4字节） */
	for (i = 0; i < _ulSize / 2; i++)
	{
		//FLASH_ProgramByte(_ulFlashAddr++, *_ucpSrc++);		
		usTemp = _ucpSrc[2 * i];
		usTemp |= (_ucpSrc[2 * i + 1] << 8);
		status = FLASH_ProgramHalfWord(_ulFlashAddr, usTemp);
		if (status != FLASH_COMPLETE)
		{
			break;
		}		
		_ulFlashAddr += 2;
	}

  	/* Flash 加锁，禁止写Flash控制寄存器 */
  	FLASH_Lock();

  	ENABLE_INT();  		/* 开中断 */

	if (status == FLASH_COMPLETE)
	{
		return 0;
	}
	return 2;
}

/*
*********************************************************************************************************
*	函 数 名: NB_FLASH_ErasePage
*	功能说明: 内部Flash扇区地址清除。
*	形    参: _ulFlashAddr : Flash地址
*	返 回 值: 0-成功，1-数据长度或地址溢出，2-写Flash出错(估计Flash寿命到)
*********************************************************************************************************
*/
u8 NB_FLASH_ErasePage(u32 _ulFlashAddr)
{
	static u8 states;
	
	DISABLE_INT();  		


	FLASH_Unlock();

 
	FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);	

	if(FLASH_ErasePage(Flash_GetSector(_ulFlashAddr)) == FLASH_COMPLETE)
	{
		states = 0;
	}
	else
	{
		states = 1;
	}
  	FLASH_Lock();

  	ENABLE_INT();  	
 return states;
}


/*
*********************************************************************************************************
*	函 数 名: bsp_WriteCpuFlash
*	功能说明: 写数据到CPU 内部Flash。
*	形    参: _ulFlashAddr : Flash地址
*			 _ucpSrc : 数据缓冲区
*			 _ulSize : 数据大小（单位是字节）
			_mode:模式。
						1：写满了清除flash
						0：写满了不清除flash，返回3（已经写满）
*	返 回 值: 0-成功，1-数据长度或地址溢出，2-写Flash出错(估计Flash寿命到),3（已经写满）
*********************************************************************************************************
*/
u8 Bsp_WriteCpuFlash(u32 _ulFlashAddr, u8 *_ucpBuf, u32 _ulSize,u8 _mode)
{
	u8 buffer[50],checksum = 0;
	u32 Temp_Addr = _ulFlashAddr;
	u16 start = 0;
	u16 mid;
	u16 ulSize = (_ulSize + 2);
	u16 end = ((2048/ulSize) - 1);
	
	u16 usTemp;
	FLASH_Status status = FLASH_COMPLETE;

	/* 如果偏移地址超过芯片容量，则不改写输出缓冲区 */
	if (_ulFlashAddr + _ulSize > FLASH_BASE_ADDR + FLASH_SIZE)
	{
		return 1;
	}

	/* 长度为0 时不继续操作  */
	if (_ulSize == 0)
	{
		return 0;
	}

	/* 长度为奇数时不继续操作  */
	if ((_ulSize % 2) != 0)
	{
		return 1;
	}	
	
	if((2048%ulSize) == 0)
	{
			if(*(u8 *)(Temp_Addr+2047) != 0xFF) //判断扇区最后一个地址是否为FF
			{
					if(_mode)
					{
							 if(NB_FLASH_ErasePage(Temp_Addr) !=0)
							 {
										return 0;
							 }					
					}
					else
					{
								goto Info1;
					}

			}	
			else
			{
					goto Judge;
			}
	}
	
	else
	{
			if(*(u8 *)(Temp_Addr+ ((2048/ulSize)*(ulSize)) - 1) != 0xFF)
			{
				if(_mode)
				{
						 if(NB_FLASH_ErasePage(Temp_Addr) !=0)
						 {
									return 0;
						 }									
				}
				else
				{
							goto Info1;
				}
			}
			else
			{
				goto Judge;
			}
			
	}

	Judge:
//	{
		 mid = ((end + 1) >> 1);
		do
		{
			if(*(u8 *)(Temp_Addr+(mid * ulSize) + (ulSize - 1)) == 0xFF)
			{
					end = mid; //向上查询
			}
			else
			{
					start = mid;//向下查询
			}
			
			mid = (end + start + 1) >>1;
			if((mid == end) || (mid == start) || (end == start))
			{
					if(*(u8 *)(Temp_Addr+(mid * ulSize) - 1) == 0xFF) //当前索引为空
					{
							Temp_Addr +=((mid - 1)*_ulSize); //flash写入，取前组
						  break;
					}//取前组
					else
					{
							Temp_Addr += (mid * ulSize); //flash写入，取当前组
							break;
					}//取当前组
					
			}
		}while(1);		
//	}
	
	memcpy(buffer,_ucpBuf,_ulSize);
	for(u16 i = 0;i < _ulSize; i++)
	{
		checksum ^=buffer[i];	
	}
	buffer[_ulSize] = checksum;//增加校验和
	buffer[_ulSize + 1] = 0x5A;//增加帧尾
	
	DISABLE_INT();  		/* 关中断 */
	/* FLASH 解锁 */
	FLASH_Unlock();
	/* 按字节模式编程（为提高效率，可以按字编程，一次写入4字节） */
	for (u32 i = 0; i < (ulSize / 2); i++)
	{
		//FLASH_ProgramByte(_ulFlashAddr++, *_ucpSrc++);		
		usTemp = buffer[2 * i];
		usTemp |= (buffer[2 * i + 1] << 8);
		status = FLASH_ProgramHalfWord(Temp_Addr, usTemp);
		if (status != FLASH_COMPLETE)
		{
			break;
		}		
		Temp_Addr += 2;
	}
  	/* Flash 加锁，禁止写Flash控制寄存器 */
  	FLASH_Lock();

  	ENABLE_INT();  		/* 开中断 */

	if (status == FLASH_COMPLETE)
	{
		return 0;
	}
	return 2;

	Info1: return 3;
}


/*
*********************************************************************************************************
*	函 数 名: bsp_ReadCpuFlash
*	功能说明: 读取CPU Flash的内容
*	形    参：_ucpDst : 目标缓冲区
*			 _ulFlashAddr : 起始地址
*			 _ulSize : 数据大小（单位是字节）
			_mode:模式。
						1：读取当前有效数据
						0：读取当前地址的所有数据，返回3（已经写满）
*	返 回 值: 0=成功，1=失败
*********************************************************************************************************
*/
u32 Bsp_ReadCpuFlash(u32 _ulFlashAddr, u8 *_ucpDst, u32 _ulSize, u8 _mode)
{
	u8 buffer[50],checksum ;
	u32 Temp_Addr = _ulFlashAddr;
	u16 start = 0;
	u16 mid;
	u16 ulSize = (_ulSize  + 2);
	u16 end = ((2048/ulSize)-1);	
	u32 i,j;

	/* 如果偏移地址超过芯片容量，则不改写输出缓冲区 */
	if (_ulFlashAddr + _ulSize > FLASH_BASE_ADDR + FLASH_SIZE)
	{
		return 1;
	}

	/* 长度为0时不继续操作,否则起始地址为奇地址会出错 */
	if (_ulSize == 0)
	{
		return 1;
	}
	
	if(*(u8 *)(Temp_Addr+(start * ulSize) + (ulSize - 1)) == 0xFF) //第一条为空
	{
			return 2;
	}
	if(_mode)
	{
			if(*(u8 *)(Temp_Addr+(end * ulSize) + (ulSize - 1)) != 0xFF)//全部写满
			{
					Temp_Addr +=(end * ulSize);
					for (i = 0; i < ulSize; i++)
					{
						buffer[i] = *(u8 *)Temp_Addr++;
					}		  
			}
			else
			{
				 mid = ((end + 1) >> 1);
				do
				{
					if(*(u8 *)(Temp_Addr+(mid * ulSize) + (ulSize - 1)) == 0xFF)
					{
							end = mid; //向上查询
					}
					else
					{
							start = mid;//向下查询
					}
					
					mid = (end + start + 1) >>1;
					if((mid == end) || (mid == start) || (end == start))
					{
							
							if(*(u8 *)(Temp_Addr+(mid * ulSize) - 1) == 0xFF) //当前索引为空
							{
									Temp_Addr += (mid * ulSize);		//flash读取，取当前组					
									for (i = 0; i < ulSize; i++)
									{
										 buffer[i] = *(u8 *)Temp_Addr++;
									}	
									break;
							}
							else
							{
									Temp_Addr +=((mid - 1) * ulSize); //flash读取，取前组
									for (i = 0; i < ulSize; i++)
									{
										 buffer[i] = *(u8 *)Temp_Addr++;
									}							
									break;
							}
							
					}
				}while(1);		
			}
			
			for(checksum = 0,i = 0;i < _ulSize; i++)
			{
					checksum ^= buffer[i];
			}
			
			if((checksum == buffer[ulSize-2]) &&(buffer[ulSize-1] == 0x5A))
			{
				memcpy(_ucpDst,buffer,_ulSize);		
			}
			else
			{
				return 1;
			}	
	}
	else //指定给100Kflash的
	{
			for(i = 0; i < (_ulSize / 16); i++) ////一条温湿度记录为16个字节,其中前14个字节为有效数据
			{
					for (j = 0; j < 18; j++)
					{
						 buffer[j] = *(u8 *)Temp_Addr++;
					}		
					for(checksum = 0,j = 0;j < 16; j++)
					{
							checksum ^= buffer[j];
					}
					
					if((checksum == buffer[16]) &&(buffer[17] == 0x5A))
					{
						memcpy((_ucpDst + (16 * i)),buffer,16);		
					}	
					else
					{
						return 1;
					}	
					Temp_Addr	= Temp_Addr + 18*(i+1);		
					if(Temp_Addr == (Flash_GetSector(Temp_Addr) + 2047 - (2048%18)))
					{
						break;
					}
			}
	}	
	return 0;	
}
/*
*********************************************************************************************************
*	函 数 名: Bsp_Write_HTParameter
*	功能说明: 156K位置写入参数。
*	形    参: u8 *buffer :参数
*	返 回 值: 0-成功，1-数据长度或地址溢出，2-写Flash出错(估计Flash寿命到)
*********************************************************************************************************
*/
u8 Bsp_Write_HTParameter(u8 *buffer,u32 _ulSize)
{
//	  u32 Buffer_Addr,Buffer_Base_Addr;
	  u16 Index_num;
	  u8 Index[2];
	  u32 ulSize = _ulSize;
	  u32 Sector_Addr;
		if((*(u8 *)(FLASH_DATA_ADDR + (ulSize + 1)) == 0xFF) &&(*(u8 *)(FLASH_DATA_ADDR+ 49*2048 +  (ulSize + 1)) == 0xFF)) //全新flash，第一次写入
		{
			  Bsp_WriteCpuFlash(FLASH_DATA_ADDR,buffer,ulSize,0); //写数据
			  Index_num = 0; //地址索引
			  MAKEBYTE(Index[0],Index[1],Index_num);
			  Bsp_WriteCpuFlash(FLASH_INDEX_ADDR,Index,2,1);//flash写入地址索引
		}
		else
		{
			Bsp_ReadCpuFlash(FLASH_INDEX_ADDR,Index,2,1);
			Index_num = MAKEWORD(Index[0],Index[1]);
			Sector_Addr = Flash_GetSector(FLASH_DATA_ADDR + Index_num*(ulSize + 2) + (2048%(ulSize + 2))*(Index_num/(2048/(ulSize + 2)))); //判断扇区
			if(Bsp_WriteCpuFlash(Sector_Addr,buffer,ulSize,0) == 3) //，写数据，指定扇区已经写满了				
			{
//				Sector_Addr = Flash_GetSector(FLASH_INDEX_ADDR + Index_num*ulSize);
				if((*(u8 *)(Sector_Addr + 0x00000800 + (ulSize + 1)) == 0xFF) && (Sector_Addr < FLASH_DATA_EndAddr)) //下一个扇区为空的
				{
					 Bsp_WriteCpuFlash((Sector_Addr + 0x00000800),buffer,ulSize,0); //写在下一个扇区,并记录地址索引
					 Index_num ++;
					 MAKEBYTE(Index[0],Index[1],Index_num);
					 Bsp_WriteCpuFlash(FLASH_INDEX_ADDR,Index,2,1);							
								
				}
				else
				{	
					if(Index_num == ((2048/(ulSize + 2))*50 - 1)) //到最后一个扇区的最后一个索引
					{
					    NB_FLASH_ErasePage(FLASH_DATA_ADDR);//擦除最初的位置
							Bsp_WriteCpuFlash(FLASH_DATA_ADDR,buffer,ulSize,0); //回到最初的位置开始写。在读的时候带加翻转标志位
							Index_num = 0; //索引清空
							MAKEBYTE(Index[0],Index[1],Index_num);
							Bsp_WriteCpuFlash(FLASH_INDEX_ADDR,Index,2,1);		
					}
					else
					{
							NB_FLASH_ErasePage(Sector_Addr + 0x00000800);//擦除下一个扇区
						  Bsp_WriteCpuFlash((Sector_Addr + 0x00000800),buffer,ulSize,0); //写在下一个扇区,并记录地址索引
							Index_num ++;
							MAKEBYTE(Index[0],Index[1],Index_num);
							Bsp_WriteCpuFlash(FLASH_INDEX_ADDR,Index,2,1);						
					}				
				}		
			}
			else //还没写满，索引自增,并写入
			{
					Bsp_ReadCpuFlash(FLASH_INDEX_ADDR,Index,2,1);
					Index_num = MAKEWORD(Index[0],Index[1]) + 1;			
					MAKEBYTE(Index[0],Index[1],Index_num);
					Bsp_WriteCpuFlash(FLASH_INDEX_ADDR,Index,2,1);					
			}		
		}		
	return 0;		
}

/*
*********************************************************************************************************
*	函 数 名: Bsp_Read_HTParameter
*	功能说明: 156K位置写入参数。
*	形    参: u8 *buffer :参数
*	返 回 值: 0-成功，1-数据长度或地址溢出，2-写Flash出错(估计Flash寿命到)
*********************************************************************************************************
*/
u8 Bsp_Read_HTParameter(struct _HT_HisSampVal_Date *HT_HisSampVal_Date)
{
	u32 i;
	u8 buffer[16],buffer1[16];
  volatile u32 Compare_Addr;
	volatile u32 Start_Addr;
	volatile u32 End_Addr;
  struct _HT_HisSampVal  *Ndata = (struct _HT_HisSampVal  *)buffer;
	struct _HT_HisSampVal  *Odata = (struct _HT_HisSampVal  *)buffer1 ;
	for(i = 0;i < 50;i++)
	{
			Bsp_ReadCpuFlash(Flash_GetSector(FLASH_DATA_ADDR + i*2048),(u8 *)Odata,16,0);
		
			if(Bsp_Compare_Time(HT_HisSampVal_Date,Odata) == 2) //在范围内,查找开始扇区地址
			{
				 if(Bsp_Sim_Compare_Time(Ndata,Odata) == 2) //新的比旧的小
				 {
						Start_Addr = (FLASH_DATA_ADDR + i*2048); //取当前扇区地址作为最小，缩小范围
				 }
				 else if(Bsp_Sim_Compare_Time(Ndata,Odata) == 2) //新的比旧的大
				 {
						Start_Addr = (FLASH_DATA_ADDR + (i-1)*2048); //取上次扇区地址作为最小
				 }
			}
			
			else if(Bsp_Compare_Time(HT_HisSampVal_Date,Odata) == 4) //在范围内,查找结束扇区地址				
			{
				 if(Bsp_Sim_Compare_Time(Ndata,Odata) == 2) //新的比旧的小
				 {
						End_Addr = (FLASH_DATA_ADDR + (i-1)*2048); //取上次扇区地址作为最大 
				 }
				 else if(Bsp_Sim_Compare_Time(Ndata,Odata) == 2) //新的比旧的大
				 {
						End_Addr = (FLASH_DATA_ADDR + i*2048); //取当前扇区地址作为最大
				 }			
			}
			Compare_Addr = Flash_GetSector(FLASH_DATA_ADDR + i*2048);
			Bsp_ReadCpuFlash(Compare_Addr,(u8 *)Ndata,16,0);
	}
	return 0;
}

u8 Bsp_Compare_Time(struct _HT_HisSampVal_Date *HT_HisSampVal_Date,struct _HT_HisSampVal  *HT_HisSampVal)
{
	struct tm time_Begin;
	struct tm time_End;
	struct tm time;
	time_Begin.tm_year = HT_HisSampVal_Date->Begin_Year;
	time_Begin.tm_mon  = HT_HisSampVal_Date->Begin_Mouths;
  time_Begin.tm_mday = HT_HisSampVal_Date->Begin_Days;
	time_Begin.tm_hour = HT_HisSampVal_Date->Begin_Hours;
  time_Begin.tm_min  = HT_HisSampVal_Date ->Begin_Minutes;
  time_Begin.tm_sec  = HT_HisSampVal_Date->Begin_Seconds;
  time_Begin.tm_isdst = -1;
	
	time_End.tm_year = HT_HisSampVal_Date->End_Year;
	time_End.tm_mon  = HT_HisSampVal_Date->End_Mouths;
  time_End.tm_mday = HT_HisSampVal_Date->End_Days;
	time_End.tm_hour = HT_HisSampVal_Date->End_Hours;
  time_End.tm_min  = HT_HisSampVal_Date ->End_Minutes;
  time_End.tm_sec  = HT_HisSampVal_Date->End_Seconds;
  time_End.tm_isdst = -1;	
	
	time.tm_year = HT_HisSampVal->Year;
	time.tm_mon  = HT_HisSampVal->Mouths;
  time.tm_mday = HT_HisSampVal->Days;
	time.tm_hour = HT_HisSampVal->Hours;
  time.tm_min  = HT_HisSampVal ->Minutes;
  time.tm_sec  = HT_HisSampVal->Seconds;
  time.tm_isdst = -1;		
	
	if(mktime(&time) >= mktime(&time_Begin)) //查询的起始时间比所在的地址时间低
	{
		 if(mktime(&time) >= mktime(&time_End)) 
		 {
				 return 1;   //不在范围内
		 }
		 else 
		 {
				 return 2;  //在范围内,相对较小
		 }		
	}
	else //查询的起始时间比所在的地址时间高
	{
		 if(mktime(&time) >= mktime(&time_End))  
		 {
				 return 3; //不在范围内
		 }
		 else
		 {
				 return 4; //在范围内，相对较大
		 }		
	}	
}

u8 Bsp_Sim_Compare_Time(struct _HT_HisSampVal  *HT_HisSampVal,struct _HT_HisSampVal  *HT_HisSampVal1)
{
	struct tm time;
	struct tm time1;

	time.tm_year = HT_HisSampVal->Year;
	time.tm_mon  = HT_HisSampVal->Mouths;
  time.tm_mday = HT_HisSampVal->Days;
	time.tm_hour = HT_HisSampVal->Hours;
  time.tm_min  = HT_HisSampVal ->Minutes;
  time.tm_sec  = HT_HisSampVal->Seconds;
  time.tm_isdst = -1;		

	time1.tm_year = HT_HisSampVal1->Year;
	time1.tm_mon  = HT_HisSampVal1->Mouths;
  time1.tm_mday = HT_HisSampVal1->Days;
	time1.tm_hour = HT_HisSampVal1->Hours;
  time1.tm_min  = HT_HisSampVal1 ->Minutes;
  time1.tm_sec  = HT_HisSampVal1->Seconds;
  time1.tm_isdst = -1;		
	
	if(mktime(&time) >= mktime(&time1)) //查询的起始时间比所在的地址时间低
	{
		 if(mktime(&time) >= mktime(&time1)) 
		 {
				 return 1;   //不在范围内
		 }
		 else 
		 {
				 return 2;  //在范围内,相对较小
		 }		
	}
	else //查询的起始时间比所在的地址时间高
	{
		 if(mktime(&time) >= mktime(&time1))  
		 {
				 return 3; //不在范围内
		 }
		 else
		 {
				 return 4; //在范围内，相对较大
		 }		
	}	
}
