#ifndef COMMAND_SYSTEM_H
#define COMMAND_SYSTEM_H 

#include "zb_common.h"
#include "zb_scheduler.h"
#include "zb_bufpool.h"
#include "zb_nwk.h"
#include "zb_aps.h"
#include "zb_zdo.h"
#include "zb_secur.h"
#include "zb_secur_api.h"

typedef struct info_from_user {
	zb_int8_t device_address;
} user_info;

typedef struct info_from_user_param {
	zb_int8_t device_address;
	zb_int8_t parameter;
} user_info_param;

typedef struct info_from_device {
	zb_int8_t device_address;
	zb_uint8_t is_data_valid;
	zb_int16_t humidity_multiplied_by_ten;
    zb_int16_t temperature_multiplied_by_ten;
} meteo_user_info;

typedef struct simple_command_package{
	zb_int8_t command_code;
} simple_cp;

typedef struct params_command_package{
	zb_int8_t command_code;
	zb_int8_t parameter;
} param_cp;

typedef struct humidity_temperature_package{
	zb_uint8_t is_data_valid;
	zb_int16_t humidity_multiplied_by_ten;
    zb_int16_t temperature_multiplied_by_ten;
} humidity_temperature_dp;


enum sensors_state{
	RES_NO_ERROR, 
	RES_NOT_READY, 
	RES_DATA_NOT_RECEIVED, 
	RES_NOT_VALID_CRC,
};

enum commands {
	SEND_PACKAGE,
	CHANGE_SENDING_PERIOD,
};

void zb_send_data_from_sensors(zb_uint8_t param) ZB_CALLBACK;
void zb_send_package_request(zb_uint8_t param) ZB_CALLBACK;


#endif
