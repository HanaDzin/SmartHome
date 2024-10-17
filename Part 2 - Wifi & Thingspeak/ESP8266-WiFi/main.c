#define F_CPU 7372800UL			
#include <avr/io.h>					
#include <util/delay.h>				
					
#include "USART_RS232_H_file.h"		
#include "ESP8266.h"
#include "TempAndHumidity.h"



int main(void) {

	char WiFiBuffer[150];
	uint8_t connectStatus;

	ADMUX = _BV(REFS0);
	ADCSRA = _BV(ADEN) | _BV(ADPS2) | _BV(ADPS1);
	
	
	uint8_t integralRh, decimalRh, integralTemp, decimalTemp, checksum;
	

	USART_Init(115200);						
	sei();									

	while (!ESP8266_Begin());
	
	ESP8266_WIFIMode(BOTH_STATION_AND_ACCESPOINT);	// 3 = Both (AP and STA) 
	ESP8266_ConnectionMode(SINGLE);					// 0 = Single; 1 = Multi 
	ESP8266_ApplicationMode(NORMAL);				// 0 = Normal Mode; 1 = Transperant Mode
	 
	if (ESP8266_connected() == ESP8266_NOT_CONNECTED_TO_AP) ESP8266_JoinAccessPoint(SSID, PASSWORD);
	ESP8266_Start(0, DOMAIN, PORT);
	
	while (1) {
		connectStatus = ESP8266_connected();
		if (connectStatus == ESP8266_NOT_CONNECTED_TO_AP) ESP8266_JoinAccessPoint(SSID, PASSWORD);
		if (connectStatus == ESP8266_TRANSMISSION_DISCONNECTED) ESP8266_Start(0, DOMAIN, PORT);
				
		Request();			
		Response();				
		integralRh = Receive_data();	
		decimalRh = Receive_data();	
		integralTemp = Receive_data();	
		decimalTemp = Receive_data();	
		checksum = Receive_data();
		
		ADCSRA |= _BV(ADSC);

		while (!(ADCSRA & _BV(ADIF)));				
		
		//_delay_ms(500);

		memset(WiFiBuffer, 0, 150);
		sprintf(WiFiBuffer, "GET /update?api_key=%s&field1=%.2d&field2=%.2d&field3=%d", API_WRITE_KEY, integralTemp, integralRh, ADC);
		ESP8266_Send(WiFiBuffer);
		_delay_ms(5000);		// Thingspeak server delay 	

	}
}
