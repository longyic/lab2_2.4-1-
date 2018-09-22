/*
 * lab2_2.4(1).c
 *
 * Created: 2018/9/19 13:05:49
 * Author : Lenovo
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include "uart.h"
volatile unsigned int edge1, edge2, pulse;
volatile int overflows, rising, counter, memory;
float freq;
void length();
void frequency_continuous();
void frequency_discrete();

int main(void)
{
	uart_init();
	
	DDRB |= (1 << PORTB1);
	DDRD |= (1 << PORTD6);
	DDRB &= ~(1 << PORTB0);
	PORTB |= (1 << PORTB1) | (1 <<PORTB0);
	
	TIMSK1 |= (1 << ICIE1) | (1 << TOIE1);      //edge interrupt and overflow interrupt
	TCCR1B |= (1 << CS10) | (1 << ICES1);      //no prescale and input capture
	
	TCCR0A |= (1 << COM0A0) | (1 << WGM01);    //toggle OC0A and CTC
	TCCR0B |= (1 << CS02);                    //prescale 64
	
	TCNT0 = 0;                         //initial time counter0
	TCNT1 = 0;                         //initial time counter1
	
	edge1 = 0;
	edge2 = 0;
	rising = 0;
	overflows = 0;
	counter = 0;
	memory = 0;

	sei();
	while (1)
	{
		if (TCNT1 >= 79)
		{
			PORTB ^= (1 << PORTB1);     //change from output to input
			TCNT1 = 0;                  //clear counter
		}
		length();
		TIMSK0 |= (1 << OCIE0A);
		if(!(PINB & 0x01))
		{
			frequency_discrete();
		}
		else
		{
			frequency_continuous();
		}
	}
}

void length()
{
	if(edge2 < edge1)
	{
		overflows--;
	}
	pulse = overflows * 65536 + (edge2 - edge1);
}


ISR(TIMER1_CAPT_vect)
{
	if(!rising){
		edge1 = ICR1;
		rising = 1;
		TCCR1B &= ~(1 << ICES1);          //input capture negative edge
		TIFR1 |= (1 << ICR1);             //clear capture flag
	}
	else{
		edge2 = ICR1;
		rising = 0;
		TIFR1 |= (1 << ICR1);             //clear capture flag
		TCCR1B |= (1 << ICES1);           //input capture positive edge
	}
}


ISR(TIMER1_OVF_vect)
{
	overflows++;
}

ISR(TIMER0_COMPA_vect)
{
	memory = pulse;
	if(memory < 500)
	{
		memory = 500;
	}
	if(memory > 5000)
	{
		memory = 5000;
	}
	counter = 14 + (memory/5000.0) * 16;
	PORTD ^= (1 << PORTD6);                        //output PD6
	TIMSK0 &= ~(1 << OCIE0A);                      //disable match interrupt
}

void frequency_continuous(void)
{
	OCR0A = counter;
	freq = 2093 - (memory/5000.0f) * 1046.5;
	printf("Frequency is %.2f Hz\n", freq);
}

void frequency_discrete(void)
{
	float range = memory/5000.0f;
	if(range > 0 && range <= 0.2125){
		freq = 2093;
		OCR0A = 14;
	}
	if(range > 0.2125 && range <= 0.325){
		freq = 1975.53;
		OCR0A = 15;
	}
	if(range > 0.325 && range <= 0.4375){
		freq = 1760;
		OCR0A = 17;
	}
	if(range > 0.4375 && range <= 0.55){
		freq = 1567.98;
		OCR0A = 19;
	}
	if(range > 0.55 && range <= 0.6625){
		freq = 1396.91;
		OCR0A = 22;
	}
	if(range > 0.6625 && range <= 0.775){
		freq = 1318.51;
		OCR0A = 23;
	}
	if(range > 0.775 && range <= 0.8875){
		freq = 1174.66;
		OCR0A = 26;
	}
	if(range > 0.8875 && range <= 1){
		freq = 1046.5;
		OCR0A = 28;
	}
	printf("Frequency is %.2f Hz\n", freq);
}


