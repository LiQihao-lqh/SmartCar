#include "stm32f10x.h"                  // Device header
#include "PWM.h"
void Motor_Init()//电机驱动代码
{	
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);		//开GPIO口时钟

		GPIO_InitTypeDef GPIO_InitStructure;		//初始化，作为pwm输出波形接口
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP;//这里用普通输出模式，deepseek也没检查出来		
	//驱动代码不是复用推挽输出，只有pwm输出是复用推文玩输出复用输出：外设控制引脚。普通：gpio控制		
		GPIO_InitStructure.GPIO_Pin=GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_11 | GPIO_Pin_12;		
		GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
		GPIO_Init(GPIOA,&GPIO_InitStructure);
		 
		Pwm_Init();
}
	
void Set_Left_Speed(int16_t CRR)//读取速度，控制正反转
	//这里的Speed作为接受数据的参数，和main里的Speed不是同一个东西
{
	if(CRR>=0)
	{
	GPIO_SetBits(GPIOA, GPIO_Pin_4); 
	GPIO_ResetBits(GPIOA, GPIO_Pin_5);	
	}else
		{	
			GPIO_SetBits(GPIOA, GPIO_Pin_5);
			GPIO_ResetBits(GPIOA, GPIO_Pin_4);
			
		}

}

void Set_Right_Speed(int16_t CRR)//读取速度，控制正反转
	//这里的Speed作为接受数据的参数，和main里的Speed不是同一个东西
{
	if(CRR>=0)
	{
	GPIO_SetBits(GPIOA, GPIO_Pin_11);
	GPIO_ResetBits(GPIOA, GPIO_Pin_12);	
	}else
		{	
			GPIO_SetBits(GPIOA, GPIO_Pin_12);
			GPIO_ResetBits(GPIOA, GPIO_Pin_11);
			
		}
} 
 