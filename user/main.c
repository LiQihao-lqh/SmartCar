#include "stm32f10x.h"                  // Device header
#include "Delay.h"	//延时函数
#include "Timer.h"
#include "OLED.h"
#include "Motor.h"
#include "Encoder_Left.h"
#include "Encoder_Right.h"
#include "Control.h"
#include "Key.h"
#include "Ir_Sensor.h"
#include "OLED_ShowBinary.h"

int main(void)
{	
	char Str[9];
	uint8_t Line;
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//全局中断分组配置，防止编码器中断和灰度冲突，抢占优先级分级
	
	IR_Init();
	key_Init();
	OLED_Init();
	Motor_Init();
	Encoder_Left_Init();
	Encoder_Right_Init();
	Timer_Init();//不放control里面，不然会被反复调用
	
	OLED_Clear();
	
	//TIM2_IRQHandler();每10ms自动cpu执行，不需要调用，每10ms就是触发条件
	

	IR_RequestMode(0,0,1);//设置传感器模式,这里是数字模式
//	OLED_ShowString(1,1,"Target:");
//	OLED_ShowString(2,1,"Real_1:");
//	OLED_ShowString(3,1,"Real_2:");
	
	
	while(1)
	{
	if(IR_IsDataReady())//判断新数据是否发送过来了
	{
	IR_ProcessNewData();//解析数据，更新内部缓存
	}
	Line = IR_GetDigitalByte();//读取传感器数据，并传递到line这个变量	
	Target_Update();//按键设置速度读取，不能放中断里面，不然每10ms读取一次容易错过导致按了没效果
	
	OLED_ShowBinary8(Line, Str);//不能写Str[9]，因为越界了，
	OLED_ShowSignedNum(1,8,Right_CRR_Get(),5);//刷新现实目标速度
	OLED_ShowSignedNum(2,8,Left_CRR_Get(),5);//刷新显示真实速度
	OLED_ShowSignedNum(3,8,Get_Right_Real(),5);//自动调节的CRR
	OLED_ShowSignedNum(4, 1,Get_Left_Real(),4);//灰度·传感器数据，行，列，字符串

		
	 Delay_ms(30);  //加延时防止一直刷新导致阻塞
	}
	
}

//#include "stm32f10x.h"
//#include "Delay.h"
//#include "Timer.h"
//#include "OLED.h"
//#include "Motor.h"
//#include "Encoder_Left.h"
//#include "Encoder_Right.h"
//#include "Control.h"
//#include "Key.h"
//#include "ir_sensor.h"        // 新增

//int main(void)
//{
//    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);  // 中断分组2，须在初始化前设置

//    key_Init();
//    OLED_Init();
//    Motor_Init();
//    Encoder_Left_Init();
//    Encoder_Right_Init();
//    Timer_Init();             // TIM4 优先级 2,1
//    IR_Init();                // 灰度传感器初始化（USART2 优先级 1,0）

//    OLED_Clear();
//    OLED_ShowString(1, 1, "Target_1:");
//    OLED_ShowString(2, 1, "Real_1:");
//    OLED_ShowString(3, 1, "Target_2:");
//    OLED_ShowString(4, 1, "Real_2:");

//    while(1)
//    {
//        Target_Update();                       // 按键更新目标速度

//        /* ----- 灰度传感器读取（非阻塞） ----- */
//        if (IR_IsDataReady())
//        {
//            IR_ProcessNewData();

//            uint8_t line = IR_GetDigitalByte();   // 8 位探头状态
//            // 简单显示：在 OLED 第 4 行后面显示数字（0~255）
//            OLED_ShowNum(4, 10, line, 3);
//            // 后期可基于 line 做偏差计算，控制电机
//        }

//        OLED_ShowSignedNum(1, 10, Get_Target(), 5);
//        OLED_ShowSignedNum(2, 8, Get_Left_Real(), 5);
//        OLED_ShowSignedNum(4, 8, Get_Right_Real(), 5);

//        Delay_ms(30);          // 暂时保留，后期可优化为 10ms 或使用标志触发
//    }
//}


