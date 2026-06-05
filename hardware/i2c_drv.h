#ifndef __I2C_DRV_H
#define __I2C_DRV_H

#include "stm32f10x.h"

void I2C2_User_Init(void);
uint8_t I2C2_Master_Transmit(uint8_t DevAddress, uint8_t RegAddress, uint8_t *pData, uint16_t Size);
uint8_t I2C2_Master_Receive(uint8_t DevAddress, uint8_t RegAddress, uint8_t *pData, uint16_t Size);

#endif
