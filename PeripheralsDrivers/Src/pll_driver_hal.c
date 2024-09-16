/*
 * pll_driver_hal.c
 *
 *  Created on: Nov 16, 2023
 *      Author: imjeviz
 */

#include <stm32f4xx.h>
#include "pll_driver_hal.h"

void pll_config(void){

	RCC->PLLCFGR &= ~(1<<RCC_PLLCFGR_PLLSRC_Pos);

	RCC->PLLCFGR &= ~(RCC_PLLCFGR_PLLM);

	RCC->PLLCFGR |= (8<<RCC_PLLCFGR_PLLM_Pos);

	RCC->PLLCFGR &= ~(RCC_PLLCFGR_PLLP);

	RCC->PLLCFGR &= ~(RCC_PLLCFGR_PLLN);

	RCC->PLLCFGR |=  (100<<RCC_PLLCFGR_PLLN_Pos);

	PWR->CR &= ~(PWR_CR_VOS);

	PWR->CR |= (PWR_CR_VOS);

	RCC->CR &= ~(RCC_CR_PLLON);

	RCC->CR |= (RCC_CR_PLLON);

	while(!(RCC->CR & RCC_CR_PLLRDY)){
		__NOP();
	}

	RCC->CFGR &= ~(RCC_CFGR_PPRE2);

	RCC->CFGR &= ~(RCC_CFGR_PPRE1);

	RCC->CFGR |= (RCC_CFGR_PPRE1_2);

	FLASH->ACR &= ~(0XF << FLASH_ACR_LATENCY_Pos);

	FLASH->ACR |= (3 << FLASH_ACR_LATENCY_Pos);

	RCC->CFGR |= (RCC_CFGR_SW_1);
}

