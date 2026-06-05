#ifndef __BSP_IR_I2C_H_
#define __BSP_IR_I2C_H_

#include "stm32f10x.h"

// 类型别名（避免污染全局，你也可以直接用 uint8_t）
typedef uint8_t u8;

#define IR_ADDRESS 0x12  // 灰度传感器 I2C 地址

void deal_IRdata(u8 *x1, u8 *x2, u8 *x3, u8 *x4,
                 u8 *x5, u8 *x6, u8 *x7, u8 *x8);
void set_adjust_mode(u8 mode);

#endif
