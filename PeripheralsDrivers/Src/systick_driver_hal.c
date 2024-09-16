/*
 * systick_driver_hal.c
 *
 *  Created on: Nov 7, 2023
 *      Author: imjeviz
 */

#include <stm32f4xx.h>
#include <systick_driver_hal.h>


uint64_t ticks = 0;
uint64_t ticks_start = 0;
uint64_t ticks_counting = 0;

static uint32_t countTicks = 0;

static void systick_reload_value(Systick_Handler_t *pSystickHandler);
static void systick_clear_value(Systick_Handler_t *pSystickHandler);
static void systick_config_clock(Systick_Handler_t *pSystickHandler);
static void systick_config_interrupt(Systick_Handler_t *pSystickHandler);



/**/
void systick_Config(Systick_Handler_t *pSystickHandler){
	//Reiniciamos el valor de la variable que cuenta tiempo
	ticks=0;

	/* 1. Programamos el reload Value */
	systick_reload_value(pSystickHandler);
	/* 2. Limpiamos el valor */
	systick_clear_value(pSystickHandler);

	/* 3. Configuramos el reloj para el timer*/
	systick_config_clock(pSystickHandler);
	/* 4. Desactivamos las interrupciones globales*/
	__disable_irq();
	/* 5. Configuramos la interrupcion*/
	systick_config_interrupt(pSystickHandler);
	//Activamos de nuevo las interrupciones globales
	/* 6. Activamos e lTimer */
	systick_SetState(pSystickHandler, SYSTICK_ON);
	__enable_irq();


}

static void systick_reload_value(Systick_Handler_t *pSystickHandler){
	//Limpiamos el registro
	pSystickHandler->pSystick->LOAD &= 0;
	// Asignamos el valor del Reload cargado en la estructura de configuración, en el registro correspondiente
	pSystickHandler->pSystick->LOAD = pSystickHandler->Systick_Config.Systick_Reload;
}

static void systick_clear_value(Systick_Handler_t *pSystickHandler){

	//Limpamos el valor actual del Systick
	pSystickHandler->pSystick->VAL = 0;

}

static void systick_config_clock(Systick_Handler_t *pSystickHandler){

	//Limpiamos elregistro
	pSystickHandler->pSystick->CTRL &= ~SysTick_CTRL_CLKSOURCE_Msk;

	// Configuramos el reloj interno como el reloj para el Timer
	pSystickHandler->pSystick->CTRL |= SysTick_CTRL_CLKSOURCE_Msk;

}
/*
 * Activamos o desactivamos las interrupciones. También las matriculamos o desmatriculamos del NVIC
 */
static void systick_config_interrupt(Systick_Handler_t *pSystickHandler){

	if(pSystickHandler->Systick_Config.Systick_IntState == SYSTICK_INT_ENABLE){
		// Limpiamos la posición
		pSystickHandler->pSystick->CTRL &= ~SysTick_CTRL_TICKINT_Msk;

		// Activamos la interrupción
		pSystickHandler->pSystick->CTRL |= SysTick_CTRL_TICKINT_Msk;

		// Matriculamos la interrupción en el NVIC
		__NVIC_EnableIRQ(SysTick_IRQn);
	}
	else{
		// Desactivamos la interrupción
		pSystickHandler->pSystick->CTRL &= ~SysTick_CTRL_TICKINT_Msk;

		// Desmatriculamos la interrupción en el NVIC
		__NVIC_DisableIRQ(SysTick_IRQn);
	}
}


/*
 * Con esta función encendemos o apagamos el Timer
 */
void systick_SetState(Systick_Handler_t *pSystickHandler, uint8_t newState){

	/* Mirammos cuál estado queremos configurar */
	switch(newState){
	case SYSTICK_ON: {
		pSystickHandler->pSystick->CTRL |= SysTick_CTRL_ENABLE_Msk;
		break;
	}
	case SYSTICK_OFF: {
		pSystickHandler->pSystick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
		break;
	}
	default: {
		__NOP();
		break;
	}
	}

}
/*
 * Devuelve la cantidad de Ticks (cuenta los ticks, o del tiempo según el Reload d)
 */
uint32_t systick_GetTicks(void){
	return countTicks;
}

void systick_Delay_ms(uint32_t wait_time_ms){
	// Captura  el primer valor de tiempo para comparar.
	ticks_start = systick_GetTicks();

	// Captura el segundo valor de tiempo para comparar.
	ticks_counting = systick_GetTicks();

	// Compara:si el valor "counting" es menor que el "start + wait"
	// Actualiza el valor "counting".
	//Repite esta operación hasta que counting sea mayor (se cuple el tiempo de espera)
	while(ticks_counting < (ticks_start + (uint32_t)wait_time_ms)){

		// actualizar el valor
		ticks_counting = systick_GetTicks();
	}
}
/*
 * El Handler de IRQ del Systick (El nombre de esta función es propio de las librerías del Cortex M4 -> )
 */
void SysTick_Handler(void){

	// Verificamos que la interrupción se lanzó
	if(SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk){

		// Limpiamos labandera
		SysTick->CTRL &= ~SysTick_CTRL_COUNTFLAG_Msk;
	}

	// Incrementamos en 1 el contador.
	countTicks++;

	/* Llamamos a la función que se debe encargar de hacer algo con esta interrupción */
	systick_Callback();
}

__attribute__((weak)) void systick_Callback(void){
	__NOP();
}
