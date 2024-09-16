/*
 * pwm_driver_hal.c
 *
 *  Created on: Nov 2, 2023
 *      Author: imjeviz
 */

#include "stm32f4xx.h"
#include "pwm_driver_hal.h"

/**/
// === Headers for private functions ===
static void pwm_enable_clock_peripheral(PWM_Handler_t *ptrPwmHandler);
static void pwm_set_mode(PWM_Handler_t *ptrPwmHandler);
static void pwm_set_compare_mode(PWM_Handler_t *ptrPwmHandler);


/* Función en la que cargamos la configuración del Timer
 * Recordar que siempre se debe comenzar con activar la señal de reloj
 * del periférico que se está utilizando.
 * Además, en este caso, debemos ser cuidadosos al momento de utilizar las interrupciones.
 * Los timer están conectadors directamente al elemento NVIC del cortex-Mx
 * Debemos configurar y/o utilizar:
 * 	- TIMx_CR1 	(control Register 1)
 * 	- TIMx_SMCR	(slave mode control register) -> mantener en 0 para modo Timer Básico
 * 	- TIMx_DIER (DMA and Interrupt enable register)
 * 	- TIMx_SR 	(Status register)
 * 	- TIMx_CNT	(Counter)
 * 	- TIMx_PSC	(Pre-scaler)
 * 	- TIMx_ARR	(Auto-reload register)
 *
 * 	Como vamos a trabajar con iterrupciones, antes de configurar una nueva,debemos desactivar
 * 	el sistema global de interrupciones, activar la IRQ específica y luego volver a encender
 * 	el sistema.
 */

void pwm_Config(PWM_Handler_t *ptrPwmHandler){

	// 1. Activar la señal de reloj del periférico requerido.
	pwm_enable_clock_peripheral(ptrPwmHandler);

	ptrPwmHandler->ptrTIMx->CR1 |= TIM_CR1_ARPE;

	/* 1a. Cargamos la frecuencia deseada */
	setFrequency(ptrPwmHandler);

	/* 2. Cargamos el valor del dutty-Cycle*/
	setDuttyCycle(ptrPwmHandler);

	/* 2a. Estamos en UP_Mode, el limite se carga en ARR y se comienza en 0 */
	/* agregue acá su código */
	pwm_set_mode(ptrPwmHandler);

	/* 3. Configuramos los bits CCxS del registro TIMy_CCMR1, de forma que sea modo salida
	 * (para cada canal hay un conjunto CCxS)
	 *
	 * 4. Además, en el mismo "case" podemos configurar el modo del PWM, su polaridad...
	 *
	 * 5. Y además activamos el preload bit, para que cada vez que exista un update-event
	 * el valor cargado en el CCRx será recargado en el registro "shadow" del PWM */

	pwm_set_compare_mode(ptrPwmHandler);


}

void pwm_enable_clock_peripheral(PWM_Handler_t *ptrPwmHandler){

	if(ptrPwmHandler->ptrTIMx == TIM2){
		RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
	}
	else if(ptrPwmHandler->ptrTIMx == TIM3){
		RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
	}
	else if(ptrPwmHandler->ptrTIMx == TIM4){
		RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;
	}
	else if(ptrPwmHandler->ptrTIMx == TIM5){
		RCC->APB1ENR |= RCC_APB1ENR_TIM5EN;
	}
	else{
		__NOP();
	}
}

void pwm_set_mode(PWM_Handler_t *ptrPwmHandler){
	ptrPwmHandler->ptrTIMx->CR1 &= ~TIM_CR1_DIR;
}

void pwm_set_compare_mode(PWM_Handler_t *ptrPwmHandler){

	switch(ptrPwmHandler->config.channel){
	case PWM_CHANNEL_1:{
		// Seleccionamos como salida el canal
		/* agregue acá su código */
		ptrPwmHandler->ptrTIMx->CCMR1 &= ~TIM_CCMR1_CC1S;

		// Configuramos el canal como PWM
		/* agregue acá su código */

		//Primero limpiamos el registro
		ptrPwmHandler->ptrTIMx->CCMR1 &= ~TIM_CCMR1_OC1M;

		//Ahora cargamos el mode PWM 1
		ptrPwmHandler->ptrTIMx->CCMR1 |= (0x6UL << TIM_CCMR1_OC1M_Pos);

		// Activamos la funcionalidad de pre-load
		/* agregue acá su código */

		//Limpiamos elregistro primero
		ptrPwmHandler->ptrTIMx->CCMR1 &= ~TIM_CCMR1_OC1PE;

		//cargamos laconfiguración
		ptrPwmHandler->ptrTIMx->CCMR1 |= TIM_CCMR1_OC1PE;

		break;
	}

	case PWM_CHANNEL_2:{
		// Seleccionamos como salida el canal
		/* agregue acá su código */

		ptrPwmHandler->ptrTIMx->CCMR1 &= ~TIM_CCMR1_CC2S;

		// Configuramos el canal como PWM
		/* agregue acá su código */

		//Primero limpiamos el registro
		ptrPwmHandler->ptrTIMx->CCMR1 &= ~TIM_CCMR1_OC2M;

		//Ahora cargamos el mode PWM 1
		ptrPwmHandler->ptrTIMx->CCMR1 |= (0x6UL << TIM_CCMR1_OC2M_Pos);

		// Activamos la funcionalidad de pre-load
		/* agregue acá su código */

		//Limpiamos elregistro primero
		ptrPwmHandler->ptrTIMx->CCMR1 &= ~TIM_CCMR1_OC2PE;

		//cargamos laconfiguración
		ptrPwmHandler->ptrTIMx->CCMR1 |= TIM_CCMR1_OC2PE;

		break;
	}

    /* agregue acá los otros dos casos */

	case PWM_CHANNEL_3:{
		// Seleccionamos como salida el canal
		/* agregue acá su código */

		ptrPwmHandler->ptrTIMx->CCMR2 &= ~TIM_CCMR2_CC3S;

		// Configuramos el canal como PWM
		/* agregue acá su código */

		//Primero limpiamos el registro
		ptrPwmHandler->ptrTIMx->CCMR2 &= ~TIM_CCMR2_OC3M;

		//Ahora cargamos el mode PWM 1
		ptrPwmHandler->ptrTIMx->CCMR2 |= (0x6UL << TIM_CCMR2_OC3M_Pos);

		// Activamos la funcionalidad de pre-load
		/* agregue acá su código */

		//Limpiamos elregistro primero
		ptrPwmHandler->ptrTIMx->CCMR2 &= ~TIM_CCMR2_OC3PE;

		//cargamos laconfiguración
		ptrPwmHandler->ptrTIMx->CCMR2 |= TIM_CCMR2_OC3PE;

		break;
	}

	case PWM_CHANNEL_4:{
		// Seleccionamos como salida el canal
		/* agregue acá su código */

		ptrPwmHandler->ptrTIMx->CCMR2 &= ~TIM_CCMR2_CC4S;

		// Configuramos el canal como PWM
		/* agregue acá su código */

		//Primero limpiamos el registro
		ptrPwmHandler->ptrTIMx->CCMR2 &= ~TIM_CCMR2_OC4M;

		//Ahora cargamos el mode PWM 1
		ptrPwmHandler->ptrTIMx->CCMR2 |= (0x6UL << TIM_CCMR2_OC4M_Pos);

		// Activamos la funcionalidad de pre-load
		/* agregue acá su código */

		//Limpiamos elregistro primero
		ptrPwmHandler->ptrTIMx->CCMR2 &= ~TIM_CCMR2_OC4PE;

		//cargamos laconfiguración
		ptrPwmHandler->ptrTIMx->CCMR2 |= TIM_CCMR2_OC4PE;

		break;
	}

	default:{
		break;
	}

	}// fin del switch-case

}

/* Función para activar el Timer y activar todo el módulo PWM */
void startPwmSignal(PWM_Handler_t *ptrPwmHandler) {
	pwm_set_compare_mode(ptrPwmHandler);
	/* agregue acá su código */

	//Reiniciamos el registro counter.
	ptrPwmHandler->ptrTIMx->CNT = 0;

	//Activamos el timer (el CNT debe comenzar a contar).
	ptrPwmHandler->ptrTIMx->CR1 |= TIM_CR1_CEN;

	/* 6. Activamos la salida seleccionada */
	enableOutput(ptrPwmHandler);

}

/* Función para desactivar el Timer y detener todo el módulo PWM*/
void stopPwmSignal(PWM_Handler_t *ptrPwmHandler) {
	/* agregue acá su código */

	//Desactivamos el Timer (el CNT debe detenerse)
	ptrPwmHandler->ptrTIMx->CR1 &= ~TIM_CR1_CEN;
}

/* Función encargada de activar cada uno de los canales con los que cuenta el TimerX */
void enableOutput(PWM_Handler_t *ptrPwmHandler) {
	switch (ptrPwmHandler->config.channel) {
	case PWM_CHANNEL_1: {
		// Activamos la salida del canal 1
		/* agregue acá su código */
		ptrPwmHandler->ptrTIMx->CCER |= TIM_CCER_CC1E;
		break;
	}
	case PWM_CHANNEL_2: {
		// Activamos la salida del canal 1
		/* agregue acá su código */
		ptrPwmHandler->ptrTIMx->CCER |= TIM_CCER_CC2E;
		break;
	}
	case PWM_CHANNEL_3: {
		// Activamos la salida del canal 1
		/* agregue acá su código */
		ptrPwmHandler->ptrTIMx->CCER |= TIM_CCER_CC3E;
		break;
	}
	case PWM_CHANNEL_4: {
		// Activamos la salida del canal 1
		/* agregue acá su código */
		ptrPwmHandler->ptrTIMx->CCER |= TIM_CCER_CC4E;
		break;
	}

	/* agregue acá su código para los otros tres casos */

	default: {
		break;
	}
	}
}

void disableOutput(PWM_Handler_t *ptrPwmHandler){
	switch (ptrPwmHandler->config.channel) {
		case PWM_CHANNEL_1: {
			// Activamos la salida del canal 1
			/* agregue acá su código */
			ptrPwmHandler->ptrTIMx->CCER &= ~TIM_CCER_CC1E;
			break;
		}
		case PWM_CHANNEL_2: {
			// Activamos la salida del canal 1
			/* agregue acá su código */
			ptrPwmHandler->ptrTIMx->CCER &= ~TIM_CCER_CC2E;
			break;
		}
		case PWM_CHANNEL_3: {
			// Activamos la salida del canal 1
			/* agregue acá su código */
			ptrPwmHandler->ptrTIMx->CCER &= ~TIM_CCER_CC3E;
			break;
		}
		case PWM_CHANNEL_4: {
			// Activamos la salida del canal 1
			/* agregue acá su código */
			ptrPwmHandler->ptrTIMx->CCER &= ~TIM_CCER_CC4E;
			break;
		}

		default: {
			break;
		}
		}
}

/*
 * La frecuencia es definida por el conjunto formado por el preescaler (PSC)
 * y el valor límite al que llega el Timer (ARR), con estos dos se establece
 * la frecuencia.
 * */
void setFrequency(PWM_Handler_t *ptrPwmHandler){

	// Cargamos el valor del prescaler, nos define la velocidad (en ns) a la cual
	// se incrementa el Timer
	ptrPwmHandler->ptrTIMx->PSC = ptrPwmHandler->config.prescaler - 1;

	// Cargamos el valor del ARR, el cual es el límite de incrementos del Timer
	// antes de hacer un update y reload.
	ptrPwmHandler->ptrTIMx->ARR = ptrPwmHandler->config.periodo - 1;
}


/* Función para actualizar la frecuencia, funciona de la mano con setFrequency */
void updateFrequency(PWM_Handler_t *ptrPwmHandler, uint16_t newFreq){
	// Actualizamos el registro que manipula el periodo
    /* agregue acá su código */
	ptrPwmHandler->ptrTIMx->ARR = newFreq - 1;

	// Llamamos a la fucnión que cambia la frecuencia
	/* agregue acá su código */
//	setFrequency(ptrPwmHandler);
}

/* El valor del dutty debe estar dado en valores de %, entre 0% y 100%*/
void setDuttyCycle(PWM_Handler_t *ptrPwmHandler){

	// Seleccionamos el canal para configurar su dutty
	switch(ptrPwmHandler->config.channel){
	case PWM_CHANNEL_1:{
		ptrPwmHandler->ptrTIMx->CCR1 = ptrPwmHandler->config.duttyCicle - 1;
		break;
	}
	/* agregue acá su código con los otros tres casos */

	case PWM_CHANNEL_2:{
		ptrPwmHandler->ptrTIMx->CCR2 = ptrPwmHandler->config.duttyCicle - 1;
		break;
	}

	case PWM_CHANNEL_3:{
		ptrPwmHandler->ptrTIMx->CCR3 = ptrPwmHandler->config.duttyCicle - 1;
		break;
	}

	case PWM_CHANNEL_4:{
		ptrPwmHandler->ptrTIMx->CCR4 = ptrPwmHandler->config.duttyCicle - 1;
		break;
	}
	default:{
		break;
	}

	}// fin del switch-case

}


/* Función para actualizar el Dutty, funciona de la mano con setDuttyCycle */
void updateDuttyCycle(PWM_Handler_t *ptrPwmHandler, uint16_t newDutty){
	// Actualizamos el registro que manipula el dutty

	// Seleccionamos el canal para configurar su dutty
	switch(ptrPwmHandler->config.channel){
	case PWM_CHANNEL_1:{
		ptrPwmHandler->ptrTIMx->CCR1 = newDutty - 1;
		break;
	}
	case PWM_CHANNEL_2:{
		ptrPwmHandler->ptrTIMx->CCR2 = newDutty - 1;
		break;
	}

	case PWM_CHANNEL_3:{
		ptrPwmHandler->ptrTIMx->CCR3 = newDutty - 1;
		break;
	}

	case PWM_CHANNEL_4:{
		ptrPwmHandler->ptrTIMx->CCR4 = newDutty - 1;
		break;
	}
	default:{
		break;
	}

	}// fin del switch-case

	// Llamamos a la fucnión que cambia el dutty y cargamos el nuevo valor
	/* agregue acá su código */
//	setDuttyCycle(ptrPwmHandler);

}


