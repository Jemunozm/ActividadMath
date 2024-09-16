/*
 * i2c_driver_hal.c
 *
 *  Created on: Oct 26, 2023
 *      Author: imjeviz
 */


#include "stm32f4xx.h"
#include "i2c_driver_hal.h"

/*
 * Recordar que se debe configurar los pines para el I2C (SDA y SCL),
 * para lo cual se necesita el modulo GPIO y los pines configurados
 * en el modo Alternate Function.
 * Además, estos pines deben ser configurados como salidas open-drain
 * y con las resistencias en modo pull-up.
 */
/* === Headers for private functions === */
static void i2c_enable_clock_peripheral(I2C_Handler_t *ptrHandlerI2C);
static void i2c_config_frequency(I2C_Handler_t *ptrHandlerI2C);
static void i2c_config_mode(I2C_Handler_t *ptrHandlerI2C);
static void i2c_enable_peripheral(I2C_Handler_t *ptrHandlerI2C);




/**
 * Configurando el puerto Serial...
 * Recordar que siempre se debe comenzar con activar la señal de reloj
 * del periferico que se está utilizando.
 */

void i2c_config(I2C_Handler_t *ptrHandlerI2C){

	/* 1. Activamos la señal señal de reloj para el modúlo I2C seleccionado */
	i2c_enable_clock_peripheral(ptrHandlerI2C);

	/* 2. Reiniciamos el periférico de forma que inicia en un estado conocido */
	ptrHandlerI2C->ptrI2Cx->CR1 |= I2C_CR1_SWRST;
	__NOP();
	ptrHandlerI2C->ptrI2Cx->CR1 &= ~I2C_CR1_SWRST;

	/* 3. Indicamos cual es la velocidad del reloj principal, que es la señal utilizada
	 * por el periférico para generar la señal de reloj para el bus I2C */
	i2c_config_frequency(ptrHandlerI2C);

	/* 4. Configuramos el modo I2C en el que el sistema funciona
	 * En esta configuración se incluye también la velocidad del reloj
	 * y el tiempo máximo para el cambio de la señal (T-Rise).
	 */
	i2c_config_mode(ptrHandlerI2C);

	/* 5. Activamos el modulo I2C */
	i2c_enable_peripheral(ptrHandlerI2C);
}

static void i2c_enable_clock_peripheral(I2C_Handler_t *ptrHandlerI2C){

	/* 1. Activamos la señal de reloj que viene desde el BUS al que pertenece el periferico */
	/* Lo debemos hacer para cada uno de las posibles opciones que tengamos (I2C1, I2C2, I2C3) */
	/* 1.1 Configuramos el USART1 */
	if (ptrHandlerI2C->ptrI2Cx == I2C1) {
		RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;
	}
	/* 1.2 Configuramos el USART6 */
	else if (ptrHandlerI2C->ptrI2Cx == I2C2) {
		RCC->APB1ENR |= RCC_APB1ENR_I2C2EN;
	}
	/* 1.3 Configuramos el USART2 */
	else if (ptrHandlerI2C->ptrI2Cx == I2C3) {
		RCC->APB1ENR |= RCC_APB1ENR_I2C3EN;
	} else {
		__NOP();
	}

}

static void i2c_config_frequency(I2C_Handler_t *ptrHandlerI2C){
	 //Limpiamos la configuración previa
	ptrHandlerI2C->ptrI2Cx->CR2 &= ~(0b111111 << I2C_CR2_FREQ_Pos);

	//Configuraremos por default en este caso para 16MHz
	ptrHandlerI2C->ptrI2Cx->CR2 |= (I2C_CLOCK_FRECUENCY_16 << I2C_CR2_FREQ_Pos);
	//ptrHandlerI2C->ptrI2Cx->CR2 |= (16 << I2C_CR2_FREQ_Pos);
}

static void i2c_config_mode(I2C_Handler_t *ptrHandlerI2C){
	 //Todo comienza con los dos registros en 0
	ptrHandlerI2C->ptrI2Cx->CCR = 0;
	ptrHandlerI2C->ptrI2Cx->TRISE = 0;

	/*Ahora configuramos deacuerdo a cada modo
	 * Primero el modo SM
	 */

	if(ptrHandlerI2C->modeI2C == I2C_MODE_SM){

		//Seleccionamos el modo standar mode
		ptrHandlerI2C->ptrI2Cx->CCR &= ~I2C_CCR_FS;

		//Configuramos el registro que se encarga de generar la señal del reloj
		ptrHandlerI2C->ptrI2Cx->CCR |= (I2C_MODE_SM_SPEED_100 << I2C_CCR_CCR_Pos);

		//Configuramos el registro que controla el tiempo T-Rise máximo
		ptrHandlerI2C->ptrI2Cx->TRISE |= I2C_MAX_RISE_TIME_SM;
	}
	else{
		//Ahora configuramos el modo Fast (FM mode)
		//Seleccionamoselmodo fast
		ptrHandlerI2C->ptrI2Cx->CCR |= I2C_CCR_FS;

		// Configuramos el registro que se encargará de generar la señal del reloj
		ptrHandlerI2C->ptrI2Cx->CCR |= (I2C_MODE_FM_SPEED_400 << I2C_CCR_CCR_Pos);

		//Configuramos el registro que controla el tiempo T-Rise máximo
		ptrHandlerI2C->ptrI2Cx->TRISE |= I2C_MAX_RISE_TIME_FM;
	}
}

static void i2c_enable_peripheral(I2C_Handler_t *ptrHandlerI2C){
	ptrHandlerI2C->ptrI2Cx->CR1 |= I2C_CR1_PE;
}

/* Generamoslacondición de Stop */
void i2c_stopTransaction(I2C_Handler_t *ptrHandlerI2C){
	ptrHandlerI2C->ptrI2Cx->CR1 |= I2C_CR1_STOP;
}

/* Generamos la condición de start */
void i2c_startTransaction(I2C_Handler_t *ptrHandlerI2C){

	/* 1. Verificamos que la linea no está ocupada - bit "busy" en I2C_CR2 */

	while (ptrHandlerI2C->ptrI2Cx->SR2 & I2C_SR2_BUSY){
		__NOP();
	}

	/* 2. Generamos la señal "start" */
	ptrHandlerI2C->ptrI2Cx->CR1 |= I2C_CR1_START;

	/* 2a. Esperamos a que la bandera del evento  "start" selevante
	 * Mientras esperamos, el valor de SB es 0,entonces la negación (!) es 1
	 */
	while(ptrHandlerI2C->ptrI2Cx->SR1 & I2C_SR1_SB){
		__NOP();
	}
}

/* Generamos la función de reStart */
void i2c_reStartTransaction(I2C_Handler_t *ptrHandlerI2C){

	//Repetimos los pasos de la función start

	/* 2. Generamos la señal "start" */
	ptrHandlerI2C->ptrI2Cx->CR1 |= I2C_CR1_START;

	/* 2a. Esperamos a que la bandera del evento  "start" selevante
	 * Mientras esperamos, el valor de SB es 0,entonces la negación (!) es 1
	 */
	while(ptrHandlerI2C->ptrI2Cx->SR1 & I2C_SR1_SB){
		__NOP();
	}

}

/* Activamos la indicación para no-ACK (indicación para el Slave de terminar) */
void i2c_sendNoAck(I2C_Handler_t *ptrHandlerI2C){
	/* (Debemos escribir cero en la posición ACK del registro de control 1) */
	ptrHandlerI2C->ptrI2Cx->CR1 &= ~I2C_CR1_ACK;
}

/* Activamos la indicación para ACK (indicación para el Slave CONTINUAR) */
void i2c_sendAck(I2C_Handler_t *ptrHandlerI2C){
	/* (Debemos escribir uno en la posición ACK del registro de control 1) */
	ptrHandlerI2C->ptrI2Cx->CR1 |= I2C_CR1_ACK;
}

/**/
void i2c_sendSlaveAddressRW(I2C_Handler_t *ptrHandlerI2C, uint8_t slaveAddress, uint8_t read0rWrite){

	/*Definimos una variable auxiliar */
	uint8_t auxByte = 0;
	(void) auxByte;

	/* Enviamos la dirección del Slave y el bit que indica que deseamos escribir (0)
	 * en el siguiente paso se envia la dirección de memoria que se desea escribir
	 */
	ptrHandlerI2C->ptrI2Cx->DR = (slaveAddress << 1) | read0rWrite;

	/* Esperamos hasta que la bandera del evento "addr" se levante.
	 * Esto nos indica que la dirección fue enviada satisfactoriamente
	 */
	while(!(ptrHandlerI2C->ptrI2Cx->SR1 & I2C_SR1_ADDR)){
		__NOP();
	}

	/* Debemos limpiar la bandera de la recepción de ACK de la "addr", para lo cual
	 * debemos leer en secuencia primero el I2C_SR1 y luego I2C_SR2
	 */
	auxByte = ptrHandlerI2C->ptrI2Cx->SR1;
	auxByte = ptrHandlerI2C->ptrI2Cx->SR2;

}

/**/
void i2c_sendMemoryAddress(I2C_Handler_t *ptrHandlerI2C, uint8_t memAddr){
	/* Enviamos la dirección de memoria que deseamos leer*/
	ptrHandlerI2C->ptrI2Cx->DR = memAddr;

	// Esperamos hasta que el byte sea trasmitido
	while(!(ptrHandlerI2C->ptrI2Cx->SR1 & I2C_SR1_TXE)){
	__NOP();
	}
}

/**/
void i2c_sendDataByte(I2C_Handler_t *ptrHandlerI2C, uint8_t dataToWrite){
	/* Cargamos el valor que deseamos escribir*/
	ptrHandlerI2C->ptrI2Cx->DR = dataToWrite;

	/* esperamos hasta que el byte sea transmitido*/
	while(!(ptrHandlerI2C->ptrI2Cx->SR1 & I2C_SR1_BTF)){
		__NOP();
	}
}

uint8_t i2c_readDataByte(I2C_Handler_t *ptrHandlerI2C){
	/* Esperamos hasta que el byte entrante sea recibido */
	while(!(ptrHandlerI2C->ptrI2Cx->SR1 & I2C_SR1_RXNE)){
		__NOP();
	}

	ptrHandlerI2C->dataI2C = ptrHandlerI2C->ptrI2Cx->DR;
	return ptrHandlerI2C->dataI2C;
}

/**/
uint8_t i2c_readSingleRegister(I2C_Handler_t *ptrHandlerI2C, uint8_t regToRead){

	/* 0. Creamos una variable auxiliar para recibir el dato que leemos */
	uint8_t auxRead = 0;

	/* 1. Generamos la condición Start*/
	i2c_startTransaction(ptrHandlerI2C);

	/* 2. Enviamos la dirección del esclavo y la indicación de ESCRIBIR */
	i2c_sendSlaveAddressRW(ptrHandlerI2C, ptrHandlerI2C->slaveAddress, I2C_MODE_WRITE);

	/* 3. Enviamos la dirección de memoria que deseamos leer */
	i2c_sendMemoryAddress(ptrHandlerI2C, regToRead);

	/* 4. Creamos una condición reStart; */
	i2c_reStartTransaction(ptrHandlerI2C);

	/* 5. Enviamos la dirección del esclavo y la indicación de LEER */
	i2c_sendSlaveAddressRW(ptrHandlerI2C, ptrHandlerI2C->slaveAddress, I2C_MODE_READ);

	/* 8. Leemos el dato que envia el esclavo */
	auxRead = i2c_readDataByte(ptrHandlerI2C);

	/* 6. Generamos lacondición de noAck, para que el Master no responda y el slave solo envie 1byte*/
	i2c_sendNoAck(ptrHandlerI2C);

	/* 7. Generamos la condición Stop, para que el slave se detenga despues de 1 byte */
	i2c_stopTransaction(ptrHandlerI2C);

	return auxRead;
}

void i2c_writeSingleRegister(I2C_Handler_t *ptrHandlerI2C, uint8_t regToRead, uint8_t newValue){

	/* 1. Generamos la condición Start */
	i2c_startTransaction(ptrHandlerI2C);

	/* 2. Enviamos la dirección del esclavo y la indicación de ESCRIBIR */
	i2c_sendSlaveAddressRW(ptrHandlerI2C, ptrHandlerI2C->slaveAddress, I2C_MODE_WRITE);

	/* 3. Enviamos la dirección de memoria que deseamos escribir */
	i2c_sendMemoryAddress(ptrHandlerI2C, regToRead);

	/* 4. Enviamos el valor que deseamos escribir en el registro seleccionado */
	i2c_sendDataByte(ptrHandlerI2C, newValue);

	/* 5. Generamos la condición Stop, para que el slave se detenga despues de 1 byte */
	i2c_stopTransaction(ptrHandlerI2C);
}
