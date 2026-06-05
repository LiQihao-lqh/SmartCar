#ifndef __IR_SENSOR_H
#define __IR_SENSOR_H

#include <stdint.h>
#include <stdbool.h>

void IR_Init(void);
void IR_RequestMode(uint8_t adjust, uint8_t aData, uint8_t dData);
bool IR_IsDataReady(void);
void IR_ProcessNewData(void);

// 快速读取数字量：[可选] 一次读取8个值
void IR_ReadDigital(uint8_t *x1, uint8_t *x2, uint8_t *x3, uint8_t *x4,
                    uint8_t *x5, uint8_t *x6, uint8_t *x7, uint8_t *x8);
// 返回一个字节，bit0~bit7 对应探头 1~8，1 表示触发
uint8_t IR_GetDigitalByte(void);

// 读取模拟量数组
void IR_ReadAnalog(uint16_t adc[8]);

#endif
