/*
 * systick_driver_hal.h
 *
 *  Created on: Nov 7, 2023
 *      Author: imjeviz
 */

#ifndef SYSTICK_DRIVER_HAL_H_
#define SYSTICK_DRIVER_HAL_H_

#include <stm32f4xx.h>

enum{
	SYSTICK_OFF = 0,
	SYSTICK_ON
};

enum{
	SYSTICK_INT_DISABLE = 0,
	SYSTICK_INT_ENABLE
};

typedef struct{
	uint32_t	Systick_Reload;
	/* Valor de las repeticiones del signal clock del procesador para hacer una cuenta
	 *  - Recordar que 16.000.000 repeticiones son 1 ms */
	uint32_t	Systick_IntState; // Activa o desactiva las interrupciones
} Systick_BasicConfig_t;


typedef struct{
	SysTick_Type *pSystick; // Esta variable apunta hacia la dirección de la estructura definida en CMSIS
	Systick_BasicConfig_t Systick_Config; // Esta variable asigna el valor de dicho registro
}Systick_Handler_t;

/* Funciones públicas del driver */
void systick_Config(Systick_Handler_t *pSystickHandler);
void systick_SetState(Systick_Handler_t *pSystickHandler, uint8_t newState);
uint32_t systick_GetTicks();

void systick_Delay_ms(uint32_t wait_time_ms);

/* Esta función debe ser sobre-escrita en el main para que el sistema funcione */
void systick_Callback(void);

#endif /* SYSTICK_DRIVER_HAL_H_ */
