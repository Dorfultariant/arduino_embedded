
#include <stdint.h>
#define F_CPU 16000000UL
#define DATA_SIZE 16
#define BAUD 9600
#define MYUBRR F_CPU / 16 / BAUD - 1

#define CODE_ARRAY_LENGTH 5
#define PS_1024 1024
#define SECOND F_CPU / (2 * PS_1024 * 0.5)

#define SLAVE_ADDRESS 170

/*
 * State machine states:
 *
 * 0 PIR Sense
 * 1 TIMER ON
 * 2 KEY_INSERTION
 * 3 PIR TIMER ALARM OFF
 */

#define PIR_SENSE 0
#define TIMER_ON 1
#define KEY_INSERTION 2
#define PIR_TIMER_ALARM_OFF 3

// Countdown in seconds
#define ALARM_TIMER 10

#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <util/delay.h>

#include "keypad.h"
#include "timer3.h"
#include "uart.h"

// Bytes to cause an alarm on UNO
const char WRONG_CODE[] = "P\0";
const char TIMES_UP[] = "T\0";

// Timer is started signal
const char MOVEMENT[] = "M\0";

// Reset
const char RESET[] = "R\0";

const int PIR_SIGNAL = PE3;
const int REARM_BTN = PG5;
const int ALARM_LED = PH3;
const int I2C_ERROR = PH4;
const int I2C_OK = PH5;

// State machine state
volatile int8_t state = 0;

/*
 TWI communication
 */
void I2C_Init();

void TWI_Tr(uint16_t address, char *data);
void I2C_Transmit(uint8_t address, const char *data);

/*
 Keypad code reading and verification:
 */
volatile uint8_t isCodeValid = 0;
int8_t read_keypad_code(char *dest, uint8_t code_len);
int verify_code(char *to_be_checked, char *correct);
// Check if c in char array
int8_t cInArr(char c, char *arr);

/*
 Timer 3 counter for 10 second countdown from PIR sense.
 */
volatile uint16_t second_counter = 0;

/*
 * Interrupt Service Routine for Timer 3 working behaviour.
 */
ISR(TIMER3_COMPA_vect)
{
    TCNT3 = 0;
    second_counter++;
    if (ALARM_TIMER <= second_counter) {
        PORTH |= (1 << ALARM_LED);
        TIMER3_Clear();
        second_counter = 0;
        I2C_Init();
        I2C_Transmit(SLAVE_ADDRESS, &TIMES_UP);
    }
}

int main(void)
{
    // Define the correct keycode.
    char correctCode[CODE_ARRAY_LENGTH] = "0423";

    // Initialize empty code given by user.
    char usersCode[CODE_ARRAY_LENGTH] = {'\0'};

    // Output demo for alarm buzzer (currently RED LED)
    DDRH |= (1 << ALARM_LED) | (1 << I2C_ERROR) | (1 << I2C_OK);

    // PIR sensor input upon movement (currently BUTTON)
    DDRE &= ~(1 << PIR_SIGNAL);

    // Input pin for rearming the system.
    DDRG &= ~(1 << REARM_BTN);

    // Initialize connection through USB for debugging.
    USART_Init(MYUBRR);
    stdin = &mystdin;
    stdout = &mystdout;

    // Keypad initialization for getting users input from keypad.
    KEYPAD_Init();

    // Test print for program start.
    printf("Hello There!\n");

    // Main logic loop, state machine.
    while (1) {
        switch (state) {
        case PIR_SENSE:
            // If PIR senses movement start timer
            if (PINE & (1 << PIR_SIGNAL)) {
                state = TIMER_ON;
                printf("Timer started\n");
                I2C_Init();
                I2C_Transmit(SLAVE_ADDRESS, MOVEMENT);
            }
            break;

        case TIMER_ON:
            // Initializing timer 3
            TIMER3_Init_CTC();

            // Setting the timer 3 to interrupt every second
            TIMER3_SetIntervalSecond();

            // Wait for correct user input.
            state = KEY_INSERTION;
            break;

        case KEY_INSERTION:
            read_keypad_code(usersCode, CODE_ARRAY_LENGTH - 1);
            isCodeValid = verify_code(usersCode, correctCode);
            if (isCodeValid) {
                // Clear timer and turn it "off"
                TIMER3_Clear();

                // Turn off alarm
                PORTH &= ~(1 << ALARM_LED);

                // Transmit information to Slave
                I2C_Init();

                // Send the code to UNO
                I2C_Transmit(SLAVE_ADDRESS, usersCode);

                // Move to Idle
                printf("Goin idle at 16Mhz...\n");
                state = PIR_TIMER_ALARM_OFF;
            }
            else {
                // Initialize connection
                I2C_Init();

                // Transmit incorrect symbol
                I2C_Transmit(SLAVE_ADDRESS, WRONG_CODE);

                // Transmit incorrect symbol
                I2C_Transmit(SLAVE_ADDRESS, usersCode);
            }
            break;

        case PIR_TIMER_ALARM_OFF: // Idle state
            // Waiting for system rearming.
            if (PING & (1 << REARM_BTN)) {
                state = PIR_SENSE;
                isCodeValid = 0;
                second_counter = 0;
            }
            break;
        }
    }

    return 0;
}

/*
 * I2C / TWI transmission initialization with 400 kHz clock.
 * @param None
 *
 * @returns void
 */
void I2C_Init()
{
    // Bit Rate generator setup to 400 000 Hz -> F_CPU / (16 + 2 * TWBR *
    // 4^(TWSR):
    TWSR = 0x00;         // Prescaler to 1
    TWBR = 0x03;         // 3x multiplier to achieve 400 kHz
    TWCR |= (1 << TWEN); // TWI enable
}

/*
 * I2C / TWI Transmission from Master to Slave address with data.
 * @param uint8_t address of the slave to transmit to.
 * @param char *data to be sent to the slave.
 *
 * @returns void
 */
void I2C_Transmit(uint8_t address, const char *data)
{
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

    // Check if the connection to Slave is successful (Slave returns ACK)
    if ((twi_stat != 0x18) && (twi_stat != 0x40)) {
        // Set error led ON
        PORTH |= (1 << I2C_ERROR);
        // Verify that OK led is OFF
        PORTH &= ~(1 << I2C_OK);
        return;
    }

    // Set ok led ON
    PORTH |= (1 << I2C_OK);
    // Set error led OFF
    PORTH &= ~(1 << I2C_ERROR);

    // Send data byte at a time until either 16 bytes has been sent or data
    // array has reached its end.
    for (uint8_t twi_d_idx = 0;
         twi_d_idx < DATA_SIZE && (data[twi_d_idx] != '\0'); twi_d_idx++) {
        TWDR = data[twi_d_idx];

        // Reset TWINT to transmit data
        TWCR = (1 << TWINT) | (1 << TWEN);

        // Wait for TWINT to set
        while (!(TWCR & (1 << TWINT)))
            ;

        twi_stat = (TWSR & 0xF8);
    }

    // STOP transmission
    TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);
}

/*
 * Stores users given key code from the keypad to the destination array to be
 * verified. When user has given code_len amount of digits (only last ones are
 * stored) and user gives [A]ccept the loop ends and the code is verified.
 * User can also give [D]elete to remove previous digit from storage.
 *
 * @param char *dest        Destination array
 * @param uint8_t code_len  Destination array length
 *
 * @returns 0 for success
 */
int8_t read_keypad_code(char *dest, uint8_t code_len)
{
    int index = 0;
    char chr = 0;

    // Ensure that we are working with empty array:
    for (uint8_t idx = 0; idx < code_len; idx++) {
        dest[idx] = '\0';
    }

    // Get users input until the user presses [A]ccept on the keypad and
    // user has given long enough password.
    while (1) {
        chr = KEYPAD_GetKey();

        // Check for digit in range 0 - 9
        if ((chr >= '0') && (chr <= '9')) {
            // We want to store the last code_len amount of digits
            if (index >= code_len) {
                memmove(dest, dest + 1, code_len - 1);
                index--;
            }
            dest[index++] = chr;
        }
        // Allow the [D]eletion of previous char if it exists
        else if (chr == 'D' && index > 0) {
            index--;
        }
        // End point to exit function if enough digits given and [A]ccept
        else if ((chr == 'A') && (index == code_len)) {
            break;

        }
    }

    // Make sure that the dest ends.
    dest[code_len] = '\0';
    return 0;
}

/*
 * This function compares two given arrays until either of them are
 * checked to '\0'.
 *
 * @param char *to_be_checked is the array to be compared
 * @param char *correct is the array to be compared against
 *
 * @returns int 1 for arrays being identical in the shortest arrays scope and 0
 * if the arrays do not match in the shortest arrays scope.
 */
int verify_code(char *to_be_checked, char *correct)
{
    uint8_t i = 0;
    while ((to_be_checked[i] != '\0') && (correct[i] != '\0')) {
        if (to_be_checked[i] != correct[i]) {
            return 0;
        }
        i++;
    }
    return 1;
}
