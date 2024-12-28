/****************************************Copyright (c)****************************************************
**                                      
**                                 http://www.powermcu.com
**
**--------------File Info---------------------------------------------------------------------------------
** File name:               playaudio.c
** Descriptions:            The playaudio application function
**
**--------------------------------------------------------------------------------------------------------
** Created by:              AVRman
** Created date:            2011-2-27
** Version:                 v1.0
** Descriptions:            The original version
**
**--------------------------------------------------------------------------------------------------------
** Modified by:             
** Modified date:           
** Version:                 
** Descriptions:            
**
*********************************************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "playaudio.h"
#include "vs1003.h"
#include "fatfs.h"
#include "usb_host.h"
#include "spi.h"
/* Private variables ---------------------------------------------------------*/
extern FATFS USBDISKFatFs;           /* File system object for USB disk logical drive */
extern FIL MyFile;                   /* File object */
extern ApplicationTypeDef Appli_state;

void VS1003_PlayMusic_2(const char* filepPath)
{
	FRESULT res;
  uint8_t rtext[32];
  uint32_t  bytesread; 	
	res = f_open(&MyFile, filepPath, FA_OPEN_EXISTING | FA_READ);
  if(res != FR_OK) 
  {
    /* 'hello.txt' file Open for read Error */
    USBH_UsrLog("f_open with read access error: %d", res);
    Error_Handler();
  }
	// 循环读取，直到文件结束
	
	MP3_CCS(1);
	MP3_DCS(0);
	HAL_Delay(5);
	HAL_SPI_Transmit(&hspi2, rtext, 4, 0xffff);
	do
	{
		res = f_read(&MyFile, rtext, sizeof(rtext), (void *)&bytesread);
		if((bytesread == 0) || (res != FR_OK)) 
		{
			USBH_UsrLog("f_read error: %d", res);
			Error_Handler();
		}
		while(MP3_DREQ == 0);  //VS1003的DREQ为高才能写入数据
//		HAL_SPI_Transmit(&hspi2, rtext, bytesread, 0xffff);
		HAL_SPI_Transmit_DMA(&hspi2, rtext, bytesread);
		while(HAL_DMA_GetState(&hdma_spi2_tx) != HAL_DMA_STATE_READY);
		
//		for(uint8_t index = 0; index < 128; index ++ )
//	 {
//		 while(  MP3_DREQ == 0 );     /* 等待空闲 */
//		 HAL_SPI_Transmit(&hspi2, ( rtext + index * 32), 32, 2);
//	 }
	
	} while (bytesread == sizeof(rtext)); // 如果读取的字节数少，说明到达文件末尾
 
	// 关闭文件
	f_close(&MyFile);
	
}
/*********************************************************************************************************
      END FILE
*********************************************************************************************************/

