/****************************************Copyright (c)****************************************************
**                                      
**                                 http://www.powermcu.com
**
**--------------File Info---------------------------------------------------------------------------------
** File name:               VS1003.c
** Descriptions:            The VS1003 application function
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
#include "vs1003.h"
#include "spi.h"
#include "music.h"
/* Private variables ---------------------------------------------------------*/

/* VS1003���ò��� */ 
/* 0 , henh.1 , hfreq.2 , lenh.3 , lfreq 5 ,������ */
uint8_t vs1003ram[5] = { 0 , 0 , 0 , 0 , 250 };
		
/*******************************************************************************
* Function Name  : VS1003_SPI_Init
* Description    : VS1003 SPI Init
* Input          : None
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/


/*******************************************************************************
* Function Name  : VS1003_WriteReg
* Description    : VS1003д�Ĵ���
* Input          : - reg: �����ַ
*				   - value: ��������
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
void VS1003_WriteReg(uint8_t reg, uint16_t value)
{
	uint8_t writeBuff[4] = {0};
	
	writeBuff[0] = VS_WRITE_COMMAND;
	writeBuff[1] = reg;
	writeBuff[2] = (value>>8);
	writeBuff[3] = value;

	while(!MP3_DREQ);           /* �ȴ����� */

	MP3_DCS(1); 
	MP3_CCS(0); 

	HAL_SPI_Transmit(&hspi2, writeBuff, 4, 1000);			/* ����VS1003д���� */ 
//	HAL_SPI_Transmit(&hspi2, &writeBuff[0], 1, 1000);			/* ����VS1003д���� */ 
//	
//	HAL_SPI_Transmit(&hspi2, &writeBuff[1], 1, 1000);	/* д�Ĵ��� */ 

//	HAL_SPI_Transmit(&hspi2, &writeBuff[2], 1, 1000);	/* д�Ĵ���ֵ */ 
//  
//	HAL_SPI_Transmit(&hspi2, &writeBuff[3], 1, 1000);	/* д�Ĵ���ֵ */ 
	MP3_CCS(1);         
   
}

/*******************************************************************************
* Function Name  : VS1003_ReadReg
* Description    : VS1003���Ĵ���
* Input          : - reg: �����ַ
* Output         : None
* Return         : - value: ����
* Attention		 : None
*******************************************************************************/
volatile uint16_t value;
uint16_t VS1003_ReadReg( uint8_t reg)
{ 
	uint8_t writeBuff[4] = {0};
	uint8_t readBuff[4] = {0};
	writeBuff[0] = VS_READ_COMMAND;
	writeBuff[1] = reg;
	writeBuff[2] = 0xff;
	writeBuff[3] = 0xff;

	while(MP3_DREQ == 0);           /* �ȴ����� */	 
	MP3_DCS(1);     
	MP3_CCS(0);

	HAL_SPI_Transmit(&hspi2, &writeBuff[0], 1, 1000);			/* ����VS1003�Ķ����� */ 
	
	HAL_SPI_Transmit(&hspi2, &writeBuff[1], 1, 1000);			/* ���ͼĴ���ֵ */ 

	HAL_SPI_TransmitReceive(&hspi2, &writeBuff[2], readBuff, 2, 1000);	/* ���Ĵ���ֵ */ 
	
	MP3_CCS(1);   

	value = (uint16_t)(readBuff[0] << 8) + readBuff[1];
	return value; 
} 


/*******************************************************************************
* Function Name  : VS1003_GetBitrate
* Description    : �õ�mp3&wma�Ĳ�����
* Input          : None
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
uint16_t VS1003_GetBitrate(void)
{
	uint16_t head0;
	uint16_t head1;            
    const uint16_t bitrate[2][16]={ {0,8,16,24,32,40,48,56,64,80,96,112,128,144,160,0}, 
                                    {0,32,40,48,56,64,80,96,112,128,160,192,224,256,320,0}}; 

    head0 = VS1003_ReadReg(SPI_HDAT0); 
    head1 = VS1003_ReadReg(SPI_HDAT1);

    switch( head1 )
    {        
        case 0x7665:	  /* WAV��ʽ */
		     return 0;    
        case 0X4D54:	  /* MIDI��ʽ */
		     return 1;    
        case 0X574D:      /* WMA��ʽ */
             head1 = head0 * 2 / 25;
             if( ( head1 % 10 ) > 5 )
			 {
			     return head1 / 10 + 1;
			 }
             else 
			 {
			     return head1 / 10;
			 }
        default:          /* MP3��ʽ */
            head1 >>= 3;
            head1 = head1 & 0x03; 
            if( head1 == 3 )
			{
			    head1 = 1;
			}
            else 
			{
			    head1 = 0;
			}
            return bitrate[head1][head0>>12];
    } 
} 

/*******************************************************************************
* Function Name  : VS1003_ResetDecodeTime
* Description    : None
* Input          : None
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/                       
void VS1003_ResetDecodeTime(void)
{
   VS1003_WriteReg(SPI_DECODE_TIME,0x0000);
   VS1003_WriteReg(SPI_DECODE_TIME,0x0000); /* �������� */
}

/*******************************************************************************
* Function Name  : VS1003_GetDecodeTime
* Description    : None
* Input          : None
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/    
uint16_t VS1003_GetDecodeTime(void)
{ 
   return VS1003_ReadReg(SPI_DECODE_TIME);   
} 


/*******************************************************************************
* Function Name  : VS1003_SoftReset
* Description    : VS1003��λ
* Input          : None
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
void VS1003_SoftReset(void)
{
	uint8_t writeBuff[4] = {0xff, 0xff, 0xff, 0xff}; 

	VS1003_WriteReg(SPI_CLOCKF, 0X9800);       /* ����vs1003��ʱ��,3��Ƶ ,1.5xADD */
	
//	VS1003_ReadReg(SPI_CLOCKF);

//	VS1003_WriteReg(SPI_AUDATA, 0XBB81);
	
//	VS1003_ReadReg(SPI_AUDATA);

//	VS1003_WriteReg(SPI_VOL,0x2020);	 /* ���� */

//	VS1003_ReadReg(SPI_VOL);	
	
	VS1003_ResetDecodeTime();     /* ��λ����ʱ�� */	
	
	/* ��vs1003����4���ֽ���Ч���ݣ���������SPI���� */	
	MP3_DCS(0);  /* ѡ�����ݴ��� */
	HAL_SPI_Transmit(&hspi2, writeBuff, 4, 1000);
	MP3_DCS(1);  /* ȡ�����ݴ��� */
	HAL_Delay(20);
} 

/*******************************************************************************
* Function Name  : VS1003_Reset
* Description    : VS1003��λ
* Input          : None
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
void VS1003_Reset(void)
{
//	const uint8_t writeBuff_1[3] = {0x00, 0x08, 0x04}; 
//	const uint8_t writeBuff_2[3] = {0x03, 0x98, 0x00};	// ����Ƶ 
//	const uint8_t writeBuff_3[3] = {0x05, 0xbb, 0x81}; 	//������48k ������
//	const uint8_t writeBuff_4[3] = {0x02, 0x00, 0x55}; 	//��������
//	const uint8_t writeBuff_5[3] = {0x0b, 0x40, 0x40};	//��������
//	const uint8_t writeBuff_6[4] = {0};
	uint8_t writeBuff[1] = {0xff};
	uint8_t readBuff[1] = {0};
	
	MP3_RST(0);
	HAL_Delay(20);
	HAL_SPI_TransmitReceive(&hspi2, writeBuff, readBuff, 1, 1000);
	MP3_DCS(1);              /* ȡ�����ݴ��� */
	MP3_CCS(1);	
	MP3_RST(1);
	HAL_Delay(20);
	while(MP3_DREQ == 0);  /* �ȴ�DREQΪ�� */
	HAL_Delay(20);
}

/*******************************************************************************
* Function Name  : VS1003_RamTest
* Description    : VS1003 RAM����
* Input          : None
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/																			 
void VS1003_RamTest(void)
{
	const uint8_t writeBuff[8] = {0x4d, 0xea, 0x6d, 0x54, 0x00, 0x00, 0x00, 0x00};
	volatile uint16_t value;
	
	VS1003_Reset(); 
	VS1003_WriteReg(SPI_MODE,0x0820); /* ����vs1003�Ĳ���ģʽ */

	while(MP3_DREQ == 0);          /* �ȴ�DREQΪ�� */
	MP3_DCS(0);	       			     /* xDCS = 1��ѡ��vs1003�����ݽӿ� */
	
	HAL_SPI_Transmit(&hspi2, writeBuff, 8, 1000);
	HAL_Delay(50);  
	MP3_DCS(1);
	value = VS1003_ReadReg(SPI_HDAT0); /* ����õ���ֵΪ0x807F���������� */
}    


void VS1003_PlayMusic()
{
	MP3_CCS(1);
	MP3_DCS(0);
	HAL_Delay(5);
	int i = 0;
	for(i=0;i<15000;i++)
	{
		 while(!MP3_DREQ);  //VS1003��DREQΪ�߲���д������
		//HAL_SPI_Transmit_DMA(&hspi2, &music[i], 1);
		HAL_SPI_Transmit(&hspi2, &music[i], 1, 0xffff);
	}
}


/*********************************************************************************************************
      END FILE
*********************************************************************************************************/

