#include "timer1.h"
#define F_CPU 16000000UL
#define SLAVE_ADDRESS 170
#define BAUD 9600
#define MYUBRR F_CPU / 16 / BAUD - 1

#define DATA_SIZE 16
#define CODE_ARRAY_LENGTH 5

#define PIR_SENSE 0
#define ALARM_ON 1
#define ALARM_OFF 2
#define WAIT_CORRECT_KEY 3
#define IDLE 4

#define MOVEMENT 0
#define BUZZ 1

#define NOTE_64_Cs3 906
#define NOTE_64_C5 856
#define NOTE_64_200 625

#include <avr/interrupt.h>
// #include <avr/io.h>
#include <stdint.h>
#include <stdio.h>
// #include <stdlib.h>
#include <util/delay.h>

#include "lcd.h"
#include "notes.h"
#include "timer1.h"
#include "uart.h"

// State machine variable
volatile uint8_t state = 0;
volatile uint8_t data_incoming = 0;

// Parser to check system condition
void parser(char *data, char *code);

/*
 * I2C / TWI communication initialization with Master.
 */
void I2C_InitSlaveReceiver(uint8_t address);

/*
 * I2C / TWI receive from Master.
 */
void I2C_Receive(char *dest);

// Array length calculator
uint16_t len(char *data);

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

void enableExternalInterrupt()
{
    // External interrupt Control Register for when Master tries to transmit
    // data to slave. UNO Doc. 70-72 table 13-1 and 2
    EICRA |= (1 << ISC01);
    EIMSK |= (1 << INT0);
}

void disableExternalInterrupt()
{
    EICRA = 0;
    EIMSK = 0;
}

// Interrupt routine for timer
ISR(TIMER1_COMPA_vect) { TCNT1 = 0; }

// Interrupt routine for I2C transfer
ISR(INT0_vect)
{
    _delay_ms(1);
    data_incoming = 1;
}

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
    char recv[DATA_SIZE] = {'\0'};

    // Initialize empty received code
    char code[CODE_ARRAY_LENGTH] = {'\0'};

    // Init debug communication Through USB
    USART_Init(MYUBRR);
    stdin = &mystdin;
    stdout = &mystdout;

    sei();
    enableExternalInterrupt();

    // Init LCD display
    lcd_init(LCD_DISP_ON);
    lcd_clrscr();

    lcd_puts("Welcome!");
    // Test print to putty
    printf("Hello There!\n");

    // Setup TWI communication with Master
    I2C_InitSlaveReceiver(SLAVE_ADDRESS);
    // lcd_clrscr();

    while (1) {
        if (data_incoming) {
            lcd_clrscr();
            I2C_Receive(recv);
            parser(recv, code);
            data_incoming = 0;
        }
        if (state == BUZZ) {
            // Something
            PORTB ^= (1 << BUILTIN);
        }
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
void parser(char *data, char *code)
{
    uint8_t idx = 0;

    while (data[idx] != '\0') {
        if (data[idx] == 'M') {
            state = MOVEMENT;

            lcd_clrscr();
            lcd_puts("Status:");
            lcd_gotoxy(0, 1);
            lcd_puts("Movement!");
        }
        else if (data[idx] == 'P') {
            state = BUZZ;
            lcd_clrscr();
            lcd_puts("Wrong Password");

            // Initialize timer 1 PWM mode
            TIMER1_Init_Mode_9();
        }
        else if (data[idx] == 'T') {
            state = BUZZ;
            lcd_clrscr();
            lcd_puts("Status:");
            lcd_gotoxy(0, 1);
            lcd_puts("TIME IS UP!");
        }
        // If the code is given, it will
        else if (CODE_ARRAY_LENGTH > idx) {
            code[idx] = data[idx];
        }
        idx++;
    }

    if (len(code) > 0) {
        lcd_gotoxy(0, 1);
        lcd_puts(code);
    }
}

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
void I22C_Receive(char *received)
{
    uint8_t twi_stat, twi_idx = 0;

    // wait for transmission:
    while (!(TWCR & (1 << TWINT)))
        ;

    // Set status
    twi_stat = (TWSR & 0xF8);

    // Reset the TWEA and TWEN and create ACK
    TWCR |= (1 << TWINT) | (1 << TWEA) | (1 << TWEN);

    // Waiting for TWINT to set:
    while (!(TWCR & (1 << TWINT)))
        ;

    // Set status
    twi_stat = (TWSR & 0xF8);

    // HEX values can be found in atmega 2560 doc page: 255, table: 24-4
    // Condition check of twi_status if previous was response of either slave
    // address or general call and NOT ACK return
    while ((twi_stat == 0x80) || (twi_stat == 0x90)) {
        received[twi_idx] = TWDR;
        twi_idx++;

        if ((received[twi_idx - 1] == '\0') || (twi_idx >= DATA_SIZE)) {
            break;
        }

        // Reset the TWEA and TWEN, create ACK
        TWCR |= (1 << TWINT) | (1 << TWEA) | (1 << TWEN);

        // Wait for TWINT to set
        while (!(TWCR & (1 << TWINT)))
            ;

        // Status update
        twi_stat = (TWSR & 0xF8);
    }

    // check for NOT ACK or general Call
    if ((twi_stat == 0x88) || (twi_stat == 0x98)) {
        received[twi_idx] = TWDR;
        twi_idx++;
    }
    else if ((twi_stat == 0xA0)) { // STOP signal
        TWCR |= (1 << TWINT);
    }
}

/*
 Function to receive data from Master:
 */
void I2C_Receive(char *received)
{

    uint8_t twi_stat, twi_idx = 0;

    // wait for transmission:
    while (!(TWCR & (1 << TWINT)))
        ;

    // Set status
    twi_stat = (TWSR & 0xF8);

    // Reset the TWEA and TWEN and create ACK
    TWCR |= (1 << TWINT) | (1 << TWEA) | (1 << TWEN);

    // Waiting for TWINT to set:
    while (!(TWCR & (1 << TWINT)))
        ;

    // Set status
    twi_stat = (TWSR & 0xF8);

    // HEX values can be found in atmega 2560 doc page: 255, table: 24-4
    // Condition check of twi_status if previous was response of either slave
    // address or general call and NOT ACK return
    if ((twi_stat == 0x80) || (twi_stat == 0x90)) {
        received[twi_idx] = TWDR;
        twi_idx++;
    }
    else if ((twi_stat == 0x88) || (twi_stat == 0x98)) {
        received[twi_idx] = TWDR;
        twi_idx++;
    }
    else if ((twi_stat == 0xA0)) { // STOP signal
        TWCR |= (1 << TWINT);
    }
    if (DATA_SIZE <= twi_idx) {
        printf("%s", received);
        twi_idx = 0;
    }
}
