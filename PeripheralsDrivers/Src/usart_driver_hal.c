/*
 * usart_driver_hal.c
 *
 *  Created on: Oct 9, 2023
 *      Author: imjeviz
 */

#include "stm32f4xx.h"
#include "usart_driver_hal.h"

uint8_t auxRxData1 = 0;
uint8_t auxRxData2 = 0;
uint8_t auxRxData6 = 0;

/* === Headers for private functions === */
static void usart_enable_clock_peripheral(USART_Handler_t *ptrUsartHandler);
static void usart_config_parity(USART_Handler_t *ptrUsartHandler);
static void usart_config_datasize(USART_Handler_t *ptrUsartHandler);
static void usart_config_stopbits(USART_Handler_t *ptrUsartHandler);
static void usart_config_baudrate(USART_Handler_t *ptrUsartHandler);
static void usart_config_mode(USART_Handler_t *ptrUsartHandler);
static void usart_config_interrupt(USART_Handler_t *ptrUsartHandler);
static void usart_enable_peripheral(USART_Handler_t *ptrUsartHandler);

/**
 * Configurando el puerto Serial...
 * Recordar que siempre se debe comenzar con activar la señal de reloj
 * del periferico que se está utilizando.
 */
void usart_Config(USART_Handler_t *ptrUsartHandler) {

	//*ptrAuxUsartHandler = *ptrUsartHandler;

	/* 0. Desactivamos las interrupciones globales mientras configuramos el sistema.*/
	__disable_irq();

	/* 1. Activamos la señal de reloj que viene desde el BUS al que pertenece el periférico */
	usart_enable_clock_peripheral(ptrUsartHandler);

	/* 2. Configuramos el tamaño del dato, la paridad y los bit de parada */
	/* En el CR1 estan parity (PCE y PS) y tamaño del dato (M) */
	/* Mientras que en CR2 estan los stopbit (STOP)*/
	/* Configuracion del Baudrate (registro BRR) */
	/* Configuramos el modo: only TX, only RX, o RXTX */
	/* Por ultimo activamos el modulo USART cuando todo esta correctamente configurado */

	// 2.1 Comienzo por limpiar los registros, para cargar la configuración desde cero
	ptrUsartHandler->ptrUSARTx->CR1 = 0;
	ptrUsartHandler->ptrUSARTx->CR2 = 0;

	// Limpiamos el registro DR
	ptrUsartHandler->ptrUSARTx->DR = 0;

	// 2.2 Configuracion del Parity:
	usart_config_parity(ptrUsartHandler);

	// 2.3 Configuramos el tamaño del dato
	usart_config_datasize(ptrUsartHandler);

	// 2.4 Configuramos los stop bits (SFR USART_CR2)
	usart_config_stopbits(ptrUsartHandler);

	// 2.5 Configuracion del Baudrate (SFR USART_BRR)
	usart_config_baudrate(ptrUsartHandler);

	// 2.6 Configuramos el modo: TX only, RX only, RXTX, disable
	usart_config_mode(ptrUsartHandler);

	// 2.8 Verificamos la configuración de las interrupciones
	usart_config_interrupt(ptrUsartHandler);

	// 2.7 Activamos el modulo serial.
	usart_enable_peripheral(ptrUsartHandler);

	/* x. Volvemos a activar las interrupciones del sistema */
	__enable_irq();
}

/**/
static void usart_enable_clock_peripheral(USART_Handler_t *ptrUsartHandler) {
	/* 1. Activamos la señal de reloj que viene desde el BUS al que pertenece el periferico */
	/* Lo debemos hacer para cada uno de las posibles opciones que tengamos (USART1, USART2, USART6) */

	/* 1.1 Configuramos el USART1 */
	if (ptrUsartHandler->ptrUSARTx == USART1) {
		RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
	}
	/* 1.2 Configuramos el USART6 */
	else if (ptrUsartHandler->ptrUSARTx == USART6) {
		RCC->APB2ENR |= RCC_APB2ENR_USART6EN;
	}
	/* 1.3 Configuramos el USART2 */
	else if (ptrUsartHandler->ptrUSARTx == USART2) {
		RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
	} else {
		__NOP();
	}
}

/**
 *
 */
static void usart_config_parity(USART_Handler_t *ptrUsartHandler) {
	// Verificamos si el parity esta activado o no
	// Tenga cuidado, el parity hace parte del tamaño de los datos...
	if (ptrUsartHandler->USART_Config.parity != USART_PARITY_NONE) {

		ptrUsartHandler->ptrUSARTx->CR1 |= USART_CR1_PCE;
		// Verificamos si se ha seleccionado ODD or EVEN
		if (ptrUsartHandler->USART_Config.parity == USART_PARITY_EVEN) {
			// Es even, entonces cargamos la configuracion adecuada
			ptrUsartHandler->ptrUSARTx->CR1 &= ~USART_CR1_PS;

		} else {
			// Si es "else" significa que la paridad seleccionada es ODD, y cargamos esta configuracion
			ptrUsartHandler->ptrUSARTx->CR1 |= USART_CR1_PS;
		}
	} else {
		// Si llegamos aca, es porque no deseamos tener el parity-check
		ptrUsartHandler->ptrUSARTx->CR1 &= ~USART_CR1_PCE;
	}
}

/**
 * Esta funcion debe estar relacionada con el parity.
 * Si NO hay parity,el tamaño debe ser 8bit.
 * SI HAY parity, el tamaño debe ser 9 bit.
 */
static void usart_config_datasize(USART_Handler_t *ptrUsartHandler) {
	// Verificamos cual es el tamaño de dato que deseamos
	if (ptrUsartHandler->USART_Config.datasize == USART_DATASIZE_8BIT) {

		// Verificamos si se esta trabajando o no con paridad
		if (ptrUsartHandler->USART_Config.parity == USART_PARITY_NONE) {
			// Deseamos trabajar con datos de 8 bits
			ptrUsartHandler->ptrUSARTx->CR1 &= ~USART_CR1_M;
		} else {
			// Si la paridad esta activa, debemos incluir un bit adicional
			ptrUsartHandler->ptrUSARTx->CR1 |= USART_CR1_M;
		}
	} else {
		// Deseamos trabajar con datos de 9 bits
		ptrUsartHandler->ptrUSARTx->CR1 |= USART_CR1_M;
	}
}

/**
 *
 */
static void usart_config_stopbits(USART_Handler_t *ptrUsartHandler) {
	switch (ptrUsartHandler->USART_Config.stopbits) {
	case USART_STOPBIT_1: {
		// Debemos cargar el valor 0b00 en los dos bits de STOP
		ptrUsartHandler->ptrUSARTx->CR2 &= ~(0x3UL << USART_CR2_STOP_Pos);
		break;
	}
	case USART_STOPBIT_0_5: {
		// Debemoscargar el valor 0b01 en los dos bits de STOP
		ptrUsartHandler->ptrUSARTx->CR2 |= USART_CR2_STOP_0;
		break;
	}
	case USART_STOPBIT_2: {
		// Debemoscargar el valor 0b10 en los dos bits de STOP
		ptrUsartHandler->ptrUSARTx->CR2 |= USART_CR2_STOP_1;
		break;
	}
	case USART_STOPBIT_1_5: {
		// Debemoscargar el valor 0b11 en los dos bits de STOP
		ptrUsartHandler->ptrUSARTx->CR2 |= USART_CR2_STOP;
		break;
	}
	default: {
		// En el casopor defecto seleccionamos 1 bit de parada
		ptrUsartHandler->ptrUSARTx->CR2 &= ~(0x3UL << USART_CR2_STOP_Pos);
		break;
	}
	}
}

/**
 * Ver tabla de valores (Tabla 73), Frec = 16MHz, overr = 0;
 */
static void usart_config_baudrate(USART_Handler_t *ptrUsartHandler) {
	// Caso para configurar cuando se trabaja con el Cristal Interno
	switch (ptrUsartHandler->USART_Config.baudrate) {
	case USART_BAUDRATE_9600: {
		// El valor a cargar es 104.1875 -> Mantiza = 104,fraction = 0.1875
		// Mantiza = 104 = 0x68, fraction = 16 * 0.1875 = 3
		// Valor a cargar 0x0683
		// Configurando el Baudrate generator para una velocidad de 9600bps
		ptrUsartHandler->ptrUSARTx->BRR = 0x0683;
		break;
	}
	case USART_BAUDRATE_19200: {
		// El valor a cargar es 52.0625 -> Mantiza = 52,fraction = 0.0625
		// Mantiza = 52 = 0x34, fraction = 16 * 0.0625 = 1
		// Valor a cargar 0x0341
		// Configurando el Baudrate generator para una velocidad de 19200bps
		ptrUsartHandler->ptrUSARTx->BRR = 0x0341;
		break;
	}
	case USART_BAUDRATE_38400: {
		// El valor a cargar es 26.0625 -> Mantiza = 26,fraction = 0.0625
		// Mantiza = 26 = 0x1A, fraction = 16 * 0.0625 = 1
		// Valor a cargar 0x01A1
		// Configurando el Baudrate generator para una velocidad de 38400bps
		ptrUsartHandler->ptrUSARTx->BRR = 0x01A1;
		break;
	}
	case USART_BAUDRATE_115200: {
		// El valor a cargar es 8.6875 -> Mantiza = 8,fraction = 0.6875
		// Mantiza = 8 = 0x8, fraction = 16 * 0.6875 = 11
		// Valor a cargar 0x008B
		// Configurando el Baudrate generator para una velocidad de 115200bps
		ptrUsartHandler->ptrUSARTx->BRR = 0x008B;
		break;
	}
	case USART_BAUDRATE_230400: {
		// El valor a cargar es 4.3125 -> Mantiza = 4,fraction = 0.3125
		// Mantiza = 4 = 0x04, fraction = 16 * 0.3125 = 5
		// Valor a cargar 0x0045
		// Configurando el Baudrate generator para una velocidad de 230400bps
		ptrUsartHandler->ptrUSARTx->BRR = 0x0045;
		break;
	}
	case USART_BAUDRATE_921600: {
		// El valor a cargar es 1.0625 -> Mantiza = 1,fraction = 0.0625
		// Mantiza = 1 = 0x01, fraction = 16 * 0.0625 = 1
		// Valor a cargar 0x0011
		// Configurando el Baudrate generator para una velocidad de 921600bps
		ptrUsartHandler->ptrUSARTx->BRR = 0x0011;
		break;
	}

	default:
		// Configurando el Baudrate generator para una velocidad de 115200bps
		ptrUsartHandler->ptrUSARTx->BRR = 0x008B;
		break;
	}
}

/**
 *
 */
static void usart_config_mode(USART_Handler_t *ptrUsartHandler) {
	switch (ptrUsartHandler->USART_Config.mode) {
	case USART_MODE_TX: {
		// Activamos la parte del sistema encargada de enviar
		ptrUsartHandler->ptrUSARTx->CR1 |= USART_CR1_TE;
		break;
	}
	case USART_MODE_RX: {
		// Activamos la parte del sistema encargada de recibir
		ptrUsartHandler->ptrUSARTx->CR1 |= USART_CR1_RE;
		break;
	}
	case USART_MODE_RXTX: {
		// Activamos ambas partes, tanto transmision como recepcion
		ptrUsartHandler->ptrUSARTx->CR1 |= USART_CR1_TE;
		ptrUsartHandler->ptrUSARTx->CR1 |= USART_CR1_RE;
		break;
	}
	case USART_MODE_DISABLE: {
		// Desactivamos ambos canales
		// Escriba acá su código
		ptrUsartHandler->ptrUSARTx->CR1 &= ~USART_CR1_TE;
		ptrUsartHandler->ptrUSARTx->CR1 &= ~USART_CR1_RE;
		ptrUsartHandler->ptrUSARTx->CR1 &= ~USART_CR1_UE;
		break;
	}
	}
}

/**
 *
 */
static void usart_config_interrupt(USART_Handler_t *ptrUsartHandler) {
	// 2.8a Interrupción por recepción
	if (ptrUsartHandler->USART_Config.enableIntRX == USART_RX_INTERRUP_ENABLE) {
		// Como está activada, debemos configurar la interrupción por recepción
		/* Debemos activar la interrupción RX en la configuración del USART */

		ptrUsartHandler->ptrUSARTx->CR1 |= USART_CR1_RXNEIE;

		/* Debemos matricular la interrupción en el NVIC */
		/* Lo debemos hacer para cada uno de las posibles opciones que tengamos (USART1, USART2, USART6) */
		if (ptrUsartHandler->ptrUSARTx == USART1) {
			__NVIC_EnableIRQ(USART1_IRQn);
			__NVIC_SetPriority(USART1_IRQn, 2);
		}

		else if (ptrUsartHandler->ptrUSARTx == USART2) {
			__NVIC_EnableIRQ(USART2_IRQn);
			__NVIC_SetPriority(USART2_IRQn, 2);
		}

		else if (ptrUsartHandler->ptrUSARTx == USART6) {
			__NVIC_EnableIRQ(USART6_IRQn);
			__NVIC_SetPriority(USART6_IRQn, 2);
		}
	}
	// 2.8b Interrupción por emisión
	else if (ptrUsartHandler->USART_Config.enableIntRX == USART_TX_INTERRUP_ENABLE) {
		// Como está activada, debemos configurar la interrupción por emisión.
		/* Debemos desactivar la interrupción TX en la configuración del USART */

		ptrUsartHandler->ptrUSARTx->CR1 |= USART_CR1_TXEIE;

		/* Debemos desmatricular la interrupción en el NVIC */
		/* Lo debemos hacer para cada uno de las posibles opciones que tengamos (USART1, USART2, USART6) */
		if (ptrUsartHandler->ptrUSARTx == USART1) {
			__NVIC_EnableIRQ(USART1_IRQn);
			__NVIC_SetPriority(USART1_IRQn, 2);
		}

		else if (ptrUsartHandler->ptrUSARTx == USART2) {
			__NVIC_EnableIRQ(USART2_IRQn);
			__NVIC_SetPriority(USART2_IRQn, 2);
		}

		else if (ptrUsartHandler->ptrUSARTx == USART6) {
			__NVIC_EnableIRQ(USART6_IRQn);
			__NVIC_SetPriority(USART6_IRQn, 2);
		}

	}
	else {
		__NOP();
	}
}

/**
 *
 */
static void usart_enable_peripheral(USART_Handler_t *ptrUsartHandler) {
	if (ptrUsartHandler->USART_Config.mode != USART_MODE_DISABLE) {
		ptrUsartHandler->ptrUSARTx->CR1 |= USART_CR1_UE;
	}
}

/*
 * función para escribir un solo char
 */
int usart_WriteChar(USART_Handler_t *ptrUsartHandler, int dataToSend) {
	while (!(ptrUsartHandler->ptrUSARTx->SR & USART_SR_TXE)) {
		__NOP();
	}
	ptrUsartHandler->ptrUSARTx->DR = dataToSend;

	return dataToSend;
}

/*
 *
 */
void usart_writeMsg(USART_Handler_t *ptrUsartHandler, char *msgToSend) {
	uint8_t i = 0;
	while (msgToSend[i] != 0) {
		usart_WriteChar(ptrUsartHandler, msgToSend[i]);
		i++;
	}
}

uint8_t usart_getRxData1(void) {
	return auxRxData1;
}
uint8_t usart_getRxData2(void) {
	return auxRxData2;
}
uint8_t usart_getRxData6(void) {
	return auxRxData6;
}


/* Handler de la interrupción del USART
 * Acá deben estar todas las interrupciones asociadas: TX, RX, PE...
 */
void USART2_IRQHandler(void) {
	// Evaluamos si la interrupción que se dio es por RX
	if (USART2->SR & USART_SR_RXNE){
		//Bajamos la bandera correspondiente leyendo el dato.

		auxRxData2 = USART2->DR;


		//Llamamos el callback
		usart2_RxCallback();

//		auxRxData2 = 0;
	// Evaluamos si la interrupción que se dio es por TX
	}
//	else if (USART2->SR & USART_SR_TXE){
//		//Bajamos la bandera correspondiente
//		USART2->SR |= USART_SR_TXE;
//
//		//Llamamos el callback
//		usart2_RxCallback();
//	}
}

/* Handler de la interrupción del USART
 * Acá deben estar todas las interrupciones asociadas: TX, RX, PE...
 */
void USART6_IRQHandler(void) {
	// Evaluamos si la interrupción que se dio es por RX
	if (USART6->SR & USART_SR_RXNE){
		//Bajamos la bandera correspondiente leyendo el dato.

		auxRxData6 = USART6->DR;

		//Llamamos el callback
		usart6_RxCallback();

	// Evaluamos si la interrupción que se dio es por TX
	}
//	else if (USART6->SR & USART_SR_TXE){
//		//Bajamos la bandera correspondiente
//		USART6->SR |= USART_SR_TXE;
//
//		//Llamamos el callback
//		usart6_RxCallback();
//	}
}

/* Handler de la interrupción del USART
 * Acá deben estar todas las interrupciones asociadas: TX, RX, PE...
 */
void USART1_IRQHandler(void) {
	// Evaluamos si la interrupción que se dio es por RX
	if (USART1->SR & USART_SR_RXNE){
		//Bajamos la bandera correspondiente leyendo el dato.

		auxRxData1 = USART1->DR;

		//Llamamos el callback
		usart1_RxCallback();

	// Evaluamos si la interrupción que se dio es por TX
	}
//	else if (USART1->SR & USART_SR_TXE){
//		//Bajamos la bandera correspondiente
//		USART1->SR |= USART_SR_TXE;
//
//		//Llamamos el callback
//		usart1_RxCallback();
//	}
}

__attribute__((weak)) void usart1_RxCallback(void) {
	/* NOTE : This function should not be modified, when the callback is needed,
	 the BasicTimer_Callback could be implemented in the main file
	 */
	__NOP();
}

__attribute__((weak)) void usart2_RxCallback(void) {
	/* NOTE : This function should not be modified, when the callback is needed,
	 the BasicTimer_Callback could be implemented in the main file
	 */
	__NOP();
}

__attribute__((weak)) void usart6_RxCallback(void) {
	/* NOTE : This function should not be modified, when the callback is needed,
	 the BasicTimer_Callback could be implemented in the main file
	 */
	__NOP();
}
