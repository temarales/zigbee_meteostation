//--------------------------------------------------------------
// File     : stm32_ub_i2c1.c
// Datum    : 06.02.2016
// Version  : 1.3
// Autor    : UB
// EMail    : mc-4u(@)t-online.de
// Web      : www.mikrocontroller-4u.de
// CPU      : STM32F4
// IDE      : CooCox CoIDE 1.7.4
// GCC      : 4.7 2012q4
// Module   : GPIO, I2C 
// Funktion : I2C-LoLevel-Funktionen (I2C-1)
//
// Hinweis  : m�gliche Pinbelegungen
//            I2C1 : SCL :[PB6, PB8] 
//                   SDA :[PB7, PB9]
//            externe PullUp-Widerst�nde an SCL+SDA notwendig
//--------------------------------------------------------------


//--------------------------------------------------------------
// Includes
//--------------------------------------------------------------
#include "stm32_ub_i2c1.h"


//--------------------------------------------------------------
// interne Funktionen
//--------------------------------------------------------------
void P_I2C1_InitI2C(void);
int16_t P_I2C1_timeout(int16_t wert);



//--------------------------------------------------------------
// Definition von I2C1
//--------------------------------------------------------------
I2C1_DEV_t I2C1DEV = {
// PORT , PIN      , Clock              , Source 
  {GPIOB,GPIO_Pin_6,RCC_AHB1Periph_GPIOB,GPIO_PinSource6}, // SCL an PB6
  {GPIOB,GPIO_Pin_7,RCC_AHB1Periph_GPIOB,GPIO_PinSource7}, // SDA an PB7
};



//--------------------------------------------------------------
// Init von I2C1
//-------------------------------------------------------------- 
void UB_I2C1_Init(void)
{
  static uint8_t init_ok=0;
  GPIO_InitTypeDef  GPIO_InitStructure;

  // initialisierung darf nur einmal gemacht werden
  if(init_ok!=0) {
    return;
  } 

  // I2C-Clock enable
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);

  // Clock Enable der Pins
  RCC_AHB1PeriphClockCmd(I2C1DEV.SCL.CLK, ENABLE); 
  RCC_AHB1PeriphClockCmd(I2C1DEV.SDA.CLK, ENABLE);

  // I2C reset
  RCC_APB1PeriphResetCmd(RCC_APB1Periph_I2C1, ENABLE);
  RCC_APB1PeriphResetCmd(RCC_APB1Periph_I2C1, DISABLE);

  // I2C Alternative-Funktions mit den IO-Pins verbinden  
  GPIO_PinAFConfig(I2C1DEV.SCL.PORT, I2C1DEV.SCL.SOURCE, GPIO_AF_I2C1); 
  GPIO_PinAFConfig(I2C1DEV.SDA.PORT, I2C1DEV.SDA.SOURCE, GPIO_AF_I2C1);

  // I2C als Alternative-Funktion als OpenDrain  
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;

  // SCL-Pin
  GPIO_InitStructure.GPIO_Pin = I2C1DEV.SCL.PIN;
  GPIO_Init(I2C1DEV.SCL.PORT, &GPIO_InitStructure);
  // SDA-Pin
  GPIO_InitStructure.GPIO_Pin = I2C1DEV.SDA.PIN;
  GPIO_Init(I2C1DEV.SDA.PORT, &GPIO_InitStructure);

  // I2C init
  P_I2C1_InitI2C();

  // init Mode speichern
  init_ok=1;
}


//--------------------------------------------------------------
// Auslesen einer Adresse per I2C von einem Slave
// slave_adr => I2C-Basis-Adresse vom Slave
// adr       => Register Adresse die gelesen wird
//
// Return_wert :
//  0...255 , Bytewert der gelesen wurde
//  < 0     , Error
//--------------------------------------------------------------
int16_t UB_I2C1_ReadByte(uint8_t slave_adr, uint8_t adr)
{
  int16_t ret_wert=0;
  uint32_t timeout=I2C1_TIMEOUT;

  // Start-Sequenz
  I2C_GenerateSTART(I2C1, ENABLE);
  
  timeout=I2C1_TIMEOUT;
  while (!I2C_GetFlagStatus(I2C1, I2C_FLAG_SB)) {
    if(timeout!=0) timeout--; else return(P_I2C1_timeout(-1));
  }

  // ACK disable
  I2C_AcknowledgeConfig(I2C1, DISABLE);

  // Slave-Adresse senden (write)
  I2C_Send7bitAddress(I2C1, slave_adr, I2C_Direction_Transmitter); 

  timeout=I2C1_TIMEOUT;
  while (!I2C_GetFlagStatus(I2C1, I2C_FLAG_ADDR)) {
    if(timeout!=0) timeout--; else return(P_I2C1_timeout(-2));
  }  

  // ADDR-Flag l�schen
  I2C1->SR2;

  timeout=I2C1_TIMEOUT;
  while (!I2C_GetFlagStatus(I2C1, I2C_FLAG_TXE)) {
    if(timeout!=0) timeout--; else return(P_I2C1_timeout(-3));
  }  

  // Adresse senden
  I2C_SendData(I2C1, adr);

  timeout=I2C1_TIMEOUT;
  while ((!I2C_GetFlagStatus(I2C1, I2C_FLAG_TXE)) || (!I2C_GetFlagStatus(I2C1, I2C_FLAG_BTF))) {
    if(timeout!=0) timeout--; else return(P_I2C1_timeout(-4));
  }

  // Start-Sequenz
  I2C_GenerateSTART(I2C1, ENABLE);

  timeout=I2C1_TIMEOUT;
  while (!I2C_GetFlagStatus(I2C1, I2C_FLAG_SB)) {
    if(timeout!=0) timeout--; else return(P_I2C1_timeout(-5));
  }

  // Slave-Adresse senden (read)
  I2C_Send7bitAddress(I2C1, slave_adr, I2C_Direction_Receiver);

  timeout=I2C1_TIMEOUT;
  while (!I2C_GetFlagStatus(I2C1, I2C_FLAG_ADDR)) {
    if(timeout!=0) timeout--; else return(P_I2C1_timeout(-6));
  }

  // ADDR-Flag l�schen
  I2C1->SR2;

  timeout=I2C1_TIMEOUT;
  while (!I2C_GetFlagStatus(I2C1, I2C_FLAG_RXNE)) {
    if(timeout!=0) timeout--; else return(P_I2C1_timeout(-7));
  } 

  // Stop-Sequenz
  I2C_GenerateSTOP(I2C1, ENABLE);

  // Daten auslesen
  ret_wert=(int16_t)(I2C_ReceiveData(I2C1));

  // ACK enable
  I2C_AcknowledgeConfig(I2C1, ENABLE);

  return(ret_wert);
}


//--------------------------------------------------------------
// Beschreiben einer Adresse per I2C von einem Slave
// slave_adr => I2C-Basis-Adresse vom Slave
// adr       => Register Adresse die beschrieben wird
// wert      => Bytewert der geschrieben wird
//
// Return_wert :
//    0   , Ok
//  < 0   , Error
//--------------------------------------------------------------
int16_t UB_I2C1_WriteByte(uint8_t slave_adr, uint8_t adr, uint8_t wert)
{
  int16_t ret_wert=0;
  uint32_t timeout=I2C1_TIMEOUT;

  // Start-Sequenz
  I2C_GenerateSTART(I2C1, ENABLE); 

  timeout=I2C1_TIMEOUT;
  while (!I2C_GetFlagStatus(I2C1, I2C_FLAG_SB)) {
    if(timeout!=0) timeout--; else return(P_I2C1_timeout(-1));
  } 

  // Slave-Adresse senden (write)
  I2C_Send7bitAddress(I2C1, slave_adr, I2C_Direction_Transmitter);

  timeout=I2C1_TIMEOUT;
  while (!I2C_GetFlagStatus(I2C1, I2C_FLAG_ADDR)) {
    if(timeout!=0) timeout--; else return(P_I2C1_timeout(-2));
  }  

  // ADDR-Flag l�schen
  I2C1->SR2;

  timeout=I2C1_TIMEOUT;
  while (!I2C_GetFlagStatus(I2C1, I2C_FLAG_TXE)) {
    if(timeout!=0) timeout--; else return(P_I2C1_timeout(-3));
  }

  // Adresse senden
  I2C_SendData(I2C1, adr);

  timeout=I2C1_TIMEOUT;
  while (!I2C_GetFlagStatus(I2C1, I2C_FLAG_TXE)) {
    if(timeout!=0) timeout--; else return(P_I2C1_timeout(-4));
  }

  // Daten senden
  I2C_SendData(I2C1, wert);

  timeout=I2C1_TIMEOUT;
  while ((!I2C_GetFlagStatus(I2C1, I2C_FLAG_TXE)) || (!I2C_GetFlagStatus(I2C1, I2C_FLAG_BTF))) {
    if(timeout!=0) timeout--; else return(P_I2C1_timeout(-5));
  }

  // Stop-Sequenz
  I2C_GenerateSTOP(I2C1, ENABLE);

  ret_wert=0; // alles ok

  return(ret_wert);
}


//--------------------------------------------------------------
// Auslesen mehrerer Adresse per I2C von einem Slave
// slave_adr => I2C-Basis-Adresse vom Slave
// adr       => Start Register Adresse ab der gelesen wird
// cnt       => Anzahl der Bytewert die gelesen werden sollen
// Daten die gelesen worden sind, stehen danach in "I2C1_DATA"
//
// Return_wert :
//    0   , Ok
//  < 0   , Error
//--------------------------------------------------------------
int16_t UB_I2C1_ReadMultiByte(uint8_t slave_adr, uint8_t adr, uint8_t cnt)
{
  int16_t ret_wert=0;
  uint32_t timeout=I2C1_TIMEOUT;
  uint8_t wert,n;

  if(cnt==0) return(-8);
  if(cnt>I2C1_MULTIBYTE_ANZ) return(-9);

  // Start-Sequenz
  I2C_GenerateSTART(I2C1, ENABLE);

  timeout=I2C1_TIMEOUT;
  while (!I2C_GetFlagStatus(I2C1, I2C_FLAG_SB)) {
    if(timeout!=0) timeout--; else return(P_I2C1_timeout(-1));
  }

  if(cnt==1) {
    // ACK disable
    I2C_AcknowledgeConfig(I2C1, DISABLE);
  }
  else {
	// ACK enable
	I2C_AcknowledgeConfig(I2C1, ENABLE);
  }

  // Slave-Adresse senden (write)
  I2C_Send7bitAddress(I2C1, slave_adr, I2C_Direction_Transmitter);

  timeout=I2C1_TIMEOUT;
  while (!I2C_GetFlagStatus(I2C1, I2C_FLAG_ADDR)) {
    if(timeout!=0) timeout--; else return(P_I2C1_timeout(-2));
  }

  // ADDR-Flag l�schen
  I2C1->SR2;

  timeout=I2C1_TIMEOUT;
  while (!I2C_GetFlagStatus(I2C1, I2C_FLAG_TXE)) {
    if(timeout!=0) timeout--; else return(P_I2C1_timeout(-3));
  }

  // Adresse senden
  I2C_SendData(I2C1, adr);

  timeout=I2C1_TIMEOUT;
  while ((!I2C_GetFlagStatus(I2C1, I2C_FLAG_TXE)) || (!I2C_GetFlagStatus(I2C1, I2C_FLAG_BTF))) {
    if(timeout!=0) timeout--; else return(P_I2C1_timeout(-4));
  }

  // Start-Sequenz
  I2C_GenerateSTART(I2C1, ENABLE);

  timeout=I2C1_TIMEOUT;
  while (!I2C_GetFlagStatus(I2C1, I2C_FLAG_SB)) {
    if(timeout!=0) timeout--; else return(P_I2C1_timeout(-5));
  }

  // Slave-Adresse senden (read)
  I2C_Send7bitAddress(I2C1, slave_adr, I2C_Direction_Receiver);

  timeout=I2C1_TIMEOUT;
  while (!I2C_GetFlagStatus(I2C1, I2C_FLAG_ADDR)) {
    if(timeout!=0) timeout--; else return(P_I2C1_timeout(-6));
  }

  // ADDR-Flag l�schen
  I2C1->SR2;

  // alle Daten auslesen
  for(n=0;n<cnt;n++) {

    if((n+1)>=cnt) {
      // ACK disable
      I2C_AcknowledgeConfig(I2C1, DISABLE);
      // Stop-Sequenz
      I2C_GenerateSTOP(I2C1, ENABLE);
    }

    timeout=I2C1_TIMEOUT;
    while (!I2C_GetFlagStatus(I2C1, I2C_FLAG_RXNE)) {
      if(timeout!=0) timeout--; else return(P_I2C1_timeout(-7));
    }

    // Daten auslesen
    wert=I2C_ReceiveData(I2C1);

    // Daten in Array speichern
    I2C1_DATA[n]=wert;
  }

  // ACK enable
  I2C_AcknowledgeConfig(I2C1, ENABLE);

  ret_wert=0; // alles ok

  return(ret_wert);
}


//--------------------------------------------------------------
// Beschreiben mehrerer Adresse per I2C von einem Slave
// slave_adr => I2C-Basis-Adresse vom Slave
// adr       => Start Register Adresse ab der beschrieben wird
// cnt       => Anzahl der Bytewert die geschrieben werden sollen
// Daten die geschrieben werden sollen, m�ssen in "I2C1_DATA" stehen
//
// Return_wert :
//    0   , Ok
//  < 0   , Error
//--------------------------------------------------------------
int16_t UB_I2C1_WriteMultiByte(uint8_t slave_adr, uint8_t adr, uint8_t cnt)
{
  int16_t ret_wert=0;
  uint32_t timeout=I2C1_TIMEOUT;
  uint8_t wert,n;

  if(cnt==0) return(-6);
  if(cnt>I2C1_MULTIBYTE_ANZ) return(-7);

  // Start-Sequenz
  I2C_GenerateSTART(I2C1, ENABLE);

  timeout=I2C1_TIMEOUT;
  while (!I2C_GetFlagStatus(I2C1, I2C_FLAG_SB)) {
    if(timeout!=0) timeout--; else return(P_I2C1_timeout(-1));
  }

  // Slave-Adresse senden (write)
  I2C_Send7bitAddress(I2C1, slave_adr, I2C_Direction_Transmitter);

  timeout=I2C1_TIMEOUT;
  while (!I2C_GetFlagStatus(I2C1, I2C_FLAG_ADDR)) {
    if(timeout!=0) timeout--; else return(P_I2C1_timeout(-2));
  }

  // ADDR-Flag l�schen
  I2C1->SR2;

  timeout=I2C1_TIMEOUT;
  while (!I2C_GetFlagStatus(I2C1, I2C_FLAG_TXE)) {
    if(timeout!=0) timeout--; else return(P_I2C1_timeout(-3));
  }

  // Adresse senden
  I2C_SendData(I2C1, adr);

  timeout=I2C1_TIMEOUT;
  while (!I2C_GetFlagStatus(I2C1, I2C_FLAG_TXE)) {
    if(timeout!=0) timeout--; else return(P_I2C1_timeout(-4));
  }

  // alle Daten senden
  for(n=0;n<cnt;n++) {
    // Daten aus Array lesen
    wert=I2C1_DATA[n];

    // Daten senden
    I2C_SendData(I2C1, wert);

    timeout=I2C1_TIMEOUT;
    while ((!I2C_GetFlagStatus(I2C1, I2C_FLAG_TXE)) || (!I2C_GetFlagStatus(I2C1, I2C_FLAG_BTF))) {
      if(timeout!=0) timeout--; else return(P_I2C1_timeout(-5));
    }
  }

  // Stop-Sequenz
  I2C_GenerateSTOP(I2C1, ENABLE);

  ret_wert=0; // alles ok

  return(ret_wert);
}


//--------------------------------------------------------------
// Schreiben eines Kommandos per I2C an einen Slave
// slave_adr => I2C-Basis-Adresse vom Slave
// cmd       => Kommando das gesendet wird
//
// Return_wert :
//    0   , Ok
//  < 0   , Error
//--------------------------------------------------------------
int16_t UB_I2C1_WriteCMD(uint8_t slave_adr, uint8_t cmd)
{
  int16_t ret_wert=0;
  uint32_t timeout=I2C1_TIMEOUT;

  // Start-Sequenz
  I2C_GenerateSTART(I2C1, ENABLE);

  timeout=I2C1_TIMEOUT;
  while (!I2C_GetFlagStatus(I2C1, I2C_FLAG_SB)) {
    if(timeout!=0) timeout--; else return(P_I2C1_timeout(-1));
  }

  // Slave-Adresse senden (write)
  I2C_Send7bitAddress(I2C1, slave_adr, I2C_Direction_Transmitter);

  timeout=I2C1_TIMEOUT;
  while (!I2C_GetFlagStatus(I2C1, I2C_FLAG_ADDR)) {
    if(timeout!=0) timeout--; else return(P_I2C1_timeout(-2));
  }

  // ADDR-Flag l�schen
  I2C1->SR2;

  timeout=I2C1_TIMEOUT;
  while (!I2C_GetFlagStatus(I2C1, I2C_FLAG_TXE)) {
    if(timeout!=0) timeout--; else return(P_I2C1_timeout(-3));
  }

  // CMD senden
  I2C_SendData(I2C1, cmd);

  timeout=I2C1_TIMEOUT;
  while ((!I2C_GetFlagStatus(I2C1, I2C_FLAG_TXE)) || (!I2C_GetFlagStatus(I2C1, I2C_FLAG_BTF))) {
    if(timeout!=0) timeout--; else return(P_I2C1_timeout(-4));
  }

  // Stop-Sequenz
  I2C_GenerateSTOP(I2C1, ENABLE);

  ret_wert=0; // alles ok

  return(ret_wert);
}

//--------------------------------------------------------------
// Auslesen einer Adresse (16bit) per I2C von einem Slave
// slave_adr => I2C-Basis-Adresse vom Slave
// adr       => Register Adresse die gelesen wird
//
// Return_wert :
//  0...255 , Bytewert der gelesen wurde
//  < 0     , Error
//--------------------------------------------------------------
int16_t UB_I2C1_ReadByte16(uint8_t slave_adr, uint16_t adr)
{
  int16_t ret_wert=0;
  uint32_t timeout=I2C1_TIMEOUT;
  uint8_t lo,hi;
  
  // Hi und Lo Adresse
  lo=(adr&0x00FF);
  hi=(adr>>8);

  // Start-Sequenz
  I2C_GenerateSTART(I2C1, ENABLE);
  
  timeout=I2C1_TIMEOUT;
  while (!I2C_GetFlagStatus(I2C1, I2C_FLAG_SB)) {
    if(timeout!=0) timeout--; else return(P_I2C1_timeout(-1));
  }

  // ACK disable
  I2C_AcknowledgeConfig(I2C1, DISABLE);

  // Slave-Adresse senden (write)
  I2C_Send7bitAddress(I2C1, slave_adr, I2C_Direction_Transmitter); 

  timeout=I2C1_TIMEOUT;
  while (!I2C_GetFlagStatus(I2C1, I2C_FLAG_ADDR)) {
    if(timeout!=0) timeout--; else return(P_I2C1_timeout(-2));
  }  

  // ADDR-Flag l�schen
  I2C1->SR2;

  timeout=I2C1_TIMEOUT;
  while (!I2C_GetFlagStatus(I2C1, I2C_FLAG_TXE)) {
    if(timeout!=0) timeout--; else return(P_I2C1_timeout(-3));
  }  

  // Adresse (hi) senden
  I2C_SendData(I2C1, hi);

  timeout=I2C1_TIMEOUT;
  while ((!I2C_GetFlagStatus(I2C1, I2C_FLAG_TXE)) || (!I2C_GetFlagStatus(I2C1, I2C_FLAG_BTF))) {
    if(timeout!=0) timeout--; else return(P_I2C1_timeout(-4));
  }
  
  // Adresse (lo) senden
  I2C_SendData(I2C1, lo);

  timeout=I2C1_TIMEOUT;
  while ((!I2C_GetFlagStatus(I2C1, I2C_FLAG_TXE)) || (!I2C_GetFlagStatus(I2C1, I2C_FLAG_BTF))) {
    if(timeout!=0) timeout--; else return(P_I2C1_timeout(-4));
  }  

  // Start-Sequenz
  I2C_GenerateSTART(I2C1, ENABLE);

  timeout=I2C1_TIMEOUT;
  while (!I2C_GetFlagStatus(I2C1, I2C_FLAG_SB)) {
    if(timeout!=0) timeout--; else return(P_I2C1_timeout(-5));
  }

  // Slave-Adresse senden (read)
  I2C_Send7bitAddress(I2C1, slave_adr, I2C_Direction_Receiver);

  timeout=I2C1_TIMEOUT;
  while (!I2C_GetFlagStatus(I2C1, I2C_FLAG_ADDR)) {
    if(timeout!=0) timeout--; else return(P_I2C1_timeout(-6));
  }

  // ADDR-Flag l�schen
  I2C1->SR2;

  timeout=I2C1_TIMEOUT;
  while (!I2C_GetFlagStatus(I2C1, I2C_FLAG_RXNE)) {
    if(timeout!=0) timeout--; else return(P_I2C1_timeout(-7));
  } 

  // Stop-Sequenz
  I2C_GenerateSTOP(I2C1, ENABLE);

  // Daten auslesen
  ret_wert=(int16_t)(I2C_ReceiveData(I2C1));

  // ACK enable
  I2C_AcknowledgeConfig(I2C1, ENABLE);

  return(ret_wert);
}


//--------------------------------------------------------------
// Beschreiben einer Adresse (16bit) per I2C von einem Slave
// slave_adr => I2C-Basis-Adresse vom Slave
// adr       => Register Adresse die beschrieben wird
// wert      => Bytewert der geschrieben wird
//
// Return_wert :
//    0   , Ok
//  < 0   , Error
//--------------------------------------------------------------
int16_t UB_I2C1_WriteByte16(uint8_t slave_adr, uint16_t adr, uint8_t wert)
{
  int16_t ret_wert=0;
  uint32_t timeout=I2C1_TIMEOUT;
  uint8_t lo,hi;
  
  // Hi und Lo Adresse
  lo=(adr&0x00FF);
  hi=(adr>>8);  

  // Start-Sequenz
  I2C_GenerateSTART(I2C1, ENABLE); 

  timeout=I2C1_TIMEOUT;
  while (!I2C_GetFlagStatus(I2C1, I2C_FLAG_SB)) {
    if(timeout!=0) timeout--; else return(P_I2C1_timeout(-1));
  } 

  // Slave-Adresse senden (write)
  I2C_Send7bitAddress(I2C1, slave_adr, I2C_Direction_Transmitter);

  timeout=I2C1_TIMEOUT;
  while (!I2C_GetFlagStatus(I2C1, I2C_FLAG_ADDR)) {
    if(timeout!=0) timeout--; else return(P_I2C1_timeout(-2));
  }  

  // ADDR-Flag l�schen
  I2C1->SR2;

  timeout=I2C1_TIMEOUT;
  while (!I2C_GetFlagStatus(I2C1, I2C_FLAG_TXE)) {
    if(timeout!=0) timeout--; else return(P_I2C1_timeout(-3));
  }

  // Adresse (hi) senden
  I2C_SendData(I2C1, hi);

  timeout=I2C1_TIMEOUT;
  while (!I2C_GetFlagStatus(I2C1, I2C_FLAG_TXE)) {
    if(timeout!=0) timeout--; else return(P_I2C1_timeout(-4));
  }
  
  // Adresse (lo) senden
  I2C_SendData(I2C1, lo);

  timeout=I2C1_TIMEOUT;
  while (!I2C_GetFlagStatus(I2C1, I2C_FLAG_TXE)) {
    if(timeout!=0) timeout--; else return(P_I2C1_timeout(-4));
  }  

  // Daten senden
  I2C_SendData(I2C1, wert);

  timeout=I2C1_TIMEOUT;
  while ((!I2C_GetFlagStatus(I2C1, I2C_FLAG_TXE)) || (!I2C_GetFlagStatus(I2C1, I2C_FLAG_BTF))) {
    if(timeout!=0) timeout--; else return(P_I2C1_timeout(-5));
  }

  // Stop-Sequenz
  I2C_GenerateSTOP(I2C1, ENABLE);

  ret_wert=0; // alles ok

  return(ret_wert);
}


//--------------------------------------------------------------
// kleine Pause (ohne Timer)
// nCount :  20.000 = ca. 1,2ms
//          200.000 = ca. 12ms
//--------------------------------------------------------------
void UB_I2C1_Delay(volatile uint32_t nCount)
{
  while(nCount--)
  {
  }
}


//--------------------------------------------------------------
// interne Funktion
// Init der I2C-Schnittstelle
//--------------------------------------------------------------
void P_I2C1_InitI2C(void)
{
  I2C_InitTypeDef  I2C_InitStructure;

  // I2C-Konfiguration
  I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
  I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
  I2C_InitStructure.I2C_OwnAddress1 = 0x00;
  I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
  I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
  I2C_InitStructure.I2C_ClockSpeed = I2C1_CLOCK_FRQ;

  // I2C enable
  I2C_Cmd(I2C1, ENABLE);

  // Init Struktur
  I2C_Init(I2C1, &I2C_InitStructure);
}

//--------------------------------------------------------------
// interne Funktion
// wird bei einem Timeout aufgerufen
// Stop, Reset und reinit der I2C-Schnittstelle
//--------------------------------------------------------------
int16_t P_I2C1_timeout(int16_t wert)
{
  int16_t ret_wert=wert;

  // Stop und Reset
  I2C_GenerateSTOP(I2C1, ENABLE);
  I2C_SoftwareResetCmd(I2C1, ENABLE);
  I2C_SoftwareResetCmd(I2C1, DISABLE);

  // I2C deinit
  I2C_DeInit(I2C1);
  // I2C init
  P_I2C1_InitI2C();
    
  return(ret_wert);
}
