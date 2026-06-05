#include "stm32f10x.h"                  // Device header
#include "Delay.h"
void key_Init(void)
{	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	GPIO_InitTypeDef GPIO_Initstructrue;
	GPIO_Initstructrue.GPIO_Mode =GPIO_Mode_IPU;
	GPIO_Initstructrue.GPIO_Pin =GPIO_Pin_1|GPIO_Pin_11;
	GPIO_Initstructrue.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&GPIO_Initstructrue);

}
uint8_t key_GetNum(void)
{
	uint8_t keyNum = 0;
	if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_1)==0)//按下增速
{	Delay_ms(20);
	while(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_1)==0);//死循环直到松手结束
	Delay_ms(20);
	keyNum = 1;
}
	if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_11)==0)//按下减速
{Delay_ms(20);
	while(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_11)==0);//死循环直到松手结束
	Delay_ms(20);
	keyNum = 2;
}
	
	
	
	return keyNum;
}


