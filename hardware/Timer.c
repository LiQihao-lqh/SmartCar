#include "stm32f10x.h"                  // Device header

void Timer_Init(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);//计时器选择
	
	TIM_InternalClockConfig(TIM4);//编码器模式不用这一步，后面有专门的编码器模式
	
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;//计时器设置
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInitStructure.TIM_Period = 1000 - 1;//设置PSC，ARR.使得每10ms检测一次
	TIM_TimeBaseInitStructure.TIM_Prescaler = 720 - 1;
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;//高级计时器关
	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseInitStructure);
	
	TIM_ClearFlag(TIM4, TIM_FLAG_Update);//// 清除更新中断标志位，防止定时器启动后立即进入中断
	TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);//允许计时器发送中断请求
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//选择中断优先级分组（还不是真正的设置优先级）
	
	NVIC_InitTypeDef NVIC_InitStructure;//中断管家
	NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;//通过引用这个结构体变量，选择要配置结构体成员
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;//打开通道
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;//真正设置优先级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_Init(&NVIC_InitStructure);
	
	
	
	TIM_Cmd(TIM4, ENABLE);
}
//具体的中断产生后去执行的内容
/*
void TIM4_IRQHandler(void)
 TIM4中断处理函数，由CPU自动调用,无需.h声明，而且可以在任意地方使用
注意：里面的内容是全工程唯一的
{
	if (TIM_GetITStatus(TIM4, TIM_IT_Update) == SET)
	{
		
		TIM_ClearITPendingBit(TIM4, TIM_IT_Update);//清除执行标志位
	}
}
*/

