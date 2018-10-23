/*
 * #FCDS - Slave Module communication example for arduino
 * How-To read/writa data, just call the functions below.
 *
 * Default I2C address for a Slave Module is 0x08
 * If you got more than one and it's not labeled, please set your new addresses one by one,
 * calling set_i2c_address_for_slave_module()
 *
 * More info: fercsa@freemail.hu - www.fercsa.com
 */

#include <Wire.h>
String your_board = "32CH_ADC";//select your board: 32CH_ADC, 24CH_ADC_8CH_PWM, 32CH_MUX
byte slave_module = 0x08;//your board's I2C address
byte MUX_pin = 36;//connect your MUX board's J10 to this pin on your MCU

/* Common functions */
void set_i2c_address_for_slave_module(byte slave_module, int value){//slave_module=current i2c address (1-127), value=new address (1-127)
	Wire.beginTransmission(slave_module);
	Wire.write(0x13);//for the packet stream checker on the start..
	Wire.write(0xF1);//command for address change
	Wire.write((byte)(value >> 8));     // MSB
	Wire.write((byte)(value & 0xFF));   // LSB
	Wire.write(0x69);//..and end bytes
	Wire.endTransmission();
}

/* ADC related functions */
unsigned int read_ANA(byte slave_module, byte channel_nr){//slave_module=current i2c address (1-127), channel_nr=analog input (0-23 or 0-31(32ch SM)), return=readout (0-4095)
	unsigned int data = 0x00;//when fail read null for sure..

	Wire.beginTransmission(slave_module);
	Wire.write(0x13);//for the packet stream checker on the start..
	Wire.write(channel_nr);
	Wire.write(0x69);//..and at the end
	Wire.endTransmission();

	Wire.requestFrom(slave_module, 4);

	while( Wire.available() ){
		data = (data << 8) | Wire.read();
	}

	if( (data >> 24) == 0x13 && (data & 0xFF) == 0x69){//packet stream checker on the start and end bytes
		if(((data >> 8) & 0x00FFFF) <= 0x0FFF)//out of range check
			return (data >> 8) & 0x00FFFF;
		else
			return 0;
	}
	else
		return 0;
}

/* MUX related functions */
void select_MUX(byte slave_module, byte channel_nr){//aka bus, address..
	unsigned int data = 0x00;//when fail read null for sure..

	Wire.beginTransmission(slave_module);
	Wire.write(0x13);//packet stream checker on the start..
	Wire.write(channel_nr);
	Wire.write(0x69);//..and at the end
	Wire.endTransmission();
}

/* PWM related functions */
void write_PWM(byte slave_module, byte slot, int value){//slave_module=current i2c address (1-127), slot=slot number (0-7), value=0-65535
	Wire.beginTransmission(slave_module);
	Wire.write(0x13);//for the packet stream checker on the start..
	Wire.write(slot + 0xA0);//PWM0->7: 0xA0->0xA7, example: PWM4 is 0xA4
	Wire.write((byte)(value >> 8));     // MSB
	Wire.write((byte)(value & 0xFF));   // LSB
	Wire.write(0x69);//..and end bytes
	Wire.endTransmission();
}

void set_PWM_divider(byte slave_module, int value){//divide by value+1 where value=0-65535
	Wire.beginTransmission(slave_module);
	Wire.write(0x13);//packet stream checker on the start..
	Wire.write(0xF2);//command
	Wire.write((byte)(value >> 8));     // MSB
	Wire.write((byte)(value & 0xFF));   // LSB
	Wire.write(0x69);//..and end bytes
	Wire.endTransmission();

	delay(50);
}

void set_PWM_fractional_divider(byte slave_module, int value){//divide by y/32 where y=0-31, not affected when x=0
	Wire.beginTransmission(slave_module);
	Wire.write(0x13);//packet stream checker on the start..
	Wire.write(0xF3);//command
	Wire.write((byte)(value >> 8));     // MSB
	Wire.write((byte)(value & 0xFF));   // LSB
	Wire.write(0x69);//..and end bytes
	Wire.endTransmission();

	delay(50);
}

int get_PWM_divider(byte slave_module){
	unsigned int data = 0x00;//when fail read null for sure..
	Wire.beginTransmission(slave_module);
	Wire.write(0x13);//packet stream checker on the start..
	Wire.write(0xF4);//command
	Wire.write(0x69);//..and end bytes
	Wire.endTransmission();

	Wire.requestFrom((int)slave_module, 4);

	while( Wire.available() ){
		data = (data << 8) | Wire.read();
	}

	if( (data >> 24) == 0x13 && (data & 0xFF) == 0x69)//packet stream checker on the start and end bytes
		return (data >> 8) & 0x00FFFF;
	else
		return 0;
}

int get_PWM_fractional_divider(byte slave_module){
	unsigned int data = 0x00;//when fail read null for sure..

	Wire.beginTransmission(slave_module);
	Wire.write(0x13);//packet stream checker on the start..
	Wire.write(0xF5);//command
	Wire.write(0x69);//..and end bytes
	Wire.endTransmission();

	Wire.requestFrom((int)slave_module, 4);

	while( Wire.available() ){
		data = (data << 8) | Wire.read();
	}

	if( (data >> 24) == 0x13 && (data & 0xFF) == 0x69)//packet stream checker on the start and end bytes
		return (data >> 8) & 0x00FFFF;
	else
		return 0;
}

void setup(){
	Wire.begin();
	
	if( your_board == "24CH_ADC_8CH_PWM" ){//activate PWMs..
		write_PWM(slave_module, 0, 0x0FFF);//PWM0
		write_PWM(slave_module, 1, 0x1FFF);//PWM1
		write_PWM(slave_module, 2, 0x2FFF);//PWM2
		write_PWM(slave_module, 3, 0x3FFF);//PWM3
		write_PWM(slave_module, 4, 0x4FFF);//PWM4
		write_PWM(slave_module, 5, 0x5FFF);//PWM5
		write_PWM(slave_module, 6, 0x6FFF);//PWM6
		write_PWM(slave_module, 7, 0x7FFF);//PWM7
	}
}

void loop(){
	if( your_board == "24CH_ADC_8CH_PWM" ){
		Serial.print("#SM");
		Serial.println(slave_module);
		Serial.println("#If a channel not terminated(connected to ground) it's gonna be floating!");
		for(byte i = 0; i < 24; i++){
			Serial.print("#Value at channel ");
			Serial.print(i);
			if(i<10)
				Serial.print(" ");
			Serial.print(" : ");
			float analog_read = read_ANA(slave_module, i);
			if(analog_read < 10)
				Serial.print(" ");
			if(analog_read < 100)
				Serial.print(" ");
			if(analog_read < 1000)
				Serial.print(" ");
			Serial.print( analog_read, 0 );
			Serial.println();
		}
		delay(1000);	
	}
	else if( your_board == "32CH_ADC" ){
		Serial.print("#SM");
		Serial.println(slave_module);
		Serial.println("#If a channel not terminated(connected to ground) it's gonna be floating!");
		for(byte i = 0; i < 32; i++){
			Serial.print("#Value at channel ");
			Serial.print(i);
			if(i<10)
				Serial.print(" ");
			Serial.print(" : ");
			float analog_read = read_ANA(slave_module, i);
			if(analog_read < 10)
				Serial.print(" ");
			if(analog_read < 100)
				Serial.print(" ");
			if(analog_read < 1000)
				Serial.print(" ");
			Serial.print( analog_read, 0 );
			Serial.println();
		}
		delay(1000);	
	}
	else if( your_board == "32CH_MUX" ){
		Serial.print("#MUX");
		Serial.println(slave_module);
		Serial.println("#If a channel not terminated(connected to ground) it's gonna be floating!");
		for(byte i = 0; i < 32; i++){
			Serial.print("#Value at channel ");
			Serial.print(i);
			if(i<10)
				Serial.print(" ");
			Serial.print(" : ");
			select_MUX(slave_module, i);//select MUX channel
			float analog_read = analogRead(MUX_pin);
			if(analog_read < 10)
				Serial.print(" ");
			if(analog_read < 100)
				Serial.print(" ");
			if(analog_read < 1000)
				Serial.print(" ");
			Serial.print( analog_read, 0 );
			Serial.println();
		}
		delay(1000);
	}
}
