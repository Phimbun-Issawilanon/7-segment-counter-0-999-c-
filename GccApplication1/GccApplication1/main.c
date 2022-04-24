#include <avr/io.h>
#include <avr/eeprom.h>
#define F_CPU 16000000UL
#include "util/delay.h"
#define  dg2on PORTB |=(1<<PORTB1)
#define  dg3on PORTB |=(1<<PORTB0)
#define  dg1on PORTB |=(1<<PORTB2)
#define  ledOn PORTB |=(1<<PORTB5)
#define  ledOff PORTB &= ~(1<<PORTB5)
#define  dg1off PORTB &= ~(1<<PORTB2)
#define  dg2off PORTB &= ~(1<<PORTB1)
#define  dg3off PORTB &= ~(1<<PORTB0)
#define EEADDR 0x00
#define EEADDR2 0x30
unsigned char  num[10]= { 0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F};
unsigned int  x, i, j;
int count;
int sw, checkRound;
volatile uint16_t EEByte,EEByte2;

int main(void)
{
	DDRD |= (1<<DDD1);   //DATA_pin
	DDRD |= (1<<DDD2);   //LATCH_pin
	DDRD |= (1<<DDD0);   //CLCOK_pin
	DDRB |= (1<<DDB5);  //led
	DDRC &= ~(1<<DDC0);  //PORTD PD0 IS USED FOR INPUT switch count up
	DDRC &= ~(1<<DDC1);  //PORTD PD0 IS USED FOR INPUT switch count down
	DDRC &= ~(1<<DDC2);  //PORTD PD0 IS USED FOR INPUT switch stop
	DDRC &= ~(1<<DDC3);  //PORTD PD0 IS USED FOR INPUT switch clear
	DDRB |= (1<<DDB0);   //segment dg1
	DDRB |= (1<<DDB1);   //segment dg2
	DDRB |= (1<<DDB2);   //segment dg3
	EEByte = eeprom_read_word((uint16_t*)EEADDR);  //count
	EEByte2 = eeprom_read_word((uint16_t*)EEADDR2); // checkround
	checkRound = EEByte2;
	count = EEByte;
	while(1){						// check sw
		if (PINC&(1<<PINC0)){   //sw count up
			ledOff;
			start(1);
		}
		else if (PINC&(1<<PINC1)){  //sw count down
			ledOff;
			start(0);
		}
		else if (PINC&(1<<PINC3)){ //sw clear
			ledOff;
			count = 0;
			checkRound = 0;
			eeprom_write_word ((uint16_t*)EEADDR, count);
			eeprom_write_word ((uint16_t*)EEADDR2, checkRound);
		}
		else {
			showSeg(count);
		}
	}
}
void start(int select){
	sw = select;
	if (sw == 1){
		swup(count);
	}
	else if (sw == 0){        //sw count down
		if (checkRound == 0){ // count down round 1
			count = 999-count;
			swdown(count);
		}
		else if (checkRound > 0) {
			swdown(count);
		}
	}
}
void swup(){
	checkRound++;			//sw count up
	eeprom_write_word ((uint16_t*)EEADDR2, checkRound);
	while(count<999){
		count+=1; //Increase The Counter
		showSeg(count);
		eeprom_write_word ((uint16_t*)EEADDR, count);
		
		if (PINC&(1<<PINC2)){								//sw stop
			eeprom_write_word ((uint16_t*)EEADDR, count);
			ledOn;
			return count;
		}
		if (PINC&(1<<PINC3)){								//sw clear
			count = 0;
			eeprom_write_word ((uint16_t*)EEADDR, count);
			return count;
		}
		if (PINC&(1<<PINC1)){							 //sw count down
			sw = 0;
			return sw;
		}
	}
}
void swdown(){
	checkRound++;			//sw count up
	eeprom_write_word ((uint16_t*)EEADDR2, checkRound);
	while(count>0){
		count-=1; //Increase The Counter
		showSeg(count);
		eeprom_write_word ((uint16_t*)EEADDR, count);
		
		if (PINC&(1<<PINC2)){							//sw stop
			eeprom_write_word ((uint16_t*)EEADDR, count);
			ledOn;
			return count;
		}
		if (PINC&(1<<PINC3)){					//sw clear
			count = 0;
			eeprom_write_word ((uint16_t*)EEADDR, count);
			return count;
		}
		if (PINC&(1<<PINC0)){				 //sw count up
			sw = 1;
			return sw;
		}
	}
}
void showSeg(int count){
	x = (count/100) % 10;   
	dg3on;
	shiftOut(~num[x]);
	_delay_ms(30);
	dg3off;
	
	i = (count/10) % 10; 
	dg2on;
	shiftOut(~num[i]);
	_delay_ms(20);
	dg2off;
	
	j = count % 10;		
	dg1on;
	shiftOut(~num[j]);
	_delay_ms(25);
	dg1off;
}
void shiftOut(unsigned char data){
	PORTD &=~(1<<PORTD2);
	for(int i=0;i<8;i++){
		PORTD &=~(1<<PORTD0);

		if((data&0x80)!=0)
		PORTD|=(1<<PORTD1);

		else PORTD &=~(1<<PORTD1);
		PORTD|=(1<<PORTD0);
		data<<=1;
		_delay_us(1);
	}
	PORTD|=(1<<PORTD2);
}