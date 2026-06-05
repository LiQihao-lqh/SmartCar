#include "stm32f10x.h"                  // Device header

void Pwm_Init(void)
{
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);//选TIM2输作为pwm波的计时器
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);			

		GPIO_InitTypeDef GPIO_InitStructure;//按照需求，配置引脚
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用,让外设接管引脚
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_0;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOA, &GPIO_InitStructure);

		TIM_InternalClockConfig(TIM2);//先选择内部输入/内部输入计时器，关闭从模式寄存器
		
		TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;//计时器配置，
		TIM_TimeBaseInitStructure.TIM_ClockDivision=TIM_CKD_DIV1;//滤波器计时器分频
		TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up;
		TIM_TimeBaseInitStructure.TIM_Period=1000-1;//ARR值，这里输出大约24khz
		TIM_TimeBaseInitStructure.TIM_Prescaler=3-1;//调整ARR,PSC的值决定PWM波的频率，10khz-20khz（超出人耳，安静）
		TIM_TimeBaseInitStructure.TIM_RepetitionCounter=0;
		TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStructure);
		
		TIM_OCInitTypeDef TIM_OCInitStructure_Pwm1;//输出比较（pwm）输出设置,左轮pwm输出，
	//通道3的配置，通道2再配置一次,PA2
		TIM_OCStructInit(&TIM_OCInitStructure_Pwm1);	//未设置的保持默认
		TIM_OCInitStructure_Pwm1.TIM_OCMode=TIM_OCMode_PWM1;//输出模式选择
		TIM_OCInitStructure_Pwm1.TIM_OutputState=TIM_OutputState_Enable;	//输出使能(开关)
		TIM_OCInitStructure_Pwm1.TIM_OCPolarity=TIM_OCPolarity_High;//输出极性，决定高电平为有效/无效 
		TIM_OCInitStructure_Pwm1.TIM_Pulse=0;//CRR值
		TIM_OC1Init(TIM2, &TIM_OCInitStructure_Pwm1);
		
		TIM_OCInitTypeDef TIM_OCInitStructure_Pwm2;
		TIM_OCStructInit(&TIM_OCInitStructure_Pwm2);	
		TIM_OCInitStructure_Pwm2.TIM_OCMode=TIM_OCMode_PWM1;
		TIM_OCInitStructure_Pwm2.TIM_OutputState=TIM_OutputState_Enable;	
		TIM_OCInitStructure_Pwm2.TIM_OCPolarity=TIM_OCPolarity_High;
		TIM_OCInitStructure_Pwm2.TIM_Pulse=0;
		TIM_OC2Init(TIM2, &TIM_OCInitStructure_Pwm2);
		
		TIM_Cmd(TIM2, ENABLE);//启动计时器	
}

void PWM_Set_Compare_PWM1(int16_t Compare)//输入参数修改CCR,对输出的pwm波进行控制
{
		TIM_SetCompare1(TIM2,Compare);//通道1，PA0
}	

void PWM_Set_Compare_PWM2(int16_t Compare)
{
		TIM_SetCompare2(TIM2,Compare);//通道2，PA1
}	


