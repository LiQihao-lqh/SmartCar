//chatgpt结构优化版
#include "stm32f10x.h"
#include "Encoder_Left.h"
#include "Encoder_Right.h"
#include "key.h"
#include "Pwm.h"
#include "Motor.h"
#include "OLED_ShowBinary.h"
#include "Ir_sensor.h"


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



static int16_t Gray_To_LineError(void)//给灰度传感器加权重，Line_Error直接用
{
    int16_t weight[8] = {-350, -250, -150, -50, 50, 150, 250, 350};//8路传感器，8位数组

    int32_t sum = 0;//根据权重计算后的综合值
    int16_t count = 0;//分路编号
	uint8_t gray;
    gray = IR_GetDigitalByte();
    for(uint8_t i = 0; i < 8; i++)//i从0开始遍历
    {
        if(gray & (1 << i))
        {
            sum += weight[i];
            count++;
        }
    }

    if(count == 0)
    {
        return 0;   // 暂时先这样，后面可以改成丢线处理
    }

    return sum / count;
}




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


        Left_Real  = Encoder_Left_Get();
        Right_Real = Encoder_Right_Get();


		
		


        // 左右轮速度差
        Line_Error  = Gray_To_LineError();//这个只能跑直线，Line_Error 数据来源换成灰度传感器才能转弯

		/*========== 4. 左右轮同步修正 ==========*/
		//根据line_error正负修正
		Turn_Out = (int32_t)(Turn_Out_Kp*Line_Error) ;
		Left_Target =Target-Turn_Out; 
		Right_Target =Target+Turn_Out; 
		
		Left_Error  = Left_Target - Left_Real;
        Right_Error = Right_Target - Right_Real;

        /*========== 3. 单轮速度闭环 ==========*/

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

int32_t Get_Target(void)
{
	return Target;
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

int32_t Left_CRR_Get(void)
{
	return Left_CRR;
}int32_t Right_CRR_Get(void)
{
	return Right_CRR;
}




/*
// 左右轮目标速度
static int32_t Left_Target = 0;
static int32_t Right_Target = 0;

// 总目标速度
static int32_t Target = 0;

// 编码器测得的实际速度
static int32_t Left_Real  = 0;
static int32_t Right_Real = 0;

// 左右轮速度误差
static int32_t Left_Error  = 0;
static int32_t Right_Error = 0;

// 左右轮最终控制输出
// 正负表示电机方向
static int32_t Left_CRR  = 0;
static int32_t Right_CRR = 0;

// 左右轮速度差
static int32_t Line_Error  = 0;

// 速度环P参数
static float Speed_Kp = 1.0f;

// 转向修正P参数
static float Turn_Out_Kp = 0.2f;

// 转向修正输出
static int32_t Turn_Out;

// PWM输出限幅
#define PWM_MAX   1000
#define PWM_MIN  -1000

// 基础PWM输出
#define Base_Pwm 100


// 更新目标速度
// 按键1加速，按键2减速
void Target_Update(void)
{
    uint8_t Key;

    Key = key_GetNum();

    if(Key == 1)
    {
        Target += 10;
    }

    if(Key == 2)
    {
        Target -= 10;
    }
}



// TIM4中断函数，10ms执行一次
// 用于速度读取、闭环计算和PWM输出
void TIM4_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM4, TIM_IT_Update) == SET)
    {
        // 读取左右轮实际速度
        Left_Real  = Encoder_Left_Get();
        Right_Real = Encoder_Right_Get();

        // 计算左右轮速度差
        // 后续可替换为灰度传感器误差
        Line_Error  = Left_Real - Right_Real;

        // 根据速度差计算转向修正量
        Turn_Out = (int32_t)Turn_Out_Kp * Line_Error;

        // 计算左右轮目标速度
        Left_Target  = Target - Turn_Out;
        Right_Target = Target + Turn_Out;

        // 计算左右轮速度误差
        Left_Error  = Left_Target - Left_Real;
        Right_Error = Right_Target - Right_Real;

        // 目标为0时停止输出
        if(Target == 0)
        {
            Left_CRR = 0;
            Right_CRR = 0;
        }
        else
        {
            // 基础PWM叠加速度误差修正
            Left_CRR  = Base_Pwm + (int32_t)(Speed_Kp * Left_Error);
            Right_CRR = Base_Pwm + (int32_t)(Speed_Kp * Right_Error);
        }

        // 左轮PWM限幅
        if(Left_CRR > PWM_MAX)
        {
            Left_CRR = PWM_MAX;
        }

        if(Left_CRR < PWM_MIN)
        {
            Left_CRR = PWM_MIN;
        }

        // 右轮PWM限幅
        if(Right_CRR > PWM_MAX)
        {
            Right_CRR = PWM_MAX;
        }

        if(Right_CRR < PWM_MIN)
        {
            Right_CRR = PWM_MIN;
        }

        // 根据正负设置电机方向
        Set_Left_Speed(Left_CRR);
        Set_Right_Speed(Right_CRR);

        // 输出左轮PWM占空比
        if(Left_CRR >= 0)
        {
            PWM_Set_Compare_PWM1(Left_CRR);
        }
        else
        {
            PWM_Set_Compare_PWM1(-Left_CRR);
        }

        // 输出右轮PWM占空比
        if(Right_CRR >= 0)
        {
            PWM_Set_Compare_PWM2(Right_CRR);
        }
        else
        {
            PWM_Set_Compare_PWM2(-Right_CRR);
        }

        // 清除TIM4中断标志位
        TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
    }
}


// 获取左轮目标速度
int32_t Get_Left_Target(void)
{
    return Left_Target;
}

// 获取右轮目标速度
int32_t Get_Right_Target(void)
{
    return Right_Target;
}

// 获取左轮实际速度
int32_t Get_Left_Real(void)
{
    return Left_Real;
}

// 获取右轮实际速度
int32_t Get_Right_Real(void)
{
    return Right_Real;
}
*/
