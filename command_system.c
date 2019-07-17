#include "command_system.h"

static void zb_init_buffer(zb_uint8_t param, zb_uint8_t address, char* data, int data_size) 
{
	zb_buf_t *buf = (zb_buf_t *)ZB_BUF_FROM_REF(param);

	zb_uint8_t *ptr = NULL;
	ZB_BUF_INITIAL_ALLOC(buf, data_size, ptr);
	memcpy(ptr, data, data_size);

	zb_apsde_data_req_t *req = ZB_GET_BUF_TAIL(buf, sizeof(zb_apsde_data_req_t));
	req->dst_addr.addr_short = address; /* send to ZC */
	req->addr_mode = ZB_APS_ADDR_MODE_16_ENDP_PRESENT;
	req->tx_options = ZB_APSDE_TX_OPT_ACK_TX;
	req->radius = 1;
	req->profileid = 2;
	req->src_endpoint = 10;
	req->dst_endpoint = 10;
	buf->u.hdr.handle = 0x11;
}

static void zb_send_data(zb_uint8_t param)
{
    zb_buf_t *buf = (zb_buf_t *)ZB_BUF_FROM_REF(param);
	
	meteo_info *info = ZB_GET_BUF_TAIL(buf, sizeof(meteo_info));
	humidity_temperature_pack data;
    data.is_data_valid = info->is_data_valid;
	data.humidity_multiplied_by_ten = info->humidity_multiplied_by_ten;
    data.temperature_multiplied_by_ten = info->temperature_multiplied_by_ten;
  
	zb_init_buffer(param, info->device_address, &data, sizeof(humidity_temperature_pack));
	ZB_SCHEDULE_CALLBACK(zb_apsde_data_request, ZB_REF_FROM_BUF(buf));  
}

void zb_send_data_from_sensors(zb_uint8_t param) ZB_CALLBACK
{
	zb_send_data(param);
}
