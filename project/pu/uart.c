
#include "uart.h"

void USART_Init(unsigned int ubrr)
{
    // Baud setup
    UBRR0H = (unsigned char)(ubrr >> 8);
    UBRR0L = (unsigned char)ubrr;

    // Transmitter / Receiver enable
    UCSR0B = (1 << RXEN0) | (1 << TXEN0);

    // Frame format 8b data 2b stop
    UCSR0C = (1 << USBS0) | (3 << UCSZ00);
}

static int uart_putchar(char c, FILE *stream)
{
    if (c == '\n') {
        uart_putchar('\r', stream);
    }
    while (!(UCSR0A & (1 << UDRE0)))
        ;

    UDR0 = c;
    return 0;
}

static int uart_readchar(FILE *stream)
{
    int c = fgetc(stream);
    if (c != -1) {
        return c;
    }
    else {
        return -1;
    }
}

// Setup write and read
FILE mystdout = FDEV_SETUP_STREAM(uart_putchar, NULL, _FDEV_SETUP_WRITE);
FILE mystdin = FDEV_SETUP_STREAM(NULL, uart_readchar, _FDEV_SETUP_READ);
