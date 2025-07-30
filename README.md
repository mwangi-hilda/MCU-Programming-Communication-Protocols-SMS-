# MCU-Programming-Communication-Protocols-SMS-
This project demonstrates how to use the SIM800L GSM module with an AVR microcontroller to receive and send SMS messages for remote control and communication.

## Features

- Send SMS to any number via AT commands
- Receive and parse incoming SMS
- Extract sender's number and message content
- Respond to specific commands (e.g., turning devices on/off)
- Text mode SMS interface via `AT+CMGF=1`
- Real-time SMS display using `AT+CNMI=2,2,0,0,0`

## Technologies

- AVR C (ATmega family)
- SIM800L GSM Module
- USART serial communication
- AVR-GCC, Microchip Studio
