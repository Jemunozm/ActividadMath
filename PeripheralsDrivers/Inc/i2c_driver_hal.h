/*
 * i2c_driver_hal.h
 *
 *  Created on: Oct 26, 2023
 *      Author: imjeviz
 */

#ifndef I2C_DRIVER_HAL_H_
#define I2C_DRIVER_HAL_H_

#include <stdint.h>
#include "stm32f4xx.h"

enum{
	I2C_MODE_SM = 0,
	I2C_MODE_FM
};

enum{
	I2C_MODE_SM_SPEED_100 = 80,
	I2C_MODE_FM_SPEED_400 = 13
};

enum{
	I2C_MAX_RISE_TIME_SM = 17,
	I2C_MAX_RISE_TIME_FM = 5
};

enum{
	I2C_MODE_WRITE = 0,
	I2C_MODE_READ
};

enum{
	I2C_CLOCK_FREQUENCY_2 = 2,
	I2C_CLOCK_FRECUENCY_3,
	I2C_CLOCK_FRECUENCY_4,
	I2C_CLOCK_FRECUENCY_5,
	I2C_CLOCK_FRECUENCY_6,
	I2C_CLOCK_FRECUENCY_7,
	I2C_CLOCK_FRECUENCY_8,
	I2C_CLOCK_FRECUENCY_9,
	I2C_CLOCK_FRECUENCY_10,
	I2C_CLOCK_FRECUENCY_11,
	I2C_CLOCK_FRECUENCY_12,
	I2C_CLOCK_FRECUENCY_13,
	I2C_CLOCK_FRECUENCY_14,
	I2C_CLOCK_FRECUENCY_15,
	I2C_CLOCK_FRECUENCY_16,
	I2C_CLOCK_FRECUENCY_17,
	I2C_CLOCK_FRECUENCY_18,
	I2C_CLOCK_FRECUENCY_19,
	I2C_CLOCK_FRECUENCY_20,
	I2C_CLOCK_FRECUENCY_21,
	I2C_CLOCK_FRECUENCY_22,
	I2C_CLOCK_FRECUENCY_23,
	I2C_CLOCK_FRECUENCY_24,
	I2C_CLOCK_FRECUENCY_25,
	I2C_CLOCK_FRECUENCY_26,
	I2C_CLOCK_FRECUENCY_27,
	I2C_CLOCK_FRECUENCY_28,
	I2C_CLOCK_FRECUENCY_29,
	I2C_CLOCK_FRECUENCY_30,
	I2C_CLOCK_FRECUENCY_31,
	I2C_CLOCK_FRECUENCY_32,
	I2C_CLOCK_FRECUENCY_33,
	I2C_CLOCK_FRECUENCY_34,
	I2C_CLOCK_FRECUENCY_35,
	I2C_CLOCK_FRECUENCY_36,
	I2C_CLOCK_FRECUENCY_37,
	I2C_CLOCK_FRECUENCY_38,
	I2C_CLOCK_FRECUENCY_39,
	I2C_CLOCK_FRECUENCY_40,
	I2C_CLOCK_FRECUENCY_41,
	I2C_CLOCK_FRECUENCY_42,
	I2C_CLOCK_FRECUENCY_43,
	I2C_CLOCK_FRECUENCY_44,
	I2C_CLOCK_FRECUENCY_45,
	I2C_CLOCK_FRECUENCY_46,
	I2C_CLOCK_FRECUENCY_47,
	I2C_CLOCK_FRECUENCY_48,
	I2C_CLOCK_FRECUENCY_49,
	I2C_CLOCK_FRECUENCY_50,
};

/*
 * Definicion del Handler para un I2C:
 * - Estructura que contiene los SFR que controlan el periferico.
 * - Elemento que indica la dirección del esclavo.
 * - Elemento que nos indica la configuracion del modo.
 * - Elemento que indica los datos.
 */

typedef struct
{
	I2C_TypeDef			*ptrI2Cx;
	uint8_t				slaveAddress;
	uint8_t				modeI2C;
	uint8_t				dataI2C;
}I2C_Handler_t;

/* Header definitions for the "public functions" of i2c_driver_hal */
void i2c_config(I2C_Handler_t *ptrHandlerI2C);
void i2c_startTransaction(I2C_Handler_t *ptrHandlerI2C);
void i2c_reStartTransaction(I2C_Handler_t *ptrHandlerI2C);
void i2c_sendSlaveAddressRW(I2C_Handler_t *ptrHandlerI2C, uint8_t slaveAddress, uint8_t read0rWrite);
void i2c_sendMemoryAddress(I2C_Handler_t *ptrHandlerI2C, uint8_t memAddr);
void i2c_sendDataByte(I2C_Handler_t *ptrHandlerI2C, uint8_t dataToWrite);
uint8_t i2c_readDataByte(I2C_Handler_t *ptrHandlerI2C);
void i2c_stopTransaction(I2C_Handler_t *ptrHandlerI2C);
void i2c_sendAck(I2C_Handler_t *ptrHandlerI2C);
void i2c_sendNoAck(I2C_Handler_t *ptrHandlerI2C);

uint8_t i2c_readSingleRegister(I2C_Handler_t *ptrHandlerI2C, uint8_t regToRead);
void i2c_writeSingleRegister(I2C_Handler_t *ptrHandlerI2C, uint8_t regToRead, uint8_t newValue);

#endif /* I2C_DRIVER_HAL_H_ */
