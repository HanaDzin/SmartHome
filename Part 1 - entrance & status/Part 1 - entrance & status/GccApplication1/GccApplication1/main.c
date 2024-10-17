#define F_CPU 8000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdio.h>

#include <avr/fuse.h>

#include "lcd.h"
#include "Keypad.h"
#include "TempAndHumidity.h"

//postavljanje fuse bitova
/*FUSES = {
	.low = FUSE_CKSEL1 & FUSE_CKSEL2 & FUSE_CKSEL3 & FUSE_SUT0,
	.high = FUSE_BOOTSZ0 & FUSE_CKOPT,
};*/

#define OPTION_TEMPERATURE 1
#define OPTION_HUMIDITY    2

enum State {
	STATE_MAIN_MENU,
	STATE_TEMPERATURE,
	STATE_HUMIDITY,
};


void init_LCD() {
	DDRD = _BV(4);

	TCCR1A = _BV(COM1B1) | _BV(WGM10);
	TCCR1B = _BV(WGM12) | _BV(CS11);
	OCR1B = 170;

	lcd_init(LCD_DISP_ON);
	lcd_clrscr();
	lcd_gotoxy(2,0);
	lcd_puts("Dobrodosli u");
	lcd_gotoxy(0, 1);
	lcd_puts("pametnu kucu :)");
	_delay_ms(1500);
	lcd_clrscr();
}

void inputPassword(char enteredPassword[]) {
	int i = 0;
	
	lcd_gotoxy(0, 0);
	lcd_puts("Unesite lozinku");
	lcd_gotoxy(0, 1);
	
	while(i < 4) {
		enteredPassword[i] = keyfind();
		lcd_puts("*");
		i++;
	}
	
	lcd_clrscr();
}

void doorUnlock () {
	lcd_clrscr();	
	lcd_puts("Kuca otkljucana");
	_delay_ms(1500);
	lcd_clrscr();
}

void buzzerSound() {
		PORTB |= (1 << PB0); // Turn on the buzzer
		_delay_ms(3000);      // Delay for sound duration
		PORTB &= ~(1 << PB0);// Turn off the buzzer

	
}

int checkPassword(uint8_t *wrongCounter) {
	
	if (*wrongCounter == 2) {
		lcd_clrscr();
		lcd_puts("Ulaz zabranjen");		
		buzzerSound();
		//DDRB &= ~(1 << PB0); // Set PB0 as input to stop sound
		lcd_clrscr();
	
		return 0;
		
		} else {
		lcd_clrscr();
		lcd_puts("Pokusajte opet");
		
		_delay_ms(500);
		lcd_clrscr();
		lcd_gotoxy(1,0);
		lcd_puts("Unesi lozinku");
		(*wrongCounter)++;
	}
	
	return 1;
	
} 

void displayMainMenu() {
		lcd_clrscr();
		lcd_puts("Odaberite opciju:");
		_delay_ms(1000);

			
		lcd_clrscr();
		lcd_gotoxy(1,0);
		lcd_puts("1. Temperatura");
		lcd_gotoxy(1, 1);
		lcd_puts("2. Vlaga");
		
		_delay_ms(2000);
}

void displayTemperature(int integralTemp, int decimalTemp) {
	char messageString[16];
	lcd_clrscr();
	lcd_puts("Trenutna temp:");
	sprintf(messageString, "%2d.%1d %cC", integralTemp, decimalTemp, 223);
	lcd_gotoxy(5, 1);
	lcd_puts(messageString);
}

void displayHumidity(int integralRh, int decimalRh) {
	char messageString[16];
	lcd_clrscr();
	lcd_puts("Trenutna vlaga:");
	sprintf(messageString, "%2d.%1d %%", integralRh, decimalRh);
	lcd_gotoxy(5, 1);
	lcd_puts(messageString);
}

int main(void) {
	
	
	init_LCD();
	
	char password[4] = {'1', '2', '3', '4'};
	char enteredPassword[4];
	int wrongCounter = 0, accessGranted = 0;
	
	uint8_t c = 0, integralRh, decimalRh, integralTemp, decimalTemp, checksum;
	char messageString[16];
	
	enum State currentState = STATE_MAIN_MENU;
	
	DDRB |= (1 << PB0);

	lcd_gotoxy(0, 1);

			
	inputPassword(enteredPassword);
	
	
	
	if(!memcmp(enteredPassword, password, sizeof(enteredPassword))) {
		doorUnlock();
		accessGranted = 1;
		} else {
		while (wrongCounter < 3) { // Change the loop condition
			if (!checkPassword(&wrongCounter)) {
				accessGranted = 0;
				break;
			}
			
		inputPassword(enteredPassword);
		if (memcmp(enteredPassword, password, sizeof(enteredPassword)) == 0) {
			doorUnlock();
			accessGranted = 1;
			break;
			}
		}

		if (wrongCounter == 3) { // Add this condition
			accessGranted = 0;
		}
	}
	

		
	while(1) {
		
			Request();
			Response();
			
			integralRh = Receive_data();
			decimalRh = Receive_data();
			integralTemp = Receive_data();
			decimalTemp = Receive_data();
			checksum = Receive_data();				
			
		
			if (accessGranted) {
				switch (currentState) {
					case STATE_MAIN_MENU:
					displayMainMenu();
					int pressedKey = keyfind();
					if (pressedKey == '1') {
						currentState = STATE_TEMPERATURE;
						} else if (pressedKey == '2') {
						currentState = STATE_HUMIDITY;
					}
					break;
					
					case STATE_TEMPERATURE:
					displayTemperature(integralTemp, decimalTemp);
					int tempKey = keyfind();
					if (tempKey == '#') {
						currentState = STATE_MAIN_MENU;
					}
					break;
					
					case STATE_HUMIDITY:
					displayHumidity(integralRh, decimalRh);
					int humidityKey = keyfind();
					if (humidityKey == '#') {
						currentState = STATE_MAIN_MENU;
					}
					break;
				}
				
				_delay_ms(1000);
	}
			}
	}
			