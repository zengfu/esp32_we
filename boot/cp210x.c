#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>  
#include <unistd.h>
#include <string.h>

#define LDO 0X0001  		//0:3.3 1:1.8V  gpio12
#define BOOT_EN  0X0002  	//0:download  1:spi flash gpio0
#define BOOT_2   0X0004  	//0:download  1:spi gpio 2   
#define ENABLE   0X0008  	//0:disable 1:enale enable_pin
int main(int argc,char** argv)
{
	int fd;
	if(argc!=2)
		return -1;
	fd=open("/dev/ttyUSB0", O_RDWR | O_NOCTTY | O_NDELAY);
	if (fd == -1)
	{
		printf("%s\n","open com port failed!");
		return -1;
	}
	unsigned long gpio;
	ioctl(fd, 0x8000, &gpio);
	printf("Last GPIO Value=0x%lx\n",gpio);
	if(!strcmp(argv[1],"normal"))
	{
		gpio=(0x04<<8)|0x00ff;
		ioctl(fd, 0x8001, &gpio);
		sleep(0.5);
		gpio=(0x0a<<8)|0x00ff;
		ioctl(fd, 0x8001, &gpio);
		sleep(0.5);
		gpio=(0xF<<8)|0x00ff;
		ioctl(fd, 0x8001, &gpio);
		close(fd);
		printf("%s\n","normal");
		ioctl(fd, 0x8000, &gpio);
		printf("GPIO Value=0x%lx\n",gpio);
	}
	else if(!strcmp(argv[1],"boot"))
	{
		gpio=0x00ff;
		ioctl(fd, 0x8001, &gpio);
		sleep(0.5);
		gpio=(0x8<<8)|0x00ff;
		ioctl(fd, 0x8001, &gpio);
		sleep(0.5);
		gpio=(0xf<<8)|0x00ff;
		ioctl(fd, 0x8001, &gpio);
		printf("%s\n","boot");
		ioctl(fd, 0x8000, &gpio);
		printf("GPIO Value=0x%lx\n",gpio);
		close(fd);
	}
	
	return 0;
}





