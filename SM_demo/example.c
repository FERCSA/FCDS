/*
 * #FCDS - Slave Module communication example for C
 * How-To read/writa data, just call the functions below.
 *
 * Default I2C address for a Slave Module is 0x08
 * If you got more than one and it's not labeled, please set your new addresses one by one,
 * calling set_i2c_address_for_slave_module() , you can find it in the arduino example
 *
 * Compile and run: gcc example.c -o example && ./example 8 0
 * Where 0x08 is your Slave Module's I2C address in decimal, 0 is the channel number
 *
 * More info: fercsa@freemail.hu - www.fercsa.com
 */
 
#include <stdio.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

int read_ANA(int slave_module, int channel_nr)//slave_module=current i2c address (1-127), channel_nr=analog input (0-23 or 0-31(32ch SM)), return=readout (0-4095)
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

int main(int argc, char** argv){
    int i = 0;
    int data = 0;
    float voltage = 0;
    float vref = 0;

    if(argv[3] == NULL){
	vref = atoi(argv[1]);
	for(i=0;i<32;i++){
	    data = read_ANA(atoi(argv[2]), i);//i2c address, channel number
	    voltage = (vref / 4095) * data;
	    printf("CH%02d 0x%04X %04d %.2fV\r\n", i, data, data, voltage);    
	}
    }
    else if(argv[1] == NULL || argv[2] == NULL || argv[3] == NULL){
	printf("Missing argument(s). Try:\r\n  ./example 3.3 8 0 - Where 3.3 is the voltage reference on board, 8 is your Slave Module's I2C address in decimal, 0 is the channel number.\r\n  OR\r\n  ./example 3.3 8   - Where 3.3 is the voltage reference on board, 8 is your Slave Module's I2C address in decimal, then list all channels at once.\r\n");
	return 1;
    }
    else{
	vref = atoi(argv[1]);
	data = read_ANA(atoi(argv[2]), atoi(argv[3]));//i2c address, channel number
	voltage = (vref / 4095) * data;
	printf("CH%02d 0x%04X %04d %.2fV\r\n", atoi(argv[3]), data, data, voltage);
    }

    return 0;
} 
