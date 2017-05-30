#include "hal_i2c.h"
#include "wm8979.h"



#define bit0  0x001
#define bit1  0x002
#define bit2  0x004 
#define bit3  0x008 
#define bit4  0x010 
#define bit5  0x020 
#define bit6  0x040 
#define bit7  0x080 
#define bit8  0x100
/*
static uint16_t allreg[]={
	0xffff,   //software reset
 	0x0000,   //power manage 1 
	0x0000,   //power manage 2
	0x0000,   //power manage 3
	0x0050,   //audio interface
	0x0000,   //companding ctrl
	0x0140,   //clock gen ctrl
	0x0000,   //additional ctrl
	0x0000,   //gpio stuff
	0x0000,   //jack
	0x0000,   //dac ctrl
	0x00ff,   //left dac volume
	0x00ff,   //right dac volume
	0x0000,   //ack detect control
	0x0100,   //adc ctrl
	0x00ff,   //left adc volume
	0x00ff,   //right adc volume
	0xffff,   //_________________reserved
	0x012c,   //eq1
	0x002c,   //eq2
	0x002c,   //eq3
	0x002c,   //eq4
	0x002c,   //eq5
	0xffff,   //_________________reserved
	0x0032,   //dac limit 1
	0x0000,   //dac limit 2
	0xffff,   //_________________reserved
	0x0000,   //notch filter 1
	0x0000,   //notch filter 2
	0x0000,   //notch filter 3
	0x0000,   //notch filter 4
	0xffff,   //_________________reserved
	0x0038,   //acl ctrl 1
	0x000b,   //acl ctrl 2
	0x0032,   //acl ctrl 3
	0x0000,   //noise gate
	0x0008,   //pll n
	0x000c,   //pll k1 
	0x0093,   //pll k2
	0x00e9,   //pll k3
	0xffff,   //_________________reserved
	0x0000,   //3d control
	0xffff,   //_________________reserved
	0x0000,   //beep control
	0x0003,   //input control
	0x0010,   //left inp pga gain ctrl
	0x0010,   //right inp pga gain ctrl
	0x0100,   //left adc boost ctrl
	0x0100,   //right adc boost ctrl
	0x0002,   //output ctrl
	0x0001,   //left mixer ctrl
	0x0001,   //right mixer ctrl
	0x0039,   //lout1 volume ctrl
	0x0039,   //rout1 volume ctrl
	0x0039,   //lout2 volume ctrl
	0x0039,   //rout2 volume ctrl
	0x0001,   //out3 mix ctrl
	0x0001,    //out4 mix ctrl
};*/
static uint16_t allreg[58]=
{
	0X0000,0X0000,0X0000,0X0000,0X0050,0X0000,0X0140,0X0000,
	0X0000,0X0000,0X0000,0X00FF,0X00FF,0X0000,0X0100,0X00FF,
	0X00FF,0X0000,0X012C,0X002C,0X002C,0X002C,0X002C,0X0000,
	0X0032,0X0000,0X0000,0X0000,0X0000,0X0000,0X0000,0X0000,
	0X0038,0X000B,0X0032,0X0000,0X0008,0X000C,0X0093,0X00E9,
	0X0000,0X0000,0X0000,0X0000,0X0003,0X0010,0X0010,0X0100,
	0X0100,0X0002,0X0001,0X0001,0X0039,0X0039,0X0039,0X0039,
	0X0001,0X0001
};

	

// 2 wire mode only support write action
static esp_err_t wm8978_write_reg(i2c_port_t i2c_num,uint8_t reg,uint16_t data)
{
	esp_err_t err;
	uint8_t buf[2];
	buf[0]=((data&0x0100)>>8)|(reg<<1);
	buf[1]=(uint8_t)(data&0xff);
	err=hal_i2c_master_mem_write(i2c_num,WM8978_ADD,buf[0],buf+1,1);
	return err;
}


static esp_err_t wm8978_write_dump()
{
	esp_err_t err;
	for (int i=0;i<WM8978_CACHEREGNUM;i++)
	{	
		// if(allreg[i]==0xffff)
		// 	continue;
		err=wm8978_write_reg(0,i,allreg[i]);
		printf("%d:%0x\n",i,allreg[i]);
		if (err!=ESP_OK)
			return err;
	}
	return err;
}

void wm8979_input_pga ()
{
	allreg[WM8978_POWER_MANAGEMENT_2] |=bit2|bit3; //left channel adn right channel enable
	allreg[WM8978_INPUT_CONTROL] &=~bit0;          //lip disconnect
	allreg[WM8978_INPUT_CONTROL] |=bit2;          //l2 disconnect
	allreg[WM8978_LEFT_INP_PGA_CONTROL]|=bit5|bit4|bit3|bit2|bit1|bit0|bit8; //35.25db
}
void wm8979_input_boost()
{
	allreg[WM8978_POWER_MANAGEMENT_2] |=bit4|bit5; //enable boost stage 20db,now 55.25db
}

void wm8979_bias()
{
	allreg[WM8978_POWER_MANAGEMENT_1] |=bit3; //enable bias
}
void wm8979_adc()
{
	allreg[WM8978_POWER_MANAGEMENT_2] |=bit0|bit1; //adc left and right enable
	allreg[WM8978_ADC_CONTROL] |=bit3;    //128x oversample
}
void wm8979_dac()
{
	allreg[WM8978_POWER_MANAGEMENT_3] |=bit0|bit1; //dac left and right enable
	allreg[WM8978_DAC_CONTROL] |=bit3; //128x oversample
}
void wm8979_output_mix()
{
	allreg[WM8978_POWER_MANAGEMENT_3] |=bit2|bit3;//enable right and left mixer;
}
void wm8979_lout1()
{
	allreg[WM8978_LOUT1_HP_CONTROL]  |=bit5|bit4|bit3|bit2|bit1|bit0|bit8;  //output 6db
	allreg[WM8978_ROUT1_HP_CONTROL]  |=bit5|bit4|bit3|bit2|bit1|bit0|bit8;  //output 6db
	allreg[WM8978_POWER_MANAGEMENT_2] |=bit8|bit7;
}
void wm8979_lout2()
{
	allreg[WM8978_POWER_MANAGEMENT_3]|=bit5|bit6;
	allreg[WM8978_OUTPUT_CONTROL]|=bit2;//1.5 speek gain
	allreg[WM8978_POWER_MANAGEMENT_1]|=bit8;
	allreg[WM8978_LOUT2_SPK_CONTROL]|=bit5|bit4|bit3|bit2|bit1|bit0|bit8;
	allreg[WM8978_ROUT2_SPK_CONTROL]|=bit5|bit4|bit3|bit2|bit1|bit0|bit8;
}
void wm8979_eq()
{
	allreg[WM8978_EQ1]&=~0x1f;
	allreg[WM8978_EQ2]&=~0x1f;
	allreg[WM8978_EQ3]&=~0x1f;
	allreg[WM8978_EQ4]&=~0x1f;
	allreg[WM8978_EQ5]&=~0x1f;
}
void wm8979_interface()
{
	allreg[WM8978_AUDIO_INTERFACE] &=~(bit6|bit5);//16bit
	allreg[WM8978_CLOCKING]|=bit0; //the codec ic is master mode 
	allreg[WM8978_CLOCKING]|=bit3|bit2;// 256/32=8
	allreg[WM8978_CLOCKING]&=~(bit7|bit6|bit5); //mclk=mclk
	allreg[WM8978_CLOCKING]|=bit7;
	//allreg[WM8978_CLOCKING]&=~bit8;//mclk is the clk source
}
void wm8979_pll()
{
	allreg[WM8978_POWER_MANAGEMENT_1]|=bit5;//enable pll
	allreg[WM8978_PLL_N]|=bit4;//mclk/2 =20m
	allreg[WM8978_PLL_N]&=0xf0;//n=9
	allreg[WM8978_PLL_N]|=0x09;
	//k=EE009F
	allreg[WM8978_PLL_K1]=0x3d;
	allreg[WM8978_PLL_K2]=0x100;
	allreg[WM8978_PLL_K3]=0x9f;					
}
void wm8979_loopback()
{
	allreg[WM8978_COMPANDING_CONTROL]|=bit0; //start loopback
}



/*

IP PGA->IP BOOST->ADC->ADC FILTERS
->DAC FILTERS->DAC->MIX->ROUT


*/   
void wm8979_init()
{
	wm8979_input_pga ();
	wm8979_input_boost();
	wm8979_bias();
	wm8979_adc();
	wm8979_dac();
	wm8979_output_mix();
	wm8979_lout2();
	wm8979_eq();
	wm8979_pll();
	wm8979_interface();
	//wm8979_loopback();
	wm8978_write_dump();
}

