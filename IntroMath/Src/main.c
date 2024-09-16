/**
 ******************************************************************************
 * @file           : main.c
 * @author         : ImJeviz
 * @brief          : Main program body
 ******************************************************************************
 */
#include <stdint.h>
#include <string.h>
#include "stm32f4xx.h"
#include "gpio_driver_hal.h"
#include "stm32_assert.h"
#include "exti_driver_hal.h"
#include "timer_driver_hal.h"
#include "usart_driver_hal.h"

//Creación de un enum con los cuatro posibles casos del encoder.
enum {
	restaDerecha = 0,
	sumaDerecha,
	sumaIzquierda,
	restaIzquierda
};

enum {
	unidad = 0,
	decena,
	centena,
	unidadM
};

enum {
	Led0 = 0,
	Led1,
	Led2,
	Led3,
	Led4,
	Led5,
	Led6,
	imprimir
};

//Definimos los pines que se van a utilizar.
GPIO_Handler_t userLed 			= { 0 }; // PinA5
GPIO_Handler_t userLedA 		= { 0 }; // PinC6
GPIO_Handler_t userLedB 		= { 0 }; // PinA7
GPIO_Handler_t userLedC 		= { 0 }; // PinC8
GPIO_Handler_t userLedD 		= { 0 }; // PinA9
GPIO_Handler_t userLedE 		= { 0 }; // PinC9
GPIO_Handler_t userLedF 		= { 0 }; // PinA7
GPIO_Handler_t userLedG 		= { 0 }; // PinA8
GPIO_Handler_t userLed0 		= { 0 }; //
GPIO_Handler_t userLed1 		= { 0 }; //
GPIO_Handler_t userLed2 		= { 0 }; //
GPIO_Handler_t userLed3 		= { 0 }; //
GPIO_Handler_t userLed4 		= { 0 }; //
GPIO_Handler_t userLed5 		= { 0 }; //
GPIO_Handler_t userLed6 		= { 0 }; //
GPIO_Handler_t userTransistorU 	= { 0 }; //
GPIO_Handler_t userTransistorD 	= { 0 }; //
GPIO_Handler_t userTransistorC 	= { 0 }; //
GPIO_Handler_t userTransistorUM = { 0 }; //
GPIO_Handler_t usarTx			= { 0 }; //
GPIO_Handler_t usarRx			= { 0 }; //

//Definimos los timers que se emplearan.
Timer_Handler_t blinkTimer 		= { 0 }; // Timer para el blinky PinA5	TIM4
Timer_Handler_t displayTimer 	= { 0 }; // Timer para el 7-segmentos	TIM2

//Definimos las lineas EXTI que vamos a utilizar.
EXTI_Config_t btnYes 			= { 0 }; //Exti linea [] para el boton si
EXTI_Config_t btnNo 			= { 0 }; //Exti linea [] para el boton no

//Definimos el USART que vmaos a utilizar
USART_Handler_t usartHojas			= { 0 }; //Tipo de usart por donde se va a comunicar

/*
 * Creación de variables globales para convertir un numero de binario
 * a un numero de 7 segmentos.
 * Teniendo en cuenta una configuración antes vista (primeras semanas de clase)
 */

// Variables de los bits que se encienden para generar cada numero del 0 al 9
uint8_t bit0 = 0;
uint8_t bit0n = 0;
uint8_t bit1 = 0;
uint8_t bit1n = 0;
uint8_t bit2 = 0;
uint8_t bit2n = 0;
uint8_t bit3 = 0;

// Variables de cada led del 7 segmento.
uint8_t pinA = 0;
uint8_t pinB = 0;
uint8_t pinC = 0;
uint8_t pinD = 0;
uint8_t pinE = 0;
uint8_t pinF = 0;
uint8_t pinG = 0;

//Variables auxiliares que nos ayudaran en el codigo
uint8_t flagRefresh = 0; //Bandera que indica el momento en que se debe refrescar el 7 segmentos
uint8_t flagBtnYes 	= 0; //Bandera que ocurre cuando se presiona el boton que inidica SI.
uint8_t flagBtnNo 	= 0; //Bandera que ocurre cuando se presiona el boton que inidica NO.

//Buffer de datos para imprimir por usart
char bufferPrint[256] = { 0 };

//Variables de ayuda durante el codigo
uint8_t caso = 0; //variable que tiene en cuenta los casos para saber a que bit se refiere para encender dicho led
uint8_t conteo = 0; //Variable que guarda el numero que se adivinará
uint8_t unidadValue = 0;//almacena el valor de la unidad para el 7segmentos
uint8_t decenaValue = 0;//almacena el valor de la decena para el 7segmentos
uint8_t flagRespuesta = 0;

//llamamos las funciones definidas al final del codigo
void initSystem(void);
void initGPIO(void);
void initTimers(void);
void initExti(void);
void initUsart(void);
void write7segments(uint8_t *numero);
void transistorSwitch(void);
void suma(uint8_t *conteo);
void resta(uint8_t *conteo);

int main(void) {

	initSystem();
	sprintf(bufferPrint, "Empezar nuevo juego\n");
	usart_writeMsg(&usartHojas, bufferPrint);
	while (1) {

		if(flagRespuesta && flagRefresh){
			transistorSwitch();
			flagRefresh = 0;
			if(flagBtnYes){
				caso = 0;
				flagRespuesta = 0;
			}
		}

		if((flagBtnYes || flagBtnNo)){
			switch(caso){
			case Led0:{
				sprintf(bufferPrint, "\n\n\n\n\n\n\n\n\n\n\n\n\n"
						"TU NUMERO SE ENCUENTRA AQUI (PRESS BOTON AZUL = YES)\n\n"
						" 1 	3	5	7	9	11	13	15	17	19\n"
						" 21	23	25	27	29	31	33	35	37	39\n"
						" 41	43	45	47	49	51	53	55	57	59\n"
						" 61	63	65	67	69	71	73	75	77	79\n"
						" 81	83	85	87	89	91	93	95	97	99\n");
				usart_writeMsg(&usartHojas, bufferPrint);
				caso++;
				flagBtnYes 	= 0;
				flagBtnNo 	= 0;
				break;
			}
			case Led1:{
				if(flagBtnYes){
					conteo |= 1;
					gpio_WritePin(&userLed0, SET);
				}
				sprintf(bufferPrint, "\n\n\n\n\n\n\n\n\n\n\n\n\n"
						"TU NUMERO SE ENCUENTRA AQUI (PRESS BOTON AZUL = YES)\n\n"
						" 2 	3	6	7	10	11	14	15	18	19\n"
						" 22	23	26	27	30	31	34	35	38	39\n"
						" 42	43	46	47	50	51	54	55	58	59\n"
						" 62	63	66	67	70	71	74	75	78	79\n"
						" 82	83	86	87	90	91	94	95	98	99\n");
				usart_writeMsg(&usartHojas, bufferPrint);
				caso++;
				flagBtnYes 	= 0;
				flagBtnNo 	= 0;
				break;
			}
			case Led2:{
				if(flagBtnYes){
					conteo |= 1<<1;
					gpio_WritePin(&userLed1, SET);
				}
				sprintf(bufferPrint, "\n\n\n\n\n\n\n\n\n\n\n\n\n"
						"TU NUMERO SE ENCUENTRA AQUI (PRESS BOTON AZUL = YES)\n\n"
						" 4 	5	6	7	12	13	14	15\n"
						" 20	21	22	23	28	29	30	31\n"
						" 36	37	38	39	44	45	46	47\n"
						" 52	53	54	55	60	61	62	63\n"
						" 68	69	70	71	76	77	78	79\n"
						" 84	85	86	87	92	93	94	95\n");
				usart_writeMsg(&usartHojas, bufferPrint);
				caso++;
				flagBtnYes 	= 0;
				flagBtnNo 	= 0;
				break;
			}
			case Led3:{
				if(flagBtnYes){
					conteo |= 1<<2;
					gpio_WritePin(&userLed2, SET);
				}
				sprintf(bufferPrint, "\n\n\n\n\n\n\n\n\n\n\n\n\n"
						"TU NUMERO SE ENCUENTRA AQUI (PRESS BOTON AZUL = YES)\n\n"
						" 8		9	10	11	12	13	14	15\n"
						" 24	25	26	27	28	29	30	31\n"
						" 40	41	42	43	44	45	46	47\n"
						" 56	57	58	59	60	61	62	63\n"
						" 72	73	74	75	76	77	78	79\n"
						" 88	89	90	91	92	93	94	95\n");
				usart_writeMsg(&usartHojas, bufferPrint);
				caso++;
				flagBtnYes 	= 0;
				flagBtnNo 	= 0;
				break;
			}
			case Led4:{
				if(flagBtnYes){
					conteo |= 1<<3;
					gpio_WritePin(&userLed3, SET);
				}
				sprintf(bufferPrint, "\n\n\n\n\n\n\n\n\n\n\n\n\n"
						"TU NUMERO SE ENCUENTRA AQUI (PRESS BOTON AZUL = YES)\n\n"
						" 16	17	18	19	20	21	22	23\n"
						" 24	25	26	27	28	29	30	31\n"
						" 48	49	50	51	52	53	54	55\n"
						" 56	57	58	59	60	61	62	63\n"
						" 80	81	82	83	84	85	86	87\n"
						" 88	89	90	91	92	93	94	95\n");
				usart_writeMsg(&usartHojas, bufferPrint);
				caso++;
				flagBtnYes 	= 0;
				flagBtnNo 	= 0;
				break;
			}
			case Led5:{
				if(flagBtnYes){
					conteo |= 1<<4;
					gpio_WritePin(&userLed4, SET);
				}
				sprintf(bufferPrint, "\n\n\n\n\n\n\n\n\n\n\n\n\n"
						"TU NUMERO SE ENCUENTRA AQUI (PRESS BOTON AZUL = YES)\n\n"
						" 32	33	34	35	36	37	38	39\n"
						" 40	41	42	43	44	45	46	47\n"
						" 48	49	50	51	52	53	54	55\n"
						" 56	57	58	59	60	61	62	63\n"
						" 96	97	98	99\n");
				usart_writeMsg(&usartHojas, bufferPrint);
				caso++;
				flagBtnYes 	= 0;
				flagBtnNo 	= 0;
				break;
			}
			case Led6:{
				if(flagBtnYes){
					conteo |= 1<<5;
					gpio_WritePin(&userLed5, SET);
				}
				sprintf(bufferPrint, "\n\n\n\n\n\n\n\n\n\n\n\n\n"
						"TU NUMERO SE ENCUENTRA AQUI (PRESS BOTON AZUL = YES)\n\n"
						" 64	65	66	67	68	69	70	71\n"
						" 72	73	74	75	76	77	78	79\n"
						" 80	81	82	83	84	85	86	87\n"
						" 88	89	90	91	92	93	94	95\n"
						" 96	97	98	99\n");
				usart_writeMsg(&usartHojas, bufferPrint);
				caso++;
				flagBtnYes 	= 0;
				flagBtnNo 	= 0;
				break;
			}
			case imprimir:{
				if(flagBtnYes){
					conteo |= 1<<6;
					gpio_WritePin(&userLed6, SET);
				}
				sprintf(bufferPrint, "\n\n\n\n\n\n\n\n\n\n\n\n\n"
						"SU NUMERO ES %d \n\n",conteo);
				usart_writeMsg(&usartHojas, bufferPrint);
				caso = 99;
				flagRespuesta = 1;
				flagBtnYes 	= 0;
				flagBtnNo 	= 0;
				break;
			}
			}
		}
	}
}


void initSystem(void){
	initGPIO();
	initTimers();
	initExti();
	initUsart();
	gpio_WritePin(&userLed, SET);

}


void initGPIO(void){
	//Configuramos los pines que se van a utilizar

	/* Configuramos el PinA5 */
	userLed.pGPIOx = GPIOA;
	userLed.pinConfig.GPIO_PinNumber = PIN_5;
	userLed.pinConfig.GPIO_PinMode = GPIO_MODE_OUT;
	userLed.pinConfig.GPIO_PinOutputType = GPIO_OTYPE_PUSHPULL;
	userLed.pinConfig.GPIO_PinOutputSpeed = GPIO_OSPEED_MEDIUM;
	userLed.pinConfig.GPIO_PinPuPdControl = GPIO_PUPDR_NOTHING;

	//Cargamos la configuracion  en los registros que gobiernan el puerto.
	gpio_Config(&userLed);

	/* Configuramos el PinC6 */
	userLedA.pGPIOx = GPIOC;
	userLedA.pinConfig.GPIO_PinNumber = PIN_6;
	userLedA.pinConfig.GPIO_PinMode = GPIO_MODE_OUT;
	userLedA.pinConfig.GPIO_PinOutputType = GPIO_OTYPE_PUSHPULL;
	userLedA.pinConfig.GPIO_PinOutputSpeed = GPIO_OSPEED_MEDIUM;
	userLedA.pinConfig.GPIO_PinPuPdControl = GPIO_PUPDR_NOTHING;

	//Cargamos la configuracion  en los registros que gobiernan el puerto.
	gpio_Config(&userLedA);

	/* Configuramos el PinA7 */
	userLedB.pGPIOx = GPIOA;
	userLedB.pinConfig.GPIO_PinNumber = PIN_7;
	userLedB.pinConfig.GPIO_PinMode = GPIO_MODE_OUT;
	userLedB.pinConfig.GPIO_PinOutputType = GPIO_OTYPE_PUSHPULL;
	userLedB.pinConfig.GPIO_PinOutputSpeed = GPIO_OSPEED_MEDIUM;
	userLedB.pinConfig.GPIO_PinPuPdControl = GPIO_PUPDR_NOTHING;

	//Cargamos la configuracion  en los registros que gobiernan el puerto.
	gpio_Config(&userLedB);

	/* Configuramos el PinC8 */
	userLedC.pGPIOx = GPIOC;
	userLedC.pinConfig.GPIO_PinNumber = PIN_8;
	userLedC.pinConfig.GPIO_PinMode = GPIO_MODE_OUT;
	userLedC.pinConfig.GPIO_PinOutputType = GPIO_OTYPE_PUSHPULL;
	userLedC.pinConfig.GPIO_PinOutputSpeed = GPIO_OSPEED_MEDIUM;
	userLedC.pinConfig.GPIO_PinPuPdControl = GPIO_PUPDR_NOTHING;

	//Cargamos la configuracion  en los registros que gobiernan el puerto.
	gpio_Config(&userLedC);

	/* Configuramos el PinA9 */
	userLedD.pGPIOx = GPIOA;
	userLedD.pinConfig.GPIO_PinNumber = PIN_9;
	userLedD.pinConfig.GPIO_PinMode = GPIO_MODE_OUT;
	userLedD.pinConfig.GPIO_PinOutputType = GPIO_OTYPE_PUSHPULL;
	userLedD.pinConfig.GPIO_PinOutputSpeed = GPIO_OSPEED_MEDIUM;
	userLedD.pinConfig.GPIO_PinPuPdControl = GPIO_PUPDR_NOTHING;

	//Cargamos la configuracion  en los registros que gobiernan el puerto.
	gpio_Config(&userLedD);

	/* Configuramos el PinC9 */
	userLedE.pGPIOx = GPIOC;
	userLedE.pinConfig.GPIO_PinNumber = PIN_9;
	userLedE.pinConfig.GPIO_PinMode = GPIO_MODE_OUT;
	userLedE.pinConfig.GPIO_PinOutputType = GPIO_OTYPE_PUSHPULL;
	userLedE.pinConfig.GPIO_PinOutputSpeed = GPIO_OSPEED_MEDIUM;
	userLedE.pinConfig.GPIO_PinPuPdControl = GPIO_PUPDR_NOTHING;

	//Cargamos la configuracion  en los registros que gobiernan el puerto.
	gpio_Config(&userLedE);

	/* Configuramos el PinA6 */
	userLedF.pGPIOx = GPIOA;
	userLedF.pinConfig.GPIO_PinNumber = PIN_6;
	userLedF.pinConfig.GPIO_PinMode = GPIO_MODE_OUT;
	userLedF.pinConfig.GPIO_PinOutputType = GPIO_OTYPE_PUSHPULL;
	userLedF.pinConfig.GPIO_PinOutputSpeed = GPIO_OSPEED_MEDIUM;
	userLedF.pinConfig.GPIO_PinPuPdControl = GPIO_PUPDR_NOTHING;

	//Cargamos la configuracion  en los registros que gobiernan el puerto.
	gpio_Config(&userLedF);

	/* Configuramos el PinA8 */
	userLedG.pGPIOx = GPIOA;
	userLedG.pinConfig.GPIO_PinNumber = PIN_8;
	userLedG.pinConfig.GPIO_PinMode = GPIO_MODE_OUT;
	userLedG.pinConfig.GPIO_PinOutputType = GPIO_OTYPE_PUSHPULL;
	userLedG.pinConfig.GPIO_PinOutputSpeed = GPIO_OSPEED_MEDIUM;
	userLedG.pinConfig.GPIO_PinPuPdControl = GPIO_PUPDR_NOTHING;

	//Cargamos la configuracion  en los registros que gobiernan el puerto.
	gpio_Config(&userLedG);

}

void initTimers(void){
	//Configuramos los timers

	/* Configuramos el timer del blink (TIM2) */
	blinkTimer.pTIMx = TIM4;
	blinkTimer.TIMx_Config.TIMx_Prescaler = 16000;
	blinkTimer.TIMx_Config.TIMx_Period = 250;
	blinkTimer.TIMx_Config.TIMx_mode = TIMER_UP_COUNTER;
	blinkTimer.TIMx_Config.TIMx_InterruptEnable = TIMER_INT_ENABLE;

	//Cargamos la configuracion del timer
	timer_Config(&blinkTimer);

	//Encendemos el timer
	timer_SetState(&blinkTimer, TIMER_ON);

	/* Configuramos el timer del 7-segmentos (TIM4) */
	displayTimer.pTIMx = TIM2;
	displayTimer.TIMx_Config.TIMx_Prescaler = 16000;
	displayTimer.TIMx_Config.TIMx_Period = 2;
	displayTimer.TIMx_Config.TIMx_mode = TIMER_UP_COUNTER;
	displayTimer.TIMx_Config.TIMx_InterruptEnable = TIMER_INT_ENABLE;

	//Cargamos la configuracion del timer
	timer_Config(&displayTimer);

	//Encendemos el timer
	timer_SetState(&displayTimer, TIMER_ON);
}

void initExti(void){

}

void initUsart(void){

}


/*
 * Funcion que recibe como parametro una variable que
 * contiene un numero en binario, para así cambiar unas
 * variables globales con las que escribimos los numeros
 * en el 7 segmentos.
 */
void write7segments(uint8_t *numero) {
	bit0 = (*numero >> 0) & 1;
	bit0n = ~bit0 & 1;
	bit1 = (*numero >> 1) & 1;
	bit1n = ~bit1 & 1;
	bit2 = (*numero >> 2) & 1;
	bit2n = ~bit2 & 1;
	bit3 = (*numero >> 3) & 1;

	/*
	 * Esta configuracion de los pines A,B,C,D,E,F,G,que son los leds que
	 * encienden al encoder, es tomada de un ejemplo en digital donde
	 * se realizan operasiones logicas de todos lo posibles casos en los
	 * que se encienda cada led de acuerdo a cada numero.
	 */
	pinA = (bit3 | bit1) | ((~(bit0 ^ bit2)) & 1);
	pinB = bit2n | ((~(bit1 ^ bit0)) & 1);
	pinC = bit2 | bit1n | bit0;
	pinD = (bit1 & bit0n) | (bit2n & bit0n) | (bit2n & bit1)
			| (bit2 & bit1n & bit0);
	pinE = (bit1 & bit0n) | (bit2n & bit0n);
	pinF = bit3 | (bit2 & bit1n) | (bit2 & bit0n) | (bit1n & bit0n);
	pinG = bit3 | (bit2 ^ bit1) | (bit1 & bit0n);

	gpio_WritePin(&userLedA, pinA);
	gpio_WritePin(&userLedB, pinB);
	gpio_WritePin(&userLedC, pinC);
	gpio_WritePin(&userLedD, pinD);
	gpio_WritePin(&userLedE, pinE);
	gpio_WritePin(&userLedF, pinF);
	gpio_WritePin(&userLedG, pinG);
}

/*
 * Funcion que recibe como parametro una variable que
 * contiente la opcion del caso a elegir, dicho caso
 * colocará a funcionar un transistor y apagará el
 * resto de ellos.
 */
void transistorSwitch(void){
	uint8_t option = 0;
	switch(option){
	case unidad:{
		gpio_WritePin(&userTransistorD, 	SET);
		gpio_WritePin(&userTransistorC, 	SET);
		gpio_WritePin(&userTransistorUM, 	SET);
		write7segments(&unidadValue);
		gpio_WritePin(&userTransistorU, 	RESET);
		option++;
		break;
	}
	case decena:{
		gpio_WritePin(&userTransistorU, 	SET);
		write7segments(&decenaValue);
		gpio_WritePin(&userTransistorD, 	RESET);
		option = 0;
		break;
	}
//	case centena:{
//		gpio_WritePin(&userTransistorD,	 	SET);
//		write7segments(&centenaValue);
//		gpio_WritePin(&userTransistorC, 	RESET);
//		option++;
//		break;
//	}
//	case unidadM:{
//		gpio_WritePin(&userTransistorC, 	SET);
//		write7segments(&unidadMValue);
//		gpio_WritePin(&userTransistorUM, 	RESET);
//		option = 0;
//		break;
//	}
	default:{
		gpio_WritePin(&userTransistorD, 	SET);
		gpio_WritePin(&userTransistorC, 	SET);
		gpio_WritePin(&userTransistorUM, 	SET);
		gpio_WritePin(&userTransistorU, 	SET);
		option = 0;
		break;
	}
	}
}

// Funcion que suma 1 a la variable que indiquemos como parametro.
void suma(uint8_t *conteo) {
	*conteo += 1;
}

// Funcion que resta 1 a la variable que indiquemos como parametro
void resta(uint8_t *conteo) {
	*conteo -= 1;
}


void Timer2_Callback(void) {
	flagRefresh = 1;
}

void Timer4_Callback(void) {
	gpio_TooglePin(&userLed);
}

void callback_ExtInt10(void) {
	flagBtnYes = 1;
}

void callback_ExtInt3(void) {
	flagBtnNo = 1;
}


/*
 * Esta función sirve para detectar problemas de parametros
 * incorrectos al momento de ejecutar un programa.
 */

void assert_failed(uint8_t *file, uint32_t line) {
	while (1) {
		//Problems
	}
}
