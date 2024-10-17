#define F_CPU 8000000UL
#include <util/delay.h>
#include <avr/io.h>
#include "TempAndHumidity.h"

uint8_t c = 0, integralRh, decimalRh, integralTemp, decimalTemp, checksum;



void Request()						/* Microcontroller send start pulse or request */
{
	DDRD |= _BV(DHT11_PIN);			// Request is sent from MCU PIN
	PORTD |= _BV(DHT11_PIN);
	PORTD &= ~_BV(DHT11_PIN);		/* set to low pin, pull down */
	_delay_ms(20);					/* wait for 20ms */
	PORTD |= _BV(DHT11_PIN);		/* set to high pin, pull up */
}

void Response()						/* receive response from DHT11 */
{
	DDRD &= ~_BV(DHT11_PIN);				// explicitly pull up PIN
	while(PIND & _BV(DHT11_PIN));			// check to see if state changed from high to low
	while((PIND & _BV(DHT11_PIN)) == 0);	// check if pulled down voltage is equal to zero
	while(PIND & _BV(DHT11_PIN));			// check to see if state change from low to high
}

uint8_t Receive_data()							/* receive data */
{	
	/*
	The data frame is of total 40 bits long, it contains 5 segments and each segment
	is 8-bit long. We check each bit if it is high or low
	*/
	for (uint8_t q=0; q<8; q++) {
		while((PIND &  _BV(DHT11_PIN)) == 0);/* check received bit 0 or 1, if pulled up */
		_delay_us(60);
		/* if high pulse is greater than 30ms */
		if(PIND & _BV(DHT11_PIN)) {
			/* then it is logic HIGH */
			c = (c << 1) | (0x01);
		} else {
			/* otherwise it is logic LOW */
			c = (c << 1);
		}								
		while(PIND & _BV(DHT11_PIN));
	}
	return c;
}