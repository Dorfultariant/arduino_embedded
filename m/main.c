
#define F_CPU 16000000UL
#define DATA_SIZE 16
#define BAUD 9600
#define MYUBRR F_CPU / 16 / BAUD - 1

#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>
#include <util/delay.h>
#include <util/setbaud.h>

#include "lcd.h"
#include "uart.h"

const int LED_BUILTIN = PB7;

const int LCD_RS = PH6;
const int LCD_RW = PB4;
const int LCD_EN = PB5;

const int LCD_D4 = PE5;
const int LCD_D5 = PG5;
const int LCD_D6 = PE3;
const int LCD_D7 = PH3;

// 32 Bytes worth of data
typedef struct tr_data {
  char tranText[DATA_SIZE];
  char recvText[DATA_SIZE];
} TR_Data;

void TWI_Init();

void TWI_Transmit(char *data);

int main(void) {

  USART_Init(MYUBRR);

  stdin = &mystdin;
  stdout = &mystdout;

  TR_Data data = {"Hello Slave!\n", ""};
  printf("Hello There!\n");

  // uint8_t twi_idx = 0;
  TWI_Init();

  while (1) {
    // Turn the led off
    DDRB &= ~(1 << LED_BUILTIN);

    TWI_Transmit(170, data.tranText);

    PORTB |= (1 << LED_BUILTIN);

    _delay_ms(1000);
  }

  return 0;
}

void TWI_Init() {
  // Bit Rate generator setup to 400 000 Hz -> F_CPU / (16 + 2 * TWBR *
  // 4^(TWSR):

  TWSR = 0x00;         // Prescaler to 1
  TWBR = 0x03;         // 3x multiplier to achieve 400 kHz
  TWCR |= (1 << TWEN); // TWI enable
}

void TWI_Transmit(uint8_t address, char *data) {
  uint8_t twi_stat = 0;

  // Start transmission:
  TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);

  while (!(TWCR & (1 << TWINT)))
    ;

  // Read status from TWI status register

  twi_stat = (TWSR & 0xF8);

  // Slave address
  TWDR = address;

  // Clear TWINT to start transmit to slave + write
  TWCR = (1 << TWINT) | (1 << TWEN);

  // Wait TWINT to set
  while (!(TWCR & (1 << TWINT)))
    ;

  twi_stat = (TWSR & 0xF8);

  // Send data byte at a time
  for (uint8_t twi_d_idx = 0; twi_d_idx < DATA_SIZE; twi_d_idx++) {
    TWDR = data[twi_d_idx];

    // Reset TWINT to transmit data
    TWCR = (1 << TWINT) | (1 << TWEN);

    // Wait for TWINT to set
    while (!(TWCR & (1 << TWINT)))
      ;

    twi_stat = (TWSR & 0xF8);
  }

  // Data sent:
  printf("%s", data);

  // STOP transmission
  TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);
}
