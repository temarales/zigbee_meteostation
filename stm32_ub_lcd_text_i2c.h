//--------------------------------------------------------------
// File     : stm32_ub_lcd_text_i2c.h
//--------------------------------------------------------------

//--------------------------------------------------------------
#ifndef __STM32F4_UB_LCD_TEXT_I2C_H
#define __STM32F4_UB_LCD_TEXT_I2C_H


//--------------------------------------------------------------
// Includes
//--------------------------------------------------------------
#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"


//--------------------------------------------------------------
// I2C Modul
//--------------------------------------------------------------
typedef enum {
  TI2C1=1,	//I2C1: SCL=[PB6, PB8] / SDA=[PB7, PB9]
  TI2C2,	//I2C2: SCL=[PB10, PF0] / SDA=[PB11, PF1]
  TI2C3,	//I2C3: SCL=[PA8] / SDA=[PC9]
}TI2C_t;

//--------------------------------------------------------------
// Struktur eines LCD Displays
//--------------------------------------------------------------
typedef struct {
	TI2C_t i2c;			// I2C Modul (1=I2C1, 2=I2C2 oder 3=I2C3)
	uint8_t slave_adr;	// I2C Adresse (z.B. 0x27)
	uint8_t maxx;		// Max. Anzahl Zeichen je Zeile (1..20)
	uint8_t maxy;		// Max Anzahl Zeilen (1..4)
	uint8_t backlight;	// LCD Hintergrundbeleuchtung (0=Aus, 1=Ein)
}LCD_TEXT_DISPLAY_t;


//--------------------------------------------------------------
// Display Modes
//--------------------------------------------------------------
typedef enum {
  TLCD_TEXT_I2C_OFF =0,    // Display=AUS, Cursor=Aus, Blinken=Aus
  TLCD_TEXT_I2C_ON,        // Display=EIN, Cursor=Aus, Blinken=Aus
  TLCD_TEXT_I2C_CURSOR,    // Display=EIN, Cursor=EIN, Blinken=Aus
  TLCD_TEXT_I2C_BLINK      // Display=EIN, Cursor=EIN, Blinken=EIN
}TLCD_TEXT_I2C_MODE_t;


//--------------------------------------------------------------
// Defines
//--------------------------------------------------------------
#define  TLCD_TEXT_I2C_INIT_PAUSE  100000  // pause beim init (>=70000)
#define  TLCD_TEXT_I2C_PAUSE        20000  // kleine Pause (>=20000)
#define  TLCD_TEXT_I2C_CLK_PAUSE      500  // pause für Clock-Impuls (>=500)


//--------------------------------------------------------------
// LCD Kommandos (siehe Datenblatt)
//--------------------------------------------------------------
#define  TLCD_TEXT_I2C_CMD_INIT_DISPLAY  0x28   // 2 Zeilen Display, 5x7 Punkte
#define  TLCD_TEXT_I2C_CMD_ENTRY_MODE    0x06   // Cursor increase, Display fix
#define  TLCD_TEXT_I2C_CMD_DISP_M0       0x08   // Display=AUS, Cursor=Aus, Blinken=Aus
#define  TLCD_TEXT_I2C_CMD_DISP_M1       0x0C   // Display=EIN, Cursor=AUS, Blinken=Aus
#define  TLCD_TEXT_I2C_CMD_DISP_M2       0x0E   // Display=EIN, Cursor=EIN, Blinken=Aus
#define  TLCD_TEXT_I2C_CMD_DISP_M3       0x0F   // Display=EIN, Cursor=EIN, Blinken=EIN 
#define  TLCD_TEXT_I2C_CMD_CLEAR         0x01   // loescht das Display



//--------------------------------------------------------------
// Globale Funktionen
//--------------------------------------------------------------
void UB_I2C_Init(LCD_TEXT_DISPLAY_t *lcd);
void UB_LCD_TEXT_I2C_Delay(uint32_t delay);
void UB_LCD_TEXT_I2C_Init(LCD_TEXT_DISPLAY_t *lcd);
void UB_LCD_TEXT_I2C_Clear(LCD_TEXT_DISPLAY_t *lcd);
void UB_LCD_TEXT_I2C_SetMode(LCD_TEXT_DISPLAY_t *lcd, TLCD_TEXT_I2C_MODE_t mode);
void UB_LCD_TEXT_I2C_String(LCD_TEXT_DISPLAY_t *lcd, uint8_t x, uint8_t y, char *ptr);
void UB_LCD_TEXT_I2C_WriteCG(LCD_TEXT_DISPLAY_t *lcd, uint8_t nr, uint8_t *pixeldata);
void UB_LCD_TEXT_I2C_PrintCG(LCD_TEXT_DISPLAY_t *lcd, uint8_t x, uint8_t y, uint8_t nr);
void UB_LCD_TEXT_I2C_Backlight_On(LCD_TEXT_DISPLAY_t *lcd);
void UB_LCD_TEXT_I2C_Backlight_Off(LCD_TEXT_DISPLAY_t *lcd);
void UB_LCD_TEXT_I2C_Backlight_Toggle(LCD_TEXT_DISPLAY_t *lcd);
uint8_t UB_LCD_TEXT_I2C_PortScan(LCD_TEXT_DISPLAY_t *lcd, uint8_t start, uint8_t ende);
//--------------------------------------------------------------
#endif // __STM32F4_UB_LCD_TEXT_I2C_H
