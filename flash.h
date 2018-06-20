#ifndef _NB_256K_CPU_FLASH_H
#define _NB_256K_CPU_FLASH_H

#include <NB_conf.h>

#define FLASH_BASE_ADDR	0x8000000			/* Flash基地址 */
#define	FLASH_SIZE		(256*1024)		/* Flash 容量 */

#define FLASH_DATA_ADDR	0x8027000			/* Flash存储策略基地址 */
#define FLASH_DATA_EndAddr	0x803F800			/* Flash存储策略结束地址 */


#define FLASH_INDEX_ADDR	0x8018000			/* Flash存储策略索引地址 */

/* 对于F103，  256K FLASH , 每个PAGE = 2K 字节，总共 128个 PAGE  */
#define SECTOR_MASK			0xFFFFF800

#define FLASH_IS_EQU		0   /* Flash内容和待写入的数据相等，不需要擦除和写操作 */
#define FLASH_REQ_WRITE		1	/* Flash不需要擦除，直接写 */
#define FLASH_REQ_ERASE		2	/* Flash需要先擦除,再写 */
#define FLASH_PARAM_ERR		3	/* 函数参数错误 */

//一条温湿度记录为16个字节，flash的1K扇区可以存储64条记录.
#pragma pack(1)
struct _HT_HisSampVal
{
	u16 Year;
	u8  Mouths;
	u8  Days;
  u8	Hours;
	u8 	Minutes;
	u8	Seconds;
	u8  TH_AlarmFlag;
	u16	Temp;
	u16	Humi;
	u8 	Blank1;
	u8	Blank2;
  u8	Blank3;
	u8	Blank4;	
};

#pragma pack(1)
struct _HT_HisSampVal_Date
{
	u16 Begin_Year;
	u8  Begin_Mouths;
	u8  Begin_Days;
  u8	Begin_Hours;
	u8 	Begin_Minutes;
	u8	Begin_Seconds;
	u16 End_Year;
	u8  End_Mouths;
	u8  End_Days;
  u8	End_Hours;
	u8 	End_Minutes;
	u8	End_Seconds;	
	u8  Read_AllOrAlarmVal;
};


u32 Flash_GetSector(u32 _ulWrAddr);
u32 NB_ReadCpuFlash(u32 _ulFlashAddr, u8 *_ucpDst, u32 _ulSize);
u8 NB_CmpCpuFlash(u32 _ulFlashAddr, u8 *_ucpBuf, u32 _ulSize);
u8  NB_WriteCpuFlash(u32 _ulFlashAddr, u8 *_ucpSrc, u32 _ulSize);
u8 NB_FLASH_ErasePage(u32 _ulFlashAddr);
u8 Bsp_Write_HTParameter(u8 *buffer,u32 _ulSize);
u8 Bsp_Read_HTParameter(struct _HT_HisSampVal_Date *HT_HisSampVal_Date);
u32 Bsp_ReadCpuFlash(u32 _ulFlashAddr, u8 *_ucpDst, u32 _ulSize, u8 _mode);
u8 Bsp_WriteCpuFlash(u32 _ulFlashAddr, u8 *_ucpBuf, u32 _ulSize,u8 mode);
u8 Bsp_Compare_Time(struct _HT_HisSampVal_Date *HT_HisSampVal_Date,struct _HT_HisSampVal  *HT_HisSampVal);
u8 Bsp_Compare_Time(struct _HT_HisSampVal_Date *HT_HisSampVal_Date,struct _HT_HisSampVal  *HT_HisSampVal);
u8 Bsp_Sim_Compare_Time(struct _HT_HisSampVal  *HT_HisSampVal,struct _HT_HisSampVal  *HT_HisSampVal1);



#endif


