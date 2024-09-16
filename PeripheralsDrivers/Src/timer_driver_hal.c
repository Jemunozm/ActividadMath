/*
 * timer_driver_hal.c
 *
 *  Created on: Sep 20, 2023
 *      Author: imjeviz
 */
#include "stm32f4xx.h"
#include "stm32_assert.h"
#include "timer_driver_hal.h"

// Variable que guarda la refencia del periférico que se esta utilizando
TIM_TypeDef *ptrTimerUsed;

// === Headers for private functions ===
static void timer_enable_clock_peripheral(Timer_Handler_t *pTimerHandler);
static void timer_set_prescaler(Timer_Handler_t *pTimerHandler);
static void timer_set_period(Timer_Handler_t *pTimerHandler);
static void timer_set_mode(Timer_Handler_t *pTimerHandler);
static void timer_config_interrupt(Timer_Handler_t *pTimerHandler);

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
void timer_Config(Timer_Handler_t *pTimerHandler){
	//Guardamos una referencia al periférico que estmoas utilizando...
	ptrTimerUsed = pTimerHandler->pTIMx;

	// 0. Desactivamos la interrupciones globales mientras configuramos configuramos el sistema.
	__disable_irq();
	// 1. Activar la señal de reloj del periférico requerido.
	timer_enable_clock_peripheral(pTimerHandler);

	// 2. Configuramos el Pre-scaler.
	timer_set_prescaler(pTimerHandler);

	// 3. Configuramos si UP_COUNT O DOWN_COUNT (mode).
	timer_set_mode(pTimerHandler);

	// 4. Configuramos el Auto-reload.
	timer_set_period(pTimerHandler);

	// 5. Configuramos la interrupcion.
	timer_config_interrupt(pTimerHandler);

	// x. Volvemos a activar las interrupciones del sistema.
	__enable_irq();

	// El timer inicia apagado.
	timer_SetState(pTimerHandler, TIMER_OFF);
}




void timer_enable_clock_peripheral(Timer_Handler_t *pTimerHandler){

	// Verificamos que es un timer permitio
	assert_param(IS_TIM_INSTANCE(pTierHandler->pTIMx));

	if(pTimerHandler->pTIMx == TIM2){
		RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
	}
	else if(pTimerHandler->pTIMx == TIM3){
		RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
	}
	else if(pTimerHandler->pTIMx == TIM4){
		RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;
	}
	else if(pTimerHandler->pTIMx == TIM5){
		RCC->APB1ENR |= RCC_APB1ENR_TIM5EN;
	}
	else if(pTimerHandler->pTIMx == TIM9){
		RCC->APB2ENR |= RCC_APB2ENR_TIM9EN;
	}
	else if(pTimerHandler->pTIMx == TIM10){
		RCC->APB2ENR |= RCC_APB2ENR_TIM10EN;
	}
	else if(pTimerHandler->pTIMx == TIM11){
		RCC->APB2ENR |= RCC_APB2ENR_TIM11EN;
	}
	else{
		__NOP();
	}
}

/**
 * El prescaler nos configura la elocidad a la que se incrementa el registro
 * CNT del timer.
 */
void timer_set_prescaler(Timer_Handler_t *pTimerHandler){

	//Verificamos que el valor del prescaler es valido
	assert_param(IS_TIMER_PRESC(pTimerHandler->TIMx_Config.TIMx_Prescaler));

	//Configuramos el valor del prescaler.
	pTimerHandler->pTIMx->PSC = pTimerHandler->TIMx_Config.TIMx_Prescaler - 1; // (16000000-1)
}

/**
 * Esta función configura el limite hasta donde el Timer para generar un
 * evento "update" (cuando esta contando de forma ascendente),o configura
 * el valor desde donde se comienza a contar, cuando el sistema funciona de forma
 * descendente.
 */
void timer_set_period(Timer_Handler_t *pTimerHandler){

	// Verificamos que el valor que genera el periodo es valido.
	assert_param(IS_TIMER_PERIOD(pTimerHandler->TIMx_Config.TIMx_Period));

	// Acá hace falta algo...

	//Configuramos el valor del autoreload
	pTimerHandler->pTIMx->ARR = pTimerHandler->TIMx_Config.TIMx_Period - 1;
}




void timer_set_mode(Timer_Handler_t *pTimerHandler){

	//Verificamos que elmodo de funcionamiento es correcto.
	assert_param(IS_TIMER_MODE(pTimerHandler->TIMx_Config.TIMx_mode));

	//Verificamos cual es el modo que se desea configurar.
	if(pTimerHandler->TIMx_Config.TIMx_mode == TIMER_UP_COUNTER){
		//Configuramos en modo Upcounter DIR = 0
		pTimerHandler->pTIMx->CR1 &= ~TIM_CR1_DIR;
	}
	else{
		pTimerHandler->pTIMx->CR1 |= TIM_CR1_DIR;
	}
}





void timer_config_interrupt(Timer_Handler_t *pTimerHandler){

	//Verificamos el posible valor configurado
	assert_param(IS_TIMER_INTERRUP(pTimerHandler->TIMx_Config.TIMx_InterruptEnabled));

	if(pTimerHandler->TIMx_Config.TIMx_InterruptEnable == TIMER_INT_ENABLE){
		// Activamos la interrupción debida al Timerx Utilizado
		pTimerHandler->pTIMx->DIER |= TIM_DIER_UIE;


		//Activamos el canal del sistema NVIC para que lea la interrupción
		if(pTimerHandler->pTIMx == TIM2){
			NVIC_EnableIRQ(TIM2_IRQn);
		}
		else if(pTimerHandler->pTIMx == TIM3){
			NVIC_EnableIRQ(TIM3_IRQn);
		}
		else if(pTimerHandler->pTIMx == TIM4){
			NVIC_EnableIRQ(TIM4_IRQn);
		}
		else if(pTimerHandler->pTIMx == TIM5){
			NVIC_EnableIRQ(TIM5_IRQn);
		}
		else if(pTimerHandler->pTIMx == TIM9){
			NVIC_EnableIRQ(TIM1_BRK_TIM9_IRQn);
		}
		else if(pTimerHandler->pTIMx == TIM10){
			NVIC_EnableIRQ(TIM1_UP_TIM10_IRQn);
		}
		else if(pTimerHandler->pTIMx == TIM11){
			NVIC_EnableIRQ(TIM1_TRG_COM_TIM11_IRQn);
		}
		else{
			__NOP();
		}
	}
	else{
		// Desactivamos la interrupción debida al Timerx Utilizado
		pTimerHandler->pTIMx->DIER &= ~TIM_DIER_UIE;

		//Desactivamos el canal del sistema NVIC para que lea la interrupción
		if(pTimerHandler->pTIMx == TIM2){
			NVIC_DisableIRQ(TIM2_IRQn);
		}
		else if(pTimerHandler->pTIMx == TIM3){
			NVIC_DisableIRQ(TIM3_IRQn);
		}
		else if(pTimerHandler->pTIMx == TIM4){
			NVIC_DisableIRQ(TIM4_IRQn);
		}
		else if(pTimerHandler->pTIMx == TIM5){
			NVIC_DisableIRQ(TIM5_IRQn);
		}
		else if(pTimerHandler->pTIMx == TIM9){
			NVIC_DisableIRQ(TIM1_BRK_TIM9_IRQn);
		}
		else if(pTimerHandler->pTIMx == TIM10){
			NVIC_DisableIRQ(TIM1_UP_TIM10_IRQn);
		}
		else if(pTimerHandler->pTIMx == TIM11){
			NVIC_DisableIRQ(TIM1_TRG_COM_TIM11_IRQn);
		}
		else{
			__NOP();
		}
	}
}




void timer_SetState(Timer_Handler_t *pTimerHandler, uint8_t newState){

	//Verificamos que el estado ingresado es adecuado
	assert_param(IS_TIMER_STATE(newState));

	// 4. Reiniciamos el registro counter.
	pTimerHandler->pTIMx->CNT = 0;


	if(newState == TIMER_ON){
		// 5a. Activamos el Timer (el CNT debe comenzar a contar.
		pTimerHandler->pTIMx->CR1 |= TIM_CR1_CEN;
	}
	else{
		// 5b. Desactivamos el Timer (el CNT debe detenerse)
		pTimerHandler->pTIMx->CR1 &= ~TIM_CR1_CEN;
	}

}



__attribute__((weak)) void Timer2_Callback(void){
	__NOP();
}

__attribute__((weak)) void Timer3_Callback(void){
	__NOP();
}

__attribute__((weak)) void Timer4_Callback(void){
	__NOP();
}

__attribute__((weak)) void Timer5_Callback(void){
	__NOP();
}

__attribute__((weak)) void Timer9_Callback(void){
	__NOP();
}

__attribute__((weak)) void Timer10_Callback(void){
	__NOP();
}

__attribute__((weak)) void Timer11_Callback(void){
	__NOP();
}


/* Esta la función a la que apunta el sistema en el vector de interrupciones.
 * Se debe utilizar usando exactamente el mismo nombre definido en el vector de interrupciones.
 * Al hacerlo correctamente, el sistema apunta a esta función y cuando la interrupción se lanza
 * el sistema inmediatamente salta a este lugar en la memoria.*/
void TIM2_IRQHandler(void){
	//Limpiamos la bandera que indica que la interrupción se ha generado.
	TIM2->SR &= ~TIM_SR_UIF;

	//Llamamos a la función que se debe encargar de hacer algo con esta interrupción.
	Timer2_Callback();

}
/* Esta la función a la que apunta el sistema en el vector de interrupciones.
 * Se debe utilizar usando exactamente el mismo nombre definido en el vector de interrupciones.
 * Al hacerlo correctamente, el sistema apunta a esta función y cuando la interrupción se lanza
 * el sistema inmediatamente salta a este lugar en la memoria.*/
void TIM3_IRQHandler(void){
	//Limpiamos la bandera que indica que la interrupción se ha generado.
	TIM3->SR &= ~TIM_SR_UIF;

	//Llamamos a la función que se debe encargar de hacer algo con esta interrupción.
	Timer3_Callback();

}
/* Esta la función a la que apunta el sistema en el vector de interrupciones.
 * Se debe utilizar usando exactamente el mismo nombre definido en el vector de interrupciones.
 * Al hacerlo correctamente, el sistema apunta a esta función y cuando la interrupción se lanza
 * el sistema inmediatamente salta a este lugar en la memoria.*/
void TIM4_IRQHandler(void){
	//Limpiamos la bandera que indica que la interrupción se ha generado.
	TIM4->SR &= ~TIM_SR_UIF;

	//Llamamos a la función que se debe encargar de hacer algo con esta interrupción.
	Timer4_Callback();

}
/* Esta la función a la que apunta el sistema en el vector de interrupciones.
 * Se debe utilizar usando exactamente el mismo nombre definido en el vector de interrupciones.
 * Al hacerlo correctamente, el sistema apunta a esta función y cuando la interrupción se lanza
 * el sistema inmediatamente salta a este lugar en la memoria.*/
void TIM5_IRQHandler(void){
	//Limpiamos la bandera que indica que la interrupción se ha generado.
	TIM5->SR &= ~TIM_SR_UIF;

	//Llamamos a la función que se debe encargar de hacer algo con esta interrupción.
	Timer5_Callback();

}

/* Esta la función a la que apunta el sistema en el vector de interrupciones.
 * Se debe utilizar usando exactamente el mismo nombre definido en el vector de interrupciones.
 * Al hacerlo correctamente, el sistema apunta a esta función y cuando la interrupción se lanza
 * el sistema inmediatamente salta a este lugar en la memoria.*/
void TIM1_BRK_TIM9_IRQHandler(void){
	//Limpiamos la bandera que indica que la interrupción se ha generado.
		TIM9->SR &= ~TIM_SR_UIF;

		//Llamamos a la función que se debe encargar de hacer algo con esta interrupción.
		Timer9_Callback();
}

/* Esta la función a la que apunta el sistema en el vector de interrupciones.
 * Se debe utilizar usando exactamente el mismo nombre definido en el vector de interrupciones.
 * Al hacerlo correctamente, el sistema apunta a esta función y cuando la interrupción se lanza
 * el sistema inmediatamente salta a este lugar en la memoria.*/
void TIM1_UP_TIM10_IRQHandler(void){
	//Limpiamos la bandera que indica que la interrupción se ha generado.
		TIM10->SR &= ~TIM_SR_UIF;

		//Llamamos a la función que se debe encargar de hacer algo con esta interrupción.
		Timer10_Callback();
}

/* Esta la función a la que apunta el sistema en el vector de interrupciones.
 * Se debe utilizar usando exactamente el mismo nombre definido en el vector de interrupciones.
 * Al hacerlo correctamente, el sistema apunta a esta función y cuando la interrupción se lanza
 * el sistema inmediatamente salta a este lugar en la memoria.*/
void TIM1_TRG_COM_TIM11_IRQHandler(void){
	//Limpiamos la bandera que indica que la interrupción se ha generado.
		TIM11->SR &= ~TIM_SR_UIF;

		//Llamamos a la función que se debe encargar de hacer algo con esta interrupción.
		Timer11_Callback();
}
