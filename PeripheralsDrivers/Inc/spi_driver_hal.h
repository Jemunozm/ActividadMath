/*
 * spi_driver_hal.h
 *
 *  Created on: Oct 22, 2023
 *      Author: imjeviz
 */

#ifndef SPI_DRIVER_HAL_H_
#define SPI_DRIVER_HAL_H_

#include <stdio.h>
#include "stm32f4xx.h"
#include "gpio_driver_hal.h"


enum
{
	SPI_MODE_SLAVE = 0,
	SPI_MODE_MASTER
};

enum
{
	SPI_DATAFRAME_8BIT = 0,
	SPI_DATAFRAME_16BIT
};

enum
{
	SPI_MSBFIRST = 0,
	SPI_LSBFIRST
};

enum
{
	SPI_MODE_0 = 0,
	SPI_MODE_1,
	SPI_MODE_2,
	SPI_MODE_3
};

enum
{
	SPI_FULLDUPLEX = 0,
	SPI_OUTDISABLED
};

enum
{
	SPI_SSM_OFF = 0,
	SPI_SSM_ON
};

enum
{
	SPI_OFF = 0,
	SPI_ON
};

enum
{
	SPI_BAUDRATE_2 = 0,
	SPI_BAUDRATE_4,
	SPI_BAUDRATE_8,
	SPI_BAUDRATE_16,
	SPI_BAUDRATE_32,
	SPI_BAUDRATE_64,
	SPI_BAUDRATE_128,
	SPI_BAUDRATE_256
};


/* Estructura para la configuración de la comunicacion:
 * Velocidad (baudrate)
 * Tamaño de los datos
 * Control de errores
 * Modo de comunicación
 */
typedef struct
{
	uint8_t mode; //Define los 4 modos que se puede configurar
	uint8_t datasize;
	uint8_t baudrate;//De4fine la velocidad que maneja el SPI
	uint8_t bitorder;
	uint8_t fullDupplexEnable;//Configura si solo recepción o bidireccional
	uint8_t	enableIntRX;
	uint8_t	enableIntTX;
}SPI_Config_t;

/*
 * Definicion del Handler para un USART:
 * - Estructura que contiene los SFR que controlan el periferico
 * - Estructura que contiene la configuración especifica del objeto
 * - Buffer de recepcion de datos
 * - Elemento que indica cuantos datos se recibieron
 * - Buffer de transmision de datos
 * - Elemento que indica cuantos datos se deben enviar.
 */
typedef struct
{
	SPI_TypeDef		*ptrSPIx;
	SPI_Config_t	SPI_Config;
	GPIO_Handler_t	SPI_slavePin;
}SPI_Handler_t;

void spi_Config(SPI_Handler_t *ptrSpiHandler);
void spi_transmit(SPI_Handler_t *ptrSpiHandler, uint8_t * ptrData, uint32_t dataSize);
void spi_receive(SPI_Handler_t *ptrSpiHandler, uint8_t * ptrData, uint32_t dataSize);
void spi_selectSlave(SPI_Handler_t *ptrSpiHandler);
void spi_unSelectSlave(SPI_Handler_t *ptrSpiHandler);

#endif /* SPI_DRIVER_HAL_H_ */
