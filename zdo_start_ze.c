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
//#include "zb_secur.h"
//#include "zb_secur_api.h"

#include "leds.h"
#include "command_system.h"
#include "DHT22.h"
#include "stm32f4xx.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_tim.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_exti.h"
#include "stm32f4xx_syscfg.h"
#include "stdio.h"
#include "math.h"

/*#define ZB_ED_ROLE ZB_CONFIG_ROLE_ZED

#ifndef ZB_ED_ROLE
#error define ZB_ED_ROLE to compile ze tests
#endif

#ifndef ZB_SECURITY
#error Define ZB_SECURITY
#endif*/

//#define ZB_TEST_DATA_SIZE 6

static volatile zb_uint8_t is_data_valid = 0;
static volatile	zb_int16_t humidity_multiplied_by_ten = 0;
static volatile zb_int16_t temperature_multiplied_by_ten = 0;
static volatile timer_period_in_sec = 8;

static void init_all(void);
static void send_data_from_sensors(zb_uint8_t param);
static void data_indication(zb_uint8_t param) ZB_CALLBACK;

void TIM2_IRQHandler(void)
{
   if(TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
   {  
		ZB_SCHEDULE_CALLBACK(send_data_from_sensors, 0);

		//if (timer_period_in_sec * 2000 == )
		//{
			//TIM2->PSC = timer_period_in_sec * 2000;
    		//TIM2->EGR = TIM_EGR_UG;
		//}
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
    }
}


static void get_dht22_data_with_indication(zb_uint8_t *is_data_valid_l, zb_int16_t *humidity_multiplied_by_ten_l, zb_int16_t *temperature_multiplied_by_ten_l)
{			
	float h = 0;
	float t = 0;
	zb_uint8_t dhtRes;
	dhtRes = getDHT22Data(GPIOA,GPIO_Pin_5,&h,&t);
	*is_data_valid_l = dhtRes;
	*humidity_multiplied_by_ten_l = (zb_int16_t)ceil(h*10);
	*temperature_multiplied_by_ten_l = (zb_int16_t)ceil(t*10);
			
	GPIO_ResetBits(GPIOD, GPIO_Pin_12 | GPIO_Pin_13| GPIO_Pin_14| GPIO_Pin_15);
	if (dhtRes == dht22Res_no_error){
		GPIO_SetBits(GPIOD, GPIO_Pin_15);
	}
	if (dhtRes == dht22Res_not_ready){
		GPIO_SetBits(GPIOD, GPIO_Pin_14);
	}
	if (dhtRes == dht22Res_data_not_received){
		GPIO_SetBits(GPIOD, GPIO_Pin_13);
	}
	if (dhtRes == dht22Res_not_valid_crc){
		GPIO_SetBits(GPIOD, GPIO_Pin_12);
	}
			
}

static void init_all(void)
{
	initDHT22();
	init_leds_on_stm();
	init_TIM2(42000, 4000);	
}

static void send_data_from_sensors(zb_uint8_t param) 
{
	if (param==0)
  	{
		ZB_GET_OUT_BUF_DELAYED(send_data_from_sensors);
        return;
  	}

	get_dht22_data_with_indication(&is_data_valid, &humidity_multiplied_by_ten, &temperature_multiplied_by_ten);

  	zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  	meteo_user_info *user_data;
  	user_data = ZB_GET_BUF_TAIL(buf, sizeof(meteo_user_info));
  	user_data->device_address = 0;
  	user_data->is_data_valid = is_data_valid;
  	user_data->humidity_multiplied_by_ten = humidity_multiplied_by_ten;
  	user_data->temperature_multiplied_by_ten = temperature_multiplied_by_ten;

	zb_send_data_from_sensors(ZB_REF_FROM_BUF(buf));
}

/*
  ZE joins to ZC(ZR), then sends APS packet.
*/

zb_ieee_addr_t g_ieee_addr = {0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02};

MAIN()
{
  ARGV_UNUSED;
  init_all();
  add_interrupt_TIM2();

#ifndef KEIL
  if ( argc < 3 )
  {
    printf("%s <read pipe path> <write pipe path>\n", argv[0]);
    return 0;
  }
#endif

  /* Init device, load IB values from nvram or set it to default */
#ifndef ZB8051
  ZB_INIT("zdo_ze", argv[1], argv[2]);
#else
  ZB_INIT("zdo_ze", "3", "3");
#endif

  ZB_IEEE_ADDR_COPY(ZB_PIB_EXTENDED_ADDRESS(), g_ieee_addr);
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
  if (buf->u.hdr.status == 0)
  {
    TRACE_MSG(TRACE_APS1, "Device STARTED OK", (FMT__0));
	zb_af_set_data_indication(data_indication);
  }
  else
  {
    TRACE_MSG(TRACE_ERROR, "Device started FAILED status %d", (FMT__D, (int)buf->u.hdr.status));
    zb_free_buf(buf);
  }
}

void data_indication(zb_uint8_t param) ZB_CALLBACK
{
	char *ptr;
	zb_buf_t *asdu = (zb_buf_t *)ZB_BUF_FROM_REF(param);

	ZB_APS_HDR_CUT_P(asdu, ptr);
	zb_int8_t *command_data = ptr;

	if ((int)ZB_BUF_LEN(asdu) > 0)
	{
		switch (*command_data){
			case SEND_PACKAGE:
				ZB_SCHEDULE_CALLBACK(send_data_from_sensors, 0);
				break;
			case CHANGE_SENDING_PERIOD:
				{
					TIM2->PSC = *(command_data + 1) * 2000;
    				TIM2->EGR = TIM_EGR_UG;
				//sprintf(first_str, "Res not ready"); 
				}
				break;
		}	
	}
	zb_free_buf(asdu);
}


/*! @} */


