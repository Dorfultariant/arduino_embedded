#define F_CPU 16000000UL
#define DATA_SIZE 16
#define BAUD 9600
#define MYUBRR F_CPU / 16 / BAUD - 1

#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>
#include <util/delay.h>

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

int main(void) {
  // Output modes for LCD
  DDRB |= (1 << LCD_RW) | (1 << LCD_EN);
  DDRE |= (1 << LCD_D4) | (1 << LCD_D6);
  DDRG |= (1 << LCD_D5);
  DDRH |= (1 << LCD_RS) | (1 << LCD_D7);

  // DDRB &= ~(1 << LED_BUILTIN);

  // LCD initialization
  // lcd_init(LCD_DISP_ON);
  // lcd_gotoxy(0, 0);
  USART_Init(MYUBRR);

  stdin = &mystdin;
  stdout = &mystdout;

  TR_Data data = {"Hello Slave!\n", ""};
  printf("Hello There!\n");

  uint8_t twi_idx = 0;
  uint8_t twi_stat = 0;

  // Bit Rate generator setup to 400 000 Hz -> F_CPU / (16 + 2 * TWBR *
  // 4^(TWSR):

  TWSR = 0x00;         // Prescaler to 1
  TWBR = 0x03;         // 3x multiplier to achieve 400 kHz
  TWCR |= (1 << TWEN); // TWI enable

  while (1) {
    // Turn the led off
    DDRB &= ~(1 << LED_BUILTIN);

    // Start transmission:
    TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);

    while (!(TWCR & (1 << TWINT)))
      ;

    // Read status from TWI status register

    twi_stat = (TWSR & 0xF8);

    // Slave address
    TWDR = 0b10101010;

    // Clear TWINT to start transmit to slave + write
    TWCR = (1 << TWINT) | (1 << TWEN);

    // Wait TWINT to set
    while (!(TWCR & (1 << TWINT)))
      ;

    twi_stat = (TWSR & 0xF8);

    // Send data byte at a time
    for (uint8_t twi_d_idx = 0; twi_d_idx < DATA_SIZE; twi_d_idx++) {
      TWDR = data.tranText[twi_d_idx];
      // Reset TWINT to transmit data
      TWCR = (1 << TWINT) | (1 << TWEN);
      // Wait for TWINT to set
      while (!(TWCR & (1 << TWINT)))
        ;

      twi_stat = (TWSR & 0xF8);
    }
    printf("Ah, General Kenobi!\n");

    // Test print to see if UART connection works
    printf("%a", data.tranText);

    // Turn on the led
    PORTB |= (1 << LED_BUILTIN);

    // STOP transmission
    TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);

    _delay_ms(1000);
  }

  return 0;
}
