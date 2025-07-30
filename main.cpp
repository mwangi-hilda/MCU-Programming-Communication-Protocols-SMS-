#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <string.h>
#include <stdbool.h>

//USART1 function prototypes
void usart1_init(uint32_t baud_rate);
void usart1_send_char(char data);
void usart1_send_string(const char *str);

char usart1_receive_char(void);
void usart1_receive_line(char* buffer, uint8_t buffer_length);

//GSM helpers
bool wait_for_ok();
void send_sms(const char* number, const char* message);

int main(void)
{
	usart1_init(115200);
	_delay_ms(3000); //Let SIM800l boot
	
	DDRB |= (1<<PB0);
	
	usart1_send_string("AT+CMGF=1\r\n");
	_delay_ms(500);
	wait_for_ok();
	
	usart1_send_string("AT+CNMI=2,2,0,0,0\r\n"); //enable incoming SMS forwarding
	_delay_ms(500);
	wait_for_ok();
	
	char line[160];
	char phone[30];
	
    while (1) 
    {
		usart1_receive_line(line, sizeof(line));
		
		if (strstr(line, "+CMT:")) //detect new SMS header
		{
			char* start = strchr(line, '"');
			if (start)
			{
				start++;
				char* end = strchr(start, '"');
				if (end)
				{
					*end = '\0';
					strncpy(phone, start, sizeof(phone));
				}
			}
			
			//get next line: actual message content
			usart1_receive_line(line, sizeof(line));
			
			if (strstr(line, "light on"))
			{
				PORTB |= (1<<PB0);
				send_sms(phone, "Light has been turned ON");
			} else if (strstr(line, "light off"))
			{
				PORTB &= ~(1<<PB0);
				send_sms(phone, "Light has been turned OFF");
			} else
			{
				send_sms(phone, "Invalid command");
			}
		}
    }
}

//serial communication set-up
void usart1_init(uint32_t baud_rate){
	uint16_t ubrr_value = (F_CPU / (16 * baud_rate)) - 1;
	UBRR1H = (unsigned char) (ubrr_value >> 8);
	UBRR1L = (unsigned char) ubrr_value;
	
	UCSR1A &= ~(1<<U2X1); //no double-speed
	UCSR1B |= (1<<RXEN1) | (1<<TXEN1); //enabling receiver & transmitter
	UCSR1B &= ~(1<<UCSZ12); //configuring the char size to 8 (together with UCSZ11 & UCSZ10)
	//asynchronous communication, no parity, 1 stop bit:
	UCSR1C &= ~((1<<UMSEL11) | (1<<UMSEL10) | (1<<UPM11) | (UPM10) | (USBS1));
	UCSR1C |= (1<<UCSZ11) | (1<<UCSZ10); //char size = 8
}

void usart1_send_char(char data){
	while(!(UCSR1A & (1<<UDRE1))); //while transmit buffer is not empty/not ready to receive data, wait
	UDR1 = data; //when empty, transmit you data to the transmit data register
}

void usart1_send_string(const char *str){
	while(*str){
		usart1_send_char(*str++);
	}
}

char usart1_receive_char(void){
	while(!(UCSR1A & (1<<RXC1))); //while there's no unread data in the receive buffer, wait
	return UDR1; //when there is unread data in the receive buffer, return that data in the buffer
}

void usart1_receive_line(char* buffer, uint8_t buffer_length){
	uint8_t i = 0;
	char c;
	do 
	{
		c = usart1_receive_char();
		if (i < buffer_length - 1)
		{
			buffer[i++] = c;
		}
	} while (c != '\n');
	buffer[i] = '\0';
}

bool wait_for_ok(){
	char buffer[64];
	for (uint8_t i = 0; i < 5; i++)
	{
		usart1_receive_line(buffer, sizeof(buffer)); //receive whatever is in the receive buffer and store it in our array buffer
		if (strstr(buffer, "OK"))
		{
			return true;
		}
		if (strstr(buffer, "ERROR"))
		{
			return false;
		}
	}
	return false; //if you don't get anything, return false
}

void send_sms(const char* number, const char* message){
	usart1_send_string("AT\r\n"); //attention command to the GSM module
	_delay_ms(500);
	if(!wait_for_ok()) return; //if it doesn't return OK just exit the function
	
	usart1_send_string("AT+CMGF=1\r\n"); //set sms to text mode
	_delay_ms(500);
	if(!wait_for_ok()) return;
	
	usart1_send_string("AT+CMGS=\"");
	usart1_send_string(number);
	usart1_send_string("\"\r\n");
	_delay_ms(500);
	
	usart1_send_string(message);
	usart1_send_char(26); //to signal end of message
	_delay_ms(5000);	
}