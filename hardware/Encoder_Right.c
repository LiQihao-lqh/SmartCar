#include "stm32f10x.h"                  // Device header

void Encoder_Right_Init(void)//使用编码器计数模式，记录编码器根据旋转记录的高低电平
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);//开时钟，为计数模块准备
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;//旋转信号经过编码，采用上拉输入，输入高低电平
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14;// 两个引脚作为A/B相输入
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	TIM_TimeBaseInitStructure.TIM_ClockDivision=TIM_CKD_DIV1;
	TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up;
	TIM_TimeBaseInitStructure.TIM_Period=65536-1;
	TIM_TimeBaseInitStructure.TIM_Prescaler=1-1;//不分频防止丢数据
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter=0;
	TIM_TimeBaseInit(TIM1,&TIM_TimeBaseInitStructure);
	
	
	TIM_ICInitTypeDef TIM_ICInitStructure;	//输入捕获，高速检测引脚高低电平变化

	TIM_ICStructInit(&TIM_ICInitStructure);//先结构初始化，再配参数

	TIM_ICInitStructure.TIM_Channel=TIM_Channel_1;//选择通道1
	TIM_ICInitStructure.TIM_ICFilter=0xF;//选择最强的滤波
	TIM_ICInit(TIM1, &TIM_ICInitStructure);

	TIM_ICInitStructure.TIM_Channel=TIM_Channel_2;//选择通道2
	TIM_ICInitStructure.TIM_ICFilter=0xF;
	TIM_ICInit(TIM1, &TIM_ICInitStructure);

	TIM_EncoderInterfaceConfig(TIM1, TIM_EncoderMode_TI12, TIM_ICPolarity_Rising, TIM_ICPolarity_Rising);
	//专属的编码器模式，选择时钟，和通道，还有上升/下降沿检测
	TIM_Cmd(TIM1, ENABLE);//启动
}

int32_t Encoder_Right_Get(void)//读取存起来的cnt（编码器的计数值）
{
	int32_t Temp;//把数据临时存起来，因为后面要清零
	Temp = (int16_t)TIM_GetCounter(TIM1);//读取cnt的值,
	//要强制转换，TIM_GetCounter(TIM1)默认是无符号型
	//不然设置负速度会直接变成65535
	TIM_SetCounter(TIM1, 0);//清零防止混乱
	return Temp;
}




