#define F_CPU 16000000UL

#define BAUD 38400
#define BAUDRATE ((F_CPU)/(BAUD*16UL)-1) //ubrr value
#include <stdlib.h>
#include <avr/io.h>
#include <util/delay.h>
#include <math.h>
void uartinit(void)
{
    /*Set baud rate */
    UBRRH = (unsigned char)(BAUDRATE>>8);//ubrr high val
    UBRRL = (unsigned char)BAUDRATE;//ubrr low val
    /*Enable receiver and transmitter */
    UCSRB = (1<<RXEN)|(1<<TXEN);
    /* Set frame format: 8data, 1stop bit */
    UCSRC = (1<<URSEL)|(3<<UCSZ0);//ursel is used to set ucsrc and ucsz0 is 8 bit format
}
void uarttransmit (int data)
{
    while(!( UCSRA & (1<<UDRE)));//wait for register to be free
    UDR=data;//enter data into the udr register
}
void uarttransmits(char * str){
    while(*str){
        uarttransmit(*str++);//repeatedly send chars
    }
}
void adcinit(){
 ADCSRA |=(1<<ADPS2);//setting prescaler to 16
 ADMUX |= (1<<ADLAR);//left adjust the result
 ADMUX &= ~((1<<REFS0)|(1<<REFS1));//clearing any previous setting
 ADMUX |= (1<<REFS1);//setting aref to avcc (with capacitor at aref) which we didnt use
}
void adcread(int pin)
{
    ADMUX &= !((1<<MUX0)|(1<<MUX1)|(1<<MUX2)|(1<<MUX3));//clear any previous setting
    switch(pin){                                                                                     
          case 0: ADMUX |= (0<<MUX0); break; //setting input pin to ADC0                               
          case 1: ADMUX |= (1<<MUX0); break;                                                           
          case 2: ADMUX |= (0<<MUX0)|(1<<MUX1); break;                                                 
          case 3: ADMUX |= (1<<MUX0)|(1<<MUX1); break;                                             
          case 4: ADMUX |= (0<<MUX0)|(0<<MUX1)|(1<<MUX2); break;          
          default: break;            
        } 
    ADCSRA |= (1<<ADEN);//enable ADSC
    ADCSRA |= (1<<ADSC);//starting conversion
    while(ADCSRA & (1<<ADSC)){};//wait for conversion to end , adsc gets reset on completion of conversion
}
void adctransmit(){
    while(ADCSRA & (1<<ADSC)){};//waiting again for no apparent reason
    int p=ADCH;//take 8 bit reading
    char itmp[10];
    itoa(p,itmp,10);//store in itmp
    uarttransmits(itmp);//transmit itmp
}
void main (void)
{
    uartinit();//initialize uart
    adcinit();//initialize adc
    DDRB &= ~(1<<PINB0);
    while(1){
       //if(bit_is_set(PINB,0)){
        adcread(0);
        uarttransmit('+');
        //adctransmit();
        _delay_ms(10);
       //}
    //else{}
    }
}
