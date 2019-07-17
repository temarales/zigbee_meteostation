//--------------------------------------------------------------
// File     : stm32_ub_lcd_text_i2c.c
// Datum    : 24.02.2018
// Version  : 1.01
// Autor    : Manfred Becker (MB)
// EMail    : -
// Web      : http://mikrocontroller.bplaced.net/
// CPU      : STM32F407 (Discovery-Board)
// IDE      : CooCox CoIDE 1.7.8
// GCC      : 4.9 2015q2
// Module   : GPIO
//          : stm32_ub_i2c1.c (ab Version 1.3)
//          : stm32_ub_i2c2.c (ab Version 1.3)
//          : stm32_ub_i2c3.c (ab Version 1.3)
// Funktion : Diese Library dient zum Ansteuern eines Text LC-Display mit HD44780 Treiber,
//            welches seriell über I2C einem PCF8574 mit dem Mikrocontroller verbunden ist.
//            Die Anzahl Zeilen und die Anzahl Zeichen je Zeile sind konfigurierbar (8x1 bis 40x4)
//            Kompatible Text LC-Display Treiber:
//            ST7066U/HD44780/SED1278/KS0066U/S6A0069X
// 
// Hinweis 1: Folgende STM32F407 I2C Module/Pins können für die Anbindung der
//            seriellen I2C Schnittstelle verwendet werden:
//            I2C1: SCL=[PB6, PB8] / SDA=[PB7, PB9]
//            I2C2: SCL=[PB10] / SDA=[PB11]
//            I2C3: SCL=[PA8] / SDA=[PC9]
//
//            Der PCF8574-kompatible 8-Bit I/O Port-Expander hat folgende Steckerbelegung:
//            Pin	Symbol	Art				Funktion
//              1	VSS		Stromversorgung	GND (Masse)
//              2	VCC		Stromversorgung	+ 5V
//              3	SDA		Signalleitung	Datenleitung (SDA = Serial Data)
//              4	SCL		Signalleitung	Takt (SCL = Serial Clock)
//
//            Die I2C Adresse des Moduls ist herstellerspezifisch.
//            Typische Werte sind: 0x27, 0x3F, 0x4E, 0x7E.
//            Manche Module haben auch Loetbruecken für A0, A1 und A2, um abweichende
//            Adressen einstellen zu koennen. Verwende die PortScan Funktion, falls
//            die Adresse unbekannt ist.
//
// Hinweis 2: Das HD44780-kompatible Anzeigemodule hat folgende Steckerbelegung:
//            Pin	Symbol	Art				Funktion
//              1	VSS		Stromversorgung	GND (Masse)
//              2	VCC		Stromversorgung	+ 5V
//              3	VEE		Stromversorgung	Kontrastspannung
//              4	RS		Datenleitung	0 = Befehl, 1 = Daten
//              5	R/W		Datenleitung	0 = schreiben, 1 = lesen
//              6	E		Datenleitung	Enable
//              7	D0		Datenleitung	Datenleitung 0
//              8	D1		Datenleitung	Datenleitung 1
//              9	D2		Datenleitung	Datenleitung 2
//             10	D3		Datenleitung	Datenleitung 3
//             11	D4		Datenleitung	Datenleitung 4
//             12	D5		Datenleitung	Datenleitung 5
//             13	D6		Datenleitung	Datenleitung 6
//             14	D7		Datenleitung	Datenleitung 7
//             15	A		Stromversorgung	Hintergrundbeleuchtung Anode (+)
//             16	K		Stromversorgung	Hintergrundbeleuchtung Kathode (-)
//
//           Verwendete Daten- und Steuerleitungen für 4-Bit-Ansteuerung
//           Symbol		Art		Beschreibung
//             Vss		Masse	GND (Massebezug)
//             VCC		+5V		Stromversorgung
//             P0 > RS	I		0 = Befehl, 1 = Daten
//             P1 > R/W	I		0 = schreiben, 1 = lesen
//             P2 > E	I		Enable
//             P3 > A	I		Hintergrundbeleuchtung (Anode)
//             P3 > D4	I/O		Daten-Bit 0 (LSB)
//             P5 > D5	I/O		Daten-Bit 1
//             P6 > D6	I/O		Daten-Bit 2
//             P7 > D7	I/O		Daten-Bit 3 (MSB)
//
//--------------------------------------------------------------
//
// HD44780 Instruction Set
// Byte:	Instruction
// 0x01:	Clear display
// 0x02:	Return cursor to home, and un-shift display
//
// Entry mode: The following control how the cursor behaves after each character is entered	
// 0x04:	move cursor right, don’t shift display
// 0x05:	move cursor right, do shift display (left)
// 0x06:	move cursor right, don’t shift display (this is the most common)	
// 0x07:	move cursor right, do shift display (left)
//
// Display control: The following control display properties	
// 0x08-0x0B:	turn display off	
// 0x0C:	display on, cursor off,
// 0x0E:	display on, cursor on, steady cursor
// 0x0F:	display on, cursor on, blinking cursor
//
// The following commands move the cursor and shift the display	
// 0x10:	Shift cursor left
// 0x14:	Shift cursor right
// 0x18:	Shift display left
// 0x1C:	Shift display right
//
// Function set: the following commands set functions of the controller.	
// These are more advanced and with a serial controller, you won’t need to mess with them. If you do have
// to change these, follow the recipe of creating a byte from the bits shown in the table above.	
// 0x80 + position:	Cursor position.
//
//--------------------------------------------------------------

//--------------------------------------------------------------
// Includes
//--------------------------------------------------------------
#include "stm32_ub_i2c1.h"
#include "stm32_ub_i2c2.h"
#include "stm32_ub_i2c3.h"
#include "stm32_ub_lcd_text_i2c.h"

//--------------------------------------------------------------
// interne Funktionen
//--------------------------------------------------------------
void P_LCD_TEXT_I2C_InitSequenz(LCD_TEXT_DISPLAY_t *lcd);
void P_LCD_TEXT_I2C_Clk(LCD_TEXT_DISPLAY_t *lcd, uint8_t wert);
void P_LCD_TEXT_I2C_Cmd(LCD_TEXT_DISPLAY_t *lcd, uint8_t wert);
void P_LCD_TEXT_I2C_Data(LCD_TEXT_DISPLAY_t *lcd, uint8_t wert);
void P_LCD_TEXT_I2C_Cursor(LCD_TEXT_DISPLAY_t *lcd, uint8_t x, uint8_t y);
void P_LCD_TEXT_I2C_Delay(volatile uint32_t nCount);

//--------------------------------------------------------------
// Pause (ohne Timer)
// Delay :  in ms
//--------------------------------------------------------------
void UB_LCD_TEXT_I2C_Delay(uint32_t delay)
{
	uint32_t ms = delay * 14400U; //14.400 ca. 1ms
	while(ms--)
	{
	}
}

//--------------------------------------------------------------
// Init von I2C3
//-------------------------------------------------------------- 
void UB_I2C_Init(LCD_TEXT_DISPLAY_t *lcd)
{
	if (lcd->i2c==TI2C1)
		UB_I2C1_Init();
	else if (lcd->i2c==TI2C2)
		UB_I2C2_Init();
	else if (lcd->i2c==TI2C3)
		UB_I2C3_Init();
}

//--------------------------------------------------------------
// Init vom Text-LCDisplay
//--------------------------------------------------------------
void UB_LCD_TEXT_I2C_Init(LCD_TEXT_DISPLAY_t *lcd)
{
	// Inittalisierung für i2c Modul

	// kleine Pause
	P_LCD_TEXT_I2C_Delay(TLCD_TEXT_I2C_INIT_PAUSE);
	// Init Sequenz starten
	P_LCD_TEXT_I2C_InitSequenz(lcd);
	// LCD-Settings einstellen
	P_LCD_TEXT_I2C_Cmd(lcd, TLCD_TEXT_I2C_CMD_INIT_DISPLAY);
	P_LCD_TEXT_I2C_Cmd(lcd, TLCD_TEXT_I2C_CMD_ENTRY_MODE);
	// Display einschalten
	P_LCD_TEXT_I2C_Cmd(lcd, TLCD_TEXT_I2C_CMD_DISP_M1);
	// Display löschen
	P_LCD_TEXT_I2C_Cmd(lcd, TLCD_TEXT_I2C_CMD_CLEAR);
	// kleine Pause
	P_LCD_TEXT_I2C_Delay(TLCD_TEXT_I2C_PAUSE);
}

//--------------------------------------------------------------
// I2C Port Scan
//--------------------------------------------------------------
uint8_t UB_LCD_TEXT_I2C_PortScan(LCD_TEXT_DISPLAY_t *lcd, uint8_t start, uint8_t ende)
{
	uint8_t slave_adr=start;
	uint8_t port=0;
	uint16_t ret;
	uint8_t adr=0b00000000;
	if (lcd->backlight) adr=0b00001000;

	do
	{
		if (lcd->i2c==TI2C1)
			ret = UB_I2C1_WriteByte(slave_adr,adr,0xff);
		else if (lcd->i2c==TI2C2)
			ret = UB_I2C2_WriteByte(slave_adr,adr,0xff);
		else if (lcd->i2c==TI2C3)
			ret = UB_I2C3_WriteByte(slave_adr,adr,0xff);
		else
			break;
		
		if (ret == 0)
			port = slave_adr; // I2C Port gefunden!!!
		else
			slave_adr++; // naechste Adresse testen
		
	} while (ret!=0 && slave_adr<=ende);

	return port;
}

//--------------------------------------------------------------
// Hintergrundbeleuchtung an
//--------------------------------------------------------------
void UB_LCD_TEXT_I2C_Backlight_On(LCD_TEXT_DISPLAY_t *lcd)
{
	// Backlight an
	lcd->backlight = 1;
	P_LCD_TEXT_I2C_Cmd(lcd, 0);
}

//--------------------------------------------------------------
// Hintergrundbeleuchtung aus
//--------------------------------------------------------------
void UB_LCD_TEXT_I2C_Backlight_Off(LCD_TEXT_DISPLAY_t *lcd)
{
	// Backlight aus
	lcd->backlight = 0;
	P_LCD_TEXT_I2C_Cmd(lcd, 0);
}

//--------------------------------------------------------------
// Hintergrundbeleuchtung toggeln
//--------------------------------------------------------------
void UB_LCD_TEXT_I2C_Backlight_Toggle(LCD_TEXT_DISPLAY_t *lcd)
{
	// Backlight toggle
	if (lcd->backlight)
		lcd->backlight = 0;
	else
		lcd->backlight = 1;
	P_LCD_TEXT_I2C_Cmd(lcd, 0);
}

//--------------------------------------------------------------
// Löscht das Text-LCDisplay
//--------------------------------------------------------------
void UB_LCD_TEXT_I2C_Clear(LCD_TEXT_DISPLAY_t *lcd)
{
	// Display löschen
	P_LCD_TEXT_I2C_Cmd(lcd, TLCD_TEXT_I2C_CMD_CLEAR);
	// kleine Pause
	P_LCD_TEXT_I2C_Delay(TLCD_TEXT_I2C_PAUSE);
}

//--------------------------------------------------------------
// Stellt einen Display Mode ein
// mode : [TLCD_TEXT_I2C_OFF, TLCD_TEXT_I2C_ON, TLCD_TEXT_I2C_CURSOR, TLCD_TEXT_I2C_BLINK]
//--------------------------------------------------------------
void UB_LCD_TEXT_I2C_SetMode(LCD_TEXT_DISPLAY_t *lcd, TLCD_TEXT_I2C_MODE_t mode)
{
	if(mode==TLCD_TEXT_I2C_OFF) P_LCD_TEXT_I2C_Cmd(lcd, TLCD_TEXT_I2C_CMD_DISP_M0);
	if(mode==TLCD_TEXT_I2C_ON) P_LCD_TEXT_I2C_Cmd(lcd, TLCD_TEXT_I2C_CMD_DISP_M1);
	if(mode==TLCD_TEXT_I2C_CURSOR) P_LCD_TEXT_I2C_Cmd(lcd, TLCD_TEXT_I2C_CMD_DISP_M2);
	if(mode==TLCD_TEXT_I2C_BLINK) P_LCD_TEXT_I2C_Cmd(lcd, TLCD_TEXT_I2C_CMD_DISP_M3);
}

//--------------------------------------------------------------
// Ausgabe von einem String auf dem Display an x,y Position
// x : 0 bis 39
// y : 0 bis 3
//--------------------------------------------------------------
void UB_LCD_TEXT_I2C_String(LCD_TEXT_DISPLAY_t *lcd, uint8_t x, uint8_t y, char *ptr)
{
	// Cursor setzen
	P_LCD_TEXT_I2C_Cursor(lcd, x, y);
	// kompletten String ausgeben
	while (*ptr != 0)
	{
		P_LCD_TEXT_I2C_Data(lcd, *ptr);
		ptr++;
	}
}

//--------------------------------------------------------------
// Speichern eines Sonderzeichens im CG-RAM vom Display
// nr : 0 bis 7 (nummer der sonderzeichens)
// pixeldata : 8 bytes mit Pixeldaten für das Zeichen
//--------------------------------------------------------------
void UB_LCD_TEXT_I2C_WriteCG(LCD_TEXT_DISPLAY_t *lcd, uint8_t nr, uint8_t *pixeldata)
{
	uint8_t n;

	if(nr>7) nr=7;

	nr=(nr<<3);
	nr|=0x40;
	P_LCD_TEXT_I2C_Cmd(lcd, nr);
	for(n=0;n<8;n++)
	{
		P_LCD_TEXT_I2C_Data(lcd, pixeldata[n]);
	}
}

//--------------------------------------------------------------
// Ausgabe von einem Sonderzeichen auf dem Display an x,y Position
// x : 0 bis 15
// y : 0 bis 1
// nr : 0 bis 7 (nummer der sonderzeichens)
//--------------------------------------------------------------
void UB_LCD_TEXT_I2C_PrintCG(LCD_TEXT_DISPLAY_t *lcd, uint8_t x, uint8_t y, uint8_t nr)
{
	if(nr>7) nr=7;

	// Cursor setzen
	P_LCD_TEXT_I2C_Cursor(lcd, x, y);
	P_LCD_TEXT_I2C_Data(lcd, nr);
}

//--------------------------------------------------------------
// interne Funktion
// einen Clock Impuls ausgeben
//--------------------------------------------------------------
void P_LCD_TEXT_I2C_Clk(LCD_TEXT_DISPLAY_t *lcd, uint8_t wert)
{
	int16_t ret_wert;
	uint8_t byte;
	uint8_t adr=0b00000000;
	if (lcd->backlight) adr=0b00001000;

	// Pin-E auf Hi > Datenbit D2=1
	byte = wert | 0b00000100;
	if (lcd->i2c==TI2C1)
		ret_wert = UB_I2C1_WriteByte(lcd->slave_adr, adr, byte);
	else if (lcd->i2c==TI2C2)
		ret_wert = UB_I2C2_WriteByte(lcd->slave_adr, adr, byte);
	else if (lcd->i2c==TI2C3)
		ret_wert = UB_I2C3_WriteByte(lcd->slave_adr, adr, byte);

	// kleine Pause
	P_LCD_TEXT_I2C_Delay(TLCD_TEXT_I2C_CLK_PAUSE);

	// Pin-E auf Lo > Datenbit D2
	byte = wert & 0b11111011;
	if (lcd->i2c==TI2C1)
		ret_wert = UB_I2C1_WriteByte(lcd->slave_adr, adr, byte);
	else if (lcd->i2c==TI2C2)
		ret_wert = UB_I2C2_WriteByte(lcd->slave_adr, adr, byte);
	else if (lcd->i2c==TI2C3)
		ret_wert = UB_I2C3_WriteByte(lcd->slave_adr, adr, byte);

	// kleine Pause
	P_LCD_TEXT_I2C_Delay(TLCD_TEXT_I2C_CLK_PAUSE);
}

//--------------------------------------------------------------
// interne Funktion
// init Sequenz für das Display
//--------------------------------------------------------------
void P_LCD_TEXT_I2C_InitSequenz(LCD_TEXT_DISPLAY_t *lcd)
{
	uint8_t wert;

	// Init Sequenz
	// Erster Init Impuls
	if (lcd->backlight)
		wert = 0b00111000;
	else
		wert = 0b00110000;
	P_LCD_TEXT_I2C_Clk(lcd, wert);
	P_LCD_TEXT_I2C_Delay(TLCD_TEXT_I2C_PAUSE);

	// Zweiter Init Impuls
	P_LCD_TEXT_I2C_Clk(lcd, wert);
	P_LCD_TEXT_I2C_Delay(TLCD_TEXT_I2C_PAUSE);

	// Dritter Init Impuls
	P_LCD_TEXT_I2C_Clk(lcd, wert);
	P_LCD_TEXT_I2C_Delay(TLCD_TEXT_I2C_PAUSE);

	// LCD-Modus einstellen (4Bit-Mode)
	if (lcd->backlight)
		wert = 0b00101000;
	else
		wert = 0b00100000;
	P_LCD_TEXT_I2C_Clk(lcd, wert);
	P_LCD_TEXT_I2C_Delay(TLCD_TEXT_I2C_PAUSE);
}

//--------------------------------------------------------------
// interne Funktion
// Kommando an das Display senden
//--------------------------------------------------------------
void P_LCD_TEXT_I2C_Cmd(LCD_TEXT_DISPLAY_t *lcd, uint8_t wert)
{
	uint8_t lobyte, hibyte;

	// RS=Lo (Command)
	if (lcd->backlight)
		hibyte = (wert & 0xf0) | 0b00001000;
	else
		hibyte = (wert & 0xf0) | 0b00000000;
	P_LCD_TEXT_I2C_Clk(lcd, hibyte);
	
	// Lo-Nibble ausgeben
	if (lcd->backlight)
		lobyte = ((wert << 4) & 0xf0) | 0b00001000;
	else
		lobyte = ((wert << 4) & 0xf0) | 0b00000000;
	P_LCD_TEXT_I2C_Clk(lcd, lobyte);
}

//--------------------------------------------------------------
// interne Funktion
// Daten an das Display senden
//--------------------------------------------------------------
void P_LCD_TEXT_I2C_Data(LCD_TEXT_DISPLAY_t *lcd, uint8_t wert)
{
	uint8_t lobyte, hibyte;

	// Hi-Nibble ausgeben
	if (lcd->backlight)
		hibyte = (wert & 0xf0)  | 0b00001001;
	else
		hibyte = (wert & 0xf0)  | 0b00000001;
	P_LCD_TEXT_I2C_Clk(lcd, hibyte);
	
	// Lo-Nibble ausgeben
	if (lcd->backlight)
		lobyte = ((wert << 4) & 0xf0) | 0b00001001;
	else
		lobyte = ((wert << 4) & 0xf0) | 0b00000001;
	P_LCD_TEXT_I2C_Clk(lcd, lobyte);
}

//--------------------------------------------------------------
// interne Funktion
// Cursor auf x,y stellen
//--------------------------------------------------------------
void P_LCD_TEXT_I2C_Cursor(LCD_TEXT_DISPLAY_t *lcd, uint8_t x, uint8_t y)
{
	// Cursor position: 0x80 + position
	uint8_t wert;

	if(x>=lcd->maxx) x=0;
	if(y>=lcd->maxy) y=0;

	if(y==0)
		wert = 0x80 + 0x00 + x;
	else if(y==1)
		wert = 0x80 + 0x40 + x;
	else if(y==2)
		wert = 0x80 + 0x14 + x;
	else if(y==3)
		wert = 0x80 + 0x54 + x;
	else
		wert = 0x80 + 0x00 + x; //handle undefined y value!

	P_LCD_TEXT_I2C_Cmd(lcd, wert);
}

//--------------------------------------------------------------
// kleine Pause (ohne Timer)
//--------------------------------------------------------------
void P_LCD_TEXT_I2C_Delay(volatile uint32_t nCount)
{
  while(nCount--)
  {
  }
}
