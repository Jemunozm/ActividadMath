/*
 * TFT_ILI9486.c
 *
 *  Created on: Nov 14, 2023
 *      Author: imjeviz
 */

#include "stm32f4xx.h"
#include "TFT_ILI9486.h"
#include "gpio_driver_hal.h"
#include "systick_driver_hal.h"
#include "math.h"
#include "stdbool.h"

GPIO_Handler_t RS = { 0 };
GPIO_Handler_t WR = { 0 };
GPIO_Handler_t RST = { 0 };

GPIO_Handler_t CS = { 0 };

GPIO_Handler_t MOSI = { 0 };
GPIO_Handler_t CLK = { 0 };

//PINES DB QUE RECIBIRAN SU ESTADO DE LAS VARIABLES DDR*
GPIO_Handler_t DB0_LOW = { 0 };
GPIO_Handler_t DB1_LOW = { 0 };
GPIO_Handler_t DB2_LOW = { 0 };
GPIO_Handler_t DB3_LOW = { 0 };
GPIO_Handler_t DB4_LOW = { 0 };
GPIO_Handler_t DB5_LOW = { 0 };
GPIO_Handler_t DB6_LOW = { 0 };
GPIO_Handler_t DB7_LOW = { 0 };

GPIO_Handler_t DB8_HIGH = { 0 };
GPIO_Handler_t DB9_HIGH = { 0 };
GPIO_Handler_t DB10_HIGH = { 0 };
GPIO_Handler_t DB11_HIGH = { 0 };
GPIO_Handler_t DB12_HIGH = { 0 };
GPIO_Handler_t DB13_HIGH = { 0 };
GPIO_Handler_t DB14_HIGH = { 0 };
GPIO_Handler_t DB15_HIGH = { 0 };

// Set direction for the 2 8 bit data ports
/*
 * Recordemos que la pantalla es de 16 bits divididos entre dos pares de 8 bits
 * estas variables seran encargadas de encender los bits especificos de esos 8 bits
 * ya sean los superiores o inferiores.
 */
uint8_t DDRA = 0xFF;
uint8_t DDRC = 0xFF;
uint8_t c = 0;
// Display w/h as modified by current rotation
uint16_t  width;
uint16_t height;
// Text cursor position and width padding
uint16_t cursor_x;
uint16_t cursor_y;
uint16_t padX;


static void tft_write_command(uint8_t c);
static void tft_write_data(uint8_t c);
static void tft_gpio_init(void);
static void tft_gpio_db_config(void);
static void wr_strobe(void);
static void init(void);
static void setAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
static void swap(uint16_t *x, uint16_t *y);
//static void pushColor(uint16_t color);
//static void pushColorl(uint16_t color, uint16_t len);

void tft_Config(void) {

	tft_gpio_init();
	init();
}

static void tft_gpio_init(void) {

	//Chip select
	CS.pGPIOx = GPIOC;
	CS.pinConfig.GPIO_PinNumber = PIN_4;
	CS.pinConfig.GPIO_PinMode = GPIO_MODE_OUT;
	CS.pinConfig.GPIO_PinOutputType = GPIO_OTYPE_PUSHPULL;
	CS.pinConfig.GPIO_PinOutputSpeed = GPIO_OSPEED_HIGH;
	CS.pinConfig.GPIO_PinPuPdControl = GPIO_PUPDR_NOTHING;
	gpio_Config(&CS);
	gpio_WritePin(&CS, SET);

	//LCD reset control pin
	RST.pGPIOx = GPIOB;
	RST.pinConfig.GPIO_PinNumber = PIN_13;
	RST.pinConfig.GPIO_PinMode = GPIO_MODE_OUT;
	RST.pinConfig.GPIO_PinOutputType = GPIO_OTYPE_PUSHPULL;
	RST.pinConfig.GPIO_PinOutputSpeed = GPIO_OSPEED_HIGH;
	RST.pinConfig.GPIO_PinPuPdControl = GPIO_PUPDR_NOTHING;
	gpio_Config(&RST);

	//LCD write control pin
	WR.pGPIOx = GPIOB;
	WR.pinConfig.GPIO_PinNumber = PIN_14;
	WR.pinConfig.GPIO_PinMode = GPIO_MODE_OUT;
	WR.pinConfig.GPIO_PinOutputType = GPIO_OTYPE_PUSHPULL;
	WR.pinConfig.GPIO_PinOutputSpeed = GPIO_OSPEED_HIGH;
	WR.pinConfig.GPIO_PinPuPdControl = GPIO_PUPDR_NOTHING;
	gpio_Config(&WR);
	gpio_WritePin(&WR, SET);

	//data selection pin
	RS.pGPIOx = GPIOB;
	RS.pinConfig.GPIO_PinNumber = PIN_5;
	RS.pinConfig.GPIO_PinMode = GPIO_MODE_OUT;
	RS.pinConfig.GPIO_PinOutputType = GPIO_OTYPE_PUSHPULL;
	RS.pinConfig.GPIO_PinOutputSpeed = GPIO_OSPEED_HIGH;
	RS.pinConfig.GPIO_PinPuPdControl = GPIO_PUPDR_NOTHING;
	gpio_Config(&RS);
	gpio_WritePin(&RS, SET);

	DB0_LOW.pGPIOx = GPIOA;
	DB0_LOW.pinConfig.GPIO_PinNumber = PIN_9;
	DB0_LOW.pinConfig.GPIO_PinMode = GPIO_MODE_OUT;
	DB0_LOW.pinConfig.GPIO_PinOutputType = GPIO_OTYPE_PUSHPULL;
	DB0_LOW.pinConfig.GPIO_PinOutputSpeed = GPIO_OSPEED_HIGH;
	DB0_LOW.pinConfig.GPIO_PinPuPdControl = GPIO_PUPDR_NOTHING;
	gpio_Config(&DB0_LOW);

	DB1_LOW.pGPIOx = GPIOB;
	DB1_LOW.pinConfig.GPIO_PinNumber = PIN_15;
	DB1_LOW.pinConfig.GPIO_PinMode = GPIO_MODE_OUT;
	DB1_LOW.pinConfig.GPIO_PinOutputType = GPIO_OTYPE_PUSHPULL;
	DB1_LOW.pinConfig.GPIO_PinOutputSpeed = GPIO_OSPEED_HIGH;
	DB1_LOW.pinConfig.GPIO_PinPuPdControl = GPIO_PUPDR_NOTHING;
	gpio_Config(&DB1_LOW);

	DB2_LOW.pGPIOx = GPIOC;
	DB2_LOW.pinConfig.GPIO_PinNumber = PIN_7;
	DB2_LOW.pinConfig.GPIO_PinMode = GPIO_MODE_OUT;
	DB2_LOW.pinConfig.GPIO_PinOutputType = GPIO_OTYPE_PUSHPULL;
	DB2_LOW.pinConfig.GPIO_PinOutputSpeed = GPIO_OSPEED_HIGH;
	DB2_LOW.pinConfig.GPIO_PinPuPdControl = GPIO_PUPDR_NOTHING;
	gpio_Config(&DB2_LOW);

	DB3_LOW.pGPIOx = GPIOB;
	DB3_LOW.pinConfig.GPIO_PinNumber = PIN_1;
	DB3_LOW.pinConfig.GPIO_PinMode = GPIO_MODE_OUT;
	DB3_LOW.pinConfig.GPIO_PinOutputType = GPIO_OTYPE_PUSHPULL;
	DB3_LOW.pinConfig.GPIO_PinOutputSpeed = GPIO_OSPEED_HIGH;
	DB3_LOW.pinConfig.GPIO_PinPuPdControl = GPIO_PUPDR_NOTHING;
	gpio_Config(&DB3_LOW);

	DB4_LOW.pGPIOx = GPIOB;
	DB4_LOW.pinConfig.GPIO_PinNumber = PIN_6;
	DB4_LOW.pinConfig.GPIO_PinMode = GPIO_MODE_OUT;
	DB4_LOW.pinConfig.GPIO_PinOutputType = GPIO_OTYPE_PUSHPULL;
	DB4_LOW.pinConfig.GPIO_PinOutputSpeed = GPIO_OSPEED_HIGH;
	DB4_LOW.pinConfig.GPIO_PinPuPdControl = GPIO_PUPDR_NOTHING;
	gpio_Config(&DB4_LOW);

	DB5_LOW.pGPIOx = GPIOB;
	DB5_LOW.pinConfig.GPIO_PinNumber = PIN_2;
	DB5_LOW.pinConfig.GPIO_PinMode = GPIO_MODE_OUT;
	DB5_LOW.pinConfig.GPIO_PinOutputType = GPIO_OTYPE_PUSHPULL;
	DB5_LOW.pinConfig.GPIO_PinOutputSpeed = GPIO_OSPEED_HIGH;
	DB5_LOW.pinConfig.GPIO_PinPuPdControl = GPIO_PUPDR_NOTHING;
	gpio_Config(&DB5_LOW);

	DB6_LOW.pGPIOx = GPIOA;
	DB6_LOW.pinConfig.GPIO_PinNumber = PIN_7;
	DB6_LOW.pinConfig.GPIO_PinMode = GPIO_MODE_OUT;
	DB6_LOW.pinConfig.GPIO_PinOutputType = GPIO_OTYPE_PUSHPULL;
	DB6_LOW.pinConfig.GPIO_PinOutputSpeed = GPIO_OSPEED_HIGH;
	DB6_LOW.pinConfig.GPIO_PinPuPdControl = GPIO_PUPDR_NOTHING;
	gpio_Config(&DB6_LOW);

	DB7_LOW.pGPIOx = GPIOB;
	DB7_LOW.pinConfig.GPIO_PinNumber = PIN_12;
	DB7_LOW.pinConfig.GPIO_PinMode = GPIO_MODE_OUT;
	DB7_LOW.pinConfig.GPIO_PinOutputType = GPIO_OTYPE_PUSHPULL;
	DB7_LOW.pinConfig.GPIO_PinOutputSpeed = GPIO_OSPEED_HIGH;
	DB7_LOW.pinConfig.GPIO_PinPuPdControl = GPIO_PUPDR_NOTHING;
	gpio_Config(&DB7_LOW);

	DB8_HIGH.pGPIOx = GPIOC;
	DB8_HIGH.pinConfig.GPIO_PinNumber = PIN_8;
	DB8_HIGH.pinConfig.GPIO_PinMode = GPIO_MODE_OUT;
	DB8_HIGH.pinConfig.GPIO_PinOutputType = GPIO_OTYPE_PUSHPULL;
	DB8_HIGH.pinConfig.GPIO_PinOutputSpeed = GPIO_OSPEED_HIGH;
	DB8_HIGH.pinConfig.GPIO_PinPuPdControl = GPIO_PUPDR_NOTHING;
	gpio_Config(&DB8_HIGH);

	DB9_HIGH.pGPIOx = GPIOC;
	DB9_HIGH.pinConfig.GPIO_PinNumber = PIN_9;
	DB9_HIGH.pinConfig.GPIO_PinMode = GPIO_MODE_OUT;
	DB9_HIGH.pinConfig.GPIO_PinOutputType = GPIO_OTYPE_PUSHPULL;
	DB9_HIGH.pinConfig.GPIO_PinOutputSpeed = GPIO_OSPEED_HIGH;
	DB9_HIGH.pinConfig.GPIO_PinPuPdControl = GPIO_PUPDR_NOTHING;
	gpio_Config(&DB9_HIGH);

	DB10_HIGH.pGPIOx = GPIOC;
	DB10_HIGH.pinConfig.GPIO_PinNumber = PIN_6;
	DB10_HIGH.pinConfig.GPIO_PinMode = GPIO_MODE_OUT;
	DB10_HIGH.pinConfig.GPIO_PinOutputType = GPIO_OTYPE_PUSHPULL;
	DB10_HIGH.pinConfig.GPIO_PinOutputSpeed = GPIO_OSPEED_HIGH;
	DB10_HIGH.pinConfig.GPIO_PinPuPdControl = GPIO_PUPDR_NOTHING;
	gpio_Config(&DB10_HIGH);

	DB11_HIGH.pGPIOx = GPIOB;
	DB11_HIGH.pinConfig.GPIO_PinNumber = PIN_8;
	DB11_HIGH.pinConfig.GPIO_PinMode = GPIO_MODE_OUT;
	DB11_HIGH.pinConfig.GPIO_PinOutputType = GPIO_OTYPE_PUSHPULL;
	DB11_HIGH.pinConfig.GPIO_PinOutputSpeed = GPIO_OSPEED_HIGH;
	DB11_HIGH.pinConfig.GPIO_PinPuPdControl = GPIO_PUPDR_NOTHING;
	gpio_Config(&DB11_HIGH);

	DB12_HIGH.pGPIOx = GPIOC;
	DB12_HIGH.pinConfig.GPIO_PinNumber = PIN_5;
	DB12_HIGH.pinConfig.GPIO_PinMode = GPIO_MODE_OUT;
	DB12_HIGH.pinConfig.GPIO_PinOutputType = GPIO_OTYPE_PUSHPULL;
	DB12_HIGH.pinConfig.GPIO_PinOutputSpeed = GPIO_OSPEED_HIGH;
	DB12_HIGH.pinConfig.GPIO_PinPuPdControl = GPIO_PUPDR_NOTHING;
	gpio_Config(&DB12_HIGH);

	DB13_HIGH.pGPIOx = GPIOB;
	DB13_HIGH.pinConfig.GPIO_PinNumber = PIN_9;
	DB13_HIGH.pinConfig.GPIO_PinMode = GPIO_MODE_OUT;
	DB13_HIGH.pinConfig.GPIO_PinOutputType = GPIO_OTYPE_PUSHPULL;
	DB13_HIGH.pinConfig.GPIO_PinOutputSpeed = GPIO_OSPEED_HIGH;
	DB13_HIGH.pinConfig.GPIO_PinPuPdControl = GPIO_PUPDR_NOTHING;
	gpio_Config(&DB13_HIGH);

	DB14_HIGH.pGPIOx = GPIOA;
	DB14_HIGH.pinConfig.GPIO_PinNumber = PIN_11;
	DB14_HIGH.pinConfig.GPIO_PinMode = GPIO_MODE_OUT;
	DB14_HIGH.pinConfig.GPIO_PinOutputType = GPIO_OTYPE_PUSHPULL;
	DB14_HIGH.pinConfig.GPIO_PinOutputSpeed = GPIO_OSPEED_HIGH;
	DB14_HIGH.pinConfig.GPIO_PinPuPdControl = GPIO_PUPDR_NOTHING;
	gpio_Config(&DB14_HIGH);

	DB15_HIGH.pGPIOx = GPIOA;
	DB15_HIGH.pinConfig.GPIO_PinNumber = PIN_6;
	DB15_HIGH.pinConfig.GPIO_PinMode = GPIO_MODE_OUT;
	DB15_HIGH.pinConfig.GPIO_PinOutputType = GPIO_OTYPE_PUSHPULL;
	DB15_HIGH.pinConfig.GPIO_PinOutputSpeed = GPIO_OSPEED_HIGH;
	DB15_HIGH.pinConfig.GPIO_PinPuPdControl = GPIO_PUPDR_NOTHING;
	gpio_Config(&DB15_HIGH);


}

static void tft_gpio_db_config(void) {

	//Organizo los pines como si fueran un registro de 8 bits de la variable DDRC

	gpio_WritePinFast(&DB0_LOW, (DDRC & 1));
	gpio_WritePinFast(&DB1_LOW, ((DDRC >> 1) & 1));
	gpio_WritePinFast(&DB2_LOW, ((DDRC >> 2) & 1));
	gpio_WritePinFast(&DB3_LOW, ((DDRC >> 3) & 1));
	gpio_WritePinFast(&DB4_LOW, ((DDRC >> 4) & 1));
	gpio_WritePinFast(&DB5_LOW, ((DDRC >> 5) & 1));
	gpio_WritePinFast(&DB6_LOW, ((DDRC >> 6) & 1));
	gpio_WritePinFast(&DB7_LOW, ((DDRC >> 7) & 1));

	//Organizo los pines como si fueran un registro de 8 bits de la variable DDRA
	gpio_WritePinFast(&DB8_HIGH, (DDRA & 1));
	gpio_WritePinFast(&DB9_HIGH, ((DDRA >> 1) & 1));
	gpio_WritePinFast(&DB10_HIGH, ((DDRA >> 2) & 1));
	gpio_WritePinFast(&DB11_HIGH, ((DDRA >> 3) & 1));
	gpio_WritePinFast(&DB12_HIGH, ((DDRA >> 4) & 1));
	gpio_WritePinFast(&DB13_HIGH, ((DDRA >> 5) & 1));
	gpio_WritePinFast(&DB14_HIGH, ((DDRA >> 6) & 1));
	gpio_WritePinFast(&DB15_HIGH, ((DDRA >> 7) & 1));
}

static void swap(uint16_t *x, uint16_t *y){
	uint16_t aux = 0;
	aux = *x;
	*x = *y;
	*y = aux;
}

static void wr_strobe(void) {
	GPIOB->ODR &= ~(SET<<14);
	GPIOB->ODR |= (SET<<14);
}

static void tft_write_command(uint8_t c) {

	gpio_WritePin(&CS, RESET);
	gpio_WritePin(&RS, RESET);
	DDRA = 0;
	DDRC = c;
	tft_gpio_db_config();
	wr_strobe();
	gpio_WritePin(&RS, SET);
	gpio_WritePin(&CS, SET);

}

static void tft_write_data(uint8_t c) {

	gpio_WritePin(&CS, RESET);
	DDRA = c>>8;
	DDRC = c;
	tft_gpio_db_config();
	wr_strobe();
	gpio_WritePin(&CS, SET);
}

void init() {
	gpio_WritePin(&RST, SET);
	systick_Delay_ms(50);
	gpio_WritePin(&RST, RESET);
	systick_Delay_ms(10);
	gpio_WritePin(&RST, SET);
	systick_Delay_ms(10);

/*
 * Prueba init 2.0
 */
//	tft_write_command(0X01);
//	tft_write_data(0x00);
//	systick_Delay_ms(50);
//	tft_write_command(0XF1);
//	tft_write_data(0x36);
//	tft_write_data(0x04);
//	tft_write_data(0x00);
//	tft_write_data(0x3C);
//	tft_write_data(0X0F);
//	tft_write_data(0x8F);
//	tft_write_command(0XF2);
//	tft_write_data(0x18);
//	tft_write_data(0xA3);
//	tft_write_data(0x12);
//	tft_write_data(0x02);
//	tft_write_data(0XB2);
//	tft_write_data(0x12);
//	tft_write_data(0xFF);
//	tft_write_data(0x10);
//	tft_write_data(0x00);
//	tft_write_command(0XF8);
//	tft_write_data(0x21);
//	tft_write_data(0x04);
//	tft_write_command(0XF9);
//	tft_write_data(0x00);
//	tft_write_data(0x08);
//	tft_write_command(0x36);
//	tft_write_data(0x08);
//	tft_write_command(0xB4);
//	tft_write_data(0x00);
//	tft_write_command(0xC1);
//	tft_write_data(0x41);
//	tft_write_command(0xC5);
//	tft_write_data(0x00);
//	tft_write_data(0x91);
//	tft_write_data(0x80);
//	tft_write_data(0x00);
//	tft_write_command(0xE0);
//	tft_write_data(0x0F);
//	tft_write_data(0x1F);
//	tft_write_data(0x1C);
//	tft_write_data(0x0C);
//	tft_write_data(0x0F);
//	tft_write_data(0x08);
//	tft_write_data(0x48);
//	tft_write_data(0x98);
//	tft_write_data(0x37);
//	tft_write_data(0x0A);
//	tft_write_data(0x13);
//	tft_write_data(0x04);
//	tft_write_data(0x11);
//	tft_write_data(0x0D);
//	tft_write_data(0x00);
//	tft_write_command(0xE1);
//	tft_write_data(0x0F);
//	tft_write_data(0x32);
//	tft_write_data(0x2E);
//	tft_write_data(0x0B);
//	tft_write_data(0x0D);
//	tft_write_data(0x05);
//	tft_write_data(0x47);
//	tft_write_data(0x75);
//	tft_write_data(0x37);
//	tft_write_data(0x06);
//	tft_write_data(0x10);
//	tft_write_data(0x03);
//	tft_write_data(0x24);
//	tft_write_data(0x20);
//	tft_write_data(0x00);
//	tft_write_command(0x3A);
//	tft_write_data(0x55);
//	tft_write_command(0x11);
//	systick_Delay_ms(150);
//	tft_write_command(0x20);
//	tft_write_command(0x36);
//	tft_write_data(0x28);
//	systick_Delay_ms(120);
//	tft_write_command(0x29);
//	systick_Delay_ms(25);

	/*
	 * Prueba init 2.0
	 */

	tft_write_command(0x01);
	tft_write_data(0x00);
	systick_Delay_ms(50);

	tft_write_command(0x28);
	tft_write_data(0x00);

	tft_write_command(0xC0);        // Power Control 1
	tft_write_data(0x0d);
	tft_write_data(0x0d);

	tft_write_command(0xC1);        // Power Control 2
	tft_write_data(0x43);
	tft_write_data(0x00);

	tft_write_command(0xC2);        // Power Control 3
	tft_write_data(0x00);

	tft_write_command(0xC5);        // VCOM Control
	tft_write_data(0x00);
	tft_write_data(0x48);

//	tft_write_command(0xC0); //                          1100.0000 Power Control 1
//	tft_write_data(0x0E);    //                          0001.0111   ... VRH1
//    tft_write_data(0x0E);    //                          0001.0101   ... VRH2
//    tft_write_command(0xC1); //                          1100.0001 Power Control 2
//    tft_write_data(0x41);    //                          0100.0001   . SAP BT
//    tft_write_data(0x00);    //                          0000.0000   ..... VC
//    tft_write_command(0xC2); //                          1100.0010 Power Control 3
//    tft_write_data(0x00);    //     nb. was 0x44         0101.0101   . DCA1 . DCA0
//
//    tft_write_command(0xC5); //VCOM
//    tft_write_data(0x00);
//    tft_write_data(0x00);
//    tft_write_data(0x00);
//    tft_write_data(0x00);

//
//	tft_write_command(0xB3);
//	tft_write_data(0x1F);


	tft_write_command(0xB6);        // Display Function Control
	tft_write_data(0x00);
	tft_write_data(0x22);           // 0x42 = Rotate display 180 deg.
	tft_write_data(0x3B);

	tft_write_command(0xE0);        // PGAMCTRL (Positive Gamma Control)
	tft_write_data(0x0f);
	tft_write_data(0x24);
	tft_write_data(0x1c);
	tft_write_data(0x0a);
	tft_write_data(0x0f);
	tft_write_data(0x08);
	tft_write_data(0x43);
	tft_write_data(0x88);
	tft_write_data(0x32);
	tft_write_data(0x0f);
	tft_write_data(0x10);
	tft_write_data(0x06);
	tft_write_data(0x0f);
	tft_write_data(0x07);
	tft_write_data(0x00);

	tft_write_command(0xE1);        // NGAMCTRL (Negative Gamma Control)
	tft_write_data(0x0F);
	tft_write_data(0x38);
	tft_write_data(0x30);
	tft_write_data(0x09);
	tft_write_data(0x0f);
	tft_write_data(0x0f);
	tft_write_data(0x4e);
	tft_write_data(0x77);
	tft_write_data(0x3c);
	tft_write_data(0x07);
	tft_write_data(0x10);
	tft_write_data(0x05);
	tft_write_data(0x23);
	tft_write_data(0x1b);
	tft_write_data(0x00);

	tft_write_command(0x20);        // Display Inversion OFF, 0x21 = ON

	tft_write_command(0x36);        // Memory Access Control
	tft_write_data(0x0A);

	tft_write_command(0x3A);        // Interface Pixel Format
	tft_write_data(0x55);

	tft_write_command(0x11);

	systick_Delay_ms(150);

	tft_write_command(0x29);
	systick_Delay_ms(25);


}

void setRotation(uint8_t rotation){
	tft_write_command(ILI9486_SET_ADDRESS_MODE);
	switch(rotation){
	case portrait:{
		tft_write_data(0x08);
		width = ILI9486_TFTWIDTH;
		height = ILI9486_TFTHEIGHT;
		break;
	}
	case landscape:{
		tft_write_data(0x68);
		width = ILI9486_TFTHEIGHT;
		height = ILI9486_TFTWIDTH;
		break;
	}
	case portraitInverted:{
		tft_write_data(0xD8);
		width = ILI9486_TFTWIDTH;
		height = ILI9486_TFTHEIGHT;
		break;
	}
	case landscapeInverted:{
		tft_write_data(0xA8);
		width = ILI9486_TFTHEIGHT;
		height = ILI9486_TFTWIDTH;
		break;
	}
	default:{
		tft_write_data(0x68);
		width = ILI9486_TFTHEIGHT;
		height = ILI9486_TFTWIDTH;
		break;
	}
	}
}

void fillScreen(uint16_t color){
	fillRect(0,0,width,height,color);
}

void fillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color){

	  // rudimentary clipping (drawChar w/big text requires this)
	  if ((x >= width) || (y >= height) || (w==0) || (h==0)) return;
	  if ((x + w - 1) >= width)  w = width  - x;
	  if ((y + h - 1) >= height) h = height - y;

	  setAddrWindow(x, y, x+w-1, y+h-1);

	  DDRA = color>>8;
	  DDRC = color;
	  tft_gpio_db_config();

	  if (h>w) swap(&w,&h);

	  if (w>15){
	    while(h--) {
	      x=w;
	      while (x>15) { x-=16;
	      wr_strobe();wr_strobe();wr_strobe();wr_strobe();
	      wr_strobe();wr_strobe();wr_strobe();wr_strobe();
	      wr_strobe();wr_strobe();wr_strobe();wr_strobe();
	      wr_strobe();wr_strobe();wr_strobe();wr_strobe();
	      }
	      while(x--) { wr_strobe();}
	    }
	  }
	  else {
	    while(h--) {
	      x=w;
	      while(x--) { wr_strobe();}
	    }
	  }
	  //CS off
	  GPIOC->ODR |= (SET<<4);
}

static void setAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1){
	if (x0>x1) swap(&x0,&x1);
	if (y0>y1) swap(&y0,&y1);
	//if((x1 >= _width) || (y1 >= _height)) return;
	//	  gpio_WritePin(&CS, RESET);
	//	  gpio_WritePin(&RS, RESET);
	//CS off
	GPIOC->ODR &= ~(SET<<4);
	//RS off
	GPIOB->ODR &= ~(SET<<5);

	/*
	 * Configura el controlador para escribir en la posición horizontal
	 */
	DDRC = ILI9486_SET_COLUMN_ADDRESS; tft_gpio_db_config(); wr_strobe();
	//RS on
	GPIOB->ODR |= (SET<<5);
	DDRC = x0>>8;	tft_gpio_db_config();	wr_strobe();
	DDRC = x0;		tft_gpio_db_config(); wr_strobe();
	DDRC = x1>>8;	tft_gpio_db_config(); wr_strobe();
	DDRC = x1;		tft_gpio_db_config(); wr_strobe();
	//RS off
	GPIOB->ODR &= ~(SET<<5); DDRC = ILI9486_SET_PAGE_ADDRESS; tft_gpio_db_config(); wr_strobe();
	//RS on
	GPIOB->ODR |= (SET<<5);
	DDRC = y0>>8; tft_gpio_db_config();	wr_strobe();
	DDRC = y0; tft_gpio_db_config();	wr_strobe();
	DDRC = y1>>8; tft_gpio_db_config();	wr_strobe();
	DDRC = y1; tft_gpio_db_config();	wr_strobe();
	//RS off
	GPIOB->ODR &= ~(SET<<5);
	DDRC = ILI9486_MEMORY_WRITE; tft_gpio_db_config(); wr_strobe();
	//RS on
	GPIOB->ODR |= (SET<<5);
}

void drawPixel(uint16_t x, uint16_t y, uint16_t color){
	// Faster range checking, possible because x and y are unsigned
	//  if ((x >= _width) || (y >= _height)) return;
	//CS off
	GPIOC->ODR &= ~(SET<<4);
	//RS off
	GPIOB->ODR &= ~(SET<<5);

	/*
	 * Configura el controlador para escribir en la posición horizontal
	 */
	DDRC = ILI9486_SET_COLUMN_ADDRESS; tft_gpio_db_config(); wr_strobe();
	//RS on
	GPIOB->ODR |= (SET<<5);

	//Configura las coordenadas horizontales (x) para el píxel.
	DDRC = x>>8; tft_gpio_db_config(); wr_strobe();
	DDRC = x; tft_gpio_db_config(); wr_strobe();
	DDRC = x>>8; tft_gpio_db_config(); wr_strobe();
	DDRC = x; tft_gpio_db_config(); wr_strobe();
	GPIOB->ODR &= ~(SET<<5);

	//Configura las coordenadas horizontales (x) para el píxel.
	DDRC = ILI9486_SET_PAGE_ADDRESS; tft_gpio_db_config(); wr_strobe();
	//RS on
	GPIOB->ODR |= (SET<<5);


	// Configura las coordenadas horizontales (y) para el píxel.
	DDRC = y>>8; tft_gpio_db_config(); wr_strobe();
	DDRC = y; tft_gpio_db_config(); wr_strobe();
	DDRC = y>>8; tft_gpio_db_config(); wr_strobe();
	DDRC = y; tft_gpio_db_config(); wr_strobe();
	GPIOB->ODR &= ~(SET<<5);


	// Configura el controlador para escribir en la memoria de píxeles
	DDRC = ILI9486_MEMORY_WRITE; tft_gpio_db_config();  wr_strobe();
	//RS on
	GPIOB->ODR |= (SET<<5);
	DDRC = color;
	DDRA = color>>8; tft_gpio_db_config();wr_strobe();
	GPIOC->ODR |= (SET<<4);

}
//
//static void pushColor(uint16_t color){
//
//	GPIOC->ODR &= ~(SET<<4);
//	DDRA    = color >> 8;
//	DDRC    = color;
//	wr_strobe();
//	GPIOC->ODR |= (SET<<4);
//}
//
//static void pushColorl(uint16_t color, uint16_t len){
//	GPIOC->ODR &= ~(SET<<4);
//	DDRA    = color;
//	DDRC    = color >> 8;
//	while(len) { len--;wr_strobe();}
//	GPIOC->ODR |= (SET<<4);
//}

void drawFastVLine(uint16_t x, uint16_t y, uint16_t h, uint16_t color){
		//CS off
		GPIOC->ODR &= ~(SET<<4);
		//RS off
		GPIOB->ODR &= ~(SET<<5);
		DDRC = ILI9486_SET_COLUMN_ADDRESS; tft_gpio_db_config(); wr_strobe();
		//RS on
		GPIOB->ODR |= (SET<<5);
		DDRC = x>>8;tft_gpio_db_config(); wr_strobe();
		DDRC = x; tft_gpio_db_config();wr_strobe();
		DDRC = x>>8; tft_gpio_db_config();wr_strobe();
		DDRC = x; tft_gpio_db_config();wr_strobe();
		//RS off
		GPIOB->ODR &= ~(SET<<5);

		DDRC = ILI9486_SET_PAGE_ADDRESS; tft_gpio_db_config(); wr_strobe();
		//RS on
		GPIOB->ODR |= (SET<<5);
		DDRC = y>>8; tft_gpio_db_config();wr_strobe();
		DDRC = y; tft_gpio_db_config();wr_strobe();
		y+=h-1;
		DDRC = y>>8; tft_gpio_db_config();wr_strobe();
		DDRC = y; tft_gpio_db_config();wr_strobe();
		//RS off
		GPIOB->ODR &= ~(SET<<5);
		DDRC = ILI9486_MEMORY_WRITE; tft_gpio_db_config(); wr_strobe();
		//RS on
		GPIOB->ODR |= (SET<<5);
		DDRC = color;
		DDRA = color>>8;tft_gpio_db_config(); wr_strobe();
		while (h>15) { h-=16;
			wr_strobe();wr_strobe();wr_strobe();wr_strobe();
			wr_strobe();wr_strobe();wr_strobe();wr_strobe();
			wr_strobe();wr_strobe();wr_strobe();wr_strobe();
			wr_strobe();wr_strobe();wr_strobe();wr_strobe();
		}
		while (h--) { wr_strobe();}
		GPIOC->ODR |= (SET<<4);
}

void drawFastHLine(uint16_t x, uint16_t y, uint16_t w, uint16_t color)
{
		//CS off
		GPIOC->ODR &= ~(SET<<4);
		//RS off
		GPIOB->ODR &= ~(SET<<5);
		DDRC = ILI9486_SET_COLUMN_ADDRESS; tft_gpio_db_config(); wr_strobe();
		//RS on
		GPIOB->ODR |= (SET<<5);
		DDRC = x>>8; tft_gpio_db_config();wr_strobe();
		DDRC = x; tft_gpio_db_config();wr_strobe();
		x+=w-1;
		DDRC = x>>8; tft_gpio_db_config();wr_strobe();
		DDRC = x; tft_gpio_db_config();wr_strobe();
		//RS off
		GPIOB->ODR &= ~(SET<<5);
		DDRC = ILI9486_SET_PAGE_ADDRESS; tft_gpio_db_config(); wr_strobe();
		//RS on
		GPIOB->ODR |= (SET<<5);
		DDRC = y>>8;tft_gpio_db_config(); wr_strobe();
		DDRC = y;tft_gpio_db_config(); wr_strobe();
		DDRC = y>>8; tft_gpio_db_config();wr_strobe();
		DDRC = y;tft_gpio_db_config(); wr_strobe();
		//RS off
		GPIOB->ODR &= ~(SET<<5);
		DDRC = ILI9486_MEMORY_WRITE; tft_gpio_db_config(); wr_strobe();
		//RS on
		GPIOB->ODR |= (SET<<5);
		DDRC = color;
		DDRA = color>>8;tft_gpio_db_config(); wr_strobe();

		while (w>15) { w-=16;
			wr_strobe();wr_strobe();wr_strobe();wr_strobe();
			wr_strobe();wr_strobe();wr_strobe();wr_strobe();
			wr_strobe();wr_strobe();wr_strobe();wr_strobe();
			wr_strobe();wr_strobe();wr_strobe();wr_strobe();
		}
		while (w--) { wr_strobe();}
		GPIOC->ODR |= (SET<<4);
}

void drawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color)

{
	_Bool steep = fabsl(y1 - y0) > fabsl(x1 - x0);
  if (steep) {
    swap(&x0, &y0);
    swap(&x1, &y1);
  }

  if (x0 > x1) {
    swap(&x0, &x1);
    swap(&y0, &y1);
  }

  int16_t dx = x1 - x0, dy = fabsl(y1 - y0);;

  int16_t err = dx>>1, ystep=-1, xs=x0, dlen=0;
  if (y0 < y1) ystep = 1;

	//CS off
	GPIOC->ODR &= ~(SET<<4);
  // Split into steep and not steep for FastH/V separation
  if(steep) {
    for (; x0<=x1; x0++) {
      dlen++;
      err -= dy;
      if (err < 0) {
        err += dx;

		//RS off
		GPIOB->ODR &= ~(SET<<5);
		DDRC = ILI9486_SET_COLUMN_ADDRESS; tft_gpio_db_config(); wr_strobe();
		//RS on
		GPIOB->ODR |= (SET<<5);
		DDRC = y0>>8; tft_gpio_db_config(); wr_strobe();
		DDRC = y0; tft_gpio_db_config(); wr_strobe();
		DDRC = y0>>8; tft_gpio_db_config(); wr_strobe();
		DDRC = y0; tft_gpio_db_config(); wr_strobe();
		//RS off
		GPIOB->ODR &= ~(SET<<5);
		DDRC = ILI9486_SET_PAGE_ADDRESS; tft_gpio_db_config(); wr_strobe();
		//RS on
		GPIOB->ODR |= (SET<<5);
		DDRC = xs>>8;tft_gpio_db_config(); wr_strobe();
		DDRC = xs; tft_gpio_db_config(); wr_strobe();
		DDRC = x1>>8; tft_gpio_db_config(); wr_strobe();
		DDRC = x1; tft_gpio_db_config(); wr_strobe();
		//RS off
		GPIOB->ODR &= ~(SET<<5);
		DDRC = ILI9486_MEMORY_WRITE; tft_gpio_db_config(); wr_strobe();
		//RS on
		GPIOB->ODR |= (SET<<5);
		DDRC = color;
		DDRA = color>>8;tft_gpio_db_config(); wr_strobe();

        while (dlen) { dlen--;wr_strobe();}

        y0 += ystep; xs=x0+1;
      }
    }
    if(dlen) drawFastVLine(y0, xs, dlen, color);
  }
  else
  {
    for (; x0<=x1; x0++) {
      dlen++;
      err -= dy;
      if (err < 0) {
        err += dx;

		//RS off
		GPIOB->ODR &= ~(SET<<5);
		DDRC = ILI9486_SET_COLUMN_ADDRESS; tft_gpio_db_config(); wr_strobe();
		//RS on
		GPIOB->ODR |= (SET<<5);
		DDRC = xs>>8; tft_gpio_db_config(); wr_strobe();
		DDRC = xs; tft_gpio_db_config(); wr_strobe();
		DDRC = x1>>8; tft_gpio_db_config(); wr_strobe();
		DDRC = x1; tft_gpio_db_config(); wr_strobe();
		//RS off
		GPIOB->ODR &= ~(SET<<5);
		DDRC = ILI9486_SET_PAGE_ADDRESS; tft_gpio_db_config(); wr_strobe();
		//RS on
		GPIOB->ODR |= (SET<<5);
		DDRC = y0>>8; tft_gpio_db_config(); wr_strobe();
		DDRC = y0; tft_gpio_db_config(); wr_strobe();
		DDRC = y0>>8; tft_gpio_db_config(); wr_strobe();
		DDRC = y0; tft_gpio_db_config(); wr_strobe();
		//RS off
		GPIOB->ODR &= ~(SET<<5);
		DDRC = ILI9486_MEMORY_WRITE; tft_gpio_db_config(); wr_strobe();
		//RS on
		GPIOB->ODR |= (SET<<5);
		DDRC = color;
		DDRA = color>>8;tft_gpio_db_config(); wr_strobe();

        while(dlen){ dlen--;wr_strobe();}

        y0 += ystep; xs=x0+1;
      }
    }
    if(dlen) drawFastHLine(xs, y0, dlen, color);
  }
  //CS high
	GPIOC->ODR |= (SET<<4);
}

void drawLineA(uint16_t x0, uint16_t y0, uint16_t r, float angulo, uint16_t color)
{
	uint16_t x1 = 0;
	uint16_t y1 = 0;
	x1 = (r*cosf(angulo*(M_PI/180))+x0);
	y1 = (r*sinf(angulo*(M_PI/180))+y0);

	_Bool steep = fabsl(y1 - y0) > fabsl(x1 - x0);
  if (steep) {
    swap(&x0, &y0);
    swap(&x1, &y1);
  }

  if (x0 > x1) {
    swap(&x0, &x1);
    swap(&y0, &y1);
  }

  int16_t dx = x1 - x0, dy = fabsl(y1 - y0);;

  int16_t err = dx>>1, ystep=-1, xs=x0, dlen=0;
  if (y0 < y1) ystep = 1;

	//CS off
	GPIOC->ODR &= ~(SET<<4);
  // Split into steep and not steep for FastH/V separation
  if(steep) {
    for (; x0<=x1; x0++) {
      dlen++;
      err -= dy;
      if (err < 0) {
        err += dx;

		//RS off
		GPIOB->ODR &= ~(SET<<5);
		DDRC = ILI9486_SET_COLUMN_ADDRESS; tft_gpio_db_config(); wr_strobe();
		//RS on
		GPIOB->ODR |= (SET<<5);
		DDRC = y0>>8; tft_gpio_db_config(); wr_strobe();
		DDRC = y0; tft_gpio_db_config(); wr_strobe();
		DDRC = y0>>8; tft_gpio_db_config(); wr_strobe();
		DDRC = y0; tft_gpio_db_config(); wr_strobe();
		//RS off
		GPIOB->ODR &= ~(SET<<5);
		DDRC = ILI9486_SET_PAGE_ADDRESS; tft_gpio_db_config(); wr_strobe();
		//RS on
		GPIOB->ODR |= (SET<<5);
		DDRC = xs>>8;tft_gpio_db_config(); wr_strobe();
		DDRC = xs; tft_gpio_db_config(); wr_strobe();
		DDRC = x1>>8; tft_gpio_db_config(); wr_strobe();
		DDRC = x1; tft_gpio_db_config(); wr_strobe();
		//RS off
		GPIOB->ODR &= ~(SET<<5);
		DDRC = ILI9486_MEMORY_WRITE; tft_gpio_db_config(); wr_strobe();
		//RS on
		GPIOB->ODR |= (SET<<5);
		DDRC = color;
		DDRA = color>>8;tft_gpio_db_config(); wr_strobe();

        while (dlen) { dlen--;wr_strobe();}

        y0 += ystep; xs=x0+1;
      }
    }
    if(dlen) drawFastVLine(y0, xs, dlen, color);
  }
  else
  {
    for (; x0<=x1; x0++) {
      dlen++;
      err -= dy;
      if (err < 0) {
        err += dx;

		//RS off
		GPIOB->ODR &= ~(SET<<5);
		DDRC = ILI9486_SET_COLUMN_ADDRESS; tft_gpio_db_config(); wr_strobe();
		//RS on
		GPIOB->ODR |= (SET<<5);
		DDRC = xs>>8; tft_gpio_db_config(); wr_strobe();
		DDRC = xs; tft_gpio_db_config(); wr_strobe();
		DDRC = x1>>8; tft_gpio_db_config(); wr_strobe();
		DDRC = x1; tft_gpio_db_config(); wr_strobe();
		//RS off
		GPIOB->ODR &= ~(SET<<5);
		DDRC = ILI9486_SET_PAGE_ADDRESS; tft_gpio_db_config(); wr_strobe();
		//RS on
		GPIOB->ODR |= (SET<<5);
		DDRC = y0>>8; tft_gpio_db_config(); wr_strobe();
		DDRC = y0; tft_gpio_db_config(); wr_strobe();
		DDRC = y0>>8; tft_gpio_db_config(); wr_strobe();
		DDRC = y0; tft_gpio_db_config(); wr_strobe();
		//RS off
		GPIOB->ODR &= ~(SET<<5);
		DDRC = ILI9486_MEMORY_WRITE; tft_gpio_db_config(); wr_strobe();
		//RS on
		GPIOB->ODR |= (SET<<5);
		DDRC = color;
		DDRA = color>>8;tft_gpio_db_config(); wr_strobe();

        while(dlen){ dlen--;wr_strobe();}

        y0 += ystep; xs=x0+1;
      }
    }
    if(dlen) drawFastHLine(xs, y0, dlen, color);
  }
  //CS high
	GPIOC->ODR |= (SET<<4);
}

uint16_t color565(uint8_t r, uint8_t g, uint8_t b){
	return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

void drawCircle(uint16_t x0, uint16_t y0, uint16_t r, uint16_t color){

	uint16_t fx = 0;
	uint16_t fy = 0;
	uint16_t fx1 = 0;
	uint16_t fy1 = 0;
	uint16_t fx2 = 0;
	uint16_t fy2 = 0;
	uint16_t fx3 = 0;
	uint16_t fy3 = 0;

	for(uint16_t t = 0; t<25;t++){
		fx = (r*cosf(t*(M_PI/50))+x0);
		fy = (r*sinf(t*(M_PI/50))+y0);
		fx1 = (r*cosf(t*(M_PI/50))+x0);
		fy1 = (r*sinf(-t*(M_PI/50))+y0);
		fx2 = (-r*cosf(t*(M_PI/50))+x0);
		fy2 = (r*sinf(t*(M_PI/50))+y0);
		fx3 = (-r*cosf(t*(M_PI/50))+x0);
		fy3 = (r*sinf(-t*(M_PI/50))+y0);

		drawPixel(fx, fy, color);
		drawPixel(fx1, fy1, color);
		drawPixel(fx2, fy2, color);
		drawPixel(fx3, fy3, color);
	}
}

void fillCircle(uint16_t x0, uint16_t y0, uint16_t r, uint16_t color)
{
	//Luego terminar
//	uint16_t fx = 0;
//	uint16_t fy = 0;
//	uint16_t fx1 = 0;
//	uint16_t fy1 = 0;
//	uint16_t fx2 = 0;
//	uint16_t fy2 = 0;
//	uint16_t fx3 = 0;
//	uint16_t fy3 = 0;
//
//	for(uint16_t t = 0; t<50;t++){
//		fx = (r*cosf(t*(M_PI/100))+x0);
//		fy = (r*sinf(t*(M_PI/100))+y0);
//		fx2 = (-r*cosf(t*(M_PI/100))+x0);
//		fy2 = (r*sinf(t*(M_PI/100))+y0);
//		drawPixel(fx, fy, color);
//		drawPixel(fx2, fy2, color);
//
//		fx1 = (r*cosf(t*(M_PI/100))+x0);
//		fy1 = (r*sinf(-t*(M_PI/100))+y0);
//		fx3 = (-r*cosf(t*(M_PI/100))+x0);
//		fy3 = (r*sinf(-t*(M_PI/100))+y0);
//		drawPixel(fx1, fy1, color);
//		drawPixel(fx3, fy3, color);
//
//	}
}

void drawRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
  drawFastHLine(x, y, w, color);
  drawFastHLine(x, (y + h - 1), w, color);
  drawFastVLine(x, y, h, color);
  drawFastVLine((x + w - 1), y, h, color);
}

