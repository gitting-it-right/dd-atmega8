#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#define BAUD 38400// baud rate
#define BAUDRATE ((F_CPU)/(BAUD*16UL)-1) // ubrr value

#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <math.h>

#include "mpu6050/mpu6050.h"
#include "mpu6050/mpu6050.c"
#include "mpu6050/mpu6050dmp6.c"
#include "mpu6050/mpu6050registers.h"

#include "i2chw/i2cmaster.h"
#include "i2chw/twimastertimeout.c"

int scaled(double reading, int i){
	double l; double u;
	switch(i){
		case 0: l = 65; u = 130; break; 
		case 1:	l = 80; u = 170; break;
		case 2: l = 70; u = 115; break;
		case 3: l = 70; u = 125; break;
		default: break;
	}
	
	if(reading < l) return 0;
	if(reading > u) return 100;
	
	return ((reading - l)/(u-l))*100;
}

void uartinit (void)
{
	UBRRH |= (unsigned char) (BAUDRATE>>8);
	UBRRL |= (unsigned char) BAUDRATE;
	UCSRB |= (1<<TXEN) | (1<<RXEN); //enable receiver and transmitter
	UCSRC |= (1<<URSEL)|(3<<UCSZ0); // frame set
}

void uarttransmit (int data)
{
	while (!( UCSRA & (1<<UDRE))); // wait till register is free
	//data = 99;
	UDR = 9; // load data in the register
	//while (!( UCSR0A & (1<<UDRE0))); // second wait
	
}

void uarttransmits(char * str){
	while (*str){
		uarttransmit(*str++);
	}
}

void adcinit(){
	
	//ADCSRA |= (1<<ADEN); // enabling adc
	ADCSRA |= (1<<ADPS2); // prescaler to 16
	ADMUX |= (1<<ADLAR); // left adjust
	ADMUX &= ~((1<<REFS0)|(1<<REFS1));// reference voltage // refs0 = 0 and refs1 = 0
	ADMUX |= (1<<REFS0);
	
}

void adcread(int pin){
	
	//while(!ADIF){}
	ADMUX &= ~((1<<MUX0)|(1<<MUX1)|(1<<MUX2)|(1<<MUX3));
	
	switch(pin){
		case 0:	ADMUX |= (0<<MUX0); break; //setting input pin to ADC0
		case 1: ADMUX |= (1<<MUX0); break;
		case 2: ADMUX |= (0<<MUX0)|(1<<MUX1); break;
		case 3: ADMUX |= (1<<MUX0)|(1<<MUX1);
					//PORTB &= ~((1<<PINB3)|(1<<PINB4)|(1<<PINB5));
					break;
		case 4: ADMUX |= (1<<MUX0)|(1<<MUX1);
					//PORTB &= ~((1<<PINB3)|(1<<PINB4)|(1<<PINB5));
					//PORTB |= (1<<PINB3);
					break;
		default: break;
	}
	ADCSRA |= (1<<ADEN); // enabling adc everytime read is done
	ADCSRA |= (1<<ADSC);// starting conversion
	while(ADCSRA & (1<<ADSC)){};
}

void adctransmit(int i){
	while(ADCSRA & (1<<ADSC)){};
	int p = ADCH;
	//int q = scaled(p,i);
	char itmp[10];
	itoa(p, itmp, 10); uarttransmits(itmp);
}
int uartreceive(){
    /* Wait for data to be received */
    while ( !(UCSRA & (1<<RXC)));
    /* Get and return received data from buffer */
    return UDR;
}


void mpu(int mode) {
	mpumode = mode;
	
	#if MPU6050_GETATTITUDE == 0
    int16_t ax = 0;
    int16_t ay = 0;
    int16_t az = 0;
    int16_t gx = 0;
    int16_t gy = 0;
    int16_t gz = 0;
    double axg = 0;
    double ayg = 0;
    double azg = 0;
    double gxds = 0;
    double gyds = 0;
    double gzds = 0;
	#endif

	#if MPU6050_GETATTITUDE == 1 || MPU6050_GETATTITUDE == 2
    double qw = 1.0f;
	double qx = 0.0f;
	double qy = 0.0f;
	double qz = 0.0f;
	double roll = 0.0f;
	double pitch = 0.0f;
	double yaw = 0.0f;
	#endif

    //init uart
	//uart_init(UART_BAUD_SELECT(UART_BAUD_RATE,F_CPU));
	
	//init interrupt
	sei();

	//init mpu6050
	mpu6050_init();
	//_delay_ms(50);

	//init mpu6050 dmp processor
	#if MPU6050_GETATTITUDE == 2
	//mpu6050_dmpInitialize();
	//mpu6050_dmpEnable();
	//_delay_ms(10);
	#endif

	#if MPU6050_GETATTITUDE == 0
	mpu6050_getRawData(&ax, &ay, &az, &gx, &gy, &gz);
	mpu6050_getConvData(&axg, &ayg, &azg, &gxds, &gyds, &gzds);
	#endif

	#if MPU6050_GETATTITUDE == 1
	mpu6050_getQuaternion(&qw, &qx, &qy, &qz);
	mpu6050_getRollPitchYaw(&roll, &pitch, &yaw);
	//_delay_ms(10);
	#endif

	#if MPU6050_GETATTITUDE == 2
	if(mpu6050_getQuaternionWait(&qw, &qx, &qy, &qz)) {
		mpu6050_getRollPitchYaw(qw, qx, qy, qz, &roll, &pitch, &yaw);
	}
	//_delay_ms(10);
	#endif

	#if MPU6050_GETATTITUDE == 0
	char itmp[10];
			
	//uarttransmit(ax);
	ltoa(ax, itmp, 10); uarttransmits(itmp);
	uarttransmit('+');
	ltoa(ay, itmp, 10); uarttransmits(itmp);
	uarttransmit('+');
	ltoa(az, itmp, 10); uarttransmits(itmp);

	//_delay_ms(100);
	#endif

	#if MPU6050_GETATTITUDE == 1 || MPU6050_GETATTITUDE == 2
	
	//roll pitch yaw
	char ptr[20];
	dtostrf(roll,50,6,ptr);
	uarttransmits(ptr);
	uarttransmit('+');
	dtostrf(pitch,50,6,ptr);
	uarttransmits(ptr);
	uarttransmit('+');
	dtostrf(yaw,50,6,ptr);
	uarttransmits(ptr);
	
	#endif
}

int main(void){
	uartinit();
	adcinit();
	
	//sei();

	DDRB |= (1<<PINB3)|(1<<PINB4)|(1<<PINB5); // selection pins MUX sensors 3 and 4
	while(1){
		uarttransmit('#');
		for (int i=0; i<4; i++)
		{
			adcread(i); adctransmit(i);
			uarttransmit('+');
		}
        _delay_ms(100);
        //PORTB |= (1<<PINB3);
	    //int adc5=uartreceive();	
        //_delay_us(10);
        //PORTB |= (0<<PINB3);
        //uarttransmit(adc5); 
		uarttransmit('+');
		//mpu(0);
		uarttransmit('+');
		//mpu(1);
		uarttransmit('~');
	}
}
