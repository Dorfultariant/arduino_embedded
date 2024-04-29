#define F_CPU 16000000UL
#define SLAVE_ADDRESS 85
#define BAUD 9600
#define MYUBRR F_CPU / 16 / BAUD - 1
#define DATA_SIZE 16

#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>
#include <util/delay.h>

#include "lcd.h"
#include "uart.h"

const int BUILTIN_LED = PB5;

const int LCD_RS = PB1;
const int LCD_RW = PB2;
const int LCD_EN = PB3;

const int LCD_D4 = PD3;
const int LCD_D5 = PD4;
const int LCD_D6 = PD5;
const int LCD_D7 = PD6;

int main(void) {

  // OUTPUTS CONTROL
  DDRB |= (1 << LCD_RS) | (1 << LCD_RW) | (1 << LCD_EN);
  // OUTPUTS DATA
  DDRD |= (1 << LCD_D4) | (1 << LCD_D5) | (1 << LCD_D6) | (1 << LCD_D7);

  // Turn off led
  DDRB &= ~(1 << BUILTIN_LED);

  USART_Init(MYUBRR);

  lcd_init(LCD_DISP_ON);
  lcd_clrscr();

  lcd_gotoxy(0, 1);

  lcd_puts("Hello There!");

  stdin = &mystdin;
  stdout = &mystdout;

  printf("Hello There!\n");

  while (1) {
    printf("Ah, General Kenobi!\n");
    PORTB ^= (1 << BUILTIN_LED);
    _delay_ms(500);
  }

  return 0;
}
