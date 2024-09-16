/*
 * adc_driver_hal.c
 *
 *  Created on: Oct 17, 2023
 *      Author: imjeviz
 */

#include "adc_driver_hal.h"
#include "gpio_driver_hal.h"
#include "stm32f4xx.h"
#include "stm32_assert.h"

/* === Headers for private functions === */
static void adc_enable_clock_peripheral(void);
static void adc_set_resolution(ADC_Config_t *adcConfig);
static void adc_set_alignment(ADC_Config_t *adcConfig);
static void adc_set_sampling_and_hold(ADC_Config_t *adcConfig);
static void adc_set_one_channel_sequence(ADC_Config_t *adcConfig);
static void adc_set_multi_channel_sequence(ADC_Config_t adcConfig[16],
		uint8_t numerodecanales);
static void adc_config_interrupt(ADC_Config_t *adcConfig);

/* Variables y elementos que necesita internamente el driver para funcionar adecuadamente*/
GPIO_Handler_t handlerADCPin = { 0 };
uint16_t adcRawData = 0;

/*
 *
 */
void adc_ConfigSingleChannel(ADC_Config_t *adcConfig) {

	/* 1. Configuramos el PinX para que cumpla la funcion del canal analogo deseado */
	adc_ConfigAnalogPin(adcConfig->channel);

	/* 2. Activamos la señal de reloj para el ADC */
	adc_enable_clock_peripheral();

	//Limpiamos los registros antes de comenzar a configurar
	ADC1->CR1 = 0;
	ADC1->CR2 = 0;

	/* Comenzamos la configuración de ADC1*/

	/* 3. Resolucion del ADC */
	adc_set_resolution(adcConfig);

	/* 4. Configuramos el modo Scan como desactivado */
	adc_ScanMode(SCAN_OFF);

	/* 5. Configuramos la aligeación de los datos (derecha o izquierda) */
	adc_set_alignment(adcConfig);

	/* 6. Desactivamos el "continuous mode"	*/
	adc_StopContinouosConv();

	/* 7. Acá se debería configurar el sampling ...*/
	adc_set_sampling_and_hold(adcConfig);

	/* 8. Configuramos la secuencia y cuantos elementos hay en las secuencias */
	adc_set_one_channel_sequence(adcConfig);

	/* 9. Configuramos el prescales del ADC en 2:1 (el más rápido que se pueda tener) */
	ADC->CCR &= ~ADC_CCR_ADCPRE;

	/* 10. Desactivamos las interrupciones globales */
	__disable_irq();

	/* 11. Configuramos la interrupción (si se encuentra activa), ademas de inscribir/remover
	 * la interrupción en el NVIC */
	adc_config_interrupt(adcConfig);

	/* 12. Activamos el moduloADC */
	adc_peripheralOnOff(ADC_ON);

	/* 13. Activamos las interrupciones globales */
	__enable_irq();
}

/* Configuracion para hacer conversiones en multiples canales y en un orden especifico */
void adc_ConfigMultichannel(ADC_Config_t channels[16], uint8_t numeroDeCanales) {

	/* 1. Configuramos el PinX para que cumpla la funcion del canal analogo deseado */
	for (uint8_t i = 0; i < (numeroDeCanales); i++) {
		adc_ConfigAnalogPin(channels[i].channel);
	}

	/* 2. Activamos la señal de reloj para el ADC */
	adc_enable_clock_peripheral();

	//Limpiamos los registros antes de comenzar a configurar
	ADC1->CR1 = 0;
	ADC1->CR2 = 0;

	/* Comenzamos la configuración de ADC1*/

	/* 3. Resolucion del ADC */
	adc_set_resolution(&channels[0]);

	/* 4. Configuramos el modo Scan como desactivado */
	adc_ScanMode(SCAN_ON);

	/* 5. Configuramos la aligeación de los datos (derecha o izquierda) */
	adc_set_alignment(&channels[0]);

	/* 6. Desactivamos el "continuous mode"	*/
	adc_StopContinouosConv();

	/* 7. Acá se debería configurar el sampling ...*/
	for (uint8_t i = 0; i < (numeroDeCanales); i++) {
		adc_set_sampling_and_hold(&channels[i]);
	}

	/* 8. Configuramos la secuencia y cuantos elementos hay en las secuencias */
	adc_set_multi_channel_sequence(channels, numeroDeCanales);

	/* 9. Configuramos el prescales del ADC en 2:1 (el más rápido que se pueda tener) */
	ADC->CCR &= ~ADC_CCR_ADCPRE;

	/* 10. Desactivamos las interrupciones globales */
	__disable_irq();

	ADC1->CR1 |= ADC_CR1_EOCIE;
	ADC1->CR2 |= ADC_CR2_EOCS;

	/* 11. Configuramos la interrupción (si se encuentra activa), ademas de inscribir/remover
	 * la interrupción en el NVIC */
	adc_config_interrupt(&channels[0]);

	/* 12. Activamos el moduloADC */
	adc_peripheralOnOff(ADC_ON);

	/* 13. Activamos las interrupciones globales */
	__enable_irq();
}

void adc_ConfigTrigger(uint8_t sourceType, PWM_Handler_t *triggerSignal) {

	//Limpiamos el registro
	ADC1->CR2 &= ~(ADC_CR2_EXTEN);


	//Hacemos el EXTSEL
	switch (sourceType) {
	case TRIGGER_DISABLE: {
		ADC1->CR2 &= ~(ADC_CR2_EXTEN);
		break;
	}
	case TRIGGER_RISING: {
		//Activamosel EXTEN
		ADC1->CR2 |= ADC_CR2_EXTEN_0;
		if (triggerSignal->ptrTIMx == TIM2) {
			switch (triggerSignal->config.channel) {
			case PWM_CHANNEL_2: {
				ADC1->CR2 |= (3 << ADC_CR2_EXTSEL_Pos);
				break;
			}
			case PWM_CHANNEL_3: {
				ADC1->CR2 |= (4 << ADC_CR2_EXTSEL_Pos);
				break;
			}
			case PWM_CHANNEL_4: {
				ADC1->CR2 |= (5 << ADC_CR2_EXTSEL_Pos);
				break;
			}
			default: {
				break;
			}
			}
		} else if (triggerSignal->ptrTIMx == TIM3) {
			if (triggerSignal->config.channel == PWM_CHANNEL_1) {
				ADC1->CR2 |= (7 << ADC_CR2_EXTSEL_Pos);
			}
		} else if (triggerSignal->ptrTIMx == TIM4) {
			if (triggerSignal->config.channel == PWM_CHANNEL_4) {
				ADC1->CR2 |= (9 << ADC_CR2_EXTSEL_Pos);
			}
		} else if (triggerSignal->ptrTIMx == TIM5) {
			switch (triggerSignal->config.channel) {
			case PWM_CHANNEL_1: {
				ADC1->CR2 |= (10 << ADC_CR2_EXTSEL_Pos);
				break;
			}
			case PWM_CHANNEL_2: {
				ADC1->CR2 |= (11 << ADC_CR2_EXTSEL_Pos);
				break;
			}
			case PWM_CHANNEL_3: {
				ADC1->CR2 |= (12 << ADC_CR2_EXTSEL_Pos);
				break;
			}
			default: {
				break;
			}
			}
		}
		break;
	}
	case TRIGGER_FALLING: {
		ADC1->CR2 |= ADC_CR2_EXTEN_1;
		if (triggerSignal->ptrTIMx == TIM2) {
			switch (triggerSignal->config.channel) {
			case PWM_CHANNEL_2: {
				ADC1->CR2 |= (3 << ADC_CR2_EXTSEL_Pos);
				break;
			}
			case PWM_CHANNEL_3: {
				ADC1->CR2 |= (4 << ADC_CR2_EXTSEL_Pos);
				break;
			}
			case PWM_CHANNEL_4: {
				ADC1->CR2 |= (5 << ADC_CR2_EXTSEL_Pos);
				break;
			}
			default: {
				break;
			}
			}
		} else if (triggerSignal->ptrTIMx == TIM3) {
			if (triggerSignal->config.channel == PWM_CHANNEL_1) {
				ADC1->CR2 |= (7 << ADC_CR2_EXTSEL_Pos);
			}
		} else if (triggerSignal->ptrTIMx == TIM4) {
			if (triggerSignal->config.channel == PWM_CHANNEL_4) {
				ADC1->CR2 |= (9 << ADC_CR2_EXTSEL_Pos);
			}
		} else if (triggerSignal->ptrTIMx == TIM5) {
			switch (triggerSignal->config.channel) {
			case PWM_CHANNEL_1: {
				ADC1->CR2 |= (10 << ADC_CR2_EXTSEL_Pos);
				break;
			}
			case PWM_CHANNEL_2: {
				ADC1->CR2 |= (11 << ADC_CR2_EXTSEL_Pos);
				break;
			}
			case PWM_CHANNEL_3: {
				ADC1->CR2 |= (12 << ADC_CR2_EXTSEL_Pos);
				break;
			}
			default: {
				break;
			}
			}
		}
		break;
		break;
	}
	}
}


/*
 * Enable Clock signal for ADC peripheral
 */
static void adc_enable_clock_peripheral(void) {
	/* 2.0 Activamos el acceso al ADC */
	RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
}

/*
 * Configures the resolution for the conversion
 */
static void adc_set_resolution(ADC_Config_t *adcConfig) {

	/* 3. Resolucion del ADC */
	//Debemos colocar cada una de las resoluciones posibles.
	//Limpiamos el registro antes deutilizarlo.
	ADC1->CR1 &= ~ADC_CR1_RES;

	/* Asignamos la resolucion que corresponda a la
	 * configuración deseada, teniendo en cuenta todas
	 * las resoluciones posibles.
	 */
	switch (adcConfig->resolution) {
	/* Configurando para la resolución de 12Bits*/
	case RESOLUTION_12_BIT: {
		// Ahora cargamos el valor en la posición, segun sea la selección
		ADC1->CR1 &= ~ADC_CR1_RES;
		break;
	}
		/* Configurando para la resolución de 10Bits*/
	case RESOLUTION_10_BIT: {
		// Ahora cargamos el valor en la posición, segun sea la selección
		ADC1->CR1 |= ADC_CR1_RES_0;
		break;
	}
		/* Configurando para la resolución de 8Bits*/
	case RESOLUTION_8_BIT: {
		// Ahora cargamos el valor en la posición, segun sea la selección
		ADC1->CR1 |= ADC_CR1_RES_1;
		break;
	}
		/* Configurando para la resolución de 6Bits*/
	case RESOLUTION_6_BIT: {
		// Ahora cargamos el valor en la posición, segun sea la selección
		ADC1->CR1 |= ADC_CR1_RES;
		break;
	}
	default: {
		__NOP();
		break;
	}
	}
}

/*
 * Set the number alignment: left or right
 */
static void adc_set_alignment(ADC_Config_t *adcConfig) {
	/* 5. Configuramos la aligeación de los datos (derecha o izquierda) */

	//Limpiamos el registro con la ubicacion del alignment
	ADC1->CR2 &= ~ADC_CR2_ALIGN;

	/* Asignamos la aligneación que corresponda a la
	 * configuración deseada, teniendo en cuenta todas
	 * las alineaciones posibles.
	 */
	switch (adcConfig->dataAlignment) {
	/* Configurando para alinear a la derecha*/
	case ALIGNMENT_RIGHT: {
		// Ahora cargamos el valor en la posición, segun sea la selección
		ADC1->CR2 &= ~ADC_CR2_ALIGN;
		break;
	}
		/* Configurando para alinear a la izquierda*/
	case ALIGNMENT_LEFT: {
		// Ahora cargamos el valor en la posición, segun sea la selección
		ADC1->CR2 |= ADC_CR2_ALIGN;
		break;
	}
	default: {
		ADC1->CR2 &= ~ADC_CR2_ALIGN;
		break;
	}
	}
}

/*
 * Relacionando con el valor del timepo de carga del capacitor HOLD
 */
static void adc_set_sampling_and_hold(ADC_Config_t *adcConfig) {

	switch (adcConfig->channel) {
	case CHANNEL_0: {
		switch (adcConfig->samplingPeriod) {
		case SAMPLING_PERIOD_3_CYCLES: {
			ADC1->SMPR2 &= ~ADC_SMPR2_SMP0;
			break;
		}
		case SAMPLING_PERIOD_15_CYCLES: {
			ADC1->SMPR2 |= ADC_SMPR2_SMP0_0;
			break;
		}
		case SAMPLING_PERIOD_28_CYCLES: {
			ADC1->SMPR2 |= ADC_SMPR2_SMP0_1;
			break;
		}
		case SAMPLING_PERIOD_56_CYCLES: {
			ADC1->SMPR2 |= (ADC_SMPR2_SMP0_0 | ADC_SMPR2_SMP0_1);
			break;
		}
		case SAMPLING_PERIOD_84_CYCLES: {
			ADC1->SMPR2 |= ADC_SMPR2_SMP0_2;
			break;
		}
		case SAMPLING_PERIOD_112_CYCLES: {
			ADC1->SMPR2 |= (ADC_SMPR2_SMP0_2 | ADC_SMPR2_SMP0_0);
			break;
		}
		case SAMPLING_PERIOD_144_CYCLES: {
			ADC1->SMPR2 |= (ADC_SMPR2_SMP0_2 | ADC_SMPR2_SMP0_1);
			break;
		}
		case SAMPLING_PERIOD_480_CYCLES: {
			ADC1->SMPR2 |= ADC_SMPR2_SMP0;
			break;
		}
		default: {
			__NOP();
			break;
		}
		}
		break;
	}
	case CHANNEL_1: {
		switch (adcConfig->samplingPeriod) {
		case SAMPLING_PERIOD_3_CYCLES: {
			ADC1->SMPR2 &= ~ADC_SMPR2_SMP1;
			break;
		}
		case SAMPLING_PERIOD_15_CYCLES: {
			ADC1->SMPR2 |= ADC_SMPR2_SMP1_0;
			break;
		}
		case SAMPLING_PERIOD_28_CYCLES: {
			ADC1->SMPR2 |= ADC_SMPR2_SMP1_1;
			break;
		}
		case SAMPLING_PERIOD_56_CYCLES: {
			ADC1->SMPR2 |= (ADC_SMPR2_SMP1_0 | ADC_SMPR2_SMP1_1);
			break;
		}
		case SAMPLING_PERIOD_84_CYCLES: {
			ADC1->SMPR2 |= ADC_SMPR2_SMP1_2;
			break;
		}
		case SAMPLING_PERIOD_112_CYCLES: {
			ADC1->SMPR2 |= (ADC_SMPR2_SMP1_2 | ADC_SMPR2_SMP1_0);
			break;
		}
		case SAMPLING_PERIOD_144_CYCLES: {
			ADC1->SMPR2 |= (ADC_SMPR2_SMP1_2 | ADC_SMPR2_SMP1_1);
			break;
		}
		case SAMPLING_PERIOD_480_CYCLES: {
			ADC1->SMPR2 |= ADC_SMPR2_SMP1;
			break;
		}
		default: {
			__NOP();
			break;
		}
		}
		break;
	}
	case CHANNEL_2: {
		switch (adcConfig->samplingPeriod) {
		case SAMPLING_PERIOD_3_CYCLES: {
			ADC1->SMPR2 &= ~ADC_SMPR2_SMP2;
			break;
		}
		case SAMPLING_PERIOD_15_CYCLES: {
			ADC1->SMPR2 |= ADC_SMPR2_SMP2_0;
			break;
		}
		case SAMPLING_PERIOD_28_CYCLES: {
			ADC1->SMPR2 |= ADC_SMPR2_SMP2_1;
			break;
		}
		case SAMPLING_PERIOD_56_CYCLES: {
			ADC1->SMPR2 |= (ADC_SMPR2_SMP2_0 | ADC_SMPR2_SMP2_1);
			break;
		}
		case SAMPLING_PERIOD_84_CYCLES: {
			ADC1->SMPR2 |= ADC_SMPR2_SMP2_2;
			break;
		}
		case SAMPLING_PERIOD_112_CYCLES: {
			ADC1->SMPR2 |= (ADC_SMPR2_SMP2_2 | ADC_SMPR2_SMP2_0);
			break;
		}
		case SAMPLING_PERIOD_144_CYCLES: {
			ADC1->SMPR2 |= (ADC_SMPR2_SMP2_2 | ADC_SMPR2_SMP2_1);
			break;
		}
		case SAMPLING_PERIOD_480_CYCLES: {
			ADC1->SMPR2 |= ADC_SMPR2_SMP2;
			break;
		}
		default: {
			__NOP();
			break;
		}
		}
		break;
	}
	case CHANNEL_3: {
		switch (adcConfig->samplingPeriod) {
		case SAMPLING_PERIOD_3_CYCLES: {
			ADC1->SMPR2 &= ~ADC_SMPR2_SMP3;
			break;
		}
		case SAMPLING_PERIOD_15_CYCLES: {
			ADC1->SMPR2 |= ADC_SMPR2_SMP3_0;
			break;
		}
		case SAMPLING_PERIOD_28_CYCLES: {
			ADC1->SMPR2 |= ADC_SMPR2_SMP3_1;
			break;
		}
		case SAMPLING_PERIOD_56_CYCLES: {
			ADC1->SMPR2 |= (ADC_SMPR2_SMP3_0 | ADC_SMPR2_SMP3_1);
			break;
		}
		case SAMPLING_PERIOD_84_CYCLES: {
			ADC1->SMPR2 |= ADC_SMPR2_SMP3_2;
			break;
		}
		case SAMPLING_PERIOD_112_CYCLES: {
			ADC1->SMPR2 |= (ADC_SMPR2_SMP3_2 | ADC_SMPR2_SMP3_0);
			break;
		}
		case SAMPLING_PERIOD_144_CYCLES: {
			ADC1->SMPR2 |= (ADC_SMPR2_SMP3_2 | ADC_SMPR2_SMP3_1);
			break;
		}
		case SAMPLING_PERIOD_480_CYCLES: {
			ADC1->SMPR2 |= ADC_SMPR2_SMP3;
			break;
		}
		default: {
			__NOP();
			break;
		}
		}
		break;
	}
	case CHANNEL_4: {
		switch (adcConfig->samplingPeriod) {
		case SAMPLING_PERIOD_3_CYCLES: {
			ADC1->SMPR2 &= ~ADC_SMPR2_SMP4;
			break;
		}
		case SAMPLING_PERIOD_15_CYCLES: {
			ADC1->SMPR2 |= ADC_SMPR2_SMP4_0;
			break;
		}
		case SAMPLING_PERIOD_28_CYCLES: {
			ADC1->SMPR2 |= ADC_SMPR2_SMP4_1;
			break;
		}
		case SAMPLING_PERIOD_56_CYCLES: {
			ADC1->SMPR2 |= (ADC_SMPR2_SMP4_0 | ADC_SMPR2_SMP4_1);
			break;
		}
		case SAMPLING_PERIOD_84_CYCLES: {
			ADC1->SMPR2 |= ADC_SMPR2_SMP4_2;
			break;
		}
		case SAMPLING_PERIOD_112_CYCLES: {
			ADC1->SMPR2 |= (ADC_SMPR2_SMP4_2 | ADC_SMPR2_SMP4_0);
			break;
		}
		case SAMPLING_PERIOD_144_CYCLES: {
			ADC1->SMPR2 |= (ADC_SMPR2_SMP4_2 | ADC_SMPR2_SMP4_1);
			break;
		}
		case SAMPLING_PERIOD_480_CYCLES: {
			ADC1->SMPR2 |= ADC_SMPR2_SMP4;
			break;
		}
		default: {
			__NOP();
			break;
		}
		}
		break;
	}
	case CHANNEL_5: {
		switch (adcConfig->samplingPeriod) {
		case SAMPLING_PERIOD_3_CYCLES: {
			ADC1->SMPR2 &= ~ADC_SMPR2_SMP5;
			break;
		}
		case SAMPLING_PERIOD_15_CYCLES: {
			ADC1->SMPR2 |= ADC_SMPR2_SMP5_0;
			break;
		}
		case SAMPLING_PERIOD_28_CYCLES: {
			ADC1->SMPR2 |= ADC_SMPR2_SMP5_1;
			break;
		}
		case SAMPLING_PERIOD_56_CYCLES: {
			ADC1->SMPR2 |= (ADC_SMPR2_SMP5_0 | ADC_SMPR2_SMP5_1);
			break;
		}
		case SAMPLING_PERIOD_84_CYCLES: {
			ADC1->SMPR2 |= ADC_SMPR2_SMP5_2;
			break;
		}
		case SAMPLING_PERIOD_112_CYCLES: {
			ADC1->SMPR2 |= (ADC_SMPR2_SMP5_2 | ADC_SMPR2_SMP5_0);
			break;
		}
		case SAMPLING_PERIOD_144_CYCLES: {
			ADC1->SMPR2 |= (ADC_SMPR2_SMP5_2 | ADC_SMPR2_SMP5_1);
			break;
		}
		case SAMPLING_PERIOD_480_CYCLES: {
			ADC1->SMPR2 |= ADC_SMPR2_SMP5;
			break;
		}
		default: {
			__NOP();
			break;
		}
		}
		break;
	}
	case CHANNEL_6: {
		switch (adcConfig->samplingPeriod) {
		case SAMPLING_PERIOD_3_CYCLES: {
			ADC1->SMPR2 &= ~ADC_SMPR2_SMP6;
			break;
		}
		case SAMPLING_PERIOD_15_CYCLES: {
			ADC1->SMPR2 |= ADC_SMPR2_SMP6_0;
			break;
		}
		case SAMPLING_PERIOD_28_CYCLES: {
			ADC1->SMPR2 |= ADC_SMPR2_SMP6_1;
			break;
		}
		case SAMPLING_PERIOD_56_CYCLES: {
			ADC1->SMPR2 |= (ADC_SMPR2_SMP6_0 | ADC_SMPR2_SMP6_1);
			break;
		}
		case SAMPLING_PERIOD_84_CYCLES: {
			ADC1->SMPR2 |= ADC_SMPR2_SMP6_2;
			break;
		}
		case SAMPLING_PERIOD_112_CYCLES: {
			ADC1->SMPR2 |= (ADC_SMPR2_SMP6_2 | ADC_SMPR2_SMP6_0);
			break;
		}
		case SAMPLING_PERIOD_144_CYCLES: {
			ADC1->SMPR2 |= (ADC_SMPR2_SMP6_2 | ADC_SMPR2_SMP6_1);
			break;
		}
		case SAMPLING_PERIOD_480_CYCLES: {
			ADC1->SMPR2 |= ADC_SMPR2_SMP6;
			break;
		}
		default: {
			__NOP();
			break;
		}
		}
		break;
	}
	case CHANNEL_7: {
		switch (adcConfig->samplingPeriod) {
		case SAMPLING_PERIOD_3_CYCLES: {
			ADC1->SMPR2 &= ~ADC_SMPR2_SMP7;
			break;
		}
		case SAMPLING_PERIOD_15_CYCLES: {
			ADC1->SMPR2 |= ADC_SMPR2_SMP7_0;
			break;
		}
		case SAMPLING_PERIOD_28_CYCLES: {
			ADC1->SMPR2 |= ADC_SMPR2_SMP7_1;
			break;
		}
		case SAMPLING_PERIOD_56_CYCLES: {
			ADC1->SMPR2 |= (ADC_SMPR2_SMP7_0 | ADC_SMPR2_SMP7_1);
			break;
		}
		case SAMPLING_PERIOD_84_CYCLES: {
			ADC1->SMPR2 |= ADC_SMPR2_SMP7_2;
			break;
		}
		case SAMPLING_PERIOD_112_CYCLES: {
			ADC1->SMPR2 |= (ADC_SMPR2_SMP7_2 | ADC_SMPR2_SMP7_0);
			break;
		}
		case SAMPLING_PERIOD_144_CYCLES: {
			ADC1->SMPR2 |= (ADC_SMPR2_SMP7_2 | ADC_SMPR2_SMP7_1);
			break;
		}
		case SAMPLING_PERIOD_480_CYCLES: {
			ADC1->SMPR2 |= ADC_SMPR2_SMP7;
			break;
		}
		default: {
			__NOP();
			break;
		}
		}
		break;
	}
	case CHANNEL_8: {
		switch (adcConfig->samplingPeriod) {
		case SAMPLING_PERIOD_3_CYCLES: {
			ADC1->SMPR2 &= ~ADC_SMPR2_SMP8;
			break;
		}
		case SAMPLING_PERIOD_15_CYCLES: {
			ADC1->SMPR2 |= ADC_SMPR2_SMP8_0;
			break;
		}
		case SAMPLING_PERIOD_28_CYCLES: {
			ADC1->SMPR2 |= ADC_SMPR2_SMP8_1;
			break;
		}
		case SAMPLING_PERIOD_56_CYCLES: {
			ADC1->SMPR2 |= (ADC_SMPR2_SMP8_0 | ADC_SMPR2_SMP8_1);
			break;
		}
		case SAMPLING_PERIOD_84_CYCLES: {
			ADC1->SMPR2 |= ADC_SMPR2_SMP8_2;
			break;
		}
		case SAMPLING_PERIOD_112_CYCLES: {
			ADC1->SMPR2 |= (ADC_SMPR2_SMP8_2 | ADC_SMPR2_SMP8_0);
			break;
		}
		case SAMPLING_PERIOD_144_CYCLES: {
			ADC1->SMPR2 |= (ADC_SMPR2_SMP8_2 | ADC_SMPR2_SMP8_1);
			break;
		}
		case SAMPLING_PERIOD_480_CYCLES: {
			ADC1->SMPR2 |= ADC_SMPR2_SMP8;
			break;
		}
		default: {
			__NOP();
			break;
		}
		}
		break;
	}
	case CHANNEL_9: {
		switch (adcConfig->samplingPeriod) {
		case SAMPLING_PERIOD_3_CYCLES: {
			ADC1->SMPR2 &= ~ADC_SMPR2_SMP9;
			break;
		}
		case SAMPLING_PERIOD_15_CYCLES: {
			ADC1->SMPR2 |= ADC_SMPR2_SMP9_0;
			break;
		}
		case SAMPLING_PERIOD_28_CYCLES: {
			ADC1->SMPR2 |= ADC_SMPR2_SMP9_1;
			break;
		}
		case SAMPLING_PERIOD_56_CYCLES: {
			ADC1->SMPR2 |= (ADC_SMPR2_SMP9_0 | ADC_SMPR2_SMP9_1);
			break;
		}
		case SAMPLING_PERIOD_84_CYCLES: {
			ADC1->SMPR2 |= ADC_SMPR2_SMP9_2;
			break;
		}
		case SAMPLING_PERIOD_112_CYCLES: {
			ADC1->SMPR2 |= (ADC_SMPR2_SMP9_2 | ADC_SMPR2_SMP9_0);
			break;
		}
		case SAMPLING_PERIOD_144_CYCLES: {
			ADC1->SMPR2 |= (ADC_SMPR2_SMP9_2 | ADC_SMPR2_SMP9_1);
			break;
		}
		case SAMPLING_PERIOD_480_CYCLES: {
			ADC1->SMPR2 |= ADC_SMPR2_SMP9;
			break;
		}
		default: {
			__NOP();
			break;
		}
		}
		break;
	}
	case CHANNEL_10: {
		switch (adcConfig->samplingPeriod) {
		case SAMPLING_PERIOD_3_CYCLES: {
			ADC1->SMPR1 &= ~ADC_SMPR1_SMP10;
			break;
		}
		case SAMPLING_PERIOD_15_CYCLES: {
			ADC1->SMPR1 |= ADC_SMPR1_SMP10_0;
			break;
		}
		case SAMPLING_PERIOD_28_CYCLES: {
			ADC1->SMPR1 |= ADC_SMPR1_SMP10_1;
			break;
		}
		case SAMPLING_PERIOD_56_CYCLES: {
			ADC1->SMPR1 |= (ADC_SMPR1_SMP10_0 | ADC_SMPR1_SMP10_1);
			break;
		}
		case SAMPLING_PERIOD_84_CYCLES: {
			ADC1->SMPR1 |= ADC_SMPR1_SMP10_2;
			break;
		}
		case SAMPLING_PERIOD_112_CYCLES: {
			ADC1->SMPR1 |= (ADC_SMPR1_SMP10_2 | ADC_SMPR1_SMP10_0);
			break;
		}
		case SAMPLING_PERIOD_144_CYCLES: {
			ADC1->SMPR1 |= (ADC_SMPR1_SMP10_2 | ADC_SMPR1_SMP10_1);
			break;
		}
		case SAMPLING_PERIOD_480_CYCLES: {
			ADC1->SMPR1 |= ADC_SMPR1_SMP10;
			break;
		}
		default: {
			__NOP();
			break;
		}
		}
		break;
	}
	case CHANNEL_11: {
		switch (adcConfig->samplingPeriod) {
		case SAMPLING_PERIOD_3_CYCLES: {
			ADC1->SMPR1 &= ~ADC_SMPR1_SMP11;
			break;
		}
		case SAMPLING_PERIOD_15_CYCLES: {
			ADC1->SMPR1 |= ADC_SMPR1_SMP11_0;
			break;
		}
		case SAMPLING_PERIOD_28_CYCLES: {
			ADC1->SMPR1 |= ADC_SMPR1_SMP11_1;
			break;
		}
		case SAMPLING_PERIOD_56_CYCLES: {
			ADC1->SMPR1 |= (ADC_SMPR1_SMP11_0 | ADC_SMPR1_SMP11_1);
			break;
		}
		case SAMPLING_PERIOD_84_CYCLES: {
			ADC1->SMPR1 |= ADC_SMPR1_SMP11_2;
			break;
		}
		case SAMPLING_PERIOD_112_CYCLES: {
			ADC1->SMPR1 |= (ADC_SMPR1_SMP11_2 | ADC_SMPR1_SMP11_0);
			break;
		}
		case SAMPLING_PERIOD_144_CYCLES: {
			ADC1->SMPR1 |= (ADC_SMPR1_SMP11_2 | ADC_SMPR1_SMP11_1);
			break;
		}
		case SAMPLING_PERIOD_480_CYCLES: {
			ADC1->SMPR1 |= ADC_SMPR1_SMP11;
			break;
		}
		default: {
			__NOP();
			break;
		}
		}
		break;
	}
	case CHANNEL_12: {
		switch (adcConfig->samplingPeriod) {
		case SAMPLING_PERIOD_3_CYCLES: {
			ADC1->SMPR1 &= ~ADC_SMPR1_SMP12;
			break;
		}
		case SAMPLING_PERIOD_15_CYCLES: {
			ADC1->SMPR1 |= ADC_SMPR1_SMP12_0;
			break;
		}
		case SAMPLING_PERIOD_28_CYCLES: {
			ADC1->SMPR1 |= ADC_SMPR1_SMP12_1;
			break;
		}
		case SAMPLING_PERIOD_56_CYCLES: {
			ADC1->SMPR1 |= (ADC_SMPR1_SMP12_0 | ADC_SMPR1_SMP12_1);
			break;
		}
		case SAMPLING_PERIOD_84_CYCLES: {
			ADC1->SMPR1 |= ADC_SMPR1_SMP12_2;
			break;
		}
		case SAMPLING_PERIOD_112_CYCLES: {
			ADC1->SMPR1 |= (ADC_SMPR1_SMP12_2 | ADC_SMPR1_SMP12_0);
			break;
		}
		case SAMPLING_PERIOD_144_CYCLES: {
			ADC1->SMPR1 |= (ADC_SMPR1_SMP12_2 | ADC_SMPR1_SMP12_1);
			break;
		}
		case SAMPLING_PERIOD_480_CYCLES: {
			ADC1->SMPR1 |= ADC_SMPR1_SMP12;
			break;
		}
		default: {
			__NOP();
			break;
		}
		}
		break;
	}
	case CHANNEL_13: {
		switch (adcConfig->samplingPeriod) {
		case SAMPLING_PERIOD_3_CYCLES: {
			ADC1->SMPR1 &= ~ADC_SMPR1_SMP13;
			break;
		}
		case SAMPLING_PERIOD_15_CYCLES: {
			ADC1->SMPR1 |= ADC_SMPR1_SMP13_0;
			break;
		}
		case SAMPLING_PERIOD_28_CYCLES: {
			ADC1->SMPR1 |= ADC_SMPR1_SMP13_1;
			break;
		}
		case SAMPLING_PERIOD_56_CYCLES: {
			ADC1->SMPR1 |= (ADC_SMPR1_SMP13_0 | ADC_SMPR1_SMP13_1);
			break;
		}
		case SAMPLING_PERIOD_84_CYCLES: {
			ADC1->SMPR1 |= ADC_SMPR1_SMP13_2;
			break;
		}
		case SAMPLING_PERIOD_112_CYCLES: {
			ADC1->SMPR1 |= (ADC_SMPR1_SMP13_2 | ADC_SMPR1_SMP13_0);
			break;
		}
		case SAMPLING_PERIOD_144_CYCLES: {
			ADC1->SMPR1 |= (ADC_SMPR1_SMP13_2 | ADC_SMPR1_SMP13_1);
			break;
		}
		case SAMPLING_PERIOD_480_CYCLES: {
			ADC1->SMPR1 |= ADC_SMPR1_SMP13;
			break;
		}
		default: {
			__NOP();
			break;
		}
		}
		break;
	}
	case CHANNEL_14: {
		switch (adcConfig->samplingPeriod) {
		case SAMPLING_PERIOD_3_CYCLES: {
			ADC1->SMPR1 &= ~ADC_SMPR1_SMP14;
			break;
		}
		case SAMPLING_PERIOD_15_CYCLES: {
			ADC1->SMPR1 |= ADC_SMPR1_SMP14_0;
			break;
		}
		case SAMPLING_PERIOD_28_CYCLES: {
			ADC1->SMPR1 |= ADC_SMPR1_SMP14_1;
			break;
		}
		case SAMPLING_PERIOD_56_CYCLES: {
			ADC1->SMPR1 |= (ADC_SMPR1_SMP14_0 | ADC_SMPR1_SMP14_1);
			break;
		}
		case SAMPLING_PERIOD_84_CYCLES: {
			ADC1->SMPR1 |= ADC_SMPR1_SMP14_2;
			break;
		}
		case SAMPLING_PERIOD_112_CYCLES: {
			ADC1->SMPR1 |= (ADC_SMPR1_SMP14_2 | ADC_SMPR1_SMP14_0);
			break;
		}
		case SAMPLING_PERIOD_144_CYCLES: {
			ADC1->SMPR1 |= (ADC_SMPR1_SMP14_2 | ADC_SMPR1_SMP14_1);
			break;
		}
		case SAMPLING_PERIOD_480_CYCLES: {
			ADC1->SMPR1 |= ADC_SMPR1_SMP14;
			break;
		}
		default: {
			__NOP();
			break;
		}
		}
		break;
	}
	case CHANNEL_15: {
		switch (adcConfig->samplingPeriod) {
		case SAMPLING_PERIOD_3_CYCLES: {
			ADC1->SMPR1 &= ~ADC_SMPR1_SMP15;
			break;
		}
		case SAMPLING_PERIOD_15_CYCLES: {
			ADC1->SMPR1 |= ADC_SMPR1_SMP15_0;
			break;
		}
		case SAMPLING_PERIOD_28_CYCLES: {
			ADC1->SMPR1 |= ADC_SMPR1_SMP15_1;
			break;
		}
		case SAMPLING_PERIOD_56_CYCLES: {
			ADC1->SMPR1 |= (ADC_SMPR1_SMP15_0 | ADC_SMPR1_SMP15_1);
			break;
		}
		case SAMPLING_PERIOD_84_CYCLES: {
			ADC1->SMPR1 |= ADC_SMPR1_SMP15_2;
			break;
		}
		case SAMPLING_PERIOD_112_CYCLES: {
			ADC1->SMPR1 |= (ADC_SMPR1_SMP15_2 | ADC_SMPR1_SMP15_0);
			break;
		}
		case SAMPLING_PERIOD_144_CYCLES: {
			ADC1->SMPR1 |= (ADC_SMPR1_SMP15_2 | ADC_SMPR1_SMP15_1);
			break;
		}
		case SAMPLING_PERIOD_480_CYCLES: {
			ADC1->SMPR1 |= ADC_SMPR1_SMP15;
			break;
		}
		default: {
			__NOP();
			break;
		}
		}
		break;
	}
	default: {
		__NOP();
		break;
	}
	}
}

/*
 * Configura el numero de elementos en la secuencia (solo un elemento)
 * Configura cual es el canal que adquiere la señal ADC
 */
static void adc_set_one_channel_sequence(ADC_Config_t *adcConfig) {
	/* 8. Configuramos la secuencia y cuantos elementos hay en las secuencias */

	/* Como solo será un elemento configuraremos el Length a un solo elemnto*/
	ADC1->SQR1 &= ~ADC_SQR1_L;

	/* Limpiamos el resgistro primero*/
	ADC1->SQR3 &= ~(ADC_SQR3_SQ1);

	/* Ahora configuraremos la cada canal de acuerdo a la elección
	 * de un solo elemento en la secuencia
	 */

	switch (adcConfig->channel) {
	case CHANNEL_0: {
		ADC1->SQR3 &= ~ADC_SQR3_SQ1;
		break;
	}
	case CHANNEL_1: {
		ADC1->SQR3 |= ADC_SQR3_SQ1_0;
		break;
	}
	case CHANNEL_2: {
		ADC1->SQR3 |= ADC_SQR3_SQ1_1;
		break;
	}
	case CHANNEL_3: {
		ADC1->SQR3 |= 3 << ADC_SQR3_SQ1_Pos;
		break;
	}
	case CHANNEL_4: {
		ADC1->SQR3 |= ADC_SQR3_SQ1_2;
		break;
	}
	case CHANNEL_5: {
		ADC1->SQR3 |= 5 << ADC_SQR3_SQ1_Pos;
		break;
	}
	case CHANNEL_6: {
		ADC1->SQR3 |= 6 << ADC_SQR3_SQ1_Pos;
		break;
	}
	case CHANNEL_7: {
		ADC1->SQR3 |= 7 << ADC_SQR3_SQ1_Pos;
		break;
	}
	case CHANNEL_8: {
		ADC1->SQR3 |= ADC_SQR3_SQ1_3;
		break;
	}
	case CHANNEL_9: {
		ADC1->SQR3 |= 9 << ADC_SQR3_SQ1_Pos;
		break;
	}
	case CHANNEL_10: {
		ADC1->SQR3 |= 10 << ADC_SQR3_SQ1_Pos;
		break;
	}
	case CHANNEL_11: {
		ADC1->SQR3 |= 11 << ADC_SQR3_SQ1_Pos;
		break;
	}
	case CHANNEL_12: {
		ADC1->SQR3 |= 12 << ADC_SQR3_SQ1_Pos;
		break;
	}
	case CHANNEL_13: {
		ADC1->SQR3 |= 13 << ADC_SQR3_SQ1_Pos;
		break;
	}
	case CHANNEL_14: {
		ADC1->SQR3 |= 14 << ADC_SQR3_SQ1_Pos;
		break;
	}
	case CHANNEL_15: {
		ADC1->SQR3 |= 15 << ADC_SQR3_SQ1_Pos;
		break;
	}
	default: {
		__NOP();
		break;
	}
	}
}

static void adc_set_multi_channel_sequence(ADC_Config_t adcConfig[16],
		uint8_t numerocanales) {
	/* 8. Configuramos la secuencia y cuantos elementos hay en las secuencias */

	/* Primero limpiamos el registro*/
	ADC1->SQR1 &= ~ADC_SQR1_L;
	/* Como solo será un elemento configuraremos el Length a un solo elemnto*/
	ADC1->SQR1 |= ((numerocanales - 1) << ADC_SQR1_L_Pos);

	/* Limpiamos el resgistro primero*/
	ADC1->SQR3 &= ~(ADC_SQR3_SQ1);
	/* Limpiamos el resgistro primero*/
	ADC1->SQR2 &= ~(ADC_SQR2_SQ7);
	/* Limpiamos el resgistro primero*/
	ADC1->SQR1 &= ~(ADC_SQR1_SQ13);

	/* Ahora configuraremos la cada canal de acuerdo a la elección
	 * de un solo elemento en la secuencia
	 */
	for (uint8_t i = 0; i < (numerocanales); i++) {
		if (i < 6) {
			ADC1->SQR3 |= ((adcConfig[i].channel) << (5 * i));
		} else if (i < 12) {
			ADC1->SQR2 |= ((adcConfig[i].channel) << (5 * (i - 6)));
		} else {
			ADC1->SQR1 |= ((adcConfig[i].channel) << (5 * (i - 12)));
		}
	}
}

/*
 * Configura el enable de la interrupcion y la activacion del NVIC
 */
static void adc_config_interrupt(ADC_Config_t *adcConfig) {
	/* 11. Configuramos la interrupción (si se encuentra activa),
	 *  ademas de inscribir/remover la interrupción en el NVIC */
	if (adcConfig->interrupState == ADC_INT_ENABLE) {
		// Como está activada, debemos configurar la interrupción
		//Primero debemos colocarlaen enable
		ADC1->CR1 |= ADC_CR1_EOCIE;
		ADC1->CR2 |= ADC_CR2_EOCS;

		/* Debemos matricular la interrupción en el NVIC */
		__NVIC_EnableIRQ(ADC_IRQn);
		__NVIC_SetPriority(ADC_IRQn, 4);

	} else {
		// Como está desactivada, debemos deshabilitar la interrupción
		ADC1->CR1 &= ~ADC_CR1_EOCIE;
		/* Debemos desmatricular la interrupción en el NVIC */
		__NVIC_DisableIRQ(ADC_IRQn);
		__NVIC_SetPriority(ADC_IRQn, 4);
	}
}

/*
 * Controla la activación y desactivación del modulo ADC desde el registro
 * CR2 del adc.
 */
void adc_peripheralOnOff(uint8_t state) {

	if (state) {
		ADC1->CR2 |= ADC_CR2_ADON;
	} else {
		ADC1->CR2 &= ~ADC_CR2_ADON;
	}
}

/* Enables and disables the Scan mode...
 * Funciona de la mano con la secuencia de varios canales.
 * No es necesario para el caso de solo un canal simple.
 */
void adc_ScanMode(uint8_t state) {
	/* 4. Configuramos el modo Scan*/
	if (state) {
		ADC1->CR1 |= ADC_CR1_SCAN;
	} else {
		ADC1->CR1 &= ~ADC_CR1_SCAN;
	}
}

/*
 * Función que comienza la conversión ADC continua
 */
void adc_StartSingleConv(void) {
	ADC1->CR2 |= ADC_CR2_SWSTART;


	/*
	 * Si overrun se lanza es porque leyó el dato antes de terminar
	 * por lo que decidimos limpiar la bandera y lanzar la interrupción
	 * que dice que ya se leyó el dato y pasa al siguiente dato.
	 */
	if (ADC1->SR & ADC_SR_OVR){
		ADC1->SR &= ~(ADC_SR_OVR);
		ADC1->SR &= ~ADC_SR_EOC;
		ADC1->SR |= ADC_SR_EOC;
	}
}

/*
 * Función que comienza la converción ADC continua
 */
void adc_StartContinouosConv(void) {

	ADC1->CR2 |= ADC_CR2_SWSTART;
	ADC1->CR2 |= ADC_CR2_CONT;

}

/*
 * Función que comienza laconversión ADC continua
 */
void adc_StopContinouosConv(void) {

	ADC1->CR2 &= ~ADC_CR2_CONT;

}

/*
 * Función que retorna el ultimo dato adquirido por el ADC
 */
uint16_t adc_GetValue(void) {
	return adcRawData;
}

/*
 * Esta es la ISR de la interrupción por conversión ADC
 */
void ADC_IRQHandler(void) {
	//Revisamos que se dio la conversion
	while(!(ADC1->CR2 & (ADC_CR2_EOCS))){
		__NOP();
	}

	if(ADC1->SR & ADC_SR_EOC){
		// Leemos el resultado de la conversión ADC y lo cargamos en una variable auxiliar
		adcRawData = ADC1->DR;
		// Hacemos el llamado a la función que se ejecutará en el main
		adc_CompleteCallback();
	}
}


/* Con esta función configuramos que pin deseamos que funcione como ADC */
void adc_ConfigAnalogPin(uint8_t adcChannel) {

	switch (adcChannel) {
	case CHANNEL_0: {
		// Cofiguramos los registro
		handlerADCPin.pGPIOx = GPIOA;
		handlerADCPin.pinConfig.GPIO_PinNumber = PIN_0;
		handlerADCPin.pinConfig.GPIO_PinMode = GPIO_MODE_ANALOG;
		handlerADCPin.pinConfig.GPIO_PinPuPdControl = GPIO_PUPDR_NOTHING;
		gpio_Config(&handlerADCPin);
		break;
	}
	case CHANNEL_1: {
		// Cofiguramos los registro
		handlerADCPin.pGPIOx = GPIOA;
		handlerADCPin.pinConfig.GPIO_PinNumber = PIN_1;
		handlerADCPin.pinConfig.GPIO_PinMode = GPIO_MODE_ANALOG;
		handlerADCPin.pinConfig.GPIO_PinPuPdControl = GPIO_PUPDR_NOTHING;
		gpio_Config(&handlerADCPin);
		break;
	}
	case CHANNEL_2: {
		// Cofiguramos los registro
		handlerADCPin.pGPIOx = GPIOA;
		handlerADCPin.pinConfig.GPIO_PinNumber = PIN_2;
		handlerADCPin.pinConfig.GPIO_PinMode = GPIO_MODE_ANALOG;
		handlerADCPin.pinConfig.GPIO_PinPuPdControl = GPIO_PUPDR_NOTHING;
		gpio_Config(&handlerADCPin);
		break;
	}
	case CHANNEL_3: {
		// Cofiguramos los registro
		handlerADCPin.pGPIOx = GPIOA;
		handlerADCPin.pinConfig.GPIO_PinNumber = PIN_3;
		handlerADCPin.pinConfig.GPIO_PinMode = GPIO_MODE_ANALOG;
		handlerADCPin.pinConfig.GPIO_PinPuPdControl = GPIO_PUPDR_NOTHING;
		gpio_Config(&handlerADCPin);
		break;
	}
	case CHANNEL_4: {
		// Cofiguramos los registro
		handlerADCPin.pGPIOx = GPIOA;
		handlerADCPin.pinConfig.GPIO_PinNumber = PIN_4;
		handlerADCPin.pinConfig.GPIO_PinMode = GPIO_MODE_ANALOG;
		handlerADCPin.pinConfig.GPIO_PinPuPdControl = GPIO_PUPDR_NOTHING;
		gpio_Config(&handlerADCPin);
		break;
	}
	case CHANNEL_5: {
		// Cofiguramos los registro
		handlerADCPin.pGPIOx = GPIOA;
		handlerADCPin.pinConfig.GPIO_PinNumber = PIN_5;
		handlerADCPin.pinConfig.GPIO_PinMode = GPIO_MODE_ANALOG;
		handlerADCPin.pinConfig.GPIO_PinPuPdControl = GPIO_PUPDR_NOTHING;
		gpio_Config(&handlerADCPin);
		break;
	}
	case CHANNEL_6: {
		// Cofiguramos los registro
		handlerADCPin.pGPIOx = GPIOA;
		handlerADCPin.pinConfig.GPIO_PinNumber = PIN_6;
		handlerADCPin.pinConfig.GPIO_PinMode = GPIO_MODE_ANALOG;
		handlerADCPin.pinConfig.GPIO_PinPuPdControl = GPIO_PUPDR_NOTHING;
		gpio_Config(&handlerADCPin);
		break;
	}
	case CHANNEL_7: {
		// Cofiguramos los registro
		handlerADCPin.pGPIOx = GPIOA;
		handlerADCPin.pinConfig.GPIO_PinNumber = PIN_7;
		handlerADCPin.pinConfig.GPIO_PinMode = GPIO_MODE_ANALOG;
		handlerADCPin.pinConfig.GPIO_PinPuPdControl = GPIO_PUPDR_NOTHING;
		gpio_Config(&handlerADCPin);
		break;
	}
	case CHANNEL_8: {
		// Cofiguramos los registro
		handlerADCPin.pGPIOx = GPIOB;
		handlerADCPin.pinConfig.GPIO_PinNumber = PIN_0;
		handlerADCPin.pinConfig.GPIO_PinMode = GPIO_MODE_ANALOG;
		handlerADCPin.pinConfig.GPIO_PinPuPdControl = GPIO_PUPDR_NOTHING;
		gpio_Config(&handlerADCPin);
		break;
	}
	case CHANNEL_9: {
		// Cofiguramos los registro
		handlerADCPin.pGPIOx = GPIOB;
		handlerADCPin.pinConfig.GPIO_PinNumber = PIN_1;
		handlerADCPin.pinConfig.GPIO_PinMode = GPIO_MODE_ANALOG;
		handlerADCPin.pinConfig.GPIO_PinPuPdControl = GPIO_PUPDR_NOTHING;
		gpio_Config(&handlerADCPin);
		break;
	}
	case CHANNEL_10: {
		// Cofiguramos los registro
		handlerADCPin.pGPIOx = GPIOC;
		handlerADCPin.pinConfig.GPIO_PinNumber = PIN_0;
		handlerADCPin.pinConfig.GPIO_PinMode = GPIO_MODE_ANALOG;
		handlerADCPin.pinConfig.GPIO_PinPuPdControl = GPIO_PUPDR_NOTHING;
		gpio_Config(&handlerADCPin);
		break;
	}
	case CHANNEL_11: {
		// Cofiguramos los registro
		handlerADCPin.pGPIOx = GPIOC;
		handlerADCPin.pinConfig.GPIO_PinNumber = PIN_1;
		handlerADCPin.pinConfig.GPIO_PinMode = GPIO_MODE_ANALOG;
		handlerADCPin.pinConfig.GPIO_PinPuPdControl = GPIO_PUPDR_NOTHING;
		gpio_Config(&handlerADCPin);
		break;
	}
	case CHANNEL_12: {
		// Cofiguramos los registro
		handlerADCPin.pGPIOx = GPIOC;
		handlerADCPin.pinConfig.GPIO_PinNumber = PIN_2;
		handlerADCPin.pinConfig.GPIO_PinMode = GPIO_MODE_ANALOG;
		handlerADCPin.pinConfig.GPIO_PinPuPdControl = GPIO_PUPDR_NOTHING;
		gpio_Config(&handlerADCPin);
		break;
	}
	case CHANNEL_13: {
		// Cofiguramos los registro
		handlerADCPin.pGPIOx = GPIOC;
		handlerADCPin.pinConfig.GPIO_PinNumber = PIN_3;
		handlerADCPin.pinConfig.GPIO_PinMode = GPIO_MODE_ANALOG;
		handlerADCPin.pinConfig.GPIO_PinPuPdControl = GPIO_PUPDR_NOTHING;
		gpio_Config(&handlerADCPin);
		break;
	}
	case CHANNEL_14: {
		// Cofiguramos los registro
		handlerADCPin.pGPIOx = GPIOC;
		handlerADCPin.pinConfig.GPIO_PinNumber = PIN_4;
		handlerADCPin.pinConfig.GPIO_PinMode = GPIO_MODE_ANALOG;
		handlerADCPin.pinConfig.GPIO_PinPuPdControl = GPIO_PUPDR_NOTHING;
		gpio_Config(&handlerADCPin);
		break;
	}
	case CHANNEL_15: {
		// Cofiguramos los registro
		handlerADCPin.pGPIOx = GPIOC;
		handlerADCPin.pinConfig.GPIO_PinNumber = PIN_5;
		handlerADCPin.pinConfig.GPIO_PinMode = GPIO_MODE_ANALOG;
		handlerADCPin.pinConfig.GPIO_PinPuPdControl = GPIO_PUPDR_NOTHING;
		gpio_Config(&handlerADCPin);
		break;
	}
	}

}



__attribute__((weak)) void adc_CompleteCallback(void) {
	__NOP();
}
