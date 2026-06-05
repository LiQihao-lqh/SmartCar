#include "bsp_IR_i2c.h"
#include "i2c_drv.h"       // 我们的标准库驱动

// 进入校准模式
void set_adjust_mode(u8 mode)
{
    I2C2_Master_Transmit(IR_ADDRESS, 0x01, &mode, 1);
}

// 读取红外数据（内部）
static void read_IRdata(u8 *buf)
{
    I2C2_Master_Receive(IR_ADDRESS, 0x30, buf, 1);
}

// 解析 8 路数据
void deal_IRdata(u8 *x1,u8 *x2,u8 *x3,u8 *x4,u8 *x5,u8 *x6,u8 *x7,u8 *x8)
{
    u8 IRbuf = 0xFF;
    read_IRdata(&IRbuf);
    
    *x1 = (IRbuf>>7)&0x01;
    *x2 = (IRbuf>>6)&0x01;
    *x3 = (IRbuf>>5)&0x01;
    *x4 = (IRbuf>>4)&0x01;
    *x5 = (IRbuf>>3)&0x01;
    *x6 = (IRbuf>>2)&0x01;
    *x7 = (IRbuf>>1)&0x01;
    *x8 = (IRbuf>>0)&0x01;
}

