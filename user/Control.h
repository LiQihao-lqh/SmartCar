#ifndef _CONTROL_H
#define _CONTROL_H
//防止变量被篡改，不用这种方法了，采用static封装
//为了防止别人单独使用这一个模块，而不调用stm32f10x.h导致出错
//将Target_Speed，Real_Speed设置为全局变量，其他.c文件调用这写参数只能到这里来
//注意，定义还是要写在 .c里面，然后extern写在对应的.h里面
//extern int32_t Target;
//extern int32_t Left_Real;
//extern int32_t Right_Real;
//设置为全工程的全局变量

#include "stm32f10x.h"//头文件自给自足原则


int32_t Get_Right_Target(void);
int32_t Get_Left_Target(void);
int32_t Get_Left_Real(void);
int32_t Get_Right_Real(void);
int32_t Left_CRR_Get(void);
int32_t Right_CRR_Get(void);
int32_t Get_Target(void);

void TIM4_IRQHandler(void);
void Target_Update(void);
#endif
