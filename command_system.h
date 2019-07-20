/// \file command_system.h  
/// Command system for communicating in meteostation.

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

/// Struct for putting data about destination device address to zb_buf
typedef struct info_from_user_addr {
	zb_int8_t device_address;
} user_info_addr;

/// Struct for putting data about destination device address and some other parameter to zb_buf
typedef struct info_from_user_param_addr {
	zb_int8_t device_address;
	zb_int8_t parameter;
} user_info_param_addr;

/// Struct for putting data about some parameter to zb_buf
typedef struct info_from_user_param {
	zb_int8_t parameter;
} user_info_param;

/// Struct for putting data about device address and sensors state to zb_buf
typedef struct info_from_device {
	zb_int8_t device_address;
	zb_uint8_t is_data_valid;
	zb_int16_t humidity_multiplied_by_ten;
    zb_int16_t temperature_multiplied_by_ten;
} meteo_user_info;

/// Struct for sending command
typedef struct __attribute__((__packed__)) simple_command_package{
	zb_int8_t command_code;
} simple_cp;

/// Struct for sending command with parameter
typedef struct __attribute__((__packed__)) params_command_package{
	zb_int8_t command_code;
	zb_int8_t parameter;
} param_cp;

/// Struct for sending sensors data
typedef struct __attribute__((__packed__)) humidity_temperature_package{
	zb_uint8_t is_data_valid;
	zb_int16_t humidity_multiplied_by_ten;
    zb_int16_t temperature_multiplied_by_ten;
} humidity_temperature_dp;

/// Set of possible sensor device state
enum sensors_state{
	RES_NO_ERROR, 
	RES_NOT_READY, 
	RES_DATA_NOT_RECEIVED, 
	RES_NOT_VALID_CRC,
};

/// Set of available commands
enum commands {
	SEND_PACKAGE,
	CHANGE_SENDING_PERIOD,
};

/// \brief Send data package from sensors.
/// \param[in] param ZBOSS param
void zb_send_data_from_sensors(zb_uint8_t param) ZB_CALLBACK;

/// \brief Send request for new data package.
/// \param[in] param ZBOSS param
void zb_send_package_request(zb_uint8_t param) ZB_CALLBACK;

/// \brief Send new period of data sending.
/// \param[in] param ZBOSS param
void zb_send_new_period(zb_uint8_t param) ZB_CALLBACK;


#endif
