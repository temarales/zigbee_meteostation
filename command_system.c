/// \file command_system.c
/// Command system for communicating in meteostation.

#include "command_system.h"

/// \brief Initialize ZBOSS buffer before sending smth to the defined address.
/// \param[in] param ZBOSS buffer param
/// \param[in] address Destination device address
/// \param[in] data Useful data to send
/// \param[in] data_size
static void zb_init_buffer(zb_uint8_t param, zb_uint8_t address, char* data, int data_size) 
{
	zb_buf_t *buf = (zb_buf_t *)ZB_BUF_FROM_REF(param);

	zb_uint8_t *ptr = NULL;
	ZB_BUF_INITIAL_ALLOC(buf, data_size, ptr);
	memcpy(ptr, data, data_size);

	zb_apsde_data_req_t *req = ZB_GET_BUF_TAIL(buf, sizeof(zb_apsde_data_req_t));
	req->dst_addr.addr_short = address;
	req->addr_mode = ZB_APS_ADDR_MODE_16_ENDP_PRESENT;
	req->tx_options = ZB_APSDE_TX_OPT_ACK_TX;
	req->radius = 1;
	req->profileid = 2;
	req->src_endpoint = 10;
	req->dst_endpoint = 10;
	buf->u.hdr.handle = 0x11;
}

void zb_send_data_from_sensors(zb_uint8_t param)
{
    zb_buf_t *buf = (zb_buf_t *)ZB_BUF_FROM_REF(param);
	
	meteo_user_info *info = ZB_GET_BUF_TAIL(buf, sizeof(meteo_user_info));
	humidity_temperature_dp data;
    data.is_data_valid = info->is_data_valid;
	data.humidity_multiplied_by_ten = info->humidity_multiplied_by_ten;
    data.temperature_multiplied_by_ten = info->temperature_multiplied_by_ten;
  
	zb_init_buffer(param, info->device_address, &data, sizeof(humidity_temperature_dp));
	ZB_SCHEDULE_CALLBACK(zb_apsde_data_request, ZB_REF_FROM_BUF(buf));  
}

/// \brief Initialize ZBOSS buffer before sending smth with data about command.
/// \param[in] param ZBOSS buffer param
/// \param[in] command Command code
static void zb_send_command(zb_uint8_t param, zb_uint8_t command)
{
	zb_buf_t *buf = (zb_buf_t *)ZB_BUF_FROM_REF(param);
	
	simple_cp command_data;
    command_data.command_code = command;
  	
	user_info_addr *info = ZB_GET_BUF_TAIL(buf, sizeof(user_info_addr));
	zb_init_buffer(param, info->device_address, &command_data, sizeof(simple_cp));
	ZB_SCHEDULE_CALLBACK(zb_apsde_data_request, ZB_REF_FROM_BUF(buf));  
}

/// \brief Initialize ZBOSS buffer before sending smth with data about command with parameter.
/// \param[in] param ZBOSS buffer param
/// \param[in] command Command code
static void zb_send_command_with_param(zb_uint8_t param, zb_uint8_t command)
{
	zb_buf_t *buf = (zb_buf_t *)ZB_BUF_FROM_REF(param);

	user_info_param_addr *info = ZB_GET_BUF_TAIL(buf, sizeof(user_info_param_addr));
	param_cp command_data;
    command_data.command_code = command;
	command_data.parameter = info->parameter;
	zb_init_buffer(param, info->device_address, &command_data, sizeof(param_cp));

	ZB_SCHEDULE_CALLBACK(zb_apsde_data_request, ZB_REF_FROM_BUF(buf));  
}

void zb_send_package_request(zb_uint8_t param) ZB_CALLBACK
{
	zb_send_command(param, SEND_PACKAGE);
}

void zb_send_new_period(zb_uint8_t param) ZB_CALLBACK
{
	zb_send_command_with_param(param, CHANGE_SENDING_PERIOD);
}
