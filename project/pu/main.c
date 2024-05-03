#include "timer1.h"
#define F_CPU 16000000UL
#define SLAVE_ADDRESS 170
#define BAUD 9600
#define MYUBRR (F_CPU / 16 / BAUD - 1)

#define DATA_SIZE 16
#define CODE_ARRAY_LENGTH 5

#include <avr/interrupt.h>
#include <stdint.h>
#include <stdio.h>
#include <util/delay.h>

#include "lcd.h"
#include "notes.h"
#include "timer1.h"
#include "uart.h"

// LCD Display PINS WARNING remember to change from lcd.h also
const int LCD_RS = PB2;
const int LCD_RW = PB3;
const int LCD_EN = PB4;

const int LCD_D4 = PD4;
const int LCD_D5 = PD5;
const int LCD_D6 = PD6;
const int LCD_D7 = PD7;

// Buzzer pin
const int BUZZER = PB1;
const int BUILTIN = PB5;

// Parser to check system condition
void parser(char *data);

// I2C / TWI communication initialization with Master.
void I2C_InitSlaveReceiver(uint8_t address);

// I2C / TWI receive from Master.
void I2C_Receive(char *dest);

// Array length calculator
uint16_t len(char *data);

// Rearm system
void rearm(char *recv);

// Interrupt routine for timer
ISR(TIMER1_COMPA_vect) { TCNT1 = 0; }

// Main function that includes the main loop
int main(void)
{
    // // LCD PINS
    // OUTPUTS CONTROL
    DDRB |= (1 << LCD_RS) | (1 << LCD_RW) | (1 << LCD_EN) | (1 << BUILTIN);
    // OUTPUTS DATA
    DDRD |= (1 << LCD_D4) | (1 << LCD_D5) | (1 << LCD_D6) | (1 << LCD_D7);

    // Buzzer OUTPUT
    DDRB |= (1 << BUZZER);

    // Initialize empty recv char array
    char recv[DATA_SIZE + 1] = {'\0'};

    // Init debug communication Through USB
    USART_Init(MYUBRR);
    stdin = &mystdin;
    stdout = &mystdout;

    // Init LCD display
    lcd_init(LCD_DISP_ON);
    lcd_clrscr();
    lcd_puts("Welcome!");

    // Setup TWI communication with Master
    I2C_InitSlaveReceiver(SLAVE_ADDRESS);

    for (;;) {
        printf("Going to take a while\n");

        // wait for transmission:
        while (!(TWCR & (1 << TWINT))) {
            // do buzzer while waiting
            // TODO: buzzer functionality
            PORTB |= (1 << BUILTIN);
            _delay_ms(200);
            PORTB &= ~(1 << BUILTIN);
            _delay_ms(200);
        }

        printf("Finally out of a while\n");
        I2C_Receive(recv);
        lcd_clrscr();

        parser(recv);


        printf("Recv: %s \n", recv);
    }
    return 0;
}

/*
 * Check the system status based on received data
 *
 * @param char *data received from Master
 * @param char *code potential storage for users code
 *
 * @returns void
 */
void parser(char *data)
{
    uint8_t idx = 0;

    if (data[idx] == 'M') {
        printf("Going to take a while\n");

        lcd_clrscr();
        lcd_puts("Status:");
        lcd_gotoxy(0, 1);
        lcd_puts("Movement!");
    }
    else if (data[idx] == 'C') {
        lcd_clrscr();
        lcd_puts("Correct Password");
        lcd_gotoxy(0, 1);
        lcd_puts(&data[1]);

        TIMER1_Clear();
    }
    else if (data[idx] == 'W') {
        lcd_clrscr();
        lcd_puts("Wrong Password:");
        lcd_gotoxy(0, 1);
        lcd_puts(&data[1]);

        // Initialize timer 1 PWM mode
        TIMER1_Init_Mode_9();
    }
    else if (data[idx] == 'T') {
        lcd_clrscr();
        lcd_puts("Status:");
        lcd_gotoxy(0, 1);
        lcd_puts("TIME IS UP!");

        // Initialize timer 1 PWM mode
        TIMER1_Init_Mode_9();
    }
    else if (data[idx] == 'R') {
        lcd_clrscr();
        lcd_puts("Status:");
        lcd_gotoxy(0, 1);
        lcd_puts("Armed");

        TIMER1_Clear();

    }
}

/*
 * Resets recv, lcd and buzzer
 */
void rearm(char *recv)
{
    // reset recv
    for (uint8_t idx = 0; idx < DATA_SIZE + 1; idx++) {
        recv[idx] = '\0';
    }

    // reset lcd
    lcd_init(LCD_DISP_ON);
    lcd_clrscr();
    lcd_puts("Welcome!");

    // clear buzzer
    TIMER1_Clear();
}

/*
 * Function to get the lentgh of a string.
 */
uint16_t len(char *data)
{
    uint16_t count = 0;
    while (data[count] != '\0') {
        count++;
    }
    return count;
}

/*
 * Function to setup device as slave receiver.
 *
 * Follows closely Atmel Mega 2560 document of which page 253 - 254 contain
 * relevant information. Figure 24-15.
 */
void I2C_InitSlaveReceiver(uint8_t address)
{
    // Devices own Slave Address
    TWAR = address;

    // Slave receiver mode setup.
    TWCR |= (1 << TWEA) | (1 << TWEN);
    // Explicitly set to 0I2C_Receive
    TWCR &= ~(1 << TWSTA) & ~(1 << TWSTO);
    // Eqv: TWCR = 0b01000100;
}

/*
 Function to receive data from Master:
 */
void I2C_Receive(char *received)
{
    uint8_t twi_stat = 0;
    uint8_t twi_idx = 0;

    // Waiting for TWINT to set:
    while (!(TWCR & (1 << TWINT))) {
        ;
    }

    // Make sure the received array is full of nulls
    for (uint8_t idx = 0; idx < DATA_SIZE; idx++) {
        received[idx] = '\0';
    }

    // Waiting for TWINT to set:
    while (!(TWCR & (1 << TWINT))) {
        ;
    }

    // Set status
    twi_stat = (TWSR & 0xF8);

    // Reset the TWEA and TWEN and create ACK
    TWCR |= (1 << TWINT) | (1 << TWEA) | (1 << TWEN);

    // Waiting for TWINT to set:
    while (!(TWCR & (1 << TWINT))) {
        ;
    }

    // Set status
    twi_stat = (TWSR & 0xF8);

    // HEX values can be found in atmega 2560 doc page: 255, table: 24-4
    // Condition check of twi_status if previous was response of either slave
    // address or general call and ACK return
    while ((twi_stat == 0x80) || (twi_stat == 0x90)) {
        received[twi_idx] = TWDR;
        twi_idx++;

        if ((received[twi_idx - 1] == '\0') || (twi_idx >= DATA_SIZE)) {
            break;
        }

        // Reset the TWEA and TWEN, create ACK
        TWCR |= (1 << TWINT) | (1 << TWEA) | (1 << TWEN);

        // Wait for TWINT to set
        while (!(TWCR & (1 << TWINT))) {
            ;
        }

        // Status update
        twi_stat = (TWSR & 0xF8);
    }

    // check for NOT ACK or general Call
    if ((twi_stat == 0x88) || (twi_stat == 0x98)) {
        received[twi_idx] = TWDR;
        twi_idx++;
    }

    // STOP signal or repeated start signal
    else if ((twi_stat == 0xA0)) {
        TWCR |= (1 << TWINT);
    }
}

/* EOF */
