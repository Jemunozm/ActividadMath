/*
 * spi_driver_hal.c
 *
 *  Created on: Oct 23, 2023
 *      Author: imjeviz
 */

#include "stm32f4xx.h"
#include "spi_driver_hal.h"

///* === Headers for private functions === */
static void spi_enable_clock_peripheral(SPI_Handler_t *ptrSpiHandler);
static void spi_config_datasize(SPI_Handler_t *ptrSpiHandler);
static void spi_config_bitorder(SPI_Handler_t *ptrSpiHandler);
static void spi_config_baudrate(SPI_Handler_t *ptrSpiHandler);
static void spi_config_mode(SPI_Handler_t *ptrSpiHandler);
static void spi_config_duplex(SPI_Handler_t *ptrSpiHandler);
static void spi_config_interrupt(SPI_Handler_t *ptrSpiHandler);

void spi_Config(SPI_Handler_t *ptrSpiHandler){
	/* 0. Cargamos el pin del Slave que vamos a utilizar */
	gpio_Config(&ptrSpiHandler->SPI_slavePin);
	/* 1. Activamos la señal de reloj para el modulo SPI seleccionado */
	spi_enable_clock_peripheral(ptrSpiHandler);
	/* 2. Limpiamos el registro de configuración*/
	ptrSpiHandler->ptrSPIx->CR1 = 0;
	/* 3. Configuramos el baudrate del SPI*/
	spi_config_baudrate(ptrSpiHandler);
	/* 4. Configuramos el modo */
	spi_config_mode(ptrSpiHandler);
	/* 5. Configuramos el modo Duplex o recepción */
	spi_config_duplex(ptrSpiHandler);
	/* 6. Configuramos el modo de transferencia*/
	spi_config_bitorder(ptrSpiHandler);
	/* 7. Activamos para que el equipo se comporte como el maestro de la red */
	ptrSpiHandler->ptrSPIx->CR1 |= SPI_CR1_MSTR;
	/* 8. Configuramos el formato del dato (tamaño) */
	spi_config_datasize(ptrSpiHandler);
	/* 9. Configuramos para que el control del pin SS (Selección del Slave
	 * 	  sea controlado por software(nosotros debemos hacer ese contro).
	 * 	  de la otra forma, será el hardware el que controla la selección del Slave*/
	ptrSpiHandler->ptrSPIx->CR1 |= SPI_CR1_SSM;
	ptrSpiHandler->ptrSPIx->CR1 |= SPI_CR1_SSI;
	/* 10. Activamos las interrupciones */
	spi_config_interrupt(ptrSpiHandler);
	/* 11. Activamos el periférico SPI */
	ptrSpiHandler->ptrSPIx->CR1 |= SPI_CR1_SPE;
}

static void spi_enable_clock_peripheral(SPI_Handler_t *ptrSpiHandler){
	/* 1. Activamos la señal de reloj que viene desde el BUS al que pertenece el periferico */
	/* Lo debemos hacer para cada uno de las posibles opciones que tengamos (SPI1, SPI2, SPI3, SPI4, SPI5) */
	/* 1.1 Configuramos el SPI1*/
	if(ptrSpiHandler->ptrSPIx == SPI1){
		RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
	}
	else if(ptrSpiHandler->ptrSPIx == SPI2){
		RCC->APB1ENR |= RCC_APB1ENR_SPI2EN;
	}
	else if(ptrSpiHandler->ptrSPIx == SPI3){
		RCC->APB1ENR |= RCC_APB1ENR_SPI3EN;
	}
	else if(ptrSpiHandler->ptrSPIx == SPI4){
		RCC->APB2ENR |= RCC_APB2ENR_SPI4EN;
	}
	else if(ptrSpiHandler->ptrSPIx == SPI5){
		RCC->APB2ENR |= RCC_APB2ENR_SPI5EN;
	}
	else{
		__NOP();
	}
}

static void spi_config_baudrate(SPI_Handler_t *ptrSpiHandler){
	//Colocamos en el registro de configuración el valor del baudrate con su debido shift de posición.
	ptrSpiHandler->ptrSPIx->CR1 |= (ptrSpiHandler->SPI_Config.baudrate << SPI_CR1_BR_Pos);
}

static void spi_config_mode(SPI_Handler_t *ptrSpiHandler){
	switch(ptrSpiHandler->SPI_Config.mode){
	case SPI_MODE_0:{
		ptrSpiHandler->ptrSPIx->CR1  &= ~(SPI_CR1_CPHA);
		ptrSpiHandler->ptrSPIx->CR1  &= ~(SPI_CR1_CPOL);
		break;
	}
	case SPI_MODE_1:{
		ptrSpiHandler->ptrSPIx->CR1  |= (SPI_CR1_CPHA);
		ptrSpiHandler->ptrSPIx->CR1  &= ~(SPI_CR1_CPOL);
		break;
	}
	case SPI_MODE_2:{
		ptrSpiHandler->ptrSPIx->CR1  &= ~(SPI_CR1_CPHA);
		ptrSpiHandler->ptrSPIx->CR1  |= (SPI_CR1_CPOL);
		break;
	}
	case SPI_MODE_3:{
		ptrSpiHandler->ptrSPIx->CR1  |= (SPI_CR1_CPHA);
		ptrSpiHandler->ptrSPIx->CR1  |= (SPI_CR1_CPOL);
		break;
	}
	default:{
		ptrSpiHandler->ptrSPIx->CR1  |= (SPI_CR1_CPHA);
		ptrSpiHandler->ptrSPIx->CR1  |= (SPI_CR1_CPOL);
		break;
	}
	}
}

static void spi_config_duplex(SPI_Handler_t *ptrSpiHandler){
	switch(ptrSpiHandler->SPI_Config.fullDupplexEnable){
	case SPI_FULLDUPLEX:{
		ptrSpiHandler->ptrSPIx->CR1 &= ~SPI_CR1_RXONLY;
		ptrSpiHandler->ptrSPIx->CR1 &= ~SPI_CR1_BIDIMODE;
		break;
	}
	case SPI_OUTDISABLED:{
		ptrSpiHandler->ptrSPIx->CR1 |= SPI_CR1_RXONLY;
		break;
	}
	default:{
		ptrSpiHandler->ptrSPIx->CR1 |= SPI_CR1_RXONLY;
		break;
	}
	}
}

static void spi_config_bitorder(SPI_Handler_t *ptrSpiHandler){
	switch(ptrSpiHandler->SPI_Config.bitorder){
	case SPI_MSBFIRST:{
		ptrSpiHandler->ptrSPIx->CR1 &= ~SPI_CR1_LSBFIRST;
		break;
	}
	case SPI_LSBFIRST:{
		ptrSpiHandler->ptrSPIx->CR1 |= SPI_CR1_LSBFIRST;
		break;
	}
	default:{
		ptrSpiHandler->ptrSPIx->CR1 &= ~SPI_CR1_LSBFIRST;
		break;
	}
	}
}

static void spi_config_datasize(SPI_Handler_t *ptrSpiHandler){
	switch(ptrSpiHandler->SPI_Config.datasize){
	case SPI_DATAFRAME_8BIT:{
		ptrSpiHandler->ptrSPIx->CR1 &= ~SPI_CR1_DFF;
		break;
	}
	case SPI_DATAFRAME_16BIT:{
		ptrSpiHandler->ptrSPIx->CR1 |= SPI_CR1_DFF;
		break;
	}
	default:{
		ptrSpiHandler->ptrSPIx->CR1 |= SPI_CR1_DFF;
		break;
	}
	}
}
void spi_transmit(SPI_Handler_t *ptrSpiHandler, uint8_t * ptrData, uint32_t dataSize){
	uint8_t auxData;
	(void) auxData;

	while(dataSize > 0){
		//Esperamos a que el buffer esté vacío
		while(!(ptrSpiHandler->ptrSPIx->SR & SPI_SR_TXE)){
			__NOP();
		}

		if(ptrSpiHandler->SPI_Config.datasize == SPI_DATAFRAME_8BIT){
			ptrSpiHandler->ptrSPIx->DR = (uint8_t) (0xFF & *ptrData);
		}else{
			ptrSpiHandler->ptrSPIx->DR = (uint16_t) (0xFFFF & *ptrData);
		}
		// Actualizamos el puntero y el número de datos que faltan por enviar
		ptrData++;
		dataSize--;
	}
	// Esperamos de nuevo a que el buffer esté vacío
	while(!(ptrSpiHandler->ptrSPIx->SR & SPI_SR_TXE)){
		__NOP();
	}

	//Esperamos a que la bandera de ocupado (busy) baje (observar que la logica cambia)
	while(ptrSpiHandler->ptrSPIx->SR & SPI_SR_BSY){
		__NOP();
	}

	/* Debemos limpiar la bandera de OverRun (que a veces se levanta).
	 * PAra esto debemos leer elDRy luego leer el SR del modulo SPI (pag 599)*/
	auxData = ptrSpiHandler->ptrSPIx->DR;
	auxData = ptrSpiHandler->ptrSPIx->SR;
}

void spi_receive(SPI_Handler_t *ptrSpiHandler, uint8_t * ptrData, uint32_t dataSize){
	while(dataSize > 0){
		//Esperamos a que el buffer esté vacío
		while(!(ptrSpiHandler->ptrSPIx->SR & SPI_SR_TXE)){
			__NOP();
		}
		//Enviamos un valor de rellno
		ptrSpiHandler->ptrSPIx->DR = 0x00;

		//Esperamos de nuevo a que el buffer tenga un dato que leer
		while(!(ptrSpiHandler->ptrSPIx->SR & SPI_SR_RXNE)){
			__NOP();
		}

		//Cargamosel valor en el puntero
		*ptrData = ptrSpiHandler->ptrSPIx->DR;

		// Actualizamos el puntero y el número de datos que faltan por enviar
		ptrData++;
		dataSize--;
	}
}

void spi_config_interrupt(SPI_Handler_t *ptrSpiHandler){
	//Configuramos ambas interrupciones
}

/* Seleccionamos el esclavo llevando el pin SS a GND */
void spi_selectSlave(SPI_Handler_t *ptrSpiHandler){
	gpio_WritePin(&ptrSpiHandler->SPI_slavePin, RESET);
}
/* Seleccionamos el esclavo llevando el pin SS a Vcc */
void spi_unSelectSlave(SPI_Handler_t *ptrSpiHandler){
	gpio_WritePin(&ptrSpiHandler->SPI_slavePin, SET);
}



