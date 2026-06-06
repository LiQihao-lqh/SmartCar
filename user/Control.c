//chatgpt结构优化版
#include "stm32f10x.h"
#include "Encoder_Left.h"
#include "Encoder_Right.h"
#include "key.h"
#include "Pwm.h"
#include "Motor.h"

/*
========================================
双电机直线控制核心逻辑

控制流程：
1. 按键修改目标速度
2. 定时读取左右轮实际速度
3. 计算误差与左右轮差值
4. 调整控制输出（CRR）
5. 输出PWM控制电机

核心思想：
- 单轮闭环：让每个轮子接近目标速度
- 差速修正：让左右轮速度尽量一致
========================================
*/

/*========== 模块内全局变量 ==========*/

// 目标速度（左右轮共同目标）
static int32_t Left_Target = 0;
static int32_t Right_Target = 0;
static int32_t Target = 0;


// 左右轮实际速度（编码器测得）
static int32_t Left_Real  = 0;
static int32_t Right_Real = 0;

// 左右轮误差
static int32_t Left_Error  = 0;
static int32_t Right_Error = 0;

// 左右轮最终输出值
// 正负代表方向
static int32_t Left_CRR  = 0;
static int32_t Right_CRR = 0;

// 寻线误差速度差
static int32_t Line_Error  = 0;

//pid控制
static float Speed_Kp=1.0f;
static float Turn_Out_Kp=0.2f;
static int32_t Turn_Out;



// PWM限幅，define作用是给数字起名字，方便修改理解
#define PWM_MAX   1000
#define PWM_MIN  -1000

//设置一个基础的crr，然后在此之上进行调节
#define Base_Pwm 100


/*
========================================
目标速度更新函数

按键1：加速
按键2：减速
========================================
*/
void Target_Update(void)
{
    uint8_t Key;

    Key = key_GetNum();

    if(Key == 1)
    {
		Target  +=10;
    }

    if(Key == 2)
    {
		
		Target  -=10;
    }

}

/*
========================================
TIM4中断函数（10ms执行一次）

控制周期：
1. 读取速度
2. 计算误差
3. 微调输出
4. 输出PWM

注意：
这里只调一点点
然后等待下一次测速再继续调整
避免系统震荡
========================================
*/
void TIM4_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM4, TIM_IT_Update) == SET)
    {

        /*========== 1. 读取左右轮实际速度 ==========*/

        Left_Real  = Encoder_Left_Get();
        Right_Real = Encoder_Right_Get();

        /*========== 2. 计算误差与速度差 ==========*/

        // 单轮误差

        // 左右轮速度差
        Line_Error  = Left_Real - Right_Real;//这个只能跑直线，Line_Error 数据来源换成灰度传感器才能转弯

		/*========== 4. 左右轮同步修正 ==========*/
		//根据line_error正负修正
		Turn_Out = (int32_t)Turn_Out_Kp*Line_Error ;
		Left_Target =Target-Turn_Out; 
		Right_Target =Target+Turn_Out; 
		
		Left_Error  = Left_Target - Left_Real;
        Right_Error = Right_Target - Right_Real;

        /*========== 3. 单轮速度闭环 ==========*/

        // 左轮调整
        if(Target==0)
		{
		Left_CRR = 0;
		Right_CRR = 0;
		}
		else
		{
		Left_CRR  = Base_Pwm + (int32_t)(Speed_Kp * Left_Error);
		Right_CRR = Base_Pwm + (int32_t)(Speed_Kp * Right_Error);
		}



        /*========== 5. 输出限幅 ==========*/

        if(Left_CRR > PWM_MAX)
        {
            Left_CRR = PWM_MAX;
        }

        if(Left_CRR < PWM_MIN)
        {
            Left_CRR = PWM_MIN;
        }

        if(Right_CRR > PWM_MAX)
        {
            Right_CRR = PWM_MAX;
        }

        if(Right_CRR < PWM_MIN)
        {
            Right_CRR = PWM_MIN;
        }

        /*========== 6. 设置电机方向 ==========*/

        // 正负决定方向
        Set_Left_Speed(Left_CRR);
        Set_Right_Speed(Right_CRR);

        /*========== 7. 输出PWM占空比 ==========*/

        // PWM只需要占空比大小
        // 方向已经由Motor模块处理

        if(Left_CRR >= 0)
        {
            PWM_Set_Compare_PWM1(Left_CRR);
        }
        else
        {
            PWM_Set_Compare_PWM1(-Left_CRR);
        }


        if(Right_CRR >= 0)
        {
            PWM_Set_Compare_PWM2(Right_CRR);
        }
        else
        {
            PWM_Set_Compare_PWM2(-Right_CRR);
        }

        /*========== 8. 清除中断标志位 ==========*/

        TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
    }
}
int32_t Get_Left_Target(void)
{
	return Left_Target;
}
int32_t Get_Right_Target(void)
{
	return Right_Target;
}
int32_t Get_Left_Real(void)
{
	return Left_Real;
}
int32_t Get_Right_Real(void)
{
	return Right_Real;
}


//原版本
//#include "stm32f10x.h"// Device header
//#include "Encoder_Left.h"
//#include "ENCODER_RIGHT.h"
//#include "key.h"
//#include "Pwm.h"
//#include "Motor.h"

///*
//算法（核心逻辑）：
//设置/读取，储存数据
//处理数据
//输出数据给被控制对象（motor，pwm的crr）
//*/

//uint8_t Key;
//int32_t Left_Target;//按键设置速度
//int32_t Left_Real;//编码器测得数据
//int32_t Right_Target;
//int32_t Right_Real;
//int32_t Target;


//int16_t Left_Error;//误差
//int16_t Left_CRR;
//int16_t Right_Error;//误差
//int16_t Right_CRR;

//int16_t Correction;

//int32_t Target_Update(void)//按键设置目标速度
//{	
//	Key=key_GetNum();//设置按键控制目标速度
//	
//	if(Key==1)//1加速，2减速
//	{
//	Target+=10;
//	}
//	if(Key==2)
//	{
//	Target-=10;
//	}
//	return  Target;
//}



//void TIM4_IRQHandler(void)//不需要timer头文件就能调用这个函数
//	//定期读取cnt，防止测速周期（时间）不固定，因为不能自动算好cnt/时间
//	{	
//	if (TIM_GetITStatus(TIM4, TIM_IT_Update) == SET)
//		{
//		
//	Left_Target=Target;		
//	Right_Target=Target;		
//	Left_Real=Encoder_Left_Get();//要写在TIM4_IRQHandler()里面，Real_Speed是这个.c文件的全局变量，所以可以在两个函数里共用
//	Right_Real=Encoder_Right_Get();
//	//10mscnt的变化值
//	//Real=Real_Speed;// Real_Speed已保存实时速度，Real变量冗余
//	

//	Left_Error=Target-Left_Real;//实时计算误差，控制CRR
//	Right_Error=Target-Right_Real;
//	Correction=Left_Real-Right_Real;
///*
//	判断控制逻辑（不用循环，会卡死）
//每10ms：
//读取速度
//计算error
//微调CCR
//等待下一次测速
//调一点————等待响应，观测一下————再调一点
//*/
//	//先达到目标速度，然后再纠正，最后统一写入CRR
//	if(Left_Error>0)//判断调节部分
//	{
//	Left_CRR+=1;
//	}
//	if(Left_Error<0)
//	{
//	Left_CRR-=1;
//	}
//	
//	if(Left_CRR > 1000)//限幅，防止CRR因为调整时间长导致增加到ARR（1000）溢出
//	{
//	Left_CRR = 1000;
//	}
//	if(Left_CRR < -1000)
//	{
//	Left_CRR = -1000;
//	}
//	
//		if(Right_Error>0)//判断调节部分
//		{
//		Right_CRR+=1;
//		}
//		if(Right_Error<0)
//		{
//		Right_CRR-=1;
//		}
//	
//		if(Right_CRR > 1000)//限幅，防止CRR因为调整时间长导致增加到ARR（1000）溢出
//		{
//		Right_CRR = 1000;
//		}
//		if(Right_CRR < -1000)
//		{
//		Right_CRR = -1000;
//		}
//		//Correction纠正部分
//		if(Correction>0)//快的减速，不然会越来越快
//	{
//	Left_CRR-=1;
//	}
//	if(Correction<0)
//	{
//	Right_CRR-=1;
//	}
//	
//	Set_Right_Speed(Right_CRR);
//	Set_Left_Speed(Left_CRR);//让控制器输出的最终值（CRR）同时包含大小和方向信息
//	//CRR决定下一时刻怎么办
//	
//	//PWM输出部分
//	if(Left_CRR>=0)
//	{
//	PWM_Set_Cpmpare_PWM1(Left_CRR);
//	}
//	if(Left_CRR<0)
//	{
//	PWM_Set_Cpmpare_PWM1(-Left_CRR);//CRR为负数就是先反向输出，再增大，实现减速
//	}
//	
//	
//		if(Right_CRR>=0)
//		{
//		PWM_Set_Compare_PWM2(Right_CRR);
//		}

//		if(Right_CRR<0)
//		{
//		PWM_Set_Compare_PWM2(-Right_CRR);//CRR为负数就是先反向输出，再增大，实现减速
//		}

//		TIM_ClearITPendingBit(TIM4, TIM_IT_Update);//清除执行标志位
//		}	
//			
//	}



	
