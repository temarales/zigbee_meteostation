/***************************************************************************
*                      ZBOSS ZigBee Pro 2007 stack                         *
*                                                                          *
*          Copyright (c) 2012 DSR Corporation Denver CO, USA.              *
*                       http://www.dsr-wireless.com                        *
*                                                                          *
*                            All rights reserved.                          *
*          Copyright (c) 2011 ClarIDy Solutions, Inc., Taipei, Taiwan.     *
*                       http://www.claridy.com/                            *
*                                                                          *
*          Copyright (c) 2011 Uniband Electronic Corporation (UBEC),       *
*                             Hsinchu, Taiwan.                             *
*                       http://www.ubec.com.tw/                            *
*                                                                          *
*          Copyright (c) 2011 DSR Corporation Denver CO, USA.              *
*                       http://www.dsr-wireless.com                        *
*                                                                          *
*                            All rights reserved.                          *
*                                                                          *
*                                                                          *
* ZigBee Pro 2007 stack, also known as ZBOSS (R) ZB stack is available     *
* under either the terms of the Commercial License or the GNU General      *
* Public License version 2.0.  As a recipient of ZigBee Pro 2007 stack, you*
* may choose which license to receive this code under (except as noted in  *
* per-module LICENSE files).                                               *
*                                                                          *
* ZBOSS is a registered trademark of DSR Corporation AKA Data Storage      *
* Research LLC.                                                            *
*                                                                          *
* GNU General Public License Usage                                         *
* This file may be used under the terms of the GNU General Public License  *
* version 2.0 as published by the Free Software Foundation and appearing   *
* in the file LICENSE.GPL included in the packaging of this file.  Please  *
* review the following information to ensure the GNU General Public        *
* License version 2.0 requirements will be met:                            *
* http://www.gnu.org/licenses/old-licenses/gpl-2.0.html.                   *
*                                                                          *
* Commercial Usage                                                         *
* Licensees holding valid ClarIDy/UBEC/DSR Commercial licenses may use     *
* this file in accordance with the ClarIDy/UBEC/DSR Commercial License     *
* Agreement provided with the Software or, alternatively, in accordance    *
* with the terms contained in a written agreement between you and          *
* ClarIDy/UBEC/DSR.                                                        *
*                                                                          *
****************************************************************************
PURPOSE: Test for ZC application written using ZDO.
*/

#include "zb_common.h"
#include "zb_scheduler.h"
#include "zb_bufpool.h"
#include "zb_nwk.h"
#include "zb_aps.h"
#include "zb_zdo.h"

#include "leds.h"
#include "command_system.h"
#include "stm32_ub_lcd_text_i2c.h"
#include "stm32f4xx.h"
#include "stdio.h"
#include "string.h"

#define ZB_TEST_DUMMY_DATA_SIZE 10

zb_ieee_addr_t g_zc_addr = {0x00, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa};

/*! \addtogroup ZB_TESTS */
/*! @{ */

#ifndef ZB_COORDINATOR_ROLE
#error Coordinator role is not compiled!
#endif

/*
  The test is: ZC starts PAN, ZR joins to it by association and send APS data packet, when ZC
  received packet, it sends packet to ZR, when ZR received packet, it sends
  packet to ZC etc.
 */

LCD_TEXT_DISPLAY_t myLCD = { 
	TI2C3,	// I2C Modul (1=I2C1, 2=I2C2 oder 3=I2C3)
	0x7E,		// I2C Adresse des LCD Displays. Falls unbekannt dann auf 0 setzen und PortScan laufen lassen.
	16,		// Anzahl Zeichen je Zeile
	2, 		// Anzahl Zeilen
	1		// LCD Hintergrundbeleuchtung (0=Aus, 1=Ein)
	};

static volatile zb_int8_t sensor_period_in_sec = 8;
static volatile char first_str[16] = {0};
static volatile char second_str[16] = {0};
static volatile zb_uint16_t dst_short_addr = 0;

static void data_indication(zb_uint8_t param) ZB_CALLBACK;
static void send_package_request(zb_uint8_t param);
static void send_new_period(zb_uint8_t param);

static void init_elements(void);

void EXTI0_IRQHandler(void) 
{
	if (0 != dst_short_addr)
	{
		if(EXTI_GetITStatus(EXTI_Line0)!= RESET)
		{
			ZB_SCHEDULE_ALARM_CANCEL(send_package_request, 0);
			ZB_SCHEDULE_ALARM(send_package_request, 0, 0.1 * ZB_TIME_ONE_SECOND);
			EXTI_ClearITPendingBit(EXTI_Line0);
		}
	}
}

void EXTI1_IRQHandler(void) 
{
	if(EXTI_GetITStatus(EXTI_Line1)!= RESET)
	{
		if (0 != dst_short_addr)
		{
			if (8 == sensor_period_in_sec)
			{
				sensor_period_in_sec = 16;
				sprintf(second_str, "Set period 16s  ");
			}
			else
			{
				sensor_period_in_sec = 8;
				sprintf(second_str, "Set period 8s  ");
			}
			ZB_SCHEDULE_ALARM_CANCEL(send_new_period, 0);
			ZB_SCHEDULE_ALARM(send_new_period, 0, 4 * ZB_TIME_ONE_SECOND);
			UB_LCD_TEXT_I2C_String(&myLCD,0,1,second_str);
			EXTI_ClearITPendingBit(EXTI_Line1);
		}
	}
}

static void send_new_period(zb_uint8_t param)
{
	if (0 == param)
  	{
		ZB_GET_OUT_BUF_DELAYED(send_new_period);
		return;
  	}
	zb_buf_t *buf = ZB_BUF_FROM_REF(param);
	user_info_param_addr *user_data;
	user_data = ZB_GET_BUF_TAIL(buf, sizeof(user_info_param_addr));
	user_data->parameter = sensor_period_in_sec;
	user_data->device_address = dst_short_addr;
	zb_send_new_period(ZB_REF_FROM_BUF(buf));
}

static void send_package_request(zb_uint8_t param) 
{
	if (param==0)
	{
		ZB_GET_OUT_BUF_DELAYED(send_package_request);
		return;
	}
	zb_buf_t *buf = ZB_BUF_FROM_REF(param);
	user_info_addr *user_data;
	user_data = ZB_GET_BUF_TAIL(buf, sizeof(user_info_addr));
	user_data->device_address = dst_short_addr;
	zb_send_package_request(ZB_REF_FROM_BUF(buf));
}

static void init_elements()
{
	SystemInit();
	init_buttons();
	init_buttons_interractions();

	// Init der I2C1-Schnittstelle
	UB_I2C_Init(&myLCD);
	// Init vom LC-Display
	UB_LCD_TEXT_I2C_Init(&myLCD);

	UB_LCD_TEXT_I2C_String(&myLCD,0,0,"Hello STM32F407-");
	UB_LCD_TEXT_I2C_String(&myLCD,0,1,"Mikrocontroller!");
}


MAIN()
{
	ARGV_UNUSED;
	init_elements();

#if !(defined KEIL || defined SDCC || defined ZB_IAR)
	if ( argc < 3 )
	{
		//printf("%s <read pipe path> <write pipe path>\n", argv[0]);
		return 0;
	}
#endif


  /* Init device, load IB values from nvram or set it to default */
#ifndef ZB8051
	ZB_INIT("zdo_zc", argv[1], argv[2]);
#else
	ZB_INIT("zdo_zc", "1", "1");
#endif
#ifdef ZB_SECURITY
	ZG->nwk.nib.security_level = 0;
#endif
	ZB_IEEE_ADDR_COPY(ZB_PIB_EXTENDED_ADDRESS(), &g_zc_addr);
	MAC_PIB().mac_pan_id = 0x000a;

  /* let's always be coordinator */
	ZB_AIB().aps_designated_coordinator = 1;
	ZB_AIB().aps_channel_mask = (1l << 13);

	if (zdo_dev_start() != RET_OK)
	{
		TRACE_MSG(TRACE_ERROR, "zdo_dev_start failed", (FMT__0));
	}
	else
	{
		zdo_main_loop();
	}
	TRACE_DEINIT();
	MAIN_RETURN(0);
}

void zb_zdo_startup_complete(zb_uint8_t param) ZB_CALLBACK
{
	zb_buf_t *buf = ZB_BUF_FROM_REF(param);
	TRACE_MSG(TRACE_APS3, ">>zb_zdo_startup_complete status %d", (FMT__D, (int)buf->u.hdr.status));
	if (buf->u.hdr.status == 0)
	{
		zb_af_set_data_indication(data_indication);
	}
	else
	{
		TRACE_MSG(TRACE_ERROR, "Device start FAILED status %d", (FMT__D, (int)buf->u.hdr.status));
	}
	zb_free_buf(buf);
}


void data_indication(zb_uint8_t param) ZB_CALLBACK
{
	humidity_temperature_dp *data;
	zb_buf_t *asdu = (zb_buf_t *)ZB_BUF_FROM_REF(param);

	zb_mac_mhr_t mac_hdr;
	zb_parse_mhr(&mac_hdr, ((asdu)->buf + (asdu)->u.hdr.mac_hdr_offset));
	dst_short_addr = mac_hdr.src_addr.addr_short;

	ZB_APS_HDR_CUT_P(asdu, data);

	if (ZB_BUF_LEN(asdu) >= sizeof(humidity_temperature_dp))
	{
		char first_str[16];
		char second_str[16];
		UB_LCD_TEXT_I2C_Clear(&myLCD);
		sprintf(second_str, "Packet received ");
		UB_LCD_TEXT_I2C_String(&myLCD,0,1,second_str);
		switch (data->is_data_valid){
			case RES_NO_ERROR:
				sprintf(first_str, "H=%hi.%hi  t=%hi.%hiC    ", data->humidity_multiplied_by_ten/10, data->humidity_multiplied_by_ten % 10, data->temperature_multiplied_by_ten / 10, data->temperature_multiplied_by_ten % 10); 
				break;
			case RES_NOT_READY:
				sprintf(first_str, "Res not ready"); 
				break;
			case RES_DATA_NOT_RECEIVED:
				sprintf(first_str, "Data not received"); 
				break;
			case RES_NOT_VALID_CRC:
				sprintf(first_str, "Res not valid"); 
				break;
		}
		UB_LCD_TEXT_I2C_String(&myLCD, 0, 0, first_str);
	}
	zb_free_buf(asdu);
}


/*! @} */
