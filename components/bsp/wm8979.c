#include "hal_i2c.h"
#include "wm8979.h"

#define wm8978_ADD 0x1a
// 2 wire mode only support write action
static esp_err_t wm8978_write_reg(i2c_port_t i2c_num,uint8_t reg,uint16_t data)
{
	esp_err_t err;
	uint8_t buf[2];
	buf[0]=((data&0x0100)>>8)|(reg<<1);
	buf[1]=(uint8_t)(data&0xff);
	err=hal_i2c_master_mem_write(i2c_num,wm8978_ADD,buf[0],buf+1,1);
	return err;
}