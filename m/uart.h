
#ifndef _UART_H
#define _UART_H

#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>

// Initialize the USART with registers
void USART_Init(unsigned int ubrr);

// Insert data into uart communication link:
static int uart_putchar(char c, FILE *stream);

// Read data from communication link
static int uart_readchar(FILE *stream);

// Export stdin and stdout
extern FILE mystdout;
extern FILE mystdin;

#endif // _UART_H
