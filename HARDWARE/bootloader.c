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
    uint16_t FramesLen = 0;//���֡����
    char FilePath[] = "0:/APP.bin";
  // char FilePath[] = "0:/USBHost.bin";
    
    switch(UDiskState)
    {
        case FileSysInit:    //�ļ�ϵͳ��ʼ��
        {
            if(f_mount(&USBFatFs_iap, "", 1)!=FR_OK)//�ж��Ƿ����
            {
                return ;
            }
            UDiskState = OpenFile;
            break;
        }
        case OpenFile:      //���ļ�
        {
          //  f_mount(&USBFatFs_iap, "", 0);//����
            if(f_open(&MyFile_iap,FilePath,FA_OPEN_EXISTING|FA_READ) != FR_OK )//���ļ��������������ʧ��
            {
                f_close(&MyFile_iap);      //�ر��ļ�
                f_mount(NULL, "", 0);   //ж��
                return ;
            }
            FileSize = f_size(&MyFile_iap);//��ȡ�ļ���С
            WriteInteger = FileSize/WriteUSBLen;
            WriteRemSIG = FileSize%WriteUSBLen;
            Write_Count = 0;//��մ���
            
            UDiskState = FlashEraseData;
            break;
        }
        case FlashEraseData:      
        {
			Flash_IfErase(FLASH_APP1_ADDR,FileSize);  			
            UDiskState = UDiskWriteData;
			
            break;
        }		
        case UDiskWriteData://��ȡ���ݲ�ִ��IAP
        {
            if((WriteInteger > 0)&&(Write_Count < WriteInteger))
            {                          
                if(f_read(&MyFile_iap,FileReadBuf,WriteUSBLen,&Len) != FR_OK)//��ȡָ�����ȵ����ݵ�RAM
                {
                    f_close(&MyFile_iap);
                    f_mount(NULL, "", 0);
                    return ;
                }
               FLASH_If_Write(((WriteUSBLen*Write_Count)+FLASH_APP1_ADDR),( uint32_t*)FileReadBuf,WriteUSBLen/4);//����FLASH����               
                Write_Count++;
            }else
            {
                if(WriteRemSIG != 0)//��ʣ������
                {
                    FramesLen = FileSize-(Write_Count*WriteUSBLen);//�õ����һ��д��ĳ���
                    if(f_read(&MyFile_iap,FileReadBuf,FramesLen,&Len) != FR_OK)//��ȡָ�����ȵ����ݵ�RAM
                    {
                        f_close(&MyFile_iap);
                        f_mount(NULL, "", 0);
                        return ;
                    }
                    FLASH_If_Write(((WriteUSBLen*Write_Count)+FLASH_APP1_ADDR),( uint32_t*)FileReadBuf,FramesLen/4);//����FLASH����
                }
                
                f_close(&MyFile_iap);//�ر��ļ�
                f_mount(NULL, "", 0);//�Ͽ�����
                Write_Count = 0;
                UDiskState = FileSysInit;
 
				FLASH_If_Erase(UPGRADE_FLAG_ADDRESS);/*clear app update flag*/
				
				//if(((*(__IO uint32_t*)(UPGRADE_FLAG_ADDRESS)))==0x08000000)
				App_Jump();//ִ��FLASH APP����
            }               
        }
        default:break;
    }  
		
/* USER CODE END 0 */


/* USER CODE END 2 */
