#include "timer1.h"

#include <avr/interrupt.h>
#include <stdint.h>
#include <stdio.h>
#include <util/delay.h>

#include "lcd.h"
#include "notes.h"
#include "timer1.h"
#include "uart.h"

#define F_CPU 16000000UL
#define SLAVE_ADDRESS 170
#define BAUD 9600
#define MYUBRR (((F_CPU / 16) / BAUD) - 1)

// Max size of transferable data
#define DATA_SIZE 16

// LCD Display PINS NOTE remember to change from lcd.h also
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
static void parser(char *data);

// I2C / TWI communication initialization with Master.
static void i2c_init_slave_receiver(uint8_t address);

// I2C / TWI receive from Master.
static void i2c_receive(char *dest);

// Rearm system
static void rearm(char *recv);

// Interrupt routine for timer
ISR(TIMER1_COMPA_vect) { TCNT1 = 0; }

// Main function that includes the main loop
int main(void)
{
    /*      LCD PINS     */
    // OUTPUTS CONTROL
    DDRB |= (1 << LCD_RS) | (1 << LCD_RW) | (1 << LCD_EN) | (1 << BUILTIN);
    // OUTPUTS DATA
    DDRD |= (1 << LCD_D4) | (1 << LCD_D5) | (1 << LCD_D6) | (1 << LCD_D7);

    // Buzzer OUTPUT
    DDRB |= (1 << BUZZER);

    // Initialize empty recv char array
    char recv[DATA_SIZE] = {'\0'};

    // Init debug communication Through USB
    usart_init(MYUBRR);
    stdin = &mystdin;
    stdout = &mystdout;

    // Init LCD display
    lcd_init(LCD_DISP_ON);
    lcd_clrscr();
    lcd_puts("Welcome!");

    // Setup TWI communication with Master
    i2c_init_slave_receiver(SLAVE_ADDRESS);

    for (;;) {
        // wait for transmission:
        while (!(TWCR & (1 << TWINT))) {
            // Built in led is blinked to indicate that board is waiting for
            // transmission.
            PORTB |= (1 << BUILTIN);
            _delay_ms(200);
            PORTB &= ~(1 << BUILTIN);
            _delay_ms(200);
        }
        // When transmission is coming, the information will be stored in the
        // recv array.
        i2c_receive(recv);

        // The received data is parsed and information is printed to the LCD.
        parser(recv);
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
static void parser(char *data)
{
    // First byte of data is checked as it represents the system state.
    uint8_t idx = 0;
    ;

    // LCD prints and buzzer is turned on
    // based on character received
    if (data[idx] == 'M') {
        lcd_clrscr();
        lcd_puts("Status:");
        lcd_gotoxy(0, 1);
        lcd_puts("Movement!");
    }

    else if (data[idx] == 'C') {
        lcd_clrscr();
        lcd_puts("Correct Password");
        lcd_gotoxy(0, 1);
        // First byte is state, print rest to LCD
        lcd_puts(&data[1]);
        // Correct password, so buzzer is offed via timer clear.
        timer1_clear();
    }

    else if (data[idx] == 'W') {
        lcd_clrscr();
        lcd_puts("Wrong Password:");
        lcd_gotoxy(0, 1);
        // First byte is state, print rest to LCD
        lcd_puts(&data[1]);

        // Initialize timer 1 PWM mode
        timer1_init_mode_9();
        // Setup for playing a Note
        timer1_set_prescaler(PS_8);
        timer1_set_target(NOTE_C3);
    }

    else if (data[idx] == 'T') {
        lcd_clrscr();
        lcd_puts("Status:");
        lcd_gotoxy(0, 1);
        lcd_puts("TIME IS UP!");

        // Initialize timer 1 PWM mode
        timer1_init_mode_9();
        // Setup for playing a Note
        timer1_set_prescaler(PS_8);
        timer1_set_target(NOTE_C3);
    }

    else if (data[idx] == 'R') {
        // Reset LCD and buzzer
        rearm(data);
    }
}

/*
 * Resets recv, lcd and buzzer
 */
static void rearm(char *recv)
{
    // Reset recv
    for (uint8_t idx = 0; (DATA_SIZE) > idx; idx++) {
        recv[idx] = '\0';
    }

    // Reset lcd
    lcd_clrscr();
    lcd_puts("Status:");
    lcd_gotoxy(0, 1);
    lcd_puts("Armed");

    // clear buzzer
    timer1_clear();
}

/*
 * Function to setup device as slave receiver.
 *
 * Follows closely Atmel Mega 2560 document of which page 253 - 254 contain
 * relevant information. Figure 24-15.
 */
static void i2c_init_slave_receiver(uint8_t address)
{
    // Devices own Slave Address
    TWAR = address;

    // Slave receiver mode setup.
    TWCR |= (1 << TWEA) | (1 << TWEN);

    // Explicitly set to 0i2c_receive
    TWCR &= ~(1 << TWSTA) & ~(1 << TWSTO);
    // Eqv: TWCR = 0b01000100;
}

/*
 Function to receive data from Master:
 */
static void i2c_receive(char *received)
{
    uint8_t twi_stat = 0;
    uint8_t twi_idx = 0;

    // Waiting for TWINT to set:
    while (!(TWCR & (1 << TWINT))) {
        ;
    }

    // Make sure the received array is full of nulls
    for (uint8_t idx = 0; DATA_SIZE > idx; idx++) {
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
    while ((0x80 == twi_stat) || (0x90 == twi_stat)) {
        received[twi_idx] = TWDR;
        twi_idx++;

        if (('\0' == received[twi_idx - 1]) || (DATA_SIZE <= twi_idx)) {
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
    if ((0x88 == twi_stat) || (0x98 == twi_stat)) {
        received[twi_idx] = TWDR;
        twi_idx++;
    }

    // STOP signal or repeated start signal
    else if ((0xA0 == twi_stat)) {
        TWCR |= (1 << TWINT);
    }
}

/* EOF */
