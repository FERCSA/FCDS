#!/usr/bin/python
import smbus
import time

bus = smbus.SMBus(1)    # 0 = /dev/i2c-0 (port I2C0 on rPi), 1 = /dev/i2c-1 (port I2C1 on rPi)

DEVICE_ADDRESS = 0x08   #7 bit I2C address
CHANNEL_NR = 24         #number of channels on the ADC, 24 or 32

def read_ANA(slave_module, channel_nr):
        bus.write_i2c_block_data(slave_module, 0x13, [channel_nr, 0x69])
        time.sleep(0.005)
        data = bus.read_i2c_block_data(slave_module, 0)
        time.sleep(0.005)
        if(data[0] == 0x13 and data[3] == 0x69):
            formated_data = data[1] << 8 | data[2]
            if(formated_data <= 0x0FFF):
                return "%2d %4d 0x%0004X" % (channel_nr, formated_data, formated_data)
            else:
                return 0
        else:
            return 0


for i in range(0, CHANNEL_NR):
    print read_ANA(DEVICE_ADDRESS, i)
