#include "i2c_drv.h"

// I2C2 初始化（400kHz, PB10/SCL, PB11/SDA）
void I2C2_User_Init(void)
{
    I2C_InitTypeDef I2C_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;

    // 时钟使能
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    // GPIO 配置：PB10(SCL), PB11(SDA) 复用开漏输出
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    // I2C 配置
    I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
    I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
    I2C_InitStructure.I2C_OwnAddress1 = 0x00;   // 主机模式不用自身地址
    I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
    I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_InitStructure.I2C_ClockSpeed = 400000;   // 400kHz

    I2C_Init(I2C2, &I2C_InitStructure);
    I2C_Cmd(I2C2, ENABLE);
}

// 简单等待标志位，带超时
static uint8_t WaitForFlag(I2C_TypeDef* I2Cx, uint32_t I2C_FLAG, FlagStatus status, uint32_t Timeout)
{
    while (Timeout--)
    {
        if (I2C_GetFlagStatus(I2Cx, I2C_FLAG) == status)
            return 1;
    }
    return 0;  // 超时
}

// 写数据到指定从机寄存器（像 HAL_Mem_Write 的功能）
// DevAddress: 7位地址（会被自动左移1位）
// RegAddress: 寄存器地址（单字节）
// Size: 要写入的数据字节数（通常1）
uint8_t I2C2_Master_Transmit(uint8_t DevAddress, uint8_t RegAddress, uint8_t *pData, uint16_t Size)
{
    uint32_t timeout = 0xFFFF;

    // 1. 产生起始条件
    I2C_GenerateSTART(I2C2, ENABLE);
    if (!WaitForFlag(I2C2, I2C_FLAG_SB, SET, timeout)) return 0;

    // 2. 发送从机地址（写方向）
    I2C_Send7bitAddress(I2C2, DevAddress << 1, I2C_Direction_Transmitter);
    if (!WaitForFlag(I2C2, I2C_FLAG_ADDR, SET, timeout)) return 0;
    // 清除ADDR标志（读SR1后读SR2）
    (void)I2C2->SR2;

    // 3. 发送寄存器地址
    I2C_SendData(I2C2, RegAddress);
    if (!WaitForFlag(I2C2, I2C_FLAG_TXE, SET, timeout)) return 0;

    // 4. 发送数据（支持多字节，但通常 Size=1）
    while (Size--)
    {
        I2C_SendData(I2C2, *pData++);
        if (!WaitForFlag(I2C2, I2C_FLAG_BTF, SET, timeout)) return 0;
    }

    // 5. 产生停止条件
    I2C_GenerateSTOP(I2C2, ENABLE);
    return 1;   // 成功
}

// 从指定从机寄存器读取数据（像 HAL_Mem_Read 的功能）
uint8_t I2C2_Master_Receive(uint8_t DevAddress, uint8_t RegAddress, uint8_t *pData, uint16_t Size)
{
    uint32_t timeout = 0xFFFF;

    // 第一步：先写寄存器地址（无数据）
    // 1. 起始条件
    I2C_GenerateSTART(I2C2, ENABLE);
    if (!WaitForFlag(I2C2, I2C_FLAG_SB, SET, timeout)) return 0;

    // 2. 发送从机地址（写方向）
    I2C_Send7bitAddress(I2C2, DevAddress << 1, I2C_Direction_Transmitter);
    if (!WaitForFlag(I2C2, I2C_FLAG_ADDR, SET, timeout)) return 0;
    (void)I2C2->SR2;

    // 3. 发送寄存器地址
    I2C_SendData(I2C2, RegAddress);
    if (!WaitForFlag(I2C2, I2C_FLAG_BTF, SET, timeout)) return 0;

    // 第二步：重复起始条件，切换到读
    I2C_GenerateSTART(I2C2, ENABLE);
    if (!WaitForFlag(I2C2, I2C_FLAG_SB, SET, timeout)) return 0;

    // 发送从机地址（读方向）
    I2C_Send7bitAddress(I2C2, DevAddress << 1, I2C_Direction_Receiver);
    if (!WaitForFlag(I2C2, I2C_FLAG_ADDR, SET, timeout)) return 0;
    (void)I2C2->SR2;

    // 接收数据
    while (Size)
    {
        if (Size == 1)
        {
            // 最后一个字节需要 NACK + STOP
            I2C_AcknowledgeConfig(I2C2, DISABLE);  // NACK
        }

        // 等待接收数据
        if (!WaitForFlag(I2C2, I2C_FLAG_RXNE, SET, timeout)) return 0;

        *pData++ = I2C_ReceiveData(I2C2);
        Size--;

        if (Size == 0)
        {
            I2C_GenerateSTOP(I2C2, ENABLE);
            I2C_AcknowledgeConfig(I2C2, ENABLE);   // 恢复 ACK 以备下次
        }
    }

    return 1;   // 成功
}

