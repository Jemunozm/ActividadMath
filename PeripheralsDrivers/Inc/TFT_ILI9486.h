/*
 * TFT_ILI9486.h
 *
 *  Created on: Nov 14, 2023
 *      Author: imjeviz
 */
#include "stm32f4xx.h"
#include "spi_driver_hal.h"

#ifndef TFT_ILI9486_H_
#define TFT_ILI9486_H_



// Comandos de inicialización
#define ILI9486_SOFT_RESET             0x01
#define ILI9486_SLEEP_OUT              0x11
#define ILI9486_DISPLAY_ON             0x29
#define ILI9486_MEMORY_ACCESS_CONTROL  0x36

// Configuración de color
#define ILI9486_PIXEL_FORMAT           0x3A
#define ILI9486_INTERFACE_PIXEL_FORMAT 0x3A
#define ILI9486_SET_COLUMN_ADDRESS     0x2A
#define ILI9486_SET_PAGE_ADDRESS       0x2B
#define ILI9486_MEMORY_WRITE           0x2C

// Configuración de orientación
#define ILI9486_MADCTL                 0x36

// Ajuste de brillo y contraste
#define ILI9486_SET_PWM                0xB0

// Ajustes de energía
#define ILI9486_SET_POWER_CONTROL      0xB1
#define ILI9486_SET_VCOM_CONTROL       0xB6

// Configuración del panel
#define ILI9486_PANEL_DRIVING      	   0xC0
#define ILI9486_FRAME_RATE_CONTROL     0xB3

// Ajustes de gamma
#define ILI9486_SET_GAMMA_CORRECTION   0xE0
#define ILI9486_SET_GAMMA_CORRECTION2  0xE1

// Otros comandos
#define ILI9486_NOP                    0x00
#define ILI9486_ENTER_SLEEP            0x10
#define ILI9486_EXIT_SLEEP             0x38
#define ILI9486_SET_TEAR_OFF           0x34
#define ILI9486_SET_TEAR_ON            0x35
#define ILI9486_SET_ADDRESS_MODE       0x36
#define ILI9486_SET_DISPLAY_OFF        0x28

// Comandos de lectura
#define ILI9486_READ_ID1               0xDA
#define ILI9486_READ_ID2               0xDB
#define ILI9486_READ_ID3               0xDC

// Comandos de retroiluminación
#define ILI9486_SET_BACKLIGHT          0x51

// Comandos de temperatura
#define ILI9486_SET_TEMPERATURE        0x55

// Ancho y Largo de la pantalla
#define ILI9486_TFTWIDTH  320
#define ILI9486_TFTHEIGHT 480

#define CLIP_CHECK


#define TFT_BLACK       0x0000
#define TFT_NAVY        0x000F
#define TFT_DARKGREEN   0x03E0
#define TFT_DARKCYAN    0x03EF
#define TFT_MAROON      0x7800
#define TFT_PURPLE      0x780F
#define TFT_OLIVE       0x7BE0
#define TFT_LIGHTGREY   0xC618
#define TFT_DARKGREY    0x7BEF
#define TFT_BLUE        0x001F
#define TFT_GREEN       0x07E0
#define TFT_CYAN        0x07FF
#define TFT_RED         0xF800
#define TFT_MAGENTA     0xF81F
#define TFT_YELLOW      0xFFE0
#define TFT_WHITE       0xFFFF
#define TFT_ORANGE      0xFD20
#define TFT_GREENYELLOW 0xAFE5
#define TFT_PINK        0xF81F


enum{
	portrait = 0,
	landscape,
	portraitInverted,
	landscapeInverted
};

void tft_Config(void);
void setRotation(uint8_t rotation);
void fillScreen(uint16_t color);
uint16_t color565(uint8_t r, uint8_t g, uint8_t b);
void drawCircle(uint16_t x0, uint16_t y0, uint16_t r, uint16_t color);
void drawPixel(uint16_t x, uint16_t y, uint16_t color);
void drawFastHLine(uint16_t x, uint16_t y, uint16_t h, uint16_t color);
void drawFastVLine(uint16_t x, uint16_t y, uint16_t h, uint16_t color);
void fillCircle(uint16_t x0, uint16_t y0, uint16_t r, uint16_t color);
void fillCircleHelper(uint16_t x0, uint16_t y0, uint16_t r, uint8_t cornername, uint16_t delta, uint16_t color);
void drawRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void fillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void drawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color);
void drawLineA(uint16_t x0, uint16_t y0, uint16_t r, float angulo, uint16_t color);

#endif /* TFT_ILI9486_H_ */
