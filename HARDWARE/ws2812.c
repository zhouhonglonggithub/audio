#include "ws2812.h"
#include "Adafruit_NeoPixel.h"
#include "math.h"
#include "tim.h"
//#include "stdlib.h"
/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/
#define FLASH_WHEEL_1 1
#define PI                      3.1415692f
/*TIM+DMA���*/
#define BIT_1                   61
#define BIT_0                   28
void WS2812B_Init(void);
void WS2812_set_flash(uint8_t flash,uint16_t angle);
void WS2812_Process(uint8_t model);


/*----------------------------------------------*
 * �ڲ�����ԭ��˵��                             *
 *----------------------------------------------*/
void WS2812_show(void);

/*----------------------------------------------*
 * ȫ�ֱ���                                     *
 *----------------------------------------------*/
uint8_t rBuffer[PIXEL_MAX] = {0};
uint8_t gBuffer[PIXEL_MAX] = {0};
uint8_t bBuffer[PIXEL_MAX] = {0};

/*----------------------------------------------*
 * ģ�鼶����                                   *
 *----------------------------------------------*/
typedef struct
{
    const uint16_t head[3];              //�ȷ���3��0�ȴ�dma�ȶ�
    uint16_t data[24 * PIXEL_MAX];       //����������
    const uint16_t tail;                 //�����һ��0����֤dma������pwm�����
} frame_buf_ST;

frame_buf_ST frame = { .head[0] = 0,
                       .head[1] = 0,
                       .head[2] = 0,
                       .tail    = 0,
                     };

uint8_t     gFlash_Mode     = 2;
uint8_t     f9_state        = 0;
uint16_t    Flash1_speed_L, Flash1_speed_R;


/*---------------------------------�ڲ�����---------------------------------*/
/*****************************************************************************
 �� �� ��  : WS2812B_Init
 ��������  : WS2812��ʼ����������led����
            TIM5��DMA�ĳ�ʼ����main��������
 �������  : void
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :
*****************************************************************************/
void WS2812B_Init(void)
{
    uint8_t i = 0;
    for(i = 0; i < 30; i++)
    {
        gBuffer[i] = 0;
        rBuffer[i] = 0;
        bBuffer[i] = 0;
    }
    WS2812_show();
}

/*****************************************************************************
 �� �� ��  : WS2812_show
 ��������  : �����������������飬��ͨ��DMA����һ֡����
 �������  : void
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :
*****************************************************************************/
void WS2812_show(void)
{
    int8_t i, j;

    for(i = 0; i < PIXEL_MAX; i++)
    {
        for(j = 0; j < 8; j++)
        {
            frame.data[24 * i + j]       = (gBuffer[i] & (0x80 >> j)) ? BIT_1 : BIT_0;
            frame.data[24 * i + j + 8]   = (rBuffer[i] & (0x80 >> j)) ? BIT_1 : BIT_0;
            frame.data[24 * i + j + 16]  = (bBuffer[i] & (0x80 >> j)) ? BIT_1 : BIT_0;
        }
    }
    HAL_TIM_PWM_Start_DMA(&htim3, TIM_CHANNEL_4, (uint32_t *)&frame, 3 + 24 * PIXEL_MAX + 1);
		HAL_Delay(1);
}

/*****************************************************************************
 �� �� ��  : HAL_TIM_PWM_PulseFinishedCallback
 ��������  : PWM����жϻص���������Ϊhal������û�йص�PWM,����Ҫ����ɺ��Լ��ص�
 �������  : void
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :
*****************************************************************************/
void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim)
{
    HAL_TIM_PWM_Stop(&htim3,TIM_CHANNEL_4);
}

/*****************************************************************************
 �� �� ��  : sign
 ��������  : �������Ϊ�����򷵻�1��Ϊ������-1�����򷵻�0
 �������  : int16_t db
 �������  : ��
 �� �� ֵ  : 0��-1,1
 ���ú���  :
 ��������  :
*****************************************************************************/
int8_t sign(int16_t db)
{
    int8_t result;

    if(db == 0)
        result = 0;
    else  if( db > 0)
        result = 1;
    else
        result = -1;

    return result;
}

/*****************************************************************************
 �� �� ��  : WS2812B_Wheel_1
 ��������  : �����ⲿ�����������ٶȣ�����������ٶ�
             �ٶ����ú���Ϊvoid WS2812_set_Wheel_1(int16_t speed_L,int16_t speed_R)��

 �������  : void
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  : WS2812B_Process
*****************************************************************************/
void WS2812B_Wheel_1(void)  //
{

    static int8_t  cnt_l, cnt_r;
    static int8_t  head_l, head_r;
    //static uint8_t  tab[8]={0,0,0,80,150,170,180,255};
    static uint8_t  tab[8] = {0, 0, 0, 0, 0, 0, 0, 255};
    uint32_t color_buf;
    uint16_t Time_L;
    uint16_t Time_R;
    uint8_t i, n;

    if(Flash1_speed_L == 0)
    {
        Time_L = 0;
    }
    else
    {
        Time_L = 1000 / (16 * fabs(Flash1_speed_L));
    }

    if(Flash1_speed_R == 0)
    {
        Time_R = 0;
    }
    else
    {
        Time_R = 1000 / (16 * fabs(Flash1_speed_R));
    }

    if(cnt_l > Time_L)
    {
        cnt_l = 0;
        n = head_l;

        for(i = 0; i < 8; i++)
        {
            if(n > 7) n = 0;

            color_buf = NEO_WHI(tab[i]);
//            SetPixelColor(n,color_buf);
//            SetPixelColor(n+8,color_buf);
            SetPixelColor(n + 16, color_buf);
            SetPixelColor(n + 24, color_buf);
            n++;
        }

        //�жϼ���
        if(sign(Flash1_speed_L) == -1)
        {
            head_l++;

            if(head_l > 7)
                head_l = 0;
        }
        else
        {
            head_l--;

            if(head_l < 0)
                head_l = 7;
        }

        //   WS2812_show();
    }

    cnt_l++;

    if(cnt_r > Time_R)
    {
        cnt_r = 0;
        n = head_r;

        for(i = 0; i < 8; i++)
        {
            if(n > 7) n = 0;

            color_buf = NEO_WHI(tab[i]);
            SetPixelColor(n, color_buf);
            SetPixelColor(n + 8, color_buf);
//            SetPixelColor(n+16,color_buf);
//            SetPixelColor(n+24,color_buf);
            n++;
        }

        //�жϼ���
        if(sign(Flash1_speed_R) == 1)
        {
            head_r++;

            if(head_r > 7)
                head_r = 0;
        }
        else
        {
            head_r--;

            if(head_r < 0)
                head_r = 7;
        }
    }

    cnt_r++;
}
/*****************************************************************************
 �� �� ��  : WS2812_flash_1
 ��������  : ��ɫ��ˮ�� 0.5sһ��

 �������  : void
 �������  : ��
 �� �� ֵ  :
 ���ú���  : WS2812_Process
 ��������  :
*****************************************************************************/
void WS2812_flash_1(void)
{
    const  uint32_t FlashPeriod_ms = 100;
    static uint8_t  state          = 0;
    static uint8_t  numl           = 15;
    static uint8_t  numr           = 15;
    static uint32_t next_time      = 0;
    uint32_t scolor;
    uint32_t timestamp = HAL_GetTick();

    uint32_t flag = 0;
    if(next_time < FlashPeriod_ms)
    {
        if((uint64_t)timestamp + HAL_MAX_DELAY -  next_time > 0)
            flag = 1;
    }
    else if(timestamp > next_time)
    {
        flag = 1;
    }

    if(flag)// && timestamp - next_time < FlashPeriod_ms*5)
    {
        next_time = timestamp + FlashPeriod_ms;
        numl++;

        if(numl > 15)
            numl = 0;
        numr = 15 - numl;
        if(numr > 15)
            numr -= 16;
        SetAllPixelColor(0);
        switch(state++)
        {
        case 1:
            scolor = FIX_RED;
            break;
        case 2:
            scolor = FIX_ORG;
            break;
        case 3:
            scolor = FIX_YLW;
            break;
        case 4:
            scolor = FIX_GRN;
            break;
        case 5:
            scolor = FIX_CYA;
            break;
        case 6:
            scolor = FIX_BLU;
            break;
        case 7:
            scolor = FIX_PUR;
            state = 1;
            break;
        default:
            state = 1;
            break;
        }
        for(int i = 0; i<15; i++)
        {
            SetPixelColor(numl+i, scolor);
        }
        for(int i = 0; i<15; i++)
            SetPixelColor(16 + numr-i, scolor);
    }
//		flag = 0;
    HAL_Delay(20);
}
/*****************************************************************************
 �� �� ��  : WS2812_flash_2
 ��������  : ��ɫ��ˮ�� 0.5sһ��
 �������  : void
 �������  : ��
 �� �� ֵ  :
 ���ú���  : WS2812_Process
 ��������  :
*****************************************************************************/
void WS2812_flash_2(void)
{
    const static uint32_t FlashPeriod_ms = 162;
    static uint8_t numl = 0;
//    static uint8_t numr = 0;
    static uint32_t next_time = 0;
    uint32_t timestamp = HAL_GetTick();

    static uint8_t  loop = 0;
    if(loop == 0) next_time = timestamp;
    loop = 1;  //�״ε��ó�ʼ��

    if(timestamp > next_time)// && timestamp - next_time < FlashPeriod_ms*5)
    {
        next_time = timestamp + FlashPeriod_ms;
        numl++;

        if(numl >= PIXEL_MAX)
            numl = 0;

        SetAllPixelColor(0);
        SetPixelColor(numl, FIX_GRN);
    }
}

/*****************************************************************************
 �� �� ��  : WS2812_flash_3
 ��������  : ��ɫ������
            �����ҷ�ʽ��������Ч��
 �������  : void
 �������  : ��
 �� �� ֵ  :
 ���ú���  : WS2812_Process
 ��������  :
*****************************************************************************/

void WS2812_flash_3(void)
{
    static uint32_t next_time      = 0;
    const  uint32_t FlashPeriod_ms = 1000;      //��������
    const  uint8_t  DPI            = 100;       //�ֱ���
    const  uint8_t  max            = 100;       //�������
    const  uint8_t  min            = 0;         //��С����
    static uint8_t  cnt            = 0;
    uint8_t  brighten;
    uint32_t timestamp = HAL_GetTick();

    static uint8_t  loop = 0;
    if(loop == 0) next_time = timestamp;
    loop = 1;  //�״ε��ó�ʼ��

    if(timestamp > next_time)// && timestamp - next_time < FlashPeriod_ms*5)
    {
        next_time = timestamp + FlashPeriod_ms / DPI;

        if(cnt++ > DPI)cnt = 0;
        brighten = (max + min) / 2 + ((max - min) / 2) * sin(2 * PI * cnt / DPI);
        SetAllPixelColor(NEO_GRN(brighten));

    }
}
/*****************************************************************************
 �� �� ��  : WS2812_flash_4
 ��������  : ��ɫ������
 �������  : void
 �������  : ��
 �� �� ֵ  :
 ���ú���  : WS2812_Process
 ��������  :
*****************************************************************************/
void WS2812_flash_4(void)
{
    static uint32_t next_time      = 0;
    const  uint32_t FlashPeriod_ms = 1000;      //��������
    const  uint8_t  DPI            = 100;       //�ֱ���
    const  uint8_t  max            = 100;       //�������
    const  uint8_t  min            = 0;         //��С����
    static uint8_t  cnt            = 0;
    uint8_t  brighten;
    uint32_t timestamp = HAL_GetTick();

    static uint8_t  loop = 0;
    if(loop == 0) next_time = timestamp;
    loop = 1;  //�״ε��ó�ʼ��

    if(timestamp > next_time)// && timestamp - next_time < FlashPeriod_ms*5)
    {
        next_time = timestamp + FlashPeriod_ms / DPI;

        if(cnt++ > DPI)cnt = 0;
        brighten = (max + min) / 2 + ((max - min) / 2) * sin(2 * PI * cnt / DPI);
        SetAllPixelColor(NEO_CYA(brighten));

    }
}
/*****************************************************************************
 �� �� ��  : WS2812_flash_5
 ��������  : ��ɫ����
 �������  : void
 �������  : ��
 �� �� ֵ  :
 ���ú���  : WS2812_Process
 ��������  :
*****************************************************************************/
void WS2812_flash_5(void)
{
    SetAllPixelColor(FIX_CYA);
    WS2812_show();
}

/*****************************************************************************
 �� �� ��  : WS2812_flash_6
 ��������  : ��ɫ����
 �������  : void
 �������  : ��
 �� �� ֵ  :
 ���ú���  : WS2812_Process
 ��������  :
*****************************************************************************/
void WS2812_flash_6(void)
{
    SetAllPixelColor(FIX_YLW);
    WS2812_show();
}

/*****************************************************************************
 �� �� ��  : WS2812_flash_7
 ��������  : ��ɫ����
 �������  : void
 �������  : ��
 �� �� ֵ  :
 ���ú���  : WS2812_Process
 ��������  :
*****************************************************************************/
void WS2812_flash_7(void)
{
    SetAllPixelColor(FIX_RED);
    WS2812_show();
}

/*****************************************************************************
 �� �� ��  : WS2812_flash_8
 ��������  : ��ɫ������
 �������  : void
 �������  : ��
 �� �� ֵ  :
 ���ú���  : WS2812_Process
 ��������  :
*****************************************************************************/
void WS2812_flash_8(void)
{
    uint32_t timestamp      = HAL_GetTick();
    static uint32_t next_time      = 0;
    const  uint32_t FlashPeriod_ms = 1000;      //��������
    const  uint8_t  DPI            = 100;       //�ֱ���
    const  uint8_t  max            = 100;       //�������
    const  uint8_t  min            = 0;         //��С����
    static uint8_t  cnt            = 0;
    uint8_t  brighten;

    static uint8_t  loop = 0;
    if(loop == 0) next_time = timestamp;
    loop = 1;  //�״ε��ó�ʼ��

    if((timestamp > next_time))// && (timestamp - next_time < FlashPeriod_ms*5))
    {
        next_time = timestamp + FlashPeriod_ms / DPI;

        if(cnt++ > DPI)cnt = 0;
        brighten = (max + min) / 2 + ((max - min) / 2) * sin(2 * PI * cnt / DPI);
        SetAllPixelColor(NEO_RED(brighten));

    }
}
/*****************************************************************************
 �� �� ��  : WS2812_flash_9
 ��������  : �߲ʱ任
 �������  : void
 �������  : ��
 �� �� ֵ  :
 ���ú���  : WS2812_Process
 ��������  :
*****************************************************************************/
void WS2812_flash_9(void)
{
    uint32_t timestamp = HAL_GetTick();
    const  static uint32_t FlashPeriod_ms = 1000;
    static uint32_t next_time = 0;

    static uint8_t  loop = 0;
    if(loop == 0) next_time = timestamp;
    loop = 1;  //�״ε��ó�ʼ��


    if(f9_state == 0)
    {
        next_time = timestamp + FlashPeriod_ms;
        f9_state = 1;
    }
    if((timestamp > next_time))// && (timestamp - next_time < FlashPeriod_ms*5))
    {
        next_time = timestamp + FlashPeriod_ms;
        f9_state++;

        if(f9_state > 7)
            f9_state = 1;
    }

    switch(f9_state)
    {
    case 1:
        SetAllPixelColor(FIX_RED);
        break;

    case 2:
        SetAllPixelColor(FIX_ORG);
        break;

    case 3:
        SetAllPixelColor(FIX_YLW);
        break;

    case 4:
        SetAllPixelColor(FIX_GRN);
        break;

    case 5:
        SetAllPixelColor(FIX_CYA);
        break;

    case 6:
        SetAllPixelColor(FIX_BLU);
        break;

    case 7:
        SetAllPixelColor(FIX_PUR);
        break;

    case 8:
        SetAllPixelColor(FIX_WHI);
        break;

    case 9:
        SetAllPixelColor(FIX_BLK);
        break;

    default:
        f9_state = 1;
        break;
    }
}


/*****************************************************************************
 �� �� ��  : set_micdir
 ��������  : ����ָʾ�����led
             ���Ժ���
 �������  : void
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :
*****************************************************************************/
void set_micdir(uint16_t data)
{
    const static uint16_t space_angle = 360/PIXEL_MAX;

//    if(data == 0)
//    {
//        rainbow(10);
//    }
    int8_t dir_led;
//    static uint32_t led_tab[PIXEL_MAX] = {0x00ff00,0x5F0000,0x3F0000,0x2F0000,0x1F0000,0x0F0000,0x080000,0x010000};//,0x010000
//    int8_t led_mab[20];
    uint8_t borrow;
    if(data < 360)
    {
        /*
        	��һ����18��LED������£�1-18����led1�ĽǶ�Ϊ350-369(09)LED2=10-29,LED3=30-59 ......
        	���Ե��������ڵ���10ʱ��Ӧ���¸�led��
        */
        dir_led = (359-data) /space_angle;  //�ĸ�ledӦ����
        borrow = (359-data) % space_angle;	// ������
        if(borrow < 10) 					//�Ƿ��λ
        {
            if(--dir_led <0)
                dir_led = PIXEL_MAX-1;
        }

        //Ӳ������һλ
        dir_led++;
        if(dir_led > PIXEL_MAX-1)
            dir_led = 0;

        SetAllPixelColor(0);
        SetPixelColor(dir_led,FIX_GRN);

//        led_mab[0] = dir_led;
//		//���ǹ�0������������led˳��
//        for(i=1; i<8; i++)
//        {
//            led_mab[2*i-1] = dir_led + i;
//            led_mab[2*i]   = dir_led - i;
//            if(led_mab[2*i-1] >= PIXEL_MAX ) led_mab[2*i-1] -= PIXEL_MAX;
//            if(led_mab[2*i]   < 0 )  led_mab[2*i] += PIXEL_MAX;
//        }
//        SetAllPixelColor(0);
//        SetPixelColor(led_mab[0],led_tab[0]);
//        for(i=1; i<8; i++)
//        {
//            SetPixelColor(led_mab[2*i],led_tab[i]);
//            SetPixelColor(led_mab[2*i-1],led_tab[i]);
//        }

    }
}



/*****************************************************************************
 �� �� ��  : WS2812_Process
 ��������  : WS2812�����̺���,Ӧ��ѭ�����ã�
 �������  : void
 �������  : ��
 �� �� ֵ  :
 ���ú���  : freertos.c StartLEDTask02()
 ��������  :
*****************************************************************************/
void WS2812_Process(uint8_t model)
{
//    const uint8_t TestMode = 0xf0;
    {
        gFlash_Mode = model;
        switch(gFlash_Mode)
        {
        case 0x01:
            rainbowCycle(10);
            //WS2812_flash_1();     //��ɫ�����
            break;

        case 0x02:
            WS2812_flash_2();       //��ɫ�����
            break;

        case 0x03:
            WS2812_flash_3();       //��ɫ������
            break;

        case 0x04:
            WS2812_flash_4();       //��ɫ������
            break;
        case 0x05:
            WS2812_flash_5();       //��ɫ����
            break;
        case 0x06:
            WS2812_flash_6();       //��ɫ����
            break;
        case 0x07:
            WS2812_flash_7();       //��ɫ����
            break;
        case 0x08:
            WS2812_flash_8();       //��ɫ����
            break;
        case 0x09:
            //WS2812_flash_9();     //��ɫ�л�
            rainbow(20);
            break;
        case 0x0A:
            rainbowCycle(10);
            break;
        case 0x0B:
            //rainbowCycle(10);
            rainbow(20);
            break;
        default:
            gFlash_Mode = 0X02;
            break;
        }
        WS2812_show();
        HAL_Delay(10);
    }
}
/*****************************************************************************
 �� �� ��  : WS2812_set_flash
 ��������  : ����flashģʽ
 �������  : uint8_t flash
 �������  : ��
 �� �� ֵ  :
 ���ú���  : uusart.c usart_cmd()
 ��������  :
*****************************************************************************/
void WS2812_set_flash(uint8_t flash,uint16_t angle)
{
#ifdef DEBUG_ERR
    if(flash > 10)
    {
        printf("flash mode order err \r\n");
    }
#endif
    gFlash_Mode = flash;
    if(gFlash_Mode == 1)
    {
        set_micdir(angle);
    }
    if(flash == 9)      //��֤flash9 ÿ�δӺ쿪ʼ
    {
        f9_state = 0;
    }
}

/*****************************************************************************
 �� �� ��  : WS2812_set_Wheel_1
 ��������  : ��ת��ģʽ�£������ٶ�
 �������  : int16_t speed_L
             int16_t speed_R
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :
*****************************************************************************/
void WS2812_set_Wheel_1(int16_t speed_L, int16_t speed_R)
{
    Flash1_speed_L = speed_L;
    Flash1_speed_R = speed_R;
}


void WS2812B_Test(void)
{
    WS2812_show();
    //return;
    //SetAllPixelColor(0xffffff);
    //SetAllPixelColor(0);
//    uint8_t cnt = 0;
    uint16_t num = 0;
    while(1)
    {
        //rainbowCycle(10);
        //rainbow(10);
//        while(!cnt--)
//        {
//            cnt = 1;
        set_micdir(num++);
        if(num >= 360)
            num = 0;
//        }

        WS2812_show();
        HAL_Delay(10);


        //theaterChase(NEO_BLU(100),100);
        //theaterChaseRainbow(10);
    }
}
void randled()
{
    uint16_t sexiang;
    for(int i=0; i<30; i++)
    {
        sexiang=rand() % 360;
        SetnLedHSVColor(i, sexiang,100,100);
    }
    WS2812_show();
    HAL_Delay(300);
}
void SetLedHSVColor(uint16_t h,uint8_t s,uint8_t v)
{
    uint8_t r,g,b;
    HSVtoRGB(&r, &g, &b, h, s,v);  //����HSVתRGB�����������rgbֵ
    setAllPixelColor(r, g, b);
}

void SetnLedHSVColor(uint16_t n,uint16_t h,uint8_t s,uint8_t v)
{
    uint8_t r,g,b;
		if(h>359)h=359;
		if(v>100)h=100;
    HSVtoRGB(&r, &g, &b, h, s,v);  //����HSVתRGB�����������rgbֵ
    setPixelColor(n, r, g, b);

}

/*HSVתRGB���� *r *g *b:rgb������ַָ�룻hɫ��0~359 s���Ͷ�0~100, v����0~100*/
void HSVtoRGB(uint8_t *r, uint8_t *g, uint8_t *b, uint16_t h, uint8_t s, uint8_t v)
{
    int i,difs;
    float RGB_min, RGB_max,RGB_Adj;

    RGB_max = v*2.55f;
    RGB_min = RGB_max*(100 - s) / 100.0f;
    i = h / 60;
    difs = h % 60;
    RGB_Adj = (RGB_max - RGB_min)*difs / 60.0f;

    switch (i) {
    case 0:
        *r = RGB_max;
        *g = RGB_min + RGB_Adj;
        *b = RGB_min;
        break;
    case 1:
        *r = RGB_max - RGB_Adj;
        *g = RGB_max;
        *b = RGB_min;
        break;
    case 2:
        *r = RGB_min;
        *g = RGB_max;
        *b = RGB_min + RGB_Adj;
        break;
    case 3:
        *r = RGB_min;
        *g = RGB_max - RGB_Adj;
        *b = RGB_max;
        break;
    case 4:
        *r = RGB_min + RGB_Adj;
        *g = RGB_min;
        *b = RGB_max;
        break;
    default:
        *r = RGB_max;
        *g = RGB_min;
        *b = RGB_max - RGB_Adj;
        break;
    }
}

