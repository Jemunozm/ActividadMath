/*
 * gpio_driver_hal.c
 *
 *  Created on: Sep 7, 2023
 *      Author: imjeviz
 */

#include "gpio_driver_hal.h"
#include "stm32f4xx.h"
#include "stm32_assert.h"

/* === Headers for private functions === */
static void gpio_enable_clock_peripheral(GPIO_Handler_t *pGPIOHandler);
static void gpio_config_mode(GPIO_Handler_t *pGPIOHandler);
static void gpio_config_output_type(GPIO_Handler_t *pGPIOHandler);
static void gpio_config_output_speed(GPIO_Handler_t *pGPIOHandler);
static void gpio_config_pullup_pulldown(GPIO_Handler_t *pGPIOHandler);
static void gpio_config_alternate_function(GPIO_Handler_t *pGPIOHandler);


/**
 * Para cualquier periférico, hay varios pasos que siempre se deben seguir en un
 * orden estricto para poder que el sistema permita configurar el periférico X.
 * Lo primero y mas importante es activar la señal del reloj principal hacia ese
 * elemento especifico (relacionado con el periférico RCC), a esto llamaremos
 * siplemente "activar el periférico o activar la señal del periférico)
 */
void gpio_Config (GPIO_Handler_t *pGPIOHandler){

	/* Verificamos que el pin seleccionado es correcto.	*/
	assert_param(IS_GPIO_PIN(pGPIOHandler->pinConfig.GPIO_PinNumber));

	// 1) Activar el periférico
	gpio_enable_clock_peripheral(pGPIOHandler);

	// Después de activado, podemos comenzar a configurar.

	// 2) Configurando el registro GPIOx_MODER
	gpio_config_mode(pGPIOHandler);

	// 3) Configurando el registro GPIOx_OTYPER
	gpio_config_output_type(pGPIOHandler);

	// 4) Configurando ahora la velocidad
	gpio_config_output_speed(pGPIOHandler);

	// 5) Configurando si se desea pull-up, pull-down o flotante.
	gpio_config_pullup_pulldown(pGPIOHandler);

	// 6) Configuración de las funciones alternativas... se verá luego, mas adelante en el curso
	gpio_config_alternate_function(pGPIOHandler);

} // Fin del GPIO_config

/*
 * Enable Clock signal for specific GPIOx port
 */
void gpio_enable_clock_peripheral(GPIO_Handler_t *pGPIOHandler){

	// Verificamos que el puerto configurado si es permitido
	assert_param(IS_GPIO_ALL_INSTANCE(pGPIOHandler->pGPIOx));

	// Verificamos para GPIOA

		if(pGPIOHandler->pGPIOx == GPIOA){
			// Escribimos 1 (SET) en la posición correspondiente al GPIOA
			RCC->AHB1ENR |= (RCC_AHB1ENR_GPIOAEN);
		}
		// Verificamos para GPIOB
		else if(pGPIOHandler->pGPIOx == GPIOB){
			// Escribimos 1 (SET) en la posición correspondiente al GPIOB
			RCC->AHB1ENR |= (RCC_AHB1ENR_GPIOBEN);
		}
		// Verificamos para GPIOC
		else if(pGPIOHandler->pGPIOx == GPIOC){
			// Escribimos 1 (SET) en la posición correspondiente al GPIOC
			RCC->AHB1ENR |= (RCC_AHB1ENR_GPIOCEN);
		}
		// Verificamos para GPIOD
		else if(pGPIOHandler->pGPIOx == GPIOD){
			// Escribimos 1 (SET) en la posición correspondiente al GPIOD
			RCC->AHB1ENR |= (RCC_AHB1ENR_GPIODEN);
		}
		// Verificamos para GPIOE
		else if(pGPIOHandler->pGPIOx == GPIOE){
			// Escribimos 1 (SET) en la posición correspondiente al GPIOE
			RCC->AHB1ENR |= (RCC_AHB1ENR_GPIOEEN);
		}
		// Verificamos para GPIOH
		else if(pGPIOHandler->pGPIOx == GPIOH){
			// Escribimos 1 (SET) en la posición correspondiente al GPIOH
			RCC->AHB1ENR |= (RCC_AHB1ENR_GPIOHEN);
		}
}

/*
 *  Configures the mode in which the pin will work:
 *  -	Input
 *  -	Output
 *  -	Analog
 *  -	Alternate Function
 */
void gpio_config_mode(GPIO_Handler_t *pGPIOHandler){

	uint32_t auxConfig = 0;

	// Verificamos si el modo que se ha seleccionado es permitido
	assert_param(IS_GPIO_MODE(pGPIOHandler->pinConfig.GPIO_PinMode));

	// Acá estmos leyendo la config, moviendo "PinNumber" veces hacia la izquierda ese valor (shift left)
	// y to-do eso lo cargamos en la variable auxConfig
	auxConfig = (pGPIOHandler->pinConfig.GPIO_PinMode << 2 * pGPIOHandler->pinConfig.GPIO_PinNumber);

	// Antes de cargar el nuevo valor,limpiamos los bits específicos de ese registro (debemos escribir 0b00)
	// para lo cual aplicamos una máscara y una operación bitwise AND
	pGPIOHandler->pGPIOx->MODER &= ~(0b11 << 2 * pGPIOHandler->pinConfig.GPIO_PinNumber);

	// Cargamos a auxConfig en el registro MODER
	pGPIOHandler->pGPIOx->MODER |= auxConfig;
}

/*
 *  Configures which type of output the PinX will use:
 *  -	Push-Pull
 *  -	OpenDrain
 */
void gpio_config_output_type(GPIO_Handler_t *pGPIOHandler){

	uint32_t auxConfig = 0;

	/* Verificamos que el tipo de salida corresponda a los que se pueden utilizar */
	assert_param(IS_GPIO_OUTPUT_TYPE(pGPIOHandler->pinConfig.GPIO_PinOutputType));

	// De nuevo, leemos y movemos el valor numero "PinNumber" de veces
	auxConfig = (pGPIOHandler->pinConfig.GPIO_PinOutputType << pGPIOHandler->pinConfig.GPIO_PinNumber);

	// Limpiamos antes de cargar
	pGPIOHandler->pGPIOx->OTYPER &= ~(SET << pGPIOHandler->pinConfig.GPIO_PinNumber);

	//cargamos el resultado sobre elregistro adecuado
	pGPIOHandler->pGPIOx->OTYPER |= auxConfig;
}

/*
 * Selects between four different possible speeds for output PinX
 * -	Low
 * -	Medium
 * -	Fast
 * -	HighSpeed
 */
void gpio_config_output_speed(GPIO_Handler_t *pGPIOHandler){

	uint32_t auxConfig = 0;


	assert_param(IS_GPIO_OSPEED(pGPIOHandler->pinConfig.GPIO_PinOutputSpeed));

	auxConfig = (pGPIOHandler->pinConfig.GPIO_PinOutputSpeed << 2*pGPIOHandler->pinConfig.GPIO_PinNumber);

	// Limpiando la posición antes de cargar la nueva configuración
	pGPIOHandler->pGPIOx->OSPEEDR &= ~(0b11 << 2 * pGPIOHandler->pinConfig.GPIO_PinNumber);

	// cargamos el resultado sobre el registro adecuado
	pGPIOHandler->pGPIOx->OSPEEDR |= auxConfig;
}


/*
 * Truns ON/OFF the pull-up and pull-down resistor foe each PinX in selected GPIO port
 */
void gpio_config_pullup_pulldown(GPIO_Handler_t *pGPIOHandler){

	uint32_t auxConfig = 0;

	//Verificamos si la configuracion cargada para ls resistencias es correcta
	assert_param(IS_GPIO_PUPDR(pGPIOHandler->pinConfig.GPIO_PinPuPdControl));

	auxConfig = (pGPIOHandler->pinConfig.GPIO_PinPuPdControl << 2*pGPIOHandler->pinConfig.GPIO_PinNumber);

	//Limpiando la posición antes de cargar la nueva configuración
	pGPIOHandler->pGPIOx->PUPDR &= ~(0b11 << 2 * pGPIOHandler->pinConfig.GPIO_PinNumber);

	//cargamoselresultado sobre elregistro adecuado
	pGPIOHandler->pGPIOx->PUPDR |= auxConfig;
}

/*
 * Allows to configure other functions (more specialized) on the selected PinX
 */
void gpio_config_alternate_function(GPIO_Handler_t *pGPIOHandler){

	uint32_t auxPosition = 0;

	if(pGPIOHandler->pinConfig.GPIO_PinMode == GPIO_MODE_ALFTN){

	//Seleccionamos primero si se debe utilizar elregistro abajo (AFRL) o el alto (AFRH)
		if(pGPIOHandler->pinConfig.GPIO_PinNumber < 8){
			//EStamos en el registro AFRL, que controlo los pines del PIN_0 al PIN_7
			auxPosition = 4 * pGPIOHandler->pinConfig.GPIO_PinNumber;

			// Limpiamos primero la posición del registro que deseamos escribir a continuación
			pGPIOHandler->pGPIOx->AFR[0] &= ~(0b1111 << auxPosition);

			// Y escribimos el valor configurando en la posición seleccionada
			pGPIOHandler->pGPIOx->AFR[0] |= (pGPIOHandler->pinConfig.GPIO_PinAltFunMode << auxPosition);
		}
		else{
			// Esatamos en el registro AFRH,que controla los pines del PIN_8 al PIN_15
			auxPosition = 4 * (pGPIOHandler->pinConfig.GPIO_PinNumber -8);

			// Limpiamos primero la posición del registro que deseamos escribir a continuación
			pGPIOHandler->pGPIOx->AFR[1] &= ~(0b1111 << auxPosition);

			// Y escribimos el valor configurando en la posición seleccionada
			pGPIOHandler->pGPIOx->AFR[1] |= (pGPIOHandler->pinConfig.GPIO_PinAltFunMode << auxPosition);

		}
	}
}

/*
 * Función utilizada para cambiar de estado el pin entregado en el handler,asignando
 * el valor entregado en la variable newState
 */
void gpio_WritePin(GPIO_Handler_t *pPinHandler,uint8_t newState){

	//Verificamos si la acción que deseamos realizar es permitida
	assert_param(IS_GPIO_PIN_ACTION(newState));

	//Limpiamos la posición que deseamos
	//pPinHandler->pGPIOx->ODR &= ~(SET << pPinHandler->pinConfig.GPIO_PinNumber);
	if(newState == SET){
		//Trabajando con la parte baja del registro
		pPinHandler->pGPIOx->BSRR |= (SET << (pPinHandler->pinConfig.GPIO_PinNumber));
	}
	else{
		//Trabajando con la parte alta del registro
		pPinHandler->pGPIOx->BSRR |= (SET << (pPinHandler->pinConfig.GPIO_PinNumber + 16));
	}
}

void gpio_WritePinFast(GPIO_Handler_t *pPinHandler,uint8_t newState){
	pPinHandler->pGPIOx->ODR &= ~(SET << pPinHandler->pinConfig.GPIO_PinNumber);
	pPinHandler->pGPIOx->ODR |= (newState << pPinHandler->pinConfig.GPIO_PinNumber);
}

/*
 * Función para leer el estado de un pin Especifico.
 */
uint8_t gpio_ReadPin(GPIO_Handler_t *pPinHandler){
	//creamos una variable auxiliar la cual luego retornaremos
	uint16_t pinValue = 0;

	// Cargamos el valor del registro IDR, desplazando a derecha tantas veces como la ubicación
	// del pin especifico
	pinValue = (pPinHandler->pGPIOx->IDR >> pPinHandler->pinConfig.GPIO_PinNumber) & 1;


	return pinValue;
}

void gpio_TooglePin(GPIO_Handler_t *pPinHandler){

	//creamos una variable auxiliar (mascara) donde guardaremos el valor del pin.
	uint8_t pinValue = 0;
	//Cambiamos el estado del pin usando XOR con un 1 en la mascara PinValue
	pinValue = gpio_ReadPin(pPinHandler) ^ 1;
	// Escribimos el nuevo estado del pin.
	gpio_WritePin(pPinHandler, pinValue);

}










