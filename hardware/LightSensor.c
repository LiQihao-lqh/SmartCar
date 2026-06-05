#include "stm32f10x.h"                  // Device header

void LightSensor(void)
{
RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	GPIO_InitTypeDef GPIO_Initstructrue;
	GPIO_Initstructrue.GPIO_Mode =GPIO_Mode_IPU;
	GPIO_Initstructrue.GPIO_Pin =GPIO_Pin_13;
	GPIO_Initstructrue.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&GPIO_Initstructrue);


}
uint8_t LightSensor_Get(void)
{
	return GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_13);
	
	
	
}


