/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    bootloader.c
  * @brief   This file provides code for the configuration
  *          of all used GPIO pins.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "bootloader.h"
#include "usb_host.h"

/* USER CODE BEGIN 0 */

    uint32_t Len = 0xFF;
    uint16_t FramesLen = 0;//最后帧长度
    char FilePath[] = "0:/APP.bin";
  // char FilePath[] = "0:/USBHost.bin";
    
    switch(UDiskState)
    {
        case FileSysInit:    //文件系统初始化
        {
            if(f_mount(&USBFatFs_iap, "", 1)!=FR_OK)//判断是否挂载
            {
                return ;
            }
            UDiskState = OpenFile;
            break;
        }
        case OpenFile:      //打开文件
        {
          //  f_mount(&USBFatFs_iap, "", 0);//挂载
            if(f_open(&MyFile_iap,FilePath,FA_OPEN_EXISTING|FA_READ) != FR_OK )//打开文件，如果不存在则失败
            {
                f_close(&MyFile_iap);      //关闭文件
                f_mount(NULL, "", 0);   //卸载
                return ;
            }
            FileSize = f_size(&MyFile_iap);//获取文件大小
            WriteInteger = FileSize/WriteUSBLen;
            WriteRemSIG = FileSize%WriteUSBLen;
            Write_Count = 0;//清空次数
            
            UDiskState = FlashEraseData;
            break;
        }
        case FlashEraseData:      
        {
			Flash_IfErase(FLASH_APP1_ADDR,FileSize);  			
            UDiskState = UDiskWriteData;
			
            break;
        }		
        case UDiskWriteData://读取数据并执行IAP
        {
            if((WriteInteger > 0)&&(Write_Count < WriteInteger))
            {                          
                if(f_read(&MyFile_iap,FileReadBuf,WriteUSBLen,&Len) != FR_OK)//读取指定长度的数据到RAM
                {
                    f_close(&MyFile_iap);
                    f_mount(NULL, "", 0);
                    return ;
                }
               FLASH_If_Write(((WriteUSBLen*Write_Count)+FLASH_APP1_ADDR),( uint32_t*)FileReadBuf,WriteUSBLen/4);//更新FLASH代码               
                Write_Count++;
            }else
            {
                if(WriteRemSIG != 0)//有剩余数据
                {
                    FramesLen = FileSize-(Write_Count*WriteUSBLen);//得到最后一次写入的长度
                    if(f_read(&MyFile_iap,FileReadBuf,FramesLen,&Len) != FR_OK)//读取指定长度的数据到RAM
                    {
                        f_close(&MyFile_iap);
                        f_mount(NULL, "", 0);
                        return ;
                    }
                    FLASH_If_Write(((WriteUSBLen*Write_Count)+FLASH_APP1_ADDR),( uint32_t*)FileReadBuf,FramesLen/4);//更新FLASH代码
                }
                
                f_close(&MyFile_iap);//关闭文件
                f_mount(NULL, "", 0);//断开连接
                Write_Count = 0;
                UDiskState = FileSysInit;
 
				FLASH_If_Erase(UPGRADE_FLAG_ADDRESS);/*clear app update flag*/
				
				//if(((*(__IO uint32_t*)(UPGRADE_FLAG_ADDRESS)))==0x08000000)
				App_Jump();//执行FLASH APP代码
            }               
        }
        default:break;
    }  
		
/* USER CODE END 0 */


/* USER CODE END 2 */
