#include "ir_sensor.h"
#include "stm32f10x.h"
#include "Delay.h"
#include <string.h>
#include <stdlib.h>

/* ========================== 常量 ========================== */
#define IR_Num         8
#define Package_size   100

/* ========================== 私有变量 ========================== */
static uint8_t  rx_buff[Package_size];
static uint8_t  new_package[Package_size];
static volatile uint8_t g_new_package_flag = 0;

static uint8_t  IR_Data_number[IR_Num];
static uint16_t IR_Data_Anglo[IR_Num];
static uint8_t  g_Amode_Data = 0;
static uint8_t  g_Dmode_Data = 0;

/* ========================== 私有函数声明 ========================== */
static void USART2_Send_U8(uint8_t ch);
static void USART2_Send_ArrayU8(uint8_t *buf, uint16_t len);
static void send_control_data(uint8_t adjust, uint8_t aData, uint8_t dData);
static void Deal_IR_Usart(uint8_t rxtemp);
static void Deal_Usart_Data(void);
static void Deal_Usart_AData(void);
static void splitString(char* mystrArray[], char *str, const char *delimiter);

/* ========================== 底层串口 ========================== */
static void USART2_Send_U8(uint8_t ch)
{
    while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);
    USART_SendData(USART2, ch);
}

static void USART2_Send_ArrayU8(uint8_t *buf, uint16_t len)
{
    while (len--)
        USART2_Send_U8(*buf++);
}

/* ========================== 控制命令发送 ========================== */
static void send_control_data(uint8_t adjust, uint8_t aData, uint8_t dData)
{
    uint8_t send_buf[8] = "$0,0,0#";
    if (adjust == 1) send_buf[1] = '1';
    else             send_buf[1] = '0';

    if (aData == 1)  { send_buf[3] = '1'; g_Amode_Data = 1; }
    else             { send_buf[3] = '0'; g_Amode_Data = 0; }

    if (dData == 1)  { send_buf[5] = '1'; g_Dmode_Data = 1; }
    else             { send_buf[5] = '0'; g_Dmode_Data = 0; }

    USART2_Send_ArrayU8(send_buf, strlen((char*)send_buf));
}

/* ========================== 接收中断 + 拼帧 ========================== */
void USART2_IRQHandler(void)
{
    uint8_t rx_temp;
    if (USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
    {
        USART_ClearITPendingBit(USART2, USART_IT_RXNE);
        rx_temp = USART_ReceiveData(USART2);
        Deal_IR_Usart(rx_temp);
    }
}

static void Deal_IR_Usart(uint8_t rxtemp)
{
    static uint8_t g_start = 0;
    static uint8_t step = 0;

    if (rxtemp == '$')
    {
        g_start = 1;
        rx_buff[step++] = rxtemp;
    }
    else
    {
        if (g_start == 0) return;

        rx_buff[step++] = rxtemp;
        if (rxtemp == '#')
        {
            g_start = 0;
            step = 0;
            memcpy(new_package, rx_buff, Package_size);
            g_new_package_flag = 1;
            memset(rx_buff, 0, Package_size);
        }
        else if (step >= Package_size)
        {
            g_start = 0;
            step = 0;
            memset(rx_buff, 0, Package_size);
        }
    }
}

/* ========================== 数字量解析 ========================== */
static void Deal_Usart_Data(void)
{
    if (new_package[1] != 'D') return;
    for (uint8_t i = 0; i < IR_Num; i++)
    {
        IR_Data_number[i] = new_package[6 + i*5] - '0';
    }
    memset(new_package, 0, Package_size);
}

/* ========================== 模拟量解析 ========================== */
static void splitString(char* mystrArray[], char *str, const char *delimiter)
{
    char *token = strtok(str, delimiter);
    int i = 0;
    while (token != NULL)
    {
        token = strtok(NULL, delimiter);
        mystrArray[i++] = token;
    }
}

static void Deal_Usart_AData(void)
{
    if (new_package[1] != 'A') return;

    char* strArray[10];
    char* strArraytemp[2];
    char  str_temp[Package_size] = {'\0'};
    char  mystr_temp[8][10] = {'\0'};

    strncpy(str_temp, (char*)new_package, strlen((char*)new_package) - 1);

    splitString(strArray, str_temp, ", ");

    for (int i = 0; i < IR_Num; i++)
    {
        strcpy(mystr_temp[i], strArray[i]);
        splitString(strArraytemp, mystr_temp[i], ": ");
        IR_Data_Anglo[i] = atoi(strArraytemp[0]);
    }

    memset(new_package, 0, Package_size);
    memset(strArray, 0, sizeof(strArray));
    memset(strArraytemp, 0, sizeof(strArraytemp));
}

/* ========================== 公共接口 ========================== */

void IR_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    // 使能时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    // TX: PA2
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    // RX: PA3
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // USART2 配置
    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART2, &USART_InitStructure);

    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
    USART_Cmd(USART2, ENABLE);

    // NVIC（不修改分组，沿用 main 中的分组2）
    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;  // 高于 TIM4 的 2
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    Delay_ms(1000);               // 等待传感器启动
    send_control_data(0, 0, 1);   // 初始请求数字量
}

void IR_RequestMode(uint8_t adjust, uint8_t aData, uint8_t dData)
{
    send_control_data(adjust, aData, dData);
}

bool IR_IsDataReady(void)
{
    return (g_new_package_flag == 1);
}

void IR_ProcessNewData(void)
{
    if (g_new_package_flag == 0) return;
    g_new_package_flag = 0;

    if (g_Dmode_Data == 1)
        Deal_Usart_Data();
    else if (g_Amode_Data == 1)
        Deal_Usart_AData();
}

void IR_ReadDigital(uint8_t *x1, uint8_t *x2, uint8_t *x3, uint8_t *x4,
                    uint8_t *x5, uint8_t *x6, uint8_t *x7, uint8_t *x8)
{
    *x1 = IR_Data_number[0];
    *x2 = IR_Data_number[1];
    *x3 = IR_Data_number[2];
    *x4 = IR_Data_number[3];
    *x5 = IR_Data_number[4];
    *x6 = IR_Data_number[5];
    *x7 = IR_Data_number[6];
    *x8 = IR_Data_number[7];
}

uint8_t IR_GetDigitalByte(void)
{
    uint8_t res = 0;
    for (int i = 0; i < IR_Num; i++)
    {
        if (IR_Data_number[i])
            res |= (1 << i);
    }
    return res;
}

void IR_ReadAnalog(uint16_t adc[8])
{
    for (int i = 0; i < 8; i++)
        adc[i] = IR_Data_Anglo[i];
}
