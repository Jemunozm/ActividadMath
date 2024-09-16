/*
 * pll_driver_hal.h
 *
 *  Created on: Nov 16, 2023
 *      Author: imjeviz
 */

#ifndef PLL_DRIVER_HAL_H_
#define PLL_DRIVER_HAL_H_

enum{
	HSI_CLOCK_CONFIGURED = 0,
	HSE_CLOCK_CONFIGURED,
	PLL_CLOCK_CONFIGURED,
};




void pll_config(void);


#endif /* PLL_DRIVER_HAL_H_ */
