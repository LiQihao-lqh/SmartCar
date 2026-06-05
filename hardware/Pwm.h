#ifndef _PWM_H
#define _PWM_H
void PWM_Set_Compare_PWM1(int16_t Compare);	//输入参数修改CCR,对输出的pwm波进行控制
void PWM_Set_Compare_PWM2(int16_t Compare);
void Pwm_Init(void);//要有输入参数，没有就写：void

#endif
