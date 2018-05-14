/*
 * #FCDS - Slave Module communication example for C
 * How-To read/writa data, just call the functions below.
 *
 * Run: gcc example.c -o example && ./example
 *
 * More info: fercsa@freemail.hu - www.fercsa.com
 */
 
#include <stdio.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <errno.h>

int read_ANA(int slave_module, int channel_nr)
{
	int fd;
	char buffer[4];
	int formated_data;

	fd = open("/dev/i2c-1", O_RDWR);

	if(fd < 0){
		printf("Error opening file: %s\n", strerror(errno));
		return 1;
	}

	if(ioctl(fd, I2C_SLAVE, slave_module) < 0){
		printf("ioctl error: %s\n", strerror(errno));
		return 1;
	}

	buffer[0] = 0x13;
	buffer[1] = channel_nr;
	buffer[2] = 0x69;

	write(fd, buffer, 3);
	usleep(1000);
	read(fd, buffer, 4);
	
	close(fd);

	if(buffer[0] == 0x13 && buffer[3] == 0x69){
		formated_data = buffer[1] << 8 | buffer[2];
		if(formated_data <= 0xFFF){
			return formated_data;
		}
		else
			return 0;
	}
	else
		return 0;
}

int write_PWM(int slave_module, int slot, int value)//slave_module=current i2c address (1-127), slot=slot number (0-7), value=0-65535
{
	int fd;
	char buffer[5];

	fd = open("/dev/i2c-1", O_RDWR);

	if(fd < 0){
		printf("Error opening file: %s\n", strerror(errno));
		return 1;
	}

	if(ioctl(fd, I2C_SLAVE, slave_module) < 0){
		printf("ioctl error: %s\n", strerror(errno));
		return 1;
	}

	buffer[0] = 0x13;//for the packet stream checker on the start..
	buffer[1] = slot + 0xA0;//PWM0->7: 0xA0->0xA7, example: PWM4 is 0xA4
	buffer[2] = value >> 8;// MSB
	buffer[3] = value & 0xFF;// LSB
	buffer[4] = 0x69;//..and end bytes

	write(fd, buffer, 5);
	usleep(1000);

	return 0;
}

int set_i2c_address_for_slave_module(int slave_module, int value)//slave_module=current i2c address (1-127), value=new address (1-127)
{
	int fd;
	char buffer[5];

	fd = open("/dev/i2c-1", O_RDWR);

	if(fd < 0){
		printf("Error opening file: %s\n", strerror(errno));
		return 1;
	}

	if(ioctl(fd, I2C_SLAVE, slave_module) < 0){
		printf("ioctl error: %s\n", strerror(errno));
		return 1;
	}

	buffer[0] = 0x13;//for the packet stream checker on the start..
	buffer[1] = 0xF1;//command for address change
	buffer[2] = value >> 8;// MSB
	buffer[3] = value & 0xFF;// LSB
	buffer[4] = 0x69;//..and end bytes

	write(fd, buffer, 5);
	usleep(1000);

	return 0;
}

 
