#include "stm32f10x.h"                  // Device header

void Encoder_Left_Init(void)//使用编码器计数模式，记录编码器根据旋转记录的高低电平
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);//开时钟，为计数模块准备
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;//旋转信号经过编码，采用上拉输入，输入高低电平
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;// 两个引脚作为A/B相输入
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	TIM_TimeBaseInitStructure.TIM_ClockDivision=TIM_CKD_DIV1;
	TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up;
	TIM_TimeBaseInitStructure.TIM_Period=65536-1;
	TIM_TimeBaseInitStructure.TIM_Prescaler=1-1;//不分频防止丢数据
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter=0;
	TIM_TimeBaseInit(TIM3,&TIM_TimeBaseInitStructure);
	
	
	TIM_ICInitTypeDef TIM_ICInitStructure;	//输入捕获，高速检测引脚高低电平变化

	TIM_ICStructInit(&TIM_ICInitStructure);//先结构初始化，再配参数


	TIM_ICInitStructure.TIM_Channel=TIM_Channel_1;//选择通道1
	TIM_ICInitStructure.TIM_ICFilter=0xF;//选择最强的滤波
	TIM_ICInit(TIM3, &TIM_ICInitStructure);

	TIM_ICInitStructure.TIM_Channel=TIM_Channel_2;//选择通道2
	TIM_ICInitStructure.TIM_ICFilter=0xF;
	TIM_ICInit(TIM3, &TIM_ICInitStructure);

	
	
	TIM_EncoderInterfaceConfig(TIM3, TIM_EncoderMode_TI12, TIM_ICPolarity_Rising, TIM_ICPolarity_Rising);
	//专属的编码器模式，选择时钟，和通道，还有上升/下降沿检测
	TIM_Cmd(TIM3, ENABLE);//启动
}

int32_t Encoder_Left_Get(void)//读取存起来的cnt（编码器的计数值）
{
	int32_t Temp;//把数据临时存起来，因为后面要清零
	Temp = (int16_t)TIM_GetCounter(TIM3);//读取cnt的值,
	//要强制转换，TIM_GetCounter(TIM3)默认是无符号型
	//不然设置负速度会直接变成65535
	TIM_SetCounter(TIM3, 0);//清零防止混乱
	return Temp;
}

/*
TIM_GetCounter(TIM3) 返回值类型默认是：

uint16_t（无符号16位）

编码器反转时，
CNT寄存器会出现负数补码。

例如：
反转时 CNT = -1

硬件实际存储的是：
1111111111111111
也就是：
0xFFFF

----------------------------------------
错误写法：

(int32_t)TIM_GetCounter(TIM3)

问题：
TIM_GetCounter() 已经先按 uint16_t 解释了。

所以：

0xFFFF
先被解释成：
65535（uint16_t）

然后再转换成 int32_t：

65535 -> 65535

只是把“65535”扩大成32位，
本质还是正数。

即：

1111111111111111
->
00000000 00000000 11111111 11111111

结果仍然是：
65535

----------------------------------------
正确写法：

(int16_t)TIM_GetCounter(TIM3)

作用：
先把这16位二进制，
按“有符号16位补码”解释。

于是：

1111111111111111

会被解释成：

-1

然后再赋值给 int32_t 时，
编译器会自动进行“符号扩展”：

11111111 11111111 11111111 11111111

最终 Temp 正确得到：
-1

----------------------------------------
重点：

关键不是：
16位还是32位

而是：

“第一次如何解释这16位二进制”

unsigned：
1111111111111111 -> 65535

signed：
1111111111111111 -> -1
*/




